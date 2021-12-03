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
#include "vtkImplicitBoolean.h"
#include "vtkSphere.h"
#include "vtkPlane.h"
#include "vtkDoubleArray.h"
#include "vtkArrayCalculator.h"
#include "vtkCleanPolyData.h"
#include "vtkMassProperties.h"
#include "vtkDistancePolyDataFilter.h"
#include "vtkHausdorffDistancePointSetFilter.h"

// vmtk
#include "vtkvmtkPolyBallLine.h"
#include "vtkvmtkCapPolyData.h"

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
	connect(m_io, SIGNAL(signalCenterlineUpdated()), this, SLOT(slotUpdatePointDataArray()));
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
	ui->labelReconCenterlineIdsArray->setText(m_preferences->GetReconCenterlineIdsArrayName());
	ui->labelReconRadiusArray->setText(m_preferences->GetReconRadiusArrayName());

	//// centerlineids table update
	//ui->listWidgetCenterlineIdsPending->clear();
	//ui->listWidgetCenterlineIdsRecon->clear();

	//vtkDataArray* centerlineIdsArray = m_io->GetCenterline()->GetCellData()->GetArray(m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
	//if (centerlineIdsArray == nullptr)
	//	return;

	//for (int i = centerlineIdsArray->GetRange()[0]; i <= centerlineIdsArray->GetRange()[1]; i++)
	//	ui->listWidgetCenterlineIdsPending->addItem(QString::number(i));
}

void Measurements::slotUpdatePointDataArray()
{
	ui->comboBoxPointDataArray->clear();
	for (int i = 0; i < m_io->GetCenterline()->GetPointData()->GetNumberOfArrays(); i++)
	{
		ui->comboBoxPointDataArray->addItem(m_io->GetCenterline()->GetPointData()->GetArrayName(i));
	}
	ui->comboBoxPointDataArray->setCurrentIndex(0);
}

void Measurements::clipCenterline(double * proximalPt, double * distalPt, DataType dataType, vtkPolyData* clippedCenterline)
{
	std::cout << "proximal point: " << proximalPt[0] << ", " << proximalPt[1] << ", " << proximalPt[2] << std::endl;
	std::cout << "distal point: " << distalPt[0] << ", " << distalPt[1] << ", " << distalPt[2] << std::endl;

	vtkPolyData* centerline;
	
	switch (dataType)
	{
	case DataType::Original:
		centerline = m_io->GetCenterline();
		break;
	case DataType::Recon:
		centerline = m_io->GetReconstructedCenterline();
		break;
	}

	if (centerline->GetNumberOfPoints() == 0)
		return;

	vtkSmartPointer<vtkKdTreePointLocator> locator = vtkSmartPointer<vtkKdTreePointLocator>::New();
	locator->SetDataSet(centerline);
	locator->BuildLocator();

	int proximalId = locator->FindClosestPoint(proximalPt);
	int distalId = locator->FindClosestPoint(distalPt);

	// threshold according to abscissas
	vtkDataArray* abscissas;
	vtkDataArray* centerlineIds;
	vtkDataArray* frenetTangent;
	vtkDataArray* radius;

	switch (dataType)
	{
	case DataType::Original:
		abscissas = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetAbscissasArrayName().toStdString().c_str());
		centerlineIds = m_io->GetCenterline()->GetCellData()->GetArray(m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
		frenetTangent  = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str());
		radius = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str());
		
		if (abscissas == nullptr)
		{
			QMessageBox msgBox;
			msgBox.setText("Invalid abscissas array for original centerline");
			msgBox.exec();
			return;
		}

		if (centerlineIds == nullptr)
		{
			QMessageBox msgBox;
			msgBox.setText("Invalid centerlineids array for original centerline");
			return;
		}

		if (frenetTangent == nullptr || frenetTangent->GetNumberOfComponents() != 3)
		{
			QMessageBox msgBox;
			msgBox.setText("Invalid Frenet tangent array for original centerline");
			msgBox.exec();
			return;
		}

		if (radius == nullptr || radius->GetNumberOfComponents() != 1)
		{
			QMessageBox msgBox;
			msgBox.setText("Invalid radius array for original centerline");
			msgBox.exec();
			return;
		}
		break;

	case DataType::Recon:
		abscissas = m_io->GetReconstructedCenterline()->GetPointData()->GetArray(m_preferences->GetReconAbscissasArrayName().toStdString().c_str());
		centerlineIds = m_io->GetReconstructedCenterline()->GetCellData()->GetArray(m_preferences->GetReconCenterlineIdsArrayName().toStdString().c_str());
		frenetTangent = m_io->GetReconstructedCenterline()->GetPointData()->GetArray(m_preferences->GetReconFrenetTangentArrayName().toStdString().c_str());
		radius = m_io->GetReconstructedCenterline()->GetPointData()->GetArray(m_preferences->GetReconRadiusArrayName().toStdString().c_str());

		if (abscissas == nullptr)
		{
			QMessageBox msgBox;
			msgBox.setText("Invalid abscissas array for reconstructed centerline");
			msgBox.exec();
			return;
		}

		if (centerlineIds == nullptr)
		{
			QMessageBox msgBox;
			msgBox.setText("Invalid centerlineids array for reconstructed centerline");
			return;
		}

		if (frenetTangent == nullptr || frenetTangent->GetNumberOfComponents() != 3)
		{
			QMessageBox msgBox;
			msgBox.setText("Invalid Frenet tangent array for reconstructed centerline");
			msgBox.exec();
			return;
		}

		if (radius == nullptr || radius->GetNumberOfComponents() != 1)
		{
			QMessageBox msgBox;
			msgBox.setText("Invalid radius array for reconstructed centerline");
			msgBox.exec();
			return;
		}

		break;
	}

	double threshold_bound[2] = {
		abscissas->GetTuple(proximalId)[0],
		abscissas->GetTuple(distalId)[0]
	};

	std::cout << "Threshold bound: [" << threshold_bound[0] << "," << threshold_bound[1] << "]"<<std::endl;

	// check threshold bound is correct
	if (threshold_bound[1] - threshold_bound[0] <= 0)
	{
		QMessageBox msgBox;
		switch (dataType)
		{
		case DataType::Original:
			msgBox.setText("Invalid proximal/distal normal point for original centerline");
			break;
		case DataType::Recon:
			msgBox.setText("Invalid proximal/distal normal point for reconstructed centerline");
			break;
		}
		msgBox.exec();
		return;
	}

	vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
	std::cout << "centerlineids range: [" << centerlineIds->GetRange()[0] << "," << centerlineIds->GetRange()[1] << "]" << std::endl;

	for (int i = centerlineIds->GetRange()[0]; i <= centerlineIds->GetRange()[1]; i++)
	{
		//std::cout << "centerlineid: " << i << std::endl;

		// threshold to get independent centerline
		vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
		threshold->ThresholdBetween(i, i);
		threshold->SetInputData(centerline);
		threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
		threshold->Update();

		//std::cout << "threshold centerline" << std::endl;
		//threshold->GetOutput()->Print(std::cout);

		// convert threshold output to vtkpolydata
		vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
		geometryFilter->SetInputData(threshold->GetOutput());
		geometryFilter->Update();

		//std::cout << "threshold centerline with geometry" << std::endl;
		//geometryFilter->GetOutput()->Print(std::cout);

		if (geometryFilter->GetOutput()->GetNumberOfPoints() == 0)
			continue;

		vtkPolyData* singleCenterline = geometryFilter->GetOutput();
		switch (dataType)
		{
		case DataType::Original:
			singleCenterline->GetPointData()->SetActiveScalars(m_preferences->GetAbscissasArrayName().toStdString().c_str());
			break;
		case DataType::Recon:
			singleCenterline->GetPointData()->SetActiveScalars(m_preferences->GetReconAbscissasArrayName().toStdString().c_str());

			break;
		}

		// compute section to clip
		vtkSmartPointer<vtkClipPolyData> clipper1 = vtkSmartPointer<vtkClipPolyData>::New();
		clipper1->SetValue(threshold_bound[0]);
		clipper1->SetInsideOut(false);
		clipper1->GenerateClippedOutputOff();
		clipper1->SetInputData(singleCenterline);
		clipper1->Update();

		vtkSmartPointer<vtkClipPolyData> clipper2 = vtkSmartPointer<vtkClipPolyData>::New();
		clipper2->SetValue(threshold_bound[1]);
		clipper2->SetInsideOut(true);
		clipper2->GenerateClippedOutputOff();
		clipper2->SetInputData(clipper1->GetOutput());
		clipper2->Update();

		//clipper2->GetOutput()->Print(std::cout);

		// check if single centerline is close to distal point
		double epsilon = 1e0;
		double distalPointDistance = sqrt(vtkMath::Distance2BetweenPoints(clipper2->GetOutput()->GetPoint(clipper2->GetOutput()->GetNumberOfPoints() - 1), distalPt));

		//std::cout << "last point: " <<
		//	clipper2->GetOutput()->GetPoint(clipper2->GetOutput()->GetNumberOfPoints() - 1)[0] << ", " <<
		//	clipper2->GetOutput()->GetPoint(clipper2->GetOutput()->GetNumberOfPoints() - 1)[1] << ", " <<
		//	clipper2->GetOutput()->GetPoint(clipper2->GetOutput()->GetNumberOfPoints() - 1)[2] << std::endl;

		//std::cout << "distal point: " <<
		//	distalPt[0] << ", " <<
		//	distalPt[1] << ", " <<
		//	distalPt[2] << std::endl;

		//std::cout << "distance: " << distalPointDistance << std::endl;

		if (distalPointDistance > epsilon)
			continue;

		appendFilter->AddInputData(clipper2->GetOutput());
	}

	appendFilter->Update();
	//appendFilter->GetOutput()->Print(std::cout);
	clippedCenterline->DeepCopy(appendFilter->GetOutput());
	return;
}

void Measurements::clipSurface(vtkPolyData * surface, vtkPolyData *clippedCenterline, DataType dataType, vtkPolyData * clippedSurface)
{
	std::cout << "datatype: " << dataType << std::endl;

	std::cout << "Computing enlarged radius" << std::endl;
	vtkSmartPointer<vtkArrayCalculator> cal = vtkSmartPointer<vtkArrayCalculator>::New();
	cal->SetInputData(clippedCenterline);
	QString radiusArrayName; 
	QString centerlineIdsArrayName;
	QString abscissasArrayName;
	QString frenetTangentArrayName;

	switch (dataType)
	{
	case DataType::Original:
		radiusArrayName = m_preferences->GetRadiusArrayName();
		centerlineIdsArrayName = m_preferences->GetCenterlineIdsArrayName();
		abscissasArrayName = m_preferences->GetAbscissasArrayName();
		frenetTangentArrayName = m_preferences->GetFrenetTangentArrayName();
		break;
	case DataType::Recon:
		radiusArrayName = m_preferences->GetReconRadiusArrayName();
		centerlineIdsArrayName = m_preferences->GetReconCenterlineIdsArrayName();
		abscissasArrayName = m_preferences->GetReconAbscissasArrayName();
		frenetTangentArrayName = m_preferences->GetReconFrenetTangentArrayName();
		break;
	}
	cal->AddScalarArrayName(radiusArrayName.toStdString().c_str());
	char buffer[999];
	sprintf(buffer, "%s * %f", radiusArrayName.toStdString(), 1.5);
	cal->SetFunction(buffer);
	cal->SetResultArrayName(radiusArrayName.toStdString().c_str());
	cal->Update();

	std::cout << "Computing enlarged radius complete" << std::endl;
	vtkPolyData* clippedCenterilne = (vtkPolyData*)cal->GetOutput();

	// create implict function with spheres along clipped centerline
	vtkSmartPointer<vtkvmtkPolyBallLine> tubeFunction = vtkSmartPointer<vtkvmtkPolyBallLine>::New();
	tubeFunction->SetInput((vtkPolyData*)cal->GetOutput());
	tubeFunction->SetPolyBallRadiusArrayName(radiusArrayName.toStdString().c_str());

	vtkNew<vtkImplicitBoolean> endSpheresFunction;
	endSpheresFunction->SetOperationTypeToUnion();

	vtkPolyData* centerlineEnlargedRadius = (vtkPolyData*)cal->GetOutput();

	vtkDataArray* centerlineIds = centerlineEnlargedRadius->GetCellData()->GetArray(centerlineIdsArrayName.toStdString().c_str());

	for (int i = centerlineIds->GetRange()[0]; i <= centerlineIds->GetRange()[1]; i++)
	{
		// threshold to get independent centerline
		vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
		threshold->ThresholdBetween(i, i);
		threshold->SetInputData(clippedCenterilne);
		threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, centerlineIdsArrayName.toStdString().c_str());
		threshold->Update();

		if (threshold->GetOutput()->GetNumberOfPoints() == 0)
			continue;

		vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
		geomFilter->SetInputData(threshold->GetOutput());
		geomFilter->Update();

		vtkPolyData* singleCenterline = geomFilter->GetOutput();

		//std::cout << "centerlineid: " << i << std::endl;
		//singleCenterline->Print(std::cout);

		singleCenterline->GetPointData()->SetActiveScalars(abscissasArrayName.toStdString().c_str());

		// get the end points
		double* center0 = singleCenterline->GetPoint(0);
		double tangent0[3];
		tangent0[0] = singleCenterline->GetPointData()->GetArray(frenetTangentArrayName.toStdString().c_str())->GetComponent(0, 0);
		tangent0[1] = singleCenterline->GetPointData()->GetArray(frenetTangentArrayName.toStdString().c_str())->GetComponent(0, 1);
		tangent0[2] = singleCenterline->GetPointData()->GetArray(frenetTangentArrayName.toStdString().c_str())->GetComponent(0, 2);
		double radius0 = singleCenterline->GetPointData()->GetArray(radiusArrayName.toStdString().c_str())->GetComponent(0, 0);

		vtkNew<vtkSphere> sphere0;
		sphere0->SetCenter(center0[0], center0[1], center0[2]);
		sphere0->SetRadius(radius0*1.5);

		vtkNew<vtkPlane> plane0;
		plane0->SetOrigin(center0[0], center0[1], center0[2]);
		plane0->SetNormal(1.0*tangent0[0], 1.0*tangent0[1], 1.0*tangent0[2]);

		vtkNew<vtkImplicitBoolean> compositeFunction0;
		compositeFunction0->AddFunction(sphere0);
		compositeFunction0->AddFunction(plane0);
		compositeFunction0->SetOperationTypeToIntersection();

		double* center1 = singleCenterline->GetPoint(singleCenterline->GetNumberOfPoints() - 1);
		double tangent1[3];
		// reverse tangent direction
		tangent1[0] = -1.0*singleCenterline->GetPointData()->GetArray(frenetTangentArrayName.toStdString().c_str())->GetComponent(singleCenterline->GetNumberOfPoints() - 1, 0);
		tangent1[1] = -1.0*singleCenterline->GetPointData()->GetArray(frenetTangentArrayName.toStdString().c_str())->GetComponent(singleCenterline->GetNumberOfPoints() - 1, 1);
		tangent1[2] = -1.0*singleCenterline->GetPointData()->GetArray(frenetTangentArrayName.toStdString().c_str())->GetComponent(singleCenterline->GetNumberOfPoints() - 1, 2);
		double radius1 = singleCenterline->GetPointData()->GetArray(radiusArrayName.toStdString().c_str())->GetComponent(singleCenterline->GetNumberOfPoints() - 1, 0);

		vtkNew<vtkSphere> sphere1;
		sphere1->SetCenter(center1[0], center1[1], center1[2]);
		sphere1->SetRadius(radius1*1.5);

		vtkNew<vtkPlane> plane1;
		plane1->SetOrigin(center1[0], center1[1], center1[2]);
		plane1->SetNormal(1.0*tangent1[0], 1.0*tangent1[1], 1.0*tangent1[2]);

		vtkNew<vtkImplicitBoolean> compositeFunction1;
		compositeFunction1->AddFunction(sphere1);
		compositeFunction1->AddFunction(plane1);
		compositeFunction1->SetOperationTypeToIntersection();

		// add to overall composite function
		endSpheresFunction->AddFunction(compositeFunction0);
		endSpheresFunction->AddFunction(compositeFunction1);
	}

	vtkNew<vtkImplicitBoolean> compositeFunction;
	compositeFunction->AddFunction(tubeFunction);
	compositeFunction->AddFunction(endSpheresFunction);
	compositeFunction->SetOperationTypeToDifference();

	// create mask array with spheres and centerline tubes
	vtkSmartPointer<vtkDoubleArray> maskArray = vtkSmartPointer<vtkDoubleArray>::New();
	maskArray->SetNumberOfComponents(1);
	maskArray->SetNumberOfTuples(surface->GetNumberOfPoints());
	maskArray->SetName("Mask");
	maskArray->FillComponent(0, 0);

	std::cout << "evaluating surface file..." << std::endl;

	compositeFunction->EvaluateFunction(surface->GetPoints()->GetData(), maskArray);

	surface->GetPointData()->AddArray(maskArray);
	surface->GetPointData()->SetActiveScalars("Mask");

	vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
	clipper->SetValue(0);
	clipper->SetInsideOut(true);
	clipper->GenerateClippedOutputOff();
	clipper->SetInputData(surface);
	clipper->Update();

	vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
	cleaner->SetInputData(clipper->GetOutput());
	cleaner->Update();

	clippedSurface->DeepCopy(cleaner->GetOutput());
}

void Measurements::resetResults()
{
	// Normal to normal
	this->ui->labelLesionLengthNTN->setText("N/A");
	this->ui->labelLesionLumenalVolumeNTN->setText("N/A");
	this->ui->labelReconLumenalVolumeNTN->setText("N/A");
	this->ui->labelLumenalVolDeltaNTN->setText("N/A");
	this->ui->labelVolumeRatioNTN->setText("N/A");
	this->ui->labelVolumeRatioPerUnitLengthNTN->setText("N/A");
	this->ui->labelAdjustedVolumeRatioNTN->setText("N/A");

	// FWHM
	this->ui->labelLesionLengthFWHM->setText("N/A");
	this->ui->labelLesionLumenalVolumeFWHM->setText("N/A");
	this->ui->labelReconLumenalVolumeFWHM->setText("N/A");
	this->ui->labelLumenalVolDeltaFWHM->setText("N/A");
	this->ui->labelVolumeRatioFWHM->setText("N/A");
	this->ui->labelVolumeRatioPerUnitLengthFWHM->setText("N/A");

	// Morphological
	this->ui->labelStenosisRadius->setText("N/A");
	this->ui->labelProximalNormalRadius->setText("N/A");
	this->ui->labelDistalNormalRadius->setText("N/A");
	this->ui->labelDoSNascet->setText("N/A");
	this->ui->labelDistanceFactor->setText("N/A");
	this->ui->labelHausdorffDistanceReconToLesion->setText("N/A");
	this->ui->labelRelativeDistanceLesionToRecon->setText("N/A");
	this->ui->labellabelRelativeDistanceReconToLesion->setText("N/A");

	// Point data array
	this->ui->labelPointDataMaximum->setText("N/A");
	this->ui->labelPointDataMinimum->setText("N/A");
	this->ui->labelPointDataGradientMaximum->setText("N/A");
	this->ui->labelPointDataGradientMinimum->setText("N/A");
}

void Measurements::slotUpdate()
{
	// reset results
	this->resetResults();

	// check data
	vtkPolyData* surface = m_io->GetSurface();
	vtkPolyData* centerline = m_io->GetCenterline();
	vtkPolyData* reconSurface = m_io->GetReconstructedSurface();
	vtkPolyData* reconCenterline = m_io->GetReconstructedCenterline();

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

	if (
		(proximalPt[0] == 0 && proximalPt[1] == 0 && proximalPt[2] == 0) || 
		(distalPt[0] == 0 && distalPt[1] == 0 && distalPt[2] == 0)
		//(stenosisPt[0] == 0 && stenosisPt[1] == 0 && stenosisPt[2] == 0)
		)
	{
		QMessageBox msgBox;
		msgBox.setText("Invalid feature points");
		msgBox.exec();
		return;
	}

	// ======================== clip NTN centerline  ========================
	std::cout << "clipping NTN centerline" << std::endl;
	vtkSmartPointer<vtkPolyData> clippedCenterline = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> clippedSurface = vtkSmartPointer<vtkPolyData>::New();
	this->clipCenterline(proximalPt, distalPt, DataType::Original, clippedCenterline);

	// check clipped centerline ok
	if (clippedCenterline->GetNumberOfPoints() != 0)
	{
		vtkDataArray* abscissasClipped = clippedCenterline->GetPointData()->GetArray(m_preferences->GetAbscissasArrayName().toStdString().c_str());
		if (abscissasClipped != nullptr && abscissasClipped->GetNumberOfTuples() > 0)
		{
			
			double ntnLength = abscissasClipped->GetRange()[1] - abscissasClipped->GetRange()[0];
			ui->labelLesionLengthNTN->setText(QString::number(ntnLength, 'f', 3));
		}

		//m_io->SetCenterline(clippedCenterline);

		// ======================== clip NTN surface  ========================
		std::cout << "clipping NTN surface" << std::endl;
		this->clipSurface(m_io->GetSurface(), clippedCenterline, DataType::Original, clippedSurface);

		// check clipped centerline ok
		if (clippedSurface->GetNumberOfPoints() != 0)
		{
			//m_io->SetSurface(clippedSurface);

			// compute original NTN features
			vtkSmartPointer<vtkvmtkCapPolyData> capperSurface = vtkSmartPointer<vtkvmtkCapPolyData>::New();
			capperSurface->SetInputData(clippedSurface);
			capperSurface->Update();

			vtkSmartPointer<vtkMassProperties> massSurface = vtkSmartPointer<vtkMassProperties>::New();
			massSurface->SetInputData(capperSurface->GetOutput());
			massSurface->Update();
			ui->labelLesionLumenalVolumeNTN->setText(QString::number(massSurface->GetVolume(), 'f', 3));
		}
	}


	// ======================== clip NTN recon centerline  ========================
	std::cout << "clipping NTN recon centerline" << std::endl;
	vtkSmartPointer<vtkPolyData> clippedReconCenterline = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> clippedReconSurface = vtkSmartPointer<vtkPolyData>::New();
	this->clipCenterline(proximalPt, distalPt, DataType::Recon, clippedReconCenterline);

	// check clipped centerline ok
	if (clippedReconCenterline->GetNumberOfPoints() != 0)
	{
		//vtkDataArray* abscissasClipped = clippedReconCenterline->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str());
		//if (abscissasClipped != nullptr && abscissasClipped->GetNumberOfTuples() > 0)
		//{
		//	double ntnLength = abscissasClipped->GetRange()[1] - abscissasClipped->GetRange()[0];
		//	ui->labelLesionLengthNTN->setText(QString::number(ntnLength, 'f', 3));
		//}

		//m_io->SetReconstructedCenterline(clippedReconCenterline);

		// ======================== clip NTN recon surface  ========================
		std::cout << "clipping NTN recon surface" << std::endl;
		this->clipSurface(m_io->GetReconstructedSurface(), clippedReconCenterline, DataType::Recon, clippedReconSurface);

		// check clipped centerline ok
		if (clippedReconSurface->GetNumberOfPoints() != 0)
		{
			//m_io->SetReconstructedSurface(clippedReconSurface);

			// compute original NTN features
			vtkSmartPointer<vtkvmtkCapPolyData> capperSurface = vtkSmartPointer<vtkvmtkCapPolyData>::New();
			capperSurface->SetInputData(clippedReconSurface);
			capperSurface->Update();

			vtkSmartPointer<vtkMassProperties> massSurface = vtkSmartPointer<vtkMassProperties>::New();
			massSurface->SetInputData(capperSurface->GetOutput());
			massSurface->Update();
			ui->labelReconLumenalVolumeNTN->setText(QString::number(massSurface->GetVolume(), 'f', 3));
		}
	}

	if (ui->labelLesionLengthNTN->text() != "N/A" && ui->labelReconLumenalVolumeNTN->text() != "N/A")
	{
		double plaqueVol = ui->labelReconLumenalVolumeNTN->text().toDouble() - ui->labelLesionLumenalVolumeNTN->text().toDouble();
		double volRatio = plaqueVol / ui->labelReconLumenalVolumeNTN->text().toDouble()*100.0;
		ui->labelLumenalVolDeltaNTN->setText(QString::number(plaqueVol, 'f', 3));
		ui->labelVolumeRatioNTN->setText(QString::number(volRatio, 'f', 3));

		vtkDataArray* abscissasReconClipped = clippedReconCenterline->GetPointData()->GetArray(m_preferences->GetReconAbscissasArrayName().toStdString().c_str());

		if (abscissasReconClipped != nullptr && abscissasReconClipped->GetNumberOfTuples() > 0)
		{
			double ntnReconLength = abscissasReconClipped->GetRange()[1] - abscissasReconClipped->GetRange()[0];
			std::cout << "NTN reconstructed lesion length: " << ntnReconLength << "mm" << std::endl;
			double volRatioPerUnitLength = volRatio / ntnReconLength;
			double adjVolRatio = volRatio*ntnReconLength;
			ui->labelVolumeRatioPerUnitLengthNTN->setText(QString::number(volRatioPerUnitLength, 'f', 3));
			ui->labelAdjustedVolumeRatioNTN->setText(QString::number(adjVolRatio, 'f', 3));
		}
	}

	// locate maximal stenosis point
	double maximalStenosisRadius = clippedCenterline->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetRange()[0];
	double proximalNormalRaidus = clippedCenterline->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetTuple(0)[0];
	double distalNormalRaidus = clippedCenterline->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetTuple(clippedCenterline->GetNumberOfPoints()-1)[0];
	double dosNascet = (1.0 - maximalStenosisRadius / proximalNormalRaidus)*100.0;

	std::cout << "Degree of stenosis: " << dosNascet << "%" << std::endl;

	ui->labelStenosisRadius->setText(QString::number(maximalStenosisRadius,'f',3));
	ui->labelProximalNormalRadius->setText(QString::number(proximalNormalRaidus, 'f', 3));
	ui->labelDistalNormalRadius->setText(QString::number(distalNormalRaidus, 'f', 3));
	ui->labelDoSNascet->setText(QString::number(dosNascet, 'f', 3));

	// FWHM section
	std::cout << "Computing FWHM section" << std::endl;

	vtkSmartPointer<vtkArrayCalculator> calDoS = vtkSmartPointer<vtkArrayCalculator>::New();
	calDoS->SetInputData(clippedCenterline);
	calDoS->AddScalarArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());
	char bufferDoS[999];
	sprintf(bufferDoS, "(1 - %s / %f) *100", m_preferences->GetRadiusArrayName().toStdString(), proximalNormalRaidus);
	calDoS->SetFunction(bufferDoS);
	calDoS->SetResultArrayName("Degree of Stenosis");
	calDoS->Update();

	vtkPolyData* clippedCenterlineDoS = (vtkPolyData*)calDoS->GetOutput();

	vtkSmartPointer<vtkClipPolyData> clipperDoS = vtkSmartPointer<vtkClipPolyData>::New();
	clipperDoS->SetInputData(clippedCenterlineDoS);
	clipperDoS->SetValue(dosNascet*0.5);
	clipperDoS->GenerateClippedOutputOff();
	clipperDoS->SetInsideOut(false);
	clipperDoS->Update();

	// get the key points
	double proximalPtFwhm[3] = {
		clipperDoS->GetOutput()->GetPoint(0)[0],
		clipperDoS->GetOutput()->GetPoint(0)[1],
		clipperDoS->GetOutput()->GetPoint(0)[2]
	};

	double distalPtFwhm[3] = {
		clipperDoS->GetOutput()->GetPoint(clipperDoS->GetOutput()->GetNumberOfPoints()-1)[0],
		clipperDoS->GetOutput()->GetPoint(clipperDoS->GetOutput()->GetNumberOfPoints() - 1)[1],
		clipperDoS->GetOutput()->GetPoint(clipperDoS->GetOutput()->GetNumberOfPoints() - 1)[2]
	};

	// ======================== clip FWHM centerline  ========================
	std::cout << "clipping FWHM centerline" << std::endl;
	vtkSmartPointer<vtkPolyData> clippedCenterlineFWHM = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> clippedSurfaceFWHM = vtkSmartPointer<vtkPolyData>::New();
	this->clipCenterline(proximalPtFwhm, distalPtFwhm, DataType::Original, clippedCenterlineFWHM);

	// check clipped centerline ok
	if (clippedCenterlineFWHM->GetNumberOfPoints() != 0)
	{
		vtkDataArray* abscissasClipped = clippedCenterlineFWHM->GetPointData()->GetArray(m_preferences->GetAbscissasArrayName().toStdString().c_str());
		if (abscissasClipped != nullptr && abscissasClipped->GetNumberOfTuples() > 0)
		{

			double fwhmLength = abscissasClipped->GetRange()[1] - abscissasClipped->GetRange()[0];
			ui->labelLesionLengthFWHM->setText(QString::number(fwhmLength, 'f', 3));
		}

		//m_io->SetCenterline(clippedCenterlineFWHM);

		// ======================== clip FWHM surface  ========================
		std::cout << "clipping FWHM surface" << std::endl;
		this->clipSurface(m_io->GetSurface(), clippedCenterlineFWHM, DataType::Original, clippedSurfaceFWHM);

		// check clipped centerline ok
		if (clippedSurfaceFWHM->GetNumberOfPoints() != 0)
		{
			//m_io->SetSurface(clippedSurfaceFWHM);

			// compute original NTN features
			vtkSmartPointer<vtkvmtkCapPolyData> capperSurface = vtkSmartPointer<vtkvmtkCapPolyData>::New();
			capperSurface->SetInputData(clippedSurfaceFWHM);
			capperSurface->Update();

			vtkSmartPointer<vtkMassProperties> massSurface = vtkSmartPointer<vtkMassProperties>::New();
			massSurface->SetInputData(capperSurface->GetOutput());
			massSurface->Update();
			ui->labelLesionLumenalVolumeFWHM->setText(QString::number(massSurface->GetVolume(), 'f', 3));
		}
	}

	// ======================== clip FWHM recon centerline  ========================
	std::cout << "clipping FWHM recon centerline" << std::endl;
	vtkSmartPointer<vtkPolyData> clippedReconCenterlineFWHM = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> clippedReconSurfaceFWHM = vtkSmartPointer<vtkPolyData>::New();
	this->clipCenterline(proximalPtFwhm, distalPtFwhm, DataType::Recon, clippedReconCenterlineFWHM);

	// check clipped centerline ok
	if (clippedReconCenterlineFWHM->GetNumberOfPoints() != 0)
	{
		//vtkDataArray* abscissasClipped = clippedReconCenterline->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str());
		//if (abscissasClipped != nullptr && abscissasClipped->GetNumberOfTuples() > 0)
		//{
		//	double ntnLength = abscissasClipped->GetRange()[1] - abscissasClipped->GetRange()[0];
		//	ui->labelLesionLengthNTN->setText(QString::number(ntnLength, 'f', 3));
		//}

		//m_io->SetReconstructedCenterline(clippedReconCenterlineFWHM);

		// ======================== clip FWHM recon surface  ========================
		std::cout << "clipping FWHM recon surface" << std::endl;
		this->clipSurface(m_io->GetReconstructedSurface(), clippedReconCenterlineFWHM, DataType::Recon, clippedReconSurfaceFWHM);

		// check clipped centerline ok
		if (clippedReconSurfaceFWHM->GetNumberOfPoints() != 0)
		{
			//m_io->SetReconstructedSurface(clippedReconSurfaceFWHM);

			// compute original NTN features
			vtkSmartPointer<vtkvmtkCapPolyData> capperSurface = vtkSmartPointer<vtkvmtkCapPolyData>::New();
			capperSurface->SetInputData(clippedReconSurfaceFWHM);
			capperSurface->Update();

			vtkSmartPointer<vtkMassProperties> massSurface = vtkSmartPointer<vtkMassProperties>::New();
			massSurface->SetInputData(capperSurface->GetOutput());
			massSurface->Update();
			ui->labelReconLumenalVolumeFWHM->setText(QString::number(massSurface->GetVolume(), 'f', 3));
		}
	}

	if (ui->labelLesionLengthFWHM->text() != "N/A" && ui->labelReconLumenalVolumeFWHM->text() != "N/A")
	{
		double plaqueVol = ui->labelReconLumenalVolumeFWHM->text().toDouble() - ui->labelLesionLumenalVolumeFWHM->text().toDouble();
		double volRatio = plaqueVol / ui->labelReconLumenalVolumeFWHM->text().toDouble()*100.0;
		ui->labelLumenalVolDeltaFWHM->setText(QString::number(plaqueVol, 'f', 3));
		ui->labelVolumeRatioFWHM->setText(QString::number(volRatio, 'f', 3));

		vtkDataArray* abscissasReconClipped = clippedReconCenterlineFWHM->GetPointData()->GetArray(m_preferences->GetReconAbscissasArrayName().toStdString().c_str());

		if (abscissasReconClipped != nullptr && abscissasReconClipped->GetNumberOfTuples() > 0)
		{
			double fwhmReconLength = abscissasReconClipped->GetRange()[1] - abscissasReconClipped->GetRange()[0];
			std::cout << "FWHM reconstructed lesion length: " << fwhmReconLength << "mm" << std::endl;
			double volRatioPerUnitLength = volRatio / fwhmReconLength;
			double adjVolRatio = volRatio*fwhmReconLength;
			ui->labelVolumeRatioPerUnitLengthFWHM->setText(QString::number(volRatioPerUnitLength, 'f', 3));
			ui->labelAdjustedVolumeRatioFWHM->setText(QString::number(adjVolRatio, 'f', 3));
		}
	}

	if (ui->labelLesionLengthNTN->text() != "N/A" && ui->labelLesionLengthFWHM->text() != "N/A")
	{
		double distanceFactor = ui->labelLesionLengthFWHM->text().toDouble() / ui->labelLesionLengthNTN->text().toDouble();

		ui->labelDistanceFactor->setText(QString::number(distanceFactor, 'f', 3));
	}

	// compute distance between surfaces
	if (clippedSurface->GetNumberOfPoints() != 0 && clippedReconSurface->GetNumberOfPoints() != 0)
	{
		vtkSmartPointer<vtkHausdorffDistancePointSetFilter> hausdorffDistanceFilter = vtkSmartPointer<vtkHausdorffDistancePointSetFilter>::New();
		hausdorffDistanceFilter->SetInputData(0, clippedSurface);
		hausdorffDistanceFilter->SetInputData(1, clippedReconSurface);
		hausdorffDistanceFilter->Update();
		
		ui->labelHausdorffDistanceReconToLesion->setText(QString::number(hausdorffDistanceFilter->GetHausdorffDistance(), 'f', 3));
		ui->labelRelativeDistanceLesionToRecon->setText(QString::number(hausdorffDistanceFilter->GetRelativeDistance()[0], 'f', 3));
		ui->labellabelRelativeDistanceReconToLesion->setText(QString::number(hausdorffDistanceFilter->GetRelativeDistance()[1], 'f', 3));
	}

	// compute point data value
	QString pointDataArrayName = ui->comboBoxPointDataArray->currentText();
	vtkDataArray* abscissasArray  = clippedCenterline->GetPointData()->GetArray(m_preferences->GetAbscissasArrayName().toStdString().c_str());
	vtkDataArray* pointDataArray = clippedCenterline->GetPointData()->GetArray(pointDataArrayName.toStdString().c_str());
	if (pointDataArray != nullptr)
	{
		ui->labelPointDataMaximum->setText(QString::number(pointDataArray->GetRange()[1], 'f', 3));
		ui->labelPointDataMinimum->setText(QString::number(pointDataArray->GetRange()[0], 'f', 3));

		if (abscissasArray != nullptr)
		{
			// derivative
			QVector<double> x;
			QVector<double> y;
			QVector<double> y_grad;

			for (int i = 0; i < clippedCenterline->GetNumberOfPoints(); i++)
			{
				x.append(abscissasArray->GetTuple(i)[0]);
				if (pointDataArray->GetNumberOfComponents() == 3)
				{
					y.append(sqrt(
						std::pow(pointDataArray->GetTuple(i)[0], 2) +
						std::pow(pointDataArray->GetTuple(i)[1], 2) +
						std::pow(pointDataArray->GetTuple(i)[2], 2)));
				}
				else
				{
					y.append(pointDataArray->GetTuple(i)[0]);
				}
			}

			for (int i = 1; i < y.length() - 1; i++)
			{
				y_grad.append((y[i + 1] - y[i - 1]) / 2);
			}

			double gradMax = *std::max_element(y_grad.constBegin(), y_grad.constEnd());
			double gradMin = *std::min_element(y_grad.constBegin(), y_grad.constEnd());

			ui->labelPointDataGradientMaximum->setText(QString::number(gradMax, 'f', 3));
			ui->labelPointDataGradientMinimum->setText(QString::number(gradMin, 'f', 3));
		}
	}


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