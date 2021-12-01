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

	std::cout << "proximal point: " << proximalPt[0] << ", " << proximalPt[1] << ", " << proximalPt[2] << std::endl;
	std::cout << "distal point: " << distalPt[0] << ", " << distalPt[1] << ", " << distalPt[2] << std::endl;

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

	int proximalId = locator->FindClosestPoint(proximalPt);
	int distalId = locator->FindClosestPoint(distalPt);
	//int stenosisId = locator->FindClosestPoint(stenosisPt);

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

	std::cout << "Threshold bound: [" << threshold_bound[0] << "," << threshold_bound[1] << "]"<<std::endl;

	// check threshold bound is correct
	if (threshold_bound[1] - threshold_bound[0] <= 0)
	{
		QMessageBox msgBox;
		msgBox.setText("Invalid proximal/distal normal point");
		msgBox.exec();
		return;
	}

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

	std::cout << "centerlineids range: [" << centerlineIds->GetRange()[0] << "," << centerlineIds->GetRange()[1] << "]" << std::endl;

	for (int i = centerlineIds->GetRange()[0]; i <= centerlineIds->GetRange()[1]; i++)
	{
		std::cout << "centerlineid: " << i << std::endl;

		// threshold to get independent centerline
		vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
		threshold->ThresholdBetween(i, i);
		threshold->SetInputData(m_io->GetCenterline());
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
		singleCenterline->GetPointData()->SetActiveScalars(m_preferences->GetAbscissasArrayName().toStdString().c_str());

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

		std::cout << "last point: " <<
			clipper2->GetOutput()->GetPoint(clipper2->GetOutput()->GetNumberOfPoints() - 1)[0] << ", " <<
			clipper2->GetOutput()->GetPoint(clipper2->GetOutput()->GetNumberOfPoints() - 1)[1] << ", " <<
			clipper2->GetOutput()->GetPoint(clipper2->GetOutput()->GetNumberOfPoints() - 1)[2] << std::endl;

		std::cout << "distal point: " <<
			distalPt[0] << ", " <<
			distalPt[1] << ", " <<
			distalPt[2] << std::endl;

		std::cout << "distance: " << distalPointDistance << std::endl;

		if (distalPointDistance > epsilon)
			continue;

		appendFilter->AddInputData(clipper2->GetOutput());
	}

	appendFilter->Update();
	//appendFilter->GetOutput()->Print(std::cout);

	vtkDataArray* abscissasClipped = appendFilter->GetOutput()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str());
	if (abscissasClipped != nullptr && abscissasClipped->GetNumberOfTuples() > 0)
	{
		double ntnLength = abscissasClipped->GetRange()[1] - abscissasClipped->GetRange()[0];
		ui->labelLesionLengthNTN->setText(QString::number(ntnLength, 'f', 3));
	}

	std::cout << "Computing enlarged radius" << std::endl;
	vtkSmartPointer<vtkArrayCalculator> cal = vtkSmartPointer<vtkArrayCalculator>::New();
	cal->SetInputData(appendFilter->GetOutput());
	cal->AddScalarArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());
	char buffer[999];
	sprintf(buffer, "%s * %f", m_preferences->GetRadiusArrayName().toStdString(), 1.5);
	cal->SetFunction(buffer);
	cal->SetResultArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());
	cal->Update();

	std::cout << "Computing enlarged radius complete" << std::endl;
	vtkPolyData* clippedCenterilne = (vtkPolyData*)cal->GetOutput();

	// ======================== clip surface  ========================
	// create implict function with spheres along clipped centerline
	vtkSmartPointer<vtkvmtkPolyBallLine> tubeFunction = vtkSmartPointer<vtkvmtkPolyBallLine>::New();
	tubeFunction->SetInput((vtkPolyData*)cal->GetOutput());
	tubeFunction->SetPolyBallRadiusArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());

	vtkNew<vtkImplicitBoolean> endSpheresFunction;
	endSpheresFunction->SetOperationTypeToUnion();

	vtkPolyData* centerlineEnlargedRadius = (vtkPolyData*)cal->GetOutput();

	vtkDataArray* centerlineIds2 = centerlineEnlargedRadius->GetCellData()->GetArray(m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());

	for (int i = centerlineIds2->GetRange()[0]; i <= centerlineIds2->GetRange()[1]; i++)
	{
		// threshold to get independent centerline
		vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
		threshold->ThresholdBetween(i, i);
		threshold->SetInputData(appendFilter->GetOutput());
		threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
		threshold->Update();

		if (threshold->GetOutput()->GetNumberOfPoints() == 0)
			continue;

		vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
		geomFilter->SetInputData(threshold->GetOutput());
		geomFilter->Update();

		vtkPolyData* singleCenterline = geomFilter->GetOutput();

		//std::cout << "centerlineid: " << i << std::endl;
		//singleCenterline->Print(std::cout);

		singleCenterline->GetPointData()->SetActiveScalars(m_preferences->GetAbscissasArrayName().toStdString().c_str());

		// get the end points
		double* center0 = singleCenterline->GetPoint(0);
		double tangent0[3];
		tangent0[0] = singleCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(0, 0);
		tangent0[1] = singleCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(0, 1);
		tangent0[2] = singleCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(0, 2);
		double radius0 = singleCenterline->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetComponent(0, 0);

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
		tangent1[0] = -1.0*singleCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(singleCenterline->GetNumberOfPoints() - 1, 0);
		tangent1[1] = -1.0*singleCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(singleCenterline->GetNumberOfPoints() - 1, 1);
		tangent1[2] = -1.0*singleCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(singleCenterline->GetNumberOfPoints() - 1, 2);
		double radius1 = singleCenterline->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetComponent(singleCenterline->GetNumberOfPoints() - 1, 0);

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
	maskArray->SetNumberOfTuples(m_io->GetSurface()->GetNumberOfPoints());
	maskArray->SetName("Mask");
	maskArray->FillComponent(0, 0);

	std::cout << "evaluating surface file..." << std::endl;

	compositeFunction->EvaluateFunction(m_io->GetSurface()->GetPoints()->GetData(), maskArray);

	m_io->GetSurface()->GetPointData()->AddArray(maskArray);
	m_io->GetSurface()->GetPointData()->SetActiveScalars("Mask");

	vtkSmartPointer<vtkClipPolyData> clipperS = vtkSmartPointer<vtkClipPolyData>::New();
	clipperS->SetValue(0);
	clipperS->SetInsideOut(true);
	clipperS->GenerateClippedOutputOff();
	clipperS->SetInputData(m_io->GetSurface());
	clipperS->Update();

	vtkSmartPointer<vtkCleanPolyData> cleanerS = vtkSmartPointer<vtkCleanPolyData>::New();
	cleanerS->SetInputData(clipperS->GetOutput());
	cleanerS->Update();

	//vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
	//geomFilter->SetInputData(clipperV->GetOutput());
	//geomFilter->Update();

	vtkSmartPointer<vtkvmtkCapPolyData> capperSurface = vtkSmartPointer<vtkvmtkCapPolyData>::New();
	capperSurface->SetInputData(cleanerS->GetOutput());
	capperSurface->Update();

	vtkSmartPointer<vtkMassProperties> massSurface = vtkSmartPointer<vtkMassProperties>::New();
	massSurface->SetInputData(capperSurface->GetOutput());
	massSurface->Update();
	ui->labelLesionLumenalVolumeNTN->setText(QString::number(massSurface->GetVolume(), 'f', 3));

	vtkPolyData* clippedSurface = cleanerS->GetOutput();
	m_io->SetSurface(clippedSurface);

	// reconstructed surface
	if (m_io->GetReconstructedSurface()->GetNumberOfPoints() != 0)
	{
		compositeFunction->EvaluateFunction(m_io->GetReconstructedSurface()->GetPoints()->GetData(), maskArray);

		m_io->GetReconstructedSurface()->GetPointData()->AddArray(maskArray);
		m_io->GetReconstructedSurface()->GetPointData()->SetActiveScalars("Mask");

		vtkSmartPointer<vtkClipPolyData> clipperR = vtkSmartPointer<vtkClipPolyData>::New();
		clipperR->SetValue(0);
		clipperR->SetInsideOut(true);
		clipperR->GenerateClippedOutputOff();
		clipperR->SetInputData(m_io->GetReconstructedSurface());
		clipperR->Update();

		vtkSmartPointer<vtkCleanPolyData> cleanerR = vtkSmartPointer<vtkCleanPolyData>::New();
		cleanerR->SetInputData(clipperR->GetOutput());
		cleanerR->Update();

		//vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
		//geomFilter->SetInputData(clipperV->GetOutput());
		//geomFilter->Update();

		vtkSmartPointer<vtkvmtkCapPolyData> capperRecon = vtkSmartPointer<vtkvmtkCapPolyData>::New();
		capperRecon->SetInputData(cleanerR->GetOutput());
		capperRecon->Update();

		vtkSmartPointer<vtkMassProperties> massRecon = vtkSmartPointer<vtkMassProperties>::New();
		massRecon->SetInputData(capperRecon->GetOutput());
		massRecon->Update();
		ui->labelReconLumenalVolumeNTN->setText(QString::number(massRecon->GetVolume(), 'f', 3));

		double plaqueVol = massRecon->GetVolume() - massSurface->GetVolume();
		double volRatio = plaqueVol / massRecon->GetVolume()*100.0;
		ui->labelLumenalVolDeltaNTN->setText(QString::number(plaqueVol, 'f', 3));
		ui->labelVolumeRatioNTN->setText(QString::number(volRatio, 'f', 3));

		if (abscissasClipped != nullptr && abscissasClipped->GetNumberOfTuples() > 0)
		{
			double ntnLength = abscissasClipped->GetRange()[1] - abscissasClipped->GetRange()[0];
			double volRatioPerUnitLength = volRatio / ntnLength;
			double adjVolRatio = volRatio*ntnLength;
			ui->labelVolumeRatioPerUnitLengthNTN->setText(QString::number(volRatioPerUnitLength, 'f', 3));
			ui->labelAdjustedVolumeRatioNTN->setText(QString::number(adjVolRatio, 'f', 3));
		}

		vtkPolyData* clippedReconSurface = capperRecon->GetOutput();
		m_io->SetReconstructedSurface(cleanerR->GetOutput());
	}

	// FWHM section
	std::cout << "Computing FWHM section" << std::endl;

	double stenosisPt[3] = {
		m_centerlinesInfoWidget->GetStenosisPoint()[0],
		m_centerlinesInfoWidget->GetStenosisPoint()[1],
		m_centerlinesInfoWidget->GetStenosisPoint()[2]
	};
	std::cout << "stenosis point: " << stenosisPt[0] << ", " << stenosisPt[1] << ", " << stenosisPt[2] << std::endl;

	if (stenosisPt[0] == 0 && stenosisPt[1] == 0 && stenosisPt[2] == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("Invalid stenosis points, FWHM calcluation abort.");
		msgBox.exec();
		return;
	}

	vtkSmartPointer<vtkKdTreePointLocator> stenosisLocator = vtkSmartPointer<vtkKdTreePointLocator>::New();
	stenosisLocator->SetDataSet(appendFilter->GetOutput());
	stenosisLocator->BuildLocator();
	int stenosisId = stenosisLocator->FindClosestPoint(stenosisPt);

	// locate maximal stenosis point
	double maximalStenosisRadius = appendFilter->GetOutput()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetRange()[stenosisId];
	double proximalNormalRaidus = appendFilter->GetOutput()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetTuple(0)[0];
	double distalNormalRaidus = appendFilter->GetOutput()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetTuple(appendFilter->GetOutput()->GetNumberOfPoints()-1)[0];
	double dosNascet = (1.0 - maximalStenosisRadius / proximalNormalRaidus)*100.0;

	std::cout << "Degree of stenosis: " << dosNascet << "%" << std::endl;

	ui->labelStenosisRadius->setText(QString::number(maximalStenosisRadius,'f',3));
	ui->labelProximalNormalRadius->setText(QString::number(proximalNormalRaidus, 'f', 3));
	ui->labelDistalNormalRadius->setText(QString::number(distalNormalRaidus, 'f', 3));
	ui->labelDoSNascet->setText(QString::number(dosNascet, 'f', 3));

	vtkSmartPointer<vtkArrayCalculator> calDoS = vtkSmartPointer<vtkArrayCalculator>::New();
	calDoS->SetInputData(appendFilter->GetOutput());
	calDoS->AddScalarArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());
	char bufferDoS[999];
	sprintf(bufferDoS, "(1 - %s / %f) *100", m_preferences->GetRadiusArrayName().toStdString(), proximalNormalRaidus);
	calDoS->SetFunction(bufferDoS);
	calDoS->SetResultArrayName("Degree of Stenosis");
	calDoS->Update();

	vtkPolyData* centerlineDoS = (vtkPolyData*)calDoS->GetOutput();
	centerlineDoS->GetPointData()->SetActiveScalars("Degree of Stenosis");

	std::cout << "centerlineDoS" << std::endl;
	centerlineDoS->Print(std::cout);

	m_io->SetCenterline(centerlineDoS);

	//vtkSmartPointer<vtkClipPolyData> clipperDoS = vtkSmartPointer<vtkClipPolyData>::New();
	//clipperDoS->SetInputData(centerlineDoS);
	//clipperDoS->SetValue(dosNascet*0.5);
	//clipperDoS->GenerateClippedOutputOff();
	//clipperDoS->SetInsideOut(false);
	//clipperDoS->Update();

	//std::cout << "clipperDoS" << std::endl;
	//clipperDoS->GetOutput()->Print(std::cout);

	//vtkPolyData* fwhmCenterline = clipperDoS->GetOutput();
	//vtkDataArray* abscissasFwhm = fwhmCenterline->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str());
	//if (abscissasFwhm != nullptr && abscissasFwhm->GetNumberOfTuples() > 0)
	//{
	//	double fwhmLength = abscissasFwhm->GetRange()[1] - abscissasFwhm->GetRange()[0];
	//	ui->labelLesionLengthFWHM->setText(QString::number(fwhmLength, 'f', 3));
	//}
	//// ===================== clip the fwhm section =====================


	//std::cout << "Computing FWHM enlarged radius" << std::endl;
	//vtkSmartPointer<vtkArrayCalculator> calFwhm = vtkSmartPointer<vtkArrayCalculator>::New();
	//calFwhm->SetInputData(clipperDoS->GetOutput());
	//calFwhm->AddScalarArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());
	//char bufferFwhm[999];
	//sprintf(bufferFwhm, "%s * %f", m_preferences->GetRadiusArrayName().toStdString(), 1.5);
	//calFwhm->SetFunction(bufferFwhm);
	//calFwhm->SetResultArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());
	//calFwhm->Update();
	//std::cout << "Computing FWHM enlarged radius complete" << std::endl;
	//vtkPolyData* centerlineEnlargedRadiusFwhm = (vtkPolyData*)calFwhm->GetOutput();

	//centerlineEnlargedRadiusFwhm->Print(std::cout);

	//// create implict function with spheres along clipped centerline
	//vtkSmartPointer<vtkvmtkPolyBallLine> tubeFunctionFwhm = vtkSmartPointer<vtkvmtkPolyBallLine>::New();
	//tubeFunctionFwhm->SetInput(centerlineEnlargedRadiusFwhm);
	//tubeFunctionFwhm->SetPolyBallRadiusArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());

	//vtkNew<vtkImplicitBoolean> endSpheresFunctionFwhm;
	//endSpheresFunctionFwhm->SetOperationTypeToUnion();

	//vtkDataArray* centerlineIdsFwhm = centerlineEnlargedRadiusFwhm->GetCellData()->GetArray(m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());

	//centerlineIdsFwhm->Print(std::cout);

	//for (int i = centerlineIdsFwhm->GetRange()[0]; i <= centerlineIdsFwhm->GetRange()[1]; i++)
	//{
	//	// threshold to get independent centerline
	//	vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
	//	threshold->ThresholdBetween(i, i);
	//	threshold->SetInputData(centerlineEnlargedRadiusFwhm);
	//	threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
	//	threshold->Update();

	//	if (threshold->GetOutput()->GetNumberOfPoints() == 0)
	//		continue;

	//	vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
	//	geomFilter->SetInputData(threshold->GetOutput());
	//	geomFilter->Update();

	//	vtkPolyData* singleCenterline = geomFilter->GetOutput();

	//	std::cout << "centerlineid: " << i << std::endl;
	//	singleCenterline->Print(std::cout);

	//	singleCenterline->GetPointData()->SetActiveScalars(m_preferences->GetAbscissasArrayName().toStdString().c_str());

	//	// get the end points
	//	double* center0 = singleCenterline->GetPoint(0);
	//	double tangent0[3];
	//	tangent0[0] = singleCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(0, 0);
	//	tangent0[1] = singleCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(0, 1);
	//	tangent0[2] = singleCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(0, 2);
	//	double radius0 = singleCenterline->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetComponent(0, 0);

	//	vtkNew<vtkSphere> sphere0;
	//	sphere0->SetCenter(center0[0], center0[1], center0[2]);
	//	sphere0->SetRadius(radius0*1.5);

	//	vtkNew<vtkPlane> plane0;
	//	plane0->SetOrigin(center0[0], center0[1], center0[2]);
	//	plane0->SetNormal(1.0*tangent0[0], 1.0*tangent0[1], 1.0*tangent0[2]);

	//	vtkNew<vtkImplicitBoolean> compositeFunction0;
	//	compositeFunction0->AddFunction(sphere0);
	//	compositeFunction0->AddFunction(plane0);
	//	compositeFunction0->SetOperationTypeToIntersection();

	//	double* center1 = singleCenterline->GetPoint(singleCenterline->GetNumberOfPoints() - 1);
	//	double tangent1[3];
	//	// reverse tangent direction
	//	tangent1[0] = -1.0*singleCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(singleCenterline->GetNumberOfPoints() - 1, 0);
	//	tangent1[1] = -1.0*singleCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(singleCenterline->GetNumberOfPoints() - 1, 1);
	//	tangent1[2] = -1.0*singleCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(singleCenterline->GetNumberOfPoints() - 1, 2);
	//	double radius1 = singleCenterline->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetComponent(singleCenterline->GetNumberOfPoints() - 1, 0);

	//	vtkNew<vtkSphere> sphere1;
	//	sphere1->SetCenter(center1[0], center1[1], center1[2]);
	//	sphere1->SetRadius(radius1*1.5);

	//	vtkNew<vtkPlane> plane1;
	//	plane1->SetOrigin(center1[0], center1[1], center1[2]);
	//	plane1->SetNormal(1.0*tangent1[0], 1.0*tangent1[1], 1.0*tangent1[2]);

	//	vtkNew<vtkImplicitBoolean> compositeFunction1;
	//	compositeFunction1->AddFunction(sphere1);
	//	compositeFunction1->AddFunction(plane1);
	//	compositeFunction1->SetOperationTypeToIntersection();

	//	// add to overall composite function
	//	endSpheresFunctionFwhm->AddFunction(compositeFunction0);
	//	endSpheresFunctionFwhm->AddFunction(compositeFunction1);
	//}

	//vtkNew<vtkImplicitBoolean> compositeFunctionFwhm;
	//compositeFunctionFwhm->AddFunction(tubeFunctionFwhm);
	//compositeFunctionFwhm->AddFunction(endSpheresFunctionFwhm);
	//compositeFunctionFwhm->SetOperationTypeToDifference();

	//// create mask array with spheres and centerline tubes
	//vtkSmartPointer<vtkDoubleArray> maskArrayFwhm = vtkSmartPointer<vtkDoubleArray>::New();
	//maskArrayFwhm->SetNumberOfComponents(1);
	//maskArrayFwhm->SetNumberOfTuples(m_io->GetSurface()->GetNumberOfPoints());
	//maskArrayFwhm->SetName("Mask");
	//maskArrayFwhm->FillComponent(0, 0);

	//std::cout << "evaluating FWHM surface file..." << std::endl;

	//compositeFunctionFwhm->EvaluateFunction(m_io->GetSurface()->GetPoints()->GetData(), maskArrayFwhm);

	//m_io->GetSurface()->GetPointData()->AddArray(maskArrayFwhm);
	//m_io->GetSurface()->GetPointData()->SetActiveScalars("Mask");

	//vtkSmartPointer<vtkClipPolyData> clipperFwhmS = vtkSmartPointer<vtkClipPolyData>::New();
	//clipperFwhmS->SetValue(0);
	//clipperFwhmS->SetInsideOut(true);
	//clipperFwhmS->GenerateClippedOutputOff();
	//clipperFwhmS->SetInputData(m_io->GetSurface());
	//clipperFwhmS->Update();

	//vtkSmartPointer<vtkCleanPolyData> cleanerFwhmS = vtkSmartPointer<vtkCleanPolyData>::New();
	//cleanerFwhmS->SetInputData(clipperFwhmS->GetOutput());
	//cleanerFwhmS->Update();

	//////vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
	//////geomFilter->SetInputData(clipperV->GetOutput());
	//////geomFilter->Update();

	//vtkSmartPointer<vtkvmtkCapPolyData> capperSurfaceFwhm = vtkSmartPointer<vtkvmtkCapPolyData>::New();
	//capperSurfaceFwhm->SetInputData(cleanerFwhmS->GetOutput());
	//capperSurfaceFwhm->Update();

	//vtkSmartPointer<vtkMassProperties> massSurfaceFwhm = vtkSmartPointer<vtkMassProperties>::New();
	//massSurfaceFwhm->SetInputData(capperSurfaceFwhm->GetOutput());
	//massSurfaceFwhm->Update();
	//ui->labelLesionLumenalVolumeFWHM->setText(QString::number(massSurfaceFwhm->GetVolume(), 'f', 3));

	//vtkPolyData* clippedSurfaceFwhm = cleanerFwhmS->GetOutput();
	//m_io->SetSurface(clippedSurfaceFwhm);

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