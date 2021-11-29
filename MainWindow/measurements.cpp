#include "measurements.h"
#include "ui_measurements.h"

// qt
#include <QMessageBox>

// vtk
#include "vtkPolyData.h"
#include "vtkCellData.h"
#include "vtkKdTreePointLocator.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkClipPolyData.h"
#include "vtkAppendPolyData.h"
#include "vtkThreshold.h"
#include "vtkGeometryFilter.h"
#include "vtkUnstructuredGrid.h"

// vmtk
#include "vtkvmtkPolyBallLine.h"

#include "io.h"
#include "preferences.h"
#include "CenterlinesInfoWidget.h"

Measurements::Measurements(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Measurements)
{
	ui->setupUi(this);

	// connections
	connect(ui->pushButtonUpdate, &QPushButton::clicked, this, &Measurements::slotUpdate);
	connect(ui->pushButtonClose, &QPushButton::clicked, this, &Measurements::slotClose);
	//connect(ui->pushButtonAddAllRecon, &QPushButton::clicked, this, &Measurements::slotReconAddAll);
	//connect(ui->pushButtonAddRecon, &QPushButton::clicked, this, &Measurements::slotReconAddCurrent);
	//connect(ui->pushButtonRemoveAllRecon, &QPushButton::clicked, this, &Measurements::slotReconRemoveAll);
	//connect(ui->pushButtonRemoveRecon, &QPushButton::clicked, this, &Measurements::slotReconRemoveCurrent);
}

Measurements::~Measurements()
{

}

void Measurements::SetPreference(Preferences *preferences)
{
	m_preferences = preferences;
}

void Measurements::SetDataIo(IO *io)
{
	m_io = io;
}

void Measurements::SetCenterlinesInfoWidget(CenterlinesInfoWidget *centerlinesInfoWidget)
{
	m_centerlinesInfoWidget = centerlinesInfoWidget;
}

void Measurements::slotClose()
{
	this->close();
}

void Measurements::slotCenterlineConfigUpdate()
{
	if (m_preferences == nullptr)
		return;

	ui->labelCenterlineIdsArray->setText(m_preferences->GetCenterlineIdsArrayName());
	ui->labelRadiusArray->setText(m_preferences->GetRadiusArrayName());

	//// centerlineids table update
	//ui->listWidgetCenterlineIdsPending->clear();
	//ui->listWidgetCenterlineIdsRecon->clear();

	//vtkDataArray* centerlineIdsArray = m_io->GetCenterline()->GetCellData()->GetArray(m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
	//if (centerlineIdsArray == nullptr)
	//	return;

	//for (int i = centerlineIdsArray->GetRange()[0]; i <= centerlineIdsArray->GetRange()[1]; i++)
	//	ui->listWidgetCenterlineIdsPending->addItem(QString::number(i));
}

void Measurements::slotUpdate()
{
	// check data
	vtkPolyData* surface = m_io->GetSurface();
	vtkPolyData* centerline = m_io->GetCenterline();
	vtkPolyData* reconSurface = m_io->GetReconstructedSurface();

	if (surface == nullptr || centerline == nullptr)
	{
		QMessageBox msgBox;
		msgBox.setText("Surface file or centerline file not found");
		msgBox.exec();
		return;
	}

	if (surface->GetNumberOfPoints() == 0 || centerline->GetNumberOfPoints() == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("Surface file or centerline file not found");
		msgBox.exec();
		return;
	}

	// get the key points
	vtkSmartPointer<vtkKdTreePointLocator> locator = vtkSmartPointer<vtkKdTreePointLocator>::New();
	locator->SetDataSet(m_io->GetCenterline());
	locator->BuildLocator();

	double proximalPt[3] = {
		m_centerlinesInfoWidget->GetProximalNormalPoint()[0],
		m_centerlinesInfoWidget->GetProximalNormalPoint()[1],
		m_centerlinesInfoWidget->GetProximalNormalPoint()[2]
	};

	double distalPt[3] = {
		m_centerlinesInfoWidget->GetDistalNormalPoint()[0],
		m_centerlinesInfoWidget->GetDistalNormalPoint()[1],
		m_centerlinesInfoWidget->GetDistalNormalPoint()[2]
	};

	double stenosisPt[3] = {
		m_centerlinesInfoWidget->GetStenosisPoint()[0],
		m_centerlinesInfoWidget->GetStenosisPoint()[1],
		m_centerlinesInfoWidget->GetStenosisPoint()[2]
	};

	std::cout << "proximal point: " << proximalPt[0] << ", " << proximalPt[1] << ", " << proximalPt[2] << std::endl;
	std::cout << "distal point: " << distalPt[0] << ", " << distalPt[1] << ", " << distalPt[2] << std::endl;
	std::cout << "stenosis point: " << stenosisPt[0] << ", " << stenosisPt[1] << ", " << stenosisPt[2] << std::endl;

	if ((proximalPt[0] == 0 && proximalPt[1] == 0 && proximalPt[2] == 0) || 
		(distalPt[0] == 0 && distalPt[1] == 0 && distalPt[2] == 0) ||
		(stenosisPt[0] == 0 && stenosisPt[1] == 0 && stenosisPt[2] == 0))
	{
		QMessageBox msgBox;
		msgBox.setText("Invalid feature points");
		msgBox.exec();
		return;
	}

	int proximalId = locator->FindClosestPoint(proximalPt);
	int distalId = locator->FindClosestPoint(distalPt);
	int stenosisId = locator->FindClosestPoint(stenosisPt);

	// threshold according to abscissas
	vtkDataArray* abscissas = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetAbscissasArrayName().toStdString().c_str());

	if (abscissas == nullptr)
	{
		QMessageBox msgBox;
		msgBox.setText("Invalid abscissas array");
		msgBox.exec();
		return;
	}

	double threshold_bound[2] = {
		abscissas->GetTuple(proximalId)[0],
		abscissas->GetTuple(distalId)[0]
	};

	vtkDataArray* centerlineIds = centerline->GetCellData()->GetArray(m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
	if (centerlineIds == nullptr)
	{
		QMessageBox msgBox;
		msgBox.setText("Invalid centerlineids array");
		msgBox.exec();
		return;
	}

	vtkDataArray* frenetTangent = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str());
	if (frenetTangent == nullptr || frenetTangent->GetNumberOfComponents() !=3 )
	{
		QMessageBox msgBox;
		msgBox.setText("Invalid Frenet tangent array");
		msgBox.exec();
		return;
	}
	
	vtkDataArray* radius = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str());
	if (radius == nullptr || radius->GetNumberOfComponents() !=1 )
	{
		QMessageBox msgBox;
		msgBox.setText("Invalid radius array");
		msgBox.exec();
		return;
	}

	vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();

	for (int i = centerlineIds->GetRange()[0]; i <= centerlineIds->GetRange()[1]; i++)
	{
		// threshold to get independent centerline
		vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
		threshold->ThresholdBetween(i, i);
		threshold->SetInputData(m_io->GetCenterline());
		threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
		threshold->Update();

		// convert threshold output to vtkpolydata
		vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
		geometryFilter->SetInputData(threshold->GetOutput());
		geometryFilter->Update();

		if (threshold->GetOutput()->GetNumberOfPoints() == 0)
			continue;

		vtkSmartPointer<vtkPolyData> singleCenterline = geometryFilter->GetOutput();
		singleCenterline->GetPointData()->SetActiveScalars(m_preferences->GetAbscissasArrayName().toStdString().c_str());

		// check if single centerline is close to distal point
		double epsilon = 1e-6;
		double distalPointDistance = sqrt(vtkMath::Distance2BetweenPoints(singleCenterline->GetPoint(singleCenterline->GetNumberOfPoints()-1),distalPt));
		if (distalPointDistance > 1e-6)
			continue;

		// compute section to clip
		vtkSmartPointer<vtkClipPolyData> clipper1 = vtkSmartPointer<vtkClipPolyData>::New();
		clipper1->SetValue(threshold_bound[0]);
		clipper1->SetInsideOut(true);
		clipper1->GenerateClippedOutputOn();
		clipper1->SetInputData(singleCenterline);
		clipper1->Update();

		vtkSmartPointer<vtkClipPolyData> clipper2 = vtkSmartPointer<vtkClipPolyData>::New();
		clipper2->SetValue(threshold_bound[1]);
		clipper2->SetInsideOut(true);
		clipper2->GenerateClippedOutputOn();
		clipper2->SetInputData(singleCenterline);
		clipper2->Update();

		appendFilter->AddInputData(clipper2->GetOutput());
	}

	appendFilter->Update();

	// ======================== clip surface  ========================
	//// get the new centerlineids 
	//centerlineIds = centerline->GetCellData()->GetArray(m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
	//if (centerlineIds == nullptr)
	//{
	//	m_statusLabel->setText("Invalid centerlineids array");
	//	m_statusProgressBar->setValue(100);

	//	this->renderCenterline();
	//	this->updateCenterlineDataTable();
	//	this->updateCenterlinesInfoWidget();
	//	return;
	//}

	// create implict function with spheres along clipped centerline
	vtkSmartPointer<vtkvmtkPolyBallLine> tubeFunction = vtkSmartPointer<vtkvmtkPolyBallLine>::New();
	tubeFunction->SetInput(appendFilter->GetOutput());
	tubeFunction->SetPolyBallRadiusArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());

	//vtkNew<vtkImplicitBoolean> endSpheresFunction;
	//endSpheresFunction->SetOperationTypeToUnion();

	//for (int i = centerlineIds->GetRange()[0]; i <= centerlineIds->GetRange()[1]; i++)
	//{
	//	// threshold to get independent centerline
	//	vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
	//	threshold->ThresholdBetween(i, i);
	//	threshold->SetInputData(m_io->GetCenterline());
	//	threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
	//	threshold->Update();

	//	if (threshold->GetOutput()->GetNumberOfPoints() == 0)
	//		continue;

	//	//singleCenterline->GetPointData()->SetActiveScalars(m_preferences->GetAbscissasArrayName().toStdString().c_str());

	//	// if the single centerline is not the one in recon centerline list, direct append to output
	//	bool recon = true;
	//	for (int j = 0; j < ui->listWidgetCenterlineIdsPending->count(); j++)
	//	{
	//		int centerlineId = ui->listWidgetCenterlineIdsPending->item(j)->text().toInt();
	//		if (centerlineId == i)
	//		{
	//			recon = false;
	//			break;
	//		}
	//	}

	//	if (recon)
	//	{
	//		// threshold for isolated centerlines
	//		vtkSmartPointer<vtkConnectivityFilter> connectFilter = vtkSmartPointer<vtkConnectivityFilter>::New();
	//		connectFilter->SetExtractionModeToAllRegions();
	//		connectFilter->SetInputData(threshold->GetOutput());
	//		connectFilter->ColorRegionsOn(); // to generate RegionId array
	//		connectFilter->Update();

	//		for (int j = 0; j < connectFilter->GetNumberOfExtractedRegions(); j++)
	//		{
	//			// threshold to get independent centerline
	//			vtkSmartPointer<vtkThreshold> threshold2 = vtkSmartPointer<vtkThreshold>::New();
	//			threshold2->ThresholdBetween(j, j);
	//			threshold2->SetInputData(connectFilter->GetOutput());
	//			threshold2->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "RegionId");
	//			threshold2->Update();

	//			// convert threshold output to vtkpolydata
	//			vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter> ::New();
	//			geometryFilter->SetInputData(threshold2->GetOutput());
	//			geometryFilter->Update();

	//			vtkSmartPointer<vtkPolyData> singleCenterline = geometryFilter->GetOutput();

	//			// get the end points
	//			double* center0 = threshold2->GetOutput()->GetPoint(0);
	//			double tangent0[3];
	//			tangent0[0] = threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(0, 0);
	//			tangent0[1] = threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(0, 1);
	//			tangent0[2] = threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(0, 2);
	//			double radius0 = threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetComponent(0, 0);

	//			vtkNew<vtkSphere> sphere0;
	//			sphere0->SetCenter(center0[0], center0[1], center0[2]);
	//			sphere0->SetRadius(radius0*1.5);

	//			vtkNew<vtkPlane> plane0;
	//			plane0->SetOrigin(center0[0], center0[1], center0[2]);
	//			plane0->SetNormal(1.0*tangent0[0], 1.0*tangent0[1], 1.0*tangent0[2]);

	//			vtkNew<vtkImplicitBoolean> compositeFunction0;
	//			compositeFunction0->AddFunction(sphere0);
	//			compositeFunction0->AddFunction(plane0);
	//			compositeFunction0->SetOperationTypeToIntersection();

	//			double* center1 = threshold2->GetOutput()->GetPoint(threshold2->GetOutput()->GetNumberOfPoints() - 1);
	//			double tangent1[3];
	//			// reverse tangent direction
	//			tangent1[0] = -1.0*threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(threshold2->GetOutput()->GetNumberOfPoints() - 1, 0);
	//			tangent1[1] = -1.0*threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(threshold2->GetOutput()->GetNumberOfPoints() - 1, 1);
	//			tangent1[2] = -1.0*threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(threshold2->GetOutput()->GetNumberOfPoints() - 1, 2);
	//			double radius1 = threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetComponent(threshold2->GetOutput()->GetNumberOfPoints() - 1, 0);

	//			vtkNew<vtkSphere> sphere1;
	//			sphere1->SetCenter(center1[0], center1[1], center1[2]);
	//			sphere1->SetRadius(radius1*1.5);

	//			vtkNew<vtkPlane> plane1;
	//			plane1->SetOrigin(center1[0], center1[1], center1[2]);
	//			plane1->SetNormal(1.0*tangent1[0], 1.0*tangent1[1], 1.0*tangent1[2]);

	//			vtkNew<vtkImplicitBoolean> compositeFunction1;
	//			compositeFunction1->AddFunction(sphere1);
	//			compositeFunction1->AddFunction(plane1);
	//			compositeFunction1->SetOperationTypeToIntersection();

	//			// add to overall composite function
	//			endSpheresFunction->AddFunction(compositeFunction0);
	//			endSpheresFunction->AddFunction(compositeFunction1);
	//		}
	//	}
	//}

	//vtkNew<vtkImplicitBoolean> compositeFunction;
	//compositeFunction->AddFunction(tubeFunction);
	//compositeFunction->AddFunction(endSpheresFunction);
	//compositeFunction->SetOperationTypeToDifference();

	//// create mask array with spheres and centerline tubes
	//vtkSmartPointer<vtkDoubleArray> maskArray = vtkSmartPointer<vtkDoubleArray>::New();
	//maskArray->SetNumberOfComponents(1);
	//maskArray->SetNumberOfTuples(m_io->GetVoronoiDiagram()->GetNumberOfPoints());
	//maskArray->SetName("Mask");
	//maskArray->FillComponent(0, 0);

	//std::cout << "evaluating voronoi diagram..." << std::endl;
	//m_statusLabel->setText("Evaluating voronoi diagram");
	//m_statusProgressBar->setValue(75);

	//compositeFunction->EvaluateFunction(m_io->GetVoronoiDiagram()->GetPoints()->GetData(), maskArray);

	//m_io->GetVoronoiDiagram()->GetPointData()->AddArray(maskArray);
	//m_io->GetVoronoiDiagram()->GetPointData()->SetActiveScalars("Mask");

	//vtkSmartPointer<vtkClipPolyData> clipperV = vtkSmartPointer<vtkClipPolyData>::New();
	//clipperV->SetValue(0);
	//clipperV->SetInsideOut(true);
	//clipperV->GenerateClippedOutputOn();
	//clipperV->SetInputData(m_io->GetVoronoiDiagram());
	//clipperV->Update();

	//vtkSmartPointer<vtkCleanPolyData> cleanerV = vtkSmartPointer<vtkCleanPolyData>::New();
	//cleanerV->SetInputData(clipperV->GetOutput());
	//cleanerV->Update();

	////vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
	////geomFilter->SetInputData(clipperV->GetOutput());
	////geomFilter->Update();

	//m_io->SetVornoiDiagram(cleanerV->GetOutput());

}

//void Measurements::slotReconAddAll()
//{
//	while (ui->listWidgetCenterlineIdsPending->count() > 0)
//	{
//		QListWidgetItem* currentItem = ui->listWidgetCenterlineIdsPending->item(0);
//		ui->listWidgetCenterlineIdsRecon->addItem(currentItem->text());
//		ui->listWidgetCenterlineIdsPending->takeItem(ui->listWidgetCenterlineIdsPending->row(currentItem));
//	}
//}

//void Measurements::slotReconAddCurrent()
//{
//	QListWidgetItem* currentItem = ui->listWidgetCenterlineIdsPending->currentItem();
//
//	if (currentItem == nullptr)
//		return;
//
//	ui->listWidgetCenterlineIdsRecon->addItem(currentItem->text());
//	ui->listWidgetCenterlineIdsPending->takeItem(ui->listWidgetCenterlineIdsPending->row(currentItem));
//}
//
//void Measurements::slotReconRemoveAll()
//{
//	while (ui->listWidgetCenterlineIdsRecon->count() > 0)
//	{
//		QListWidgetItem* currentItem = ui->listWidgetCenterlineIdsRecon->item(0);
//		ui->listWidgetCenterlineIdsPending->addItem(currentItem->text());
//		ui->listWidgetCenterlineIdsRecon->takeItem(ui->listWidgetCenterlineIdsRecon->row(currentItem));
//	}
//}
//
//void Measurements::slotReconRemoveCurrent()
//{
//	QListWidgetItem* currentItem = ui->listWidgetCenterlineIdsRecon->currentItem();
//
//	if (currentItem == nullptr)
//		return;
//	if (currentItem == nullptr)
//		return;
//	ui->listWidgetCenterlineIdsPending->addItem(currentItem->text());
//	ui->listWidgetCenterlineIdsRecon->takeItem(ui->listWidgetCenterlineIdsRecon->row(currentItem));
//}