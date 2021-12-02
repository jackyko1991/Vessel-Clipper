#include "mainWindow.h"
#include "interactorStyleCenterline.h"

// qt
#include "QPushButton"
#include "QFileDialog"
#include "QProgressBar"
#include "QStatusBar"
#include "QtConcurrent"
#include <QtConcurrent/qtconcurrentrun.h>
#include "QSlider"
#include "QDoubleSpinBox"
#include "QTableWidget"
#include "QComboBox"
#include <QMessageBox>
#include <QAction>

// vtk
#include "QVTKWidget.h"
#include "vtkRenderWindow.h"
#include "vtkProperty.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkBoxWidget.h"
#include "vtkPlanes.h"
#include "vtkClipPolyData.h"
#include "vtkConnectivityFilter.h"
#include "vtkKdTreePointLocator.h"
#include "vtkFeatureEdges.h"
#include "vtkStripper.h"
#include "vtkThreshold.h"
#include <vtkVersion.h>
#include "vtkGeometryFilter.h"
#include "vtkImplicitPolyDataDistance.h"
#include "vtkCenterOfMass.h"
#include "vtkArrowSource.h"
#include "vtkUnstructuredGrid.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkAppendPolyData.h"
#include "vtkCleanPolyData.h"
#include "vtkPlaneSource.h"
#include "vtkCellData.h"
#include "vtkTextMapper.h"
#include "vtkProperty2D.h"
#include "vtkLookupTable.h"
#include "vtkTriangleFilter.h"
#include "vtkParametricSpline.h"
#include "vtkParametricFunctionSource.h"
#include "vtkSplineFilter.h"
#include "vtkSphere.h"
#include "vtkPlane.h"
#include "vtkImplicitBoolean.h"
#include "vtkDoubleArray.h"
#include "vtkCylinder.h"
#include "vtkPointInterpolator.h"
#include "vtkVoronoiKernel.h"
#include "vtkPolyDataNormals.h"
#include "vtkDelaunay3D.h"
#include "vtkArrayCalculator.h"
#include "vtkMarchingCubes.h"

# include "vtkXMLImageDataWriter.h"

// vmtk
#include "vtkvmtkPolyDataCenterlines.h"
#include "vtkvmtkCenterlineAttributesFilter.h"
#include "vtkvmtkCenterlineGeometry.h"
#include "vtkvmtkCenterlineBranchExtractor.h"
#include "vtkvmtkCapPolyData.h"
#include "vtkvmtkInternalTetrahedraExtractor.h"
#include "vtkvmtkVoronoiDiagram3D.h"
#include "vtkvmtkSimplifyVoronoiDiagram.h"
#include "vtkvmtkPolyDataSurfaceRemeshing.h"
#include "vtkvmtkPolyBallLine.h"
#include "vtkvmtkPolyBallModeller.h"

// widgets
#include "branch_operation.h"
#include "preferences.h"
#include "measurements.h"

MainWindow::MainWindow(QMainWindow *parent) : ui(new Ui::MainWindow)
{
	setWindowIcon(QIcon(":icon/icon.ico"));
	vtkObject::GlobalWarningDisplayOff();
	ui->setupUi(this);

	// io object
	m_io = new IO;

	// qvtk widget start
	ui->qvtkWidget->GetRenderWindow()->AddRenderer(m_renderer);

	// interactor style
	vtkSmartPointer<MouseInteractorStyleCenterline> style = vtkSmartPointer<MouseInteractorStyleCenterline>::New();
	style->SetDataIo(m_io);
	ui->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);

	// text actors
	vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();
	textActor->SetInput("Press G to Locate the Centerline Source/Target");
	textActor->SetPosition(5, 5);
	textActor->GetTextProperty()->SetFontSize(16);
	textActor->GetTextProperty()->SetColor(23*1.0/255, 255*1.0/255, 58*1.0/255);
	m_renderer->AddActor2D(textActor);

	// create objects for the label and progress bar
	m_statusLabel = new QLabel(this);
	m_statusLabel->setMinimumWidth(200);
	m_statusLabel->setMaximumWidth(200);
	m_statusProgressBar = new QProgressBar(this);

	// set text for the label
	m_statusLabel->setText("Ready");

	// make progress bar text invisible
	m_statusProgressBar->setTextVisible(true);

	// add the two controls to the status bar
	ui->statusBar->addPermanentWidget(m_statusLabel);
	ui->statusBar->addPermanentWidget(m_statusProgressBar, 1);

	// centerline table
	ui->tableWidgetCenterline->verticalHeader()->setVisible(false);
	ui->tableWidgetCenterline->setSelectionBehavior(QAbstractItemView::SelectRows);

	// boundary caps table
	ui->tableWidgetDomain->verticalHeader()->setVisible(false);
	ui->tableWidgetDomain->setSelectionBehavior(QAbstractItemView::SelectRows);

	// centerline key point table
	ui->tableWidgetCenterlineKeyPoints->verticalHeader()->setVisible(false);
	ui->tableWidgetCenterlineKeyPoints->setSelectionBehavior(QAbstractItemView::SelectRows);

	// settings
	m_preferences = new Preferences();
	m_preferences->SetIO(m_io);

	// utils
	m_branchOperation = new BranchOperation();
	m_measurements = new Measurements();
	m_measurements->SetDataIo(m_io);
	m_measurements->SetPreference(m_preferences);
	m_measurements->SetCenterlinesInfoWidget(ui->centerlinesInfoWidget);

	// actors
	m_surfaceActor->SetMapper(m_surfaceMapper);
	m_surfaceActor->GetProperty()->SetColor(1, 1, 1);
	m_surfaceActor->GetProperty()->SetOpacity(ui->doubleSpinBoxOpacity->value());

	m_centerlineActor->SetMapper(m_centerlineMapper);
	m_centerlineActor->GetProperty()->SetColor(1, 1, 1);
	
	m_proximalNormalActor->SetMapper(m_proximalNormalMapper);
	m_distalNormalActor->SetMapper(m_distalNormalMapper);
	m_proximalNormalActor->GetProperty()->SetColor(1, 1, 0.5);
	m_distalNormalActor->GetProperty()->SetColor(1, 1, 0.5);

	m_reconSurfaceActor->SetMapper(m_reconSurfaceMapper);
	m_reconSurfaceActor->GetProperty()->SetColor(1, 1, 1);
	m_reconSurfaceActor->GetProperty()->SetOpacity(ui->doubleSpinBoxReconSurfaceOpacity->value());

	m_reconCenterlineActor->SetMapper(m_reconCenterlineMapper);
	m_reconCenterlineActor->GetProperty()->SetColor(1, 1, 1);

	// voronoi diagram
	vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	lut->SetNumberOfColors(256);
	lut->SetHueRange(0, 0.667);
	lut->Build();

	m_voronoiMapper->SetLookupTable(lut);
	m_voronoiMapper->ScalarVisibilityOn();

	m_vornoiActor->SetMapper(m_voronoiMapper);
	m_vornoiActor->GetProperty()->SetOpacity(this->ui->doubleSpinBoxVoronoiOpacity->value());
	
	// add actors to renderer
	m_renderer->AddActor(m_surfaceActor);
	m_renderer->AddActor(m_centerlineActor);
	m_renderer->AddActor(m_proximalNormalActor);
	m_renderer->AddActor(m_distalNormalActor);
	m_renderer->AddActor(m_vornoiActor);
	m_renderer->AddActor(m_reconSurfaceActor);
	m_renderer->AddActor(m_reconCenterlineActor);

	this->createFirstBifurationPoint();
	this->createCurrentPickingPoint();
	this->createClipper();
	this->createOutlineBoundingBox();

	// signal slot connections
	connect(ui->pushButtonSurface, &QPushButton::clicked, this, &MainWindow::slotBrowseSurface);
	connect(ui->pushButtonCenterline, &QPushButton::clicked, this, &MainWindow::slotBrowseCenterline);
	connect(ui->horizontalSliderOpacity, &QSlider::valueChanged, this, &MainWindow::slotSliderOpacityChanged);
	connect(ui->doubleSpinBoxOpacity, QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,&MainWindow::slotSpinBoxOpacityChanged);
	connect(ui->tableWidgetCenterline, &QTableWidget::currentCellChanged, this, &MainWindow::slotCurrentPickingPoint);
	connect(ui->pushButtonSetFirstBifucation, &QPushButton::clicked, this, &MainWindow::slotSetFirstBifurcation);
	connect(ui->pushButtonAutoLocateFirstBifurcation, &QPushButton::clicked, this, &MainWindow::slotAutoLocateFirstBifurcation);
	connect(ui->doubleSpinBoxProximalBound, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::slotBoundChanged);
	connect(ui->doubleSpinBoxDistalBound, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::slotBoundChanged);
	connect(ui->pushButtonSetClipper, &QPushButton::clicked, this, &MainWindow::slotSetClipper);
	connect(ui->horizontalSliderSize, &QSlider::valueChanged, this, &MainWindow::slotSliderSizeChanged);
	connect(ui->doubleSpinBoxSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::slotSpinBoxSizeChanged);
	connect(ui->horizontalSliderThickness, &QSlider::valueChanged, this, &MainWindow::slotSliderThicknessChanged);
	connect(ui->doubleSpinBoxThickness, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::slotSpinBoxThicknessChanged);
	connect(ui->horizontalSliderRotate, &QSlider::valueChanged, this, &MainWindow::slotSliderRotateChanged);
	connect(ui->doubleSpinBoxRotate, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::slotSpinBoxRotateChanged);
	connect(ui->pushButtonClipProximal, &QPushButton::clicked, this, &MainWindow::slotClipProximal);
	connect(ui->pushButtonClipDistal, &QPushButton::clicked, this, &MainWindow::slotClipDistal);
	connect(ui->pushButtonResetSurface, &QPushButton::clicked, this, &MainWindow::slotResetSurface);
	connect(ui->pushButtonResetCenterline, &QPushButton::clicked, this, &MainWindow::slotResetCenterline);
	connect(ui->pushButtonSaveSurface, &QPushButton::clicked, this, &MainWindow::slotSaveSurface);
	connect(ui->pushButtonCapping, &QPushButton::clicked, this, &MainWindow::slotSurfaceCapping);
	connect(ui->pushButtonDeleteCap, &QPushButton::clicked, this, &MainWindow::slotRemoveCap);
	connect(ui->comboBoxClipperStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::slotSetClipper);
	connect(ui->pushButtonDeleteAllCap, &QPushButton::clicked, this, &MainWindow::slotRemoveAllCaps);
	connect(ui->checkBoxWireframe, &QCheckBox::stateChanged, this, &MainWindow::slotDisplayWireframe);

	// centerline 
	connect(ui->pushButtonAddCenterlineKeyPoint, &QPushButton::clicked, this, &MainWindow::slotAddCenterlineKeyPoint);
	connect(ui->pushButtonSurfaceCappingCenterline, &QPushButton::clicked, this, &MainWindow::slotSurfaceCapping);
	connect(m_io, SIGNAL(centerlineKeyPointUpdated()), this, SLOT(slotCenterlineKeyPointUpdated()));
	connect(ui->pushButtonRemoveCenterlineKeyPoint, &QPushButton::clicked, this, &MainWindow::slotRemoveCenterlineKeyPoint);
	connect(ui->pushButtonRemoveAllCenterlineKeyPoint, &QPushButton::clicked, this, &MainWindow::slotRemoveAllCenterlineKeyPoint);
	connect(ui->pushButtonSaveCenterline_3, &QPushButton::clicked, this, &MainWindow::slotSaveCenterline);
	connect(ui->pushButtonComputeCenterline, &QPushButton::clicked, this, &MainWindow::slotComputeCenterline);
	connect(ui->tableWidgetCenterlineKeyPoints, &QTableWidget::currentCellChanged, this, &MainWindow::slotCurrentCenterlineKeyPoint);
	connect(m_preferences, SIGNAL(signalComboBoxesUpdated()), this, SLOT(slotCenterlineConfigUpdate()));
	connect(m_preferences, SIGNAL(signalComboBoxesUpdated()), m_measurements, SLOT(slotCenterlineConfigUpdate()));
	connect(ui->checkBoxCenterlineVisisble, &QCheckBox::stateChanged, this, &MainWindow::renderCenterline);

	// clipping
	connect(ui->pushButtonSaveCenterline, &QPushButton::clicked, this, &MainWindow::slotSaveCenterline);

	// voronoi
	connect(ui->pushButtonLoadVoronoi, &QPushButton::clicked, this, &MainWindow::slotBrowseVoronoi);
	connect(ui->pushButtonComputeVoronoi, &QPushButton::clicked, this, &MainWindow::slotComputeVoronoi);
	connect(ui->horizontalSliderVoronoiOpacity, &QSlider::valueChanged, this, &MainWindow::slotSliderVoronoiOpacityChanged);
	connect(ui->doubleSpinBoxVoronoiOpacity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::slotSpinBoxVoronoiOpacityChanged);
	connect(ui->pushButtonSaveVoronoi, &QPushButton::clicked, this, &MainWindow::slotSaveVoronoi);

	// recon
	connect(ui->horizontalSliderSmooth, &QSlider::valueChanged, this, &MainWindow::slotReconSmoothValueSliderChanged);
	connect(ui->doubleSpinBoxSmooth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::slotReconSmoothValueSpinBoxChanged);
	connect(ui->pushButtonAddAllRecon, &QPushButton::clicked, this, &MainWindow::slotReconAddAll);
	connect(ui->pushButtonAddRecon, &QPushButton::clicked, this, &MainWindow::slotReconAddCurrent);
	connect(ui->pushButtonRemoveAllRecon, &QPushButton::clicked, this, &MainWindow::slotReconRemoveAll);
	connect(ui->pushButtonRemoveRecon, &QPushButton::clicked, this, &MainWindow::slotReconRemoveCurrent);
	connect(ui->pushButtonReconClip, &QPushButton::clicked, this, &MainWindow::slotReconClip);
	connect(ui->pushButtonReconInterpolate, &QPushButton::clicked, this, &MainWindow::slotReconInterpolate);
	connect(ui->pushButtonResetRecon, &QPushButton::clicked, this, &MainWindow::slotResetRecon);
	connect(ui->pushButtonLoadReconCenterline, &QPushButton::clicked, this, &MainWindow::slotBrowseReconCenterline);
	connect(ui->pushButtonSaveReconCenterline, &QPushButton::clicked, this, &MainWindow::slotSaveReconCenterline);
	connect(ui->pushButtonSaveReconVoronoi, &QPushButton::clicked, this, &MainWindow::slotSaveVoronoi);
	connect(ui->pushButtonReconstruct, &QPushButton::clicked, this, &MainWindow::slotReconstruct);
	connect(ui->horizontalSliderReconSurfaceOpacity, &QSlider::valueChanged, this, &MainWindow::slotSliderReconSurfaceOpacityChanged);
	connect(ui->doubleSpinBoxReconSurfaceOpacity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::slotSpinBoxReconSurfaceOpacityChanged);
	connect(ui->pushButtonLoadReconSurface, &QPushButton::clicked, this, &MainWindow::slotBrowseReconSurface);
	connect(ui->pushButtonSaveReconSurface, &QPushButton::clicked, this, &MainWindow::slotSaveReconSurface);
	connect(ui->checkBoxReconCenterlineVisisble, &QCheckBox::stateChanged, this, &MainWindow::renderReconCenterline);

	// extend

	// domain
	connect(ui->tableWidgetDomain, &QTableWidget::itemChanged, this, &MainWindow::slotBoundaryCapTableItemChanged);
	connect(ui->tableWidgetDomain, &QTableWidget::currentCellChanged, this, &MainWindow::slotCurrentBoundaryCap);
	connect(ui->pushButtonSaveDomain, &QPushButton::clicked, this, &MainWindow::slotSaveDomain);

	// fiducial
	connect(ui->pushButtonAddFiducial, &QPushButton::clicked, this, &MainWindow::slotAddFiducial);
	connect(ui->pushButtonRemoveFiducial, &QPushButton::clicked, this, &MainWindow::slotRemoveFiducial);
	connect(ui->tableWidgetFiducial, &QTableWidget::currentCellChanged, this, &MainWindow::slotCurrentFiducial);
	connect(ui->pushButtonSaveDomain_3, &QPushButton::clicked, this, &MainWindow::slotSaveDomain);

	// actions
	connect(ui->actionBranch, &QAction::triggered, this, &MainWindow::slotActionBranch);
	connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::slotActionPreferences);
	connect(ui->pushButtonCenterlineConfigure, &QPushButton::clicked, this, &MainWindow::slotCenterlineConfigure);
	connect(ui->actionLoad_Surface, &QAction::triggered, this, &MainWindow::slotBrowseSurface);
	connect(ui->actionLoad_Centerline, &QAction::triggered, this, &MainWindow::slotBrowseCenterline);
	connect(ui->actionMeasurements, &QAction::triggered, this, &MainWindow::slotActionMeasurements);

	// set stenosis and normal points
	connect(ui->centerlinesInfoWidget, SIGNAL(signalSetStenosis()), this, SLOT(slotSetStenosisPoint()));
	connect(ui->centerlinesInfoWidget, SIGNAL(signalSetProximalNormal()), this, SLOT(slotSetProximalNormalPoint()));
	connect(ui->centerlinesInfoWidget, SIGNAL(signalSetDistalNormal()), this, SLOT(slotSetDistalNormalPoint()));
	connect(ui->centerlinesInfoWidget, SIGNAL(signalSetCenterlineIdsArray(QString)), this, SLOT(slotSetCenterlineIdsArray(QString)));
	connect(ui->centerlinesInfoWidget, SIGNAL(signalSetAbscissasArray(QString)), this, SLOT(slotSetAbscissasArray(QString)));

	// shortcut, remove for release
	//ui->lineEditSurface->setText("Z:/data/intracranial");
	//ui->lineEditCenterline->setText("Z:/data/intracranial");

	ui->lineEditSurface->setText("Z:/data/intracranial/data_ESASIS_followup/medical/001/baseline/");
	ui->lineEditCenterline->setText("Z:/data/intracranial/data_ESASIS_followup/medical/001/baseline/");
	ui->lineEditCenterline->setText("Z:/data/intracranial/data_ESASIS_followup/medical/001/baseline/CFD_OpenFOAM_result/centerlines");
	ui->lineEditVoronoi->setText("Z:/data/intracranial/data_ESASIS_followup/medical/001/baseline/recon_stenosis");
	ui->lineEditReconSurface->setText("Z:/data/intracranial/data_ESASIS_followup/medical/001/baseline/recon_stenosis");
	
	ui->lineEditSurface->setText("D:/Projects/Vessel-Clipper/Data");
	ui->lineEditCenterline->setText("D:/Projects/Vessel-Clipper/Data");
	ui->lineEditVoronoi->setText("D:/Projects/Vessel-Clipper/Data");
	ui->lineEditReconSurface->setText("D:/Projects/Vessel-Clipper/Data");
	ui->lineEditReconCenterline->setText("D:/Projects/Vessel-Clipper/Data");
};

MainWindow::~MainWindow()
{
	delete m_io;
}

void MainWindow::slotBrowseSurface()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open Surface File"), ui->lineEditSurface->text(), tr("Surface Files (*.stl *.vtk *.vtp)"));

	if (fileName.isNull())
		return;

	ui->lineEditSurface->setText(fileName);

	m_statusLabel->setText("Loading surface file...");
	m_statusProgressBar->setValue(51);

	this->enableUI(false);
	this->m_io->SetSurfacePath(fileName);

	// Instantiate the watcher to unlock
	m_ioWatcher = new QFutureWatcher<bool>;
	connect(m_ioWatcher, SIGNAL(finished()), this, SLOT(readSurfaceFileComplete()));

	// use QtConcurrent to run the read file on a new thread;
	QFuture<bool> future = QtConcurrent::run(this->m_io, &IO::ReadSurface);	
	m_ioWatcher->setFuture(future);
}

void MainWindow::slotBrowseCenterline()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open Centerline File"), ui->lineEditCenterline->text(), tr("Centerline Files (*.vtk *.vtp)"));

	if (fileName.isNull())
		return;

	ui->lineEditCenterline->setText(fileName);

	m_statusLabel->setText("Loading centerline file...");
	m_statusProgressBar->setValue(51);

	this->enableUI(false);
	this->m_io->SetCenterlinePath(fileName);

	// Instantiate the watcher to unlock
	m_ioWatcher = new QFutureWatcher<bool>;
	connect(m_ioWatcher, SIGNAL(finished()), this, SLOT(readCenterlineFileComplete()));

	// use QtConcurrent to run the read file on a new thread;
	QFuture<bool> future = QtConcurrent::run(this->m_io, &IO::ReadCenterline);
	m_ioWatcher->setFuture(future);
}

void MainWindow::slotSliderOpacityChanged()
{
	double opacity = ui->horizontalSliderOpacity->value()*1.0 / 100.;
	ui->doubleSpinBoxOpacity->setValue(opacity);
	m_surfaceActor->GetProperty()->SetOpacity(opacity);

	// boundary caps
	QList<vtkActor*>::iterator itr;
	for (itr = m_boundaryCapActors.begin(); itr < m_boundaryCapActors.end(); ++itr)
	{
		(*itr)->GetProperty()->SetOpacity(ui->doubleSpinBoxOpacity->value());
	}
	ui->qvtkWidget->update();
}

void MainWindow::slotSpinBoxOpacityChanged()
{
	ui->horizontalSliderOpacity->setValue(ui->doubleSpinBoxOpacity->value()* 100);
	m_surfaceActor->GetProperty()->SetOpacity(ui->doubleSpinBoxOpacity->value());

	// boundary caps
	QList<vtkActor*>::iterator itr;
	for (itr = m_boundaryCapActors.begin(); itr < m_boundaryCapActors.end(); ++itr)
	{
		(*itr)->GetProperty()->SetOpacity(ui->doubleSpinBoxOpacity->value());
	}

	ui->qvtkWidget->update();
}

void MainWindow::slotSliderVoronoiOpacityChanged()
{
	double opacity = ui->horizontalSliderVoronoiOpacity->value()*1.0 / 100.;
	ui->doubleSpinBoxVoronoiOpacity->setValue(opacity);
	m_vornoiActor->GetProperty()->SetOpacity(opacity);
	ui->qvtkWidget->update();
}

void MainWindow::slotSpinBoxVoronoiOpacityChanged()
{
	ui->horizontalSliderVoronoiOpacity->setValue(ui->doubleSpinBoxVoronoiOpacity->value() * 100);
	m_vornoiActor->GetProperty()->SetOpacity(ui->doubleSpinBoxVoronoiOpacity->value());
	ui->qvtkWidget->update();
}

void MainWindow::slotSaveVoronoi()
{
	if (m_io->GetVoronoiDiagram()->GetNumberOfPoints() == 0)
		return;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Voronoi Diagram File"), ui->lineEditVoronoi->text(), tr("Voronoi Diagram Files (*.vtk *.vtp)"));

	if (fileName.isNull())
		return;

	m_io->WriteVoronoi(fileName);
}

void MainWindow::slotCurrentPickingPoint()
{
	if (ui->tableWidgetCenterline->currentRow() < 0)
		return;

	m_currentPickingSphereSource->SetCenter(m_io->GetCenterline()->GetPoint(ui->tableWidgetCenterline->currentRow()));
	m_currentPickingActor->VisibilityOn();
	ui->qvtkWidget->update();

	// centerlines plot widget update
	ui->centerlinesInfoWidget->SetCursorPosition(
		m_io->GetCenterline()->GetPoint(ui->tableWidgetCenterline->currentRow())[0],
		m_io->GetCenterline()->GetPoint(ui->tableWidgetCenterline->currentRow())[1],
		m_io->GetCenterline()->GetPoint(ui->tableWidgetCenterline->currentRow())[2]
	);
}

void MainWindow::slotSetFirstBifurcation()
{
	int current_row = ui->tableWidgetCenterline->currentRow();
	if (current_row < 0)
		return;

	QVector<double> point(3);
	point[0] = m_io->GetCenterline()->GetPoint(current_row)[0];
	point[1] = m_io->GetCenterline()->GetPoint(current_row)[1];
	point[2] = m_io->GetCenterline()->GetPoint(current_row)[2];

	m_io->SetCenterlineFirstBifurcationPoint(point);
	
	this->updateCenterlineDataTable();
	ui->tableWidgetCenterline->selectRow(current_row);
	m_currentPickingActor->VisibilityOff();

	this->renderFirstBifurcationPoint();
}

void MainWindow::slotAutoLocateFirstBifurcation()
{
	if (m_io->GetCenterline()->GetNumberOfPoints() == 0)
		return;
	m_io->AutoLocateFirstBifurcationPoint();
	this->renderFirstBifurcationPoint();
	this->updateCenterlineDataTable();
}

void MainWindow::slotBoundChanged()
{
	this->updateCenterlineDataTable();
}

void MainWindow::slotSetClipper()
{
	int currentPickingId = ui->tableWidgetCenterline->currentRow();

	if (currentPickingId < 0)
	{
		m_clipperActor->VisibilityOff();
		ui->qvtkWidget->update();
		return;
	}

	m_clppingPointId = currentPickingId;

	// centerline tangent
	double* tangent = m_io->GetCenterline()->GetPointData()->GetArray("FrenetTangent")->GetTuple(currentPickingId);

	m_clipTransform->Identity();

	m_clipTransform->Translate(m_io->GetCenterline()->GetPoint(currentPickingId));
	m_clipTransform->RotateWXYZ(ui->doubleSpinBoxRotate->value(), tangent[0], tangent[1], tangent[2]);
	if (ui->comboBoxClipperStyle->currentText() == "Cylinder")
	{
		m_clipTransform->RotateX(90);
	}
	double w = atan(sqrt(pow(tangent[0], 2) + pow(tangent[1], 2)) / tangent[2]) * 180 / 3.14;
	m_clipTransform->RotateWXYZ(w, -tangent[1], tangent[0], 0);
	if (ui->comboBoxClipperStyle->currentText() == "Box")
		m_clipTransform->Scale(ui->doubleSpinBoxSize->value(), ui->doubleSpinBoxSize->value(), ui->doubleSpinBoxThickness->value());
	else if (ui->comboBoxClipperStyle->currentText() == "Cylinder")
		m_clipTransform->Scale(ui->doubleSpinBoxSize->value(), ui->doubleSpinBoxThickness->value(), ui->doubleSpinBoxSize->value());

	vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	transformFilter->SetTransform(m_clipTransform);

	if (ui->comboBoxClipperStyle->currentText() == "Box")
	{
		vtkSmartPointer<vtkCubeSource> clipBox = vtkSmartPointer<vtkCubeSource>::New();
		transformFilter->SetInputConnection(clipBox->GetOutputPort());
		transformFilter->Update();

		m_clipperMapper->SetInputConnection(transformFilter->GetOutputPort());
	}
	else if (ui->comboBoxClipperStyle->currentText() == "Cylinder")
	{
		vtkSmartPointer<vtkCylinderSource> clipCylinder = vtkSmartPointer<vtkCylinderSource>::New();
		clipCylinder->SetResolution(100);
		transformFilter->SetInputConnection(clipCylinder->GetOutputPort());
		transformFilter->Update();

		m_clipperMapper->SetInputConnection(transformFilter->GetOutputPort());
	}

	m_clipperActor->VisibilityOn();
	ui->qvtkWidget->update();
}

void MainWindow::slotSliderSizeChanged()
{
	ui->doubleSpinBoxSize->setValue(ui->horizontalSliderSize->value());
	this->slotSetClipper();
}

void MainWindow::slotSpinBoxSizeChanged()
{
	ui->horizontalSliderSize->setValue(ui->doubleSpinBoxSize->value());
	this->slotSetClipper();
}

void MainWindow::slotSliderThicknessChanged()
{
	ui->doubleSpinBoxThickness->setValue(ui->horizontalSliderThickness->value());
	this->slotSetClipper();
}

void MainWindow::slotSpinBoxThicknessChanged()
{
	ui->horizontalSliderThickness->setValue(ui->doubleSpinBoxThickness->value());
	this->slotSetClipper();
}

void MainWindow::slotSliderRotateChanged()
{
	ui->doubleSpinBoxRotate->setValue(ui->horizontalSliderRotate->value());
	this->slotSetClipper();
}

void MainWindow::slotSpinBoxRotateChanged()
{
	ui->horizontalSliderRotate->setValue(ui->doubleSpinBoxRotate->value());
	this->slotSetClipper();
}

void MainWindow::slotClipProximal()
{
	clip(1);
}

void MainWindow::slotClipDistal()
{
	clip(-1);
}

void MainWindow::slotResetSurface()
{
	m_io->GetSurface()->DeepCopy(m_io->GetOriginalSurface());
	ui->qvtkWidget->update();
}

void MainWindow::slotResetCenterline()
{
	m_io->SetCenterline(m_io->GetOriginalCenterline());
	this->updateCenterlinesInfoWidget();
	this->renderFirstBifurcationPoint();
	m_preferences->slotUpdateArrays();
	this->updateCenterlineDataTable();
	ui->checkBoxCenterlineVisisble->setChecked(true);
	ui->qvtkWidget->update();
}

void MainWindow::slotSaveSurface()
{
	if (m_io->GetSurface()->GetNumberOfPoints() == 0)
		return;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Surface File"), ui->lineEditSurface->text(), tr("Surface Files (*.stl *.vtk *.vtp)"));

	if (fileName.isNull())
		return;

	m_io->WriteSurface(fileName);
}

void MainWindow::slotSaveCenterline()
{
	if (m_io->GetCenterline()->GetNumberOfPoints() == 0)
		return;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Centerline File"), ui->lineEditCenterline->text(), tr("Centerline Files (*.vtk *.vtp)"));

	if (fileName.isNull())
		return;

	m_io->WriteCenterline(fileName);
}

void MainWindow::slotDisplayWireframe(int state)
{
	if (state == Qt::CheckState::Checked)
	{
		m_surfaceActor->GetProperty()->SetRepresentationToWireframe();

		QList<vtkActor*>::iterator itr;
		for (itr = m_boundaryCapActors.begin(); itr != m_boundaryCapActors.end(); ++itr)
		{
			(*itr)->GetProperty()->SetRepresentationToWireframe();
		}
	}
	else if (state == Qt::CheckState::Unchecked)
	{
		m_surfaceActor->GetProperty()->SetRepresentationToSurface();
		QList<vtkActor*>::iterator itr;
		for (itr = m_boundaryCapActors.begin(); itr != m_boundaryCapActors.end(); ++itr)
		{
			(*itr)->GetProperty()->SetRepresentationToSurface();
		}
	}

	ui->qvtkWidget->update();
}

void MainWindow::slotBrowseVoronoi()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open Voronoi Diagram"), ui->lineEditVoronoi->text(), tr("Voronoi Files (*.vtk *.vtp)"));

	if (fileName.isNull())
		return;

	ui->lineEditVoronoi->setText(fileName);

	m_statusLabel->setText("Loading Voronoi Diagram file...");
	m_statusProgressBar->setValue(51);

	this->enableUI(false);
	this->m_io->SetVoronoiPath(fileName);

	// Instantiate the watcher to unlock
	m_ioWatcher = new QFutureWatcher<bool>;
	connect(m_ioWatcher, SIGNAL(finished()), this, SLOT(readVoronoiFileComplete()));

	// use QtConcurrent to run the read file on a new thread;
	QFuture<bool> future = QtConcurrent::run(this->m_io, &IO::ReadVoronoi);
	m_ioWatcher->setFuture(future);
}

void MainWindow::slotComputeVoronoi()
{
	vtkPolyData* source = m_io->GetSurface();
	if (source->GetNumberOfPoints() == 0)
		return;

	std::cout << "start to compute Voronoi diagram..." << std::endl;

	// need to optimize for non ui-blocking

	m_statusLabel->setText("Start to compute Voronoi diagram...");
	m_statusProgressBar->setValue(0);

	vtkSmartPointer<vtkvmtkCapPolyData> capper = vtkSmartPointer<vtkvmtkCapPolyData>::New();
	capper->SetInputData(source);
	capper->Update();

	m_statusLabel->setText("Computing surface normal...");
	m_statusProgressBar->setValue(10);

	vtkSmartPointer<vtkPolyDataNormals> surfaceNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
	surfaceNormals->SetInputData(capper->GetOutput());
	surfaceNormals->SplittingOff();
	surfaceNormals->AutoOrientNormalsOn();
	surfaceNormals->SetFlipNormals(false);
	surfaceNormals->ComputePointNormalsOn();
	surfaceNormals->ConsistencyOn();
	surfaceNormals->Update();

	std::cout << "performing delaunay tessellation..." << std::endl;

	m_statusLabel->setText("Performing delaunay tessllation...");
	m_statusProgressBar->setValue(10);

	vtkSmartPointer<vtkUnstructuredGrid> delaunayTessellation = vtkSmartPointer<vtkUnstructuredGrid>::New();

	vtkSmartPointer<vtkDelaunay3D> delaunayTessellator = vtkSmartPointer<vtkDelaunay3D>::New();
	delaunayTessellator->CreateDefaultLocator();
	delaunayTessellator->SetInputConnection(surfaceNormals->GetOutputPort());
	delaunayTessellator->SetTolerance(0.001);
	delaunayTessellator->Update();
	delaunayTessellation->DeepCopy(delaunayTessellator->GetOutput());

	vtkDataArray* normalsArray = surfaceNormals->GetOutput()->GetPointData()->GetNormals();
	delaunayTessellation->GetPointData()->AddArray(normalsArray);

	std::cout << "extracting internal tetrahedra..." << std::endl;
	m_statusLabel->setText("Extracting internal tetrahedra...");
	m_statusProgressBar->setValue(30);

	vtkSmartPointer<vtkvmtkInternalTetrahedraExtractor> internalTetrahedraExtractor = vtkSmartPointer<vtkvmtkInternalTetrahedraExtractor>::New();
	internalTetrahedraExtractor->SetInputData(delaunayTessellation);
	internalTetrahedraExtractor->SetOutwardNormalsArrayName(normalsArray->GetName());
	internalTetrahedraExtractor->RemoveSubresolutionTetrahedraOn();
	internalTetrahedraExtractor->SetSubresolutionFactor(1e-2); //1.0
	internalTetrahedraExtractor->SetSurface(surfaceNormals->GetOutput());

	if (capper->GetCapCenterIds()->GetNumberOfIds() > 0)
	{
		internalTetrahedraExtractor->UseCapsOn();
		internalTetrahedraExtractor->SetCapCenterIds(capper->GetCapCenterIds());
		internalTetrahedraExtractor->Update();
	}

	delaunayTessellation->DeepCopy(internalTetrahedraExtractor->GetOutput());

	std::cout << "computing Voronoi diagram..." << std::endl;
	m_statusLabel->setText("Computing Vornoi diagram...");
	m_statusProgressBar->setValue(60);

	vtkSmartPointer<vtkvmtkVoronoiDiagram3D> voronoiDiagramFilter = vtkSmartPointer<vtkvmtkVoronoiDiagram3D>::New();
	voronoiDiagramFilter->SetInputData(delaunayTessellation);
	voronoiDiagramFilter->SetRadiusArrayName("Radius");
	voronoiDiagramFilter->Update();

	std::cout << "simplifying Voronoi diagram..." << std::endl;
	m_statusLabel->setText("Simplifying Vornoi diagram...");
	m_statusProgressBar->setValue(80);

	vtkSmartPointer<vtkvmtkSimplifyVoronoiDiagram> voronoiDiagramSimplifier = vtkSmartPointer<vtkvmtkSimplifyVoronoiDiagram>::New();
	voronoiDiagramSimplifier->SetInputData(voronoiDiagramFilter->GetOutput());
	voronoiDiagramSimplifier->SetUnremovablePointIds(voronoiDiagramFilter->GetPoleIds());
	voronoiDiagramSimplifier->Update();

	m_io->SetVornoiDiagram(voronoiDiagramSimplifier->GetOutput());

	std::cout << "Voronoi diagram complete" << std::endl;
	m_statusLabel->setText("Voronoi diagram complete");
	m_statusProgressBar->setValue(100);

	this->renderVoronoi();
}

void MainWindow::slotReconSmoothValueSliderChanged()
{
	ui->doubleSpinBoxSmooth->setValue(ui->horizontalSliderSmooth->value() / 100.0);
}

void MainWindow::slotReconSmoothValueSpinBoxChanged()
{
	ui->horizontalSliderSmooth->setValue(ui->doubleSpinBoxSmooth->value() * 100);
}

void MainWindow::slotReconAddAll()
{
	while (ui->listWidgetCenterlineIdsPending->count() > 0)
	{
		QListWidgetItem* currentItem = ui->listWidgetCenterlineIdsPending->item(0);
		ui->listWidgetCenterlineIdsRecon->addItem(currentItem->text());
		ui->listWidgetCenterlineIdsPending->takeItem(ui->listWidgetCenterlineIdsPending->row(currentItem));
	}
}

void MainWindow::slotReconAddCurrent()
{
	QListWidgetItem* currentItem = ui->listWidgetCenterlineIdsPending->currentItem();

	if (currentItem == nullptr)
		return;

	ui->listWidgetCenterlineIdsRecon->addItem(currentItem->text());
	ui->listWidgetCenterlineIdsPending->takeItem(ui->listWidgetCenterlineIdsPending->row(currentItem));
}

void MainWindow::slotReconRemoveAll()
{
	while (ui->listWidgetCenterlineIdsRecon->count() > 0)
	{
		QListWidgetItem* currentItem = ui->listWidgetCenterlineIdsRecon->item(0);
		ui->listWidgetCenterlineIdsPending->addItem(currentItem->text());
		ui->listWidgetCenterlineIdsRecon->takeItem(ui->listWidgetCenterlineIdsRecon->row(currentItem));
	}
}

void MainWindow::slotReconRemoveCurrent()
{
	QListWidgetItem* currentItem = ui->listWidgetCenterlineIdsRecon->currentItem();

	if (currentItem == nullptr)
		return;
	if (currentItem == nullptr)
		return;
	ui->listWidgetCenterlineIdsPending->addItem(currentItem->text());
	ui->listWidgetCenterlineIdsRecon->takeItem(ui->listWidgetCenterlineIdsRecon->row(currentItem));
}

void MainWindow::slotReconClip()
{
	vtkPolyData* centerline = m_io->GetCenterline();
	if (centerline == nullptr)
		return;

	vtkDataArray* centerlineIds = centerline->GetCellData()->GetArray(m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
	if (centerlineIds == nullptr)
	{
		m_statusLabel->setText("Invalid centerlineids array");
		m_statusProgressBar->setValue(100);
		return;
	}

	// clip centerline
	m_statusLabel->setText("Clipping centerline and Voronoi diagram");
	m_statusProgressBar->setValue(10);

	vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
	
	// threshold with abscissas
	vtkSmartPointer<vtkKdTreePointLocator> locator = vtkSmartPointer<vtkKdTreePointLocator>::New();
	locator->SetDataSet(m_io->GetCenterline());
	locator->BuildLocator();

	double proximalPt[3] = {
		ui->centerlinesInfoWidget->GetProximalNormalPoint()[0],
		ui->centerlinesInfoWidget->GetProximalNormalPoint()[1],
		ui->centerlinesInfoWidget->GetProximalNormalPoint()[2]
	};

	double distalPt[3] = {
		ui->centerlinesInfoWidget->GetDistalNormalPoint()[0],
		ui->centerlinesInfoWidget->GetDistalNormalPoint()[1],
		ui->centerlinesInfoWidget->GetDistalNormalPoint()[2]
	};

	int proximalId = locator->FindClosestPoint(proximalPt);
	int distalId = locator->FindClosestPoint(distalPt);

	vtkDataArray* abscissas = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetAbscissasArrayName().toStdString().c_str());

	if (abscissas == nullptr)
	{
		m_statusLabel->setText("Invalid abscissas array, fail to clip");
		m_statusProgressBar->setValue(100);
	}

	double threshold_bound[2] = {
		abscissas->GetTuple(proximalId)[0],
		abscissas->GetTuple(distalId)[0]
	};

	for (int i = centerlineIds->GetRange()[0]; i <= centerlineIds->GetRange()[1]; i++)
	{
		// threshold to get independent centerline
		vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
		threshold->ThresholdBetween(i, i);
		threshold->SetInputData(m_io->GetCenterline());
		threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
		threshold->Update();

		// convert threshold output to vtkpolydata
		vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter> ::New();
		geometryFilter->SetInputData(threshold->GetOutput());
		geometryFilter->Update();

		if (threshold->GetOutput()->GetNumberOfPoints() == 0)
			continue;

		vtkSmartPointer<vtkPolyData> singleCenterline = geometryFilter->GetOutput();
		singleCenterline->GetPointData()->SetActiveScalars(m_preferences->GetAbscissasArrayName().toStdString().c_str());

		// if the single centerline is not the one in recon centerline list, direct append to output
		bool recon = true;
		for (int j = 0; j < ui->listWidgetCenterlineIdsPending->count(); j++)
		{
			int centerlineId = ui->listWidgetCenterlineIdsPending->item(j)->text().toInt();
			if (centerlineId == i)
			{
				recon = false;
				break;
			}
		}

		if (recon)
		{
			// compute section to clip
			vtkSmartPointer<vtkClipPolyData> clipper1 = vtkSmartPointer<vtkClipPolyData>::New();
			clipper1->SetValue(threshold_bound[0]);
			clipper1->SetInsideOut(true);
			clipper1->GenerateClippedOutputOn();
			clipper1->SetInputData(singleCenterline);
			clipper1->Update();

			vtkSmartPointer<vtkClipPolyData> clipper2 = vtkSmartPointer<vtkClipPolyData>::New();
			clipper2->SetValue(threshold_bound[1]);
			clipper2->SetInsideOut(false);
			clipper2->GenerateClippedOutputOn();
			clipper2->SetInputData(singleCenterline);
			clipper2->Update();

			vtkSmartPointer<vtkAppendPolyData> appendFilter2 = vtkSmartPointer<vtkAppendPolyData>::New();
			appendFilter2->AddInputData(clipper1->GetOutput());
			appendFilter2->AddInputData(clipper2->GetOutput());
			appendFilter2->Update();

			appendFilter->AddInputData(appendFilter2->GetOutput());
		}
		else
		{
			appendFilter->AddInputData(singleCenterline);
		}
	}

	appendFilter->Update();
	m_io->SetReconstructedCenterline(appendFilter->GetOutput());

	this->ui->checkBoxCenterlineVisisble->setChecked(false);
	this->renderReconCenterline();

	m_statusLabel->setText("Clipping centerline complete");
	m_statusProgressBar->setValue(50);

	// ======================== clip voronoi diagram  ========================
	vtkPolyData* voronoiDiagram = m_io->GetVoronoiDiagram();
	vtkDataArray* frenetTangent = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str());
	vtkDataArray* radius = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str());

	if (voronoiDiagram->GetNumberOfPoints() == 0)
	{
		m_statusLabel->setText("Voronoi diagram not found");
		m_statusProgressBar->setValue(100);

		//this->renderCenterline();
		//this->updateCenterlineDataTable();
		//this->updateCenterlinesInfoWidget();
		return;
	}

	if (frenetTangent == nullptr || radius == nullptr)
	{
		m_statusLabel->setText("Invalid Frenet tanget/ radius array");
		m_statusProgressBar->setValue(100);

		//this->renderCenterline();
		//this->updateCenterlineDataTable();
		//this->updateCenterlinesInfoWidget();
		return;
	}

	if (frenetTangent->GetNumberOfComponents() != 3 || radius->GetNumberOfComponents() != 1)
	{
		m_statusLabel->setText("Invalid Frenet tanget/ radius array");
		m_statusProgressBar->setValue(100);

		//this->renderCenterline();
		//this->updateCenterlineDataTable();
		//this->updateCenterlinesInfoWidget();
		return;
	}

	// get the new centerlineids 
	centerlineIds = centerline->GetCellData()->GetArray(m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
	if (centerlineIds == nullptr)
	{
		m_statusLabel->setText("Invalid centerlineids array");
		m_statusProgressBar->setValue(100);
		
		//this->renderCenterline();
		//this->updateCenterlineDataTable();
		//this->updateCenterlinesInfoWidget();
		return;
	}

	// create implict function with spheres along clipped centerline
	vtkSmartPointer<vtkvmtkPolyBallLine> tubeFunction = vtkSmartPointer<vtkvmtkPolyBallLine>::New();
	tubeFunction->SetInput(m_io->GetReconstructedCenterline());
	tubeFunction->SetPolyBallRadiusArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());

	vtkNew<vtkImplicitBoolean> endSpheresFunction;
	endSpheresFunction->SetOperationTypeToUnion();

	for (int i = centerlineIds->GetRange()[0]; i <= centerlineIds->GetRange()[1]; i++)
	{
		// threshold to get independent centerline
		vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
		threshold->ThresholdBetween(i, i);
		threshold->SetInputData(m_io->GetReconstructedCenterline());
		threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
		threshold->Update();

		if (threshold->GetOutput()->GetNumberOfPoints() == 0)
			continue;

		//singleCenterline->GetPointData()->SetActiveScalars(m_preferences->GetAbscissasArrayName().toStdString().c_str());

		// if the single centerline is not the one in recon centerline list, direct append to output
		bool recon = true;
		for (int j = 0; j < ui->listWidgetCenterlineIdsPending->count(); j++)
		{
			int centerlineId = ui->listWidgetCenterlineIdsPending->item(j)->text().toInt();
			if (centerlineId == i)
			{
				recon = false;
				break;
			}
		}

		if (recon)
		{
			// threshold for isolated centerlines
			vtkSmartPointer<vtkConnectivityFilter> connectFilter = vtkSmartPointer<vtkConnectivityFilter>::New();
			connectFilter->SetExtractionModeToAllRegions();
			connectFilter->SetInputData(threshold->GetOutput());
			connectFilter->ColorRegionsOn(); // to generate RegionId array
			connectFilter->Update();

			for (int j = 0; j < connectFilter->GetNumberOfExtractedRegions(); j++)
			{
				// threshold to get independent centerline
				vtkSmartPointer<vtkThreshold> threshold2 = vtkSmartPointer<vtkThreshold>::New();
				threshold2->ThresholdBetween(j, j);
				threshold2->SetInputData(connectFilter->GetOutput());
				threshold2->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "RegionId");
				threshold2->Update();

				// convert threshold output to vtkpolydata
				vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter> ::New();
				geometryFilter->SetInputData(threshold2->GetOutput());
				geometryFilter->Update();

				vtkSmartPointer<vtkPolyData> singleCenterline = geometryFilter->GetOutput();

				// get the end points
				double* center0 = threshold2->GetOutput()->GetPoint(0);
				double tangent0[3];
				tangent0[0] = threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(0, 0);
				tangent0[1] = threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(0, 1);
				tangent0[2] = threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(0, 2);
				double radius0 = threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetComponent(0, 0);

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

				double* center1 = threshold2->GetOutput()->GetPoint(threshold2->GetOutput()->GetNumberOfPoints() - 1);
				double tangent1[3];
				// reverse tangent direction
				tangent1[0] = -1.0*threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(threshold2->GetOutput()->GetNumberOfPoints() - 1, 0);
				tangent1[1] = -1.0*threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(threshold2->GetOutput()->GetNumberOfPoints() - 1, 1);
				tangent1[2] = -1.0*threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(threshold2->GetOutput()->GetNumberOfPoints() - 1, 2);
				double radius1 = threshold2->GetOutput()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetComponent(threshold2->GetOutput()->GetNumberOfPoints() - 1, 0);

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
		}
	}

	vtkNew<vtkImplicitBoolean> compositeFunction;
	compositeFunction->AddFunction(tubeFunction);
	compositeFunction->AddFunction(endSpheresFunction);
	compositeFunction->SetOperationTypeToDifference();

	// create mask array with spheres and centerline tubes
	vtkSmartPointer<vtkDoubleArray> maskArray = vtkSmartPointer<vtkDoubleArray>::New();
	maskArray->SetNumberOfComponents(1);
	maskArray->SetNumberOfTuples(m_io->GetVoronoiDiagram()->GetNumberOfPoints());
	maskArray->SetName("Mask");
	maskArray->FillComponent(0, 0);

	std::cout << "evaluating voronoi diagram..." << std::endl;
	m_statusLabel->setText("Evaluating voronoi diagram");
	m_statusProgressBar->setValue(75);
	
	compositeFunction->EvaluateFunction(m_io->GetVoronoiDiagram()->GetPoints()->GetData(), maskArray);

	m_io->GetVoronoiDiagram()->GetPointData()->AddArray(maskArray);
	m_io->GetVoronoiDiagram()->GetPointData()->SetActiveScalars("Mask");

	vtkSmartPointer<vtkClipPolyData> clipperV = vtkSmartPointer<vtkClipPolyData>::New();
	clipperV->SetValue(0);
	clipperV->SetInsideOut(true);
	clipperV->GenerateClippedOutputOn();
	clipperV->SetInputData(m_io->GetVoronoiDiagram());
	clipperV->Update();

	vtkSmartPointer<vtkCleanPolyData> cleanerV = vtkSmartPointer<vtkCleanPolyData>::New();
	cleanerV->SetInputData(clipperV->GetOutput());
	cleanerV->Update();

	//vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
	//geomFilter->SetInputData(clipperV->GetOutput());
	//geomFilter->Update();

	m_io->SetVornoiDiagram(cleanerV->GetOutput());

	this->renderVoronoi();
	//this->renderReconCenterline();
	//this->updateCenterlineDataTable();
	//this->updateCenterlinesInfoWidget();

	m_statusLabel->setText("Clip Voronoi diagram complete");
	m_statusProgressBar->setValue(100);
}

void MainWindow::slotReconInterpolate()
{
	vtkDataArray* centerlineIds = m_io->GetCenterline()->GetCellData()->GetArray(m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
	if (centerlineIds == nullptr)
		return;

	// clip centerline
	m_statusLabel->setText("Interpolating centerline");
	m_statusProgressBar->setValue(10);

	vtkSmartPointer<vtkAppendPolyData> centerlineAppendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
	vtkSmartPointer<vtkAppendPolyData> voronoiAppendFilter = vtkSmartPointer<vtkAppendPolyData>::New();

	int count = 0;

	double maxVoronoiShrinkFactor = 0.3;

	for (int i = centerlineIds->GetRange()[0]; i <= centerlineIds->GetRange()[1]; i++)
	{
		// threshold to get independent lines
		vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
		threshold->ThresholdBetween(i, i);
		threshold->SetInputData(m_io->GetCenterline());
		threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
		threshold->Update();

		// convert threshold output to vtkpolydata
		vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter> ::New();
		geometryFilter->SetInputData(threshold->GetOutput());
		geometryFilter->Update();

		// perform connectivity to get proximal and distal centerlines
		vtkSmartPointer<vtkConnectivityFilter> connectFilter = vtkSmartPointer<vtkConnectivityFilter>::New();
		connectFilter->SetInputData(geometryFilter->GetOutput());
		connectFilter->SetExtractionModeToAllRegions();
		connectFilter->SetColorRegions(true);
		connectFilter->Update();

		// interpolation with spline
		vtkNew<vtkParametricSpline> spline;
		spline->SetPoints(geometryFilter->GetOutput()->GetPoints());

		vtkNew<vtkParametricFunctionSource> functionSource;
		functionSource->SetParametricFunction(spline);
		functionSource->Update();

		vtkSmartPointer<vtkSplineFilter> splineFilter = vtkSmartPointer<vtkSplineFilter>::New();
		splineFilter->SetInputData(functionSource->GetOutput());
		splineFilter->SetSubdivideToLength();
		splineFilter->SetLength(0.5);
		splineFilter->Update();

		// recompute centerline attributes
		vtkSmartPointer<vtkvmtkCenterlineAttributesFilter> attributeFilter = vtkSmartPointer<vtkvmtkCenterlineAttributesFilter>::New();
		attributeFilter->SetInputData(splineFilter->GetOutput());
		attributeFilter->SetAbscissasArrayName("Abscissas");
		attributeFilter->SetParallelTransportNormalsArrayName("ParallelTransportNormals");
		attributeFilter->Update();

		vtkSmartPointer<vtkvmtkCenterlineGeometry> centerlineGeometryFilter = vtkSmartPointer<vtkvmtkCenterlineGeometry>::New();
		centerlineGeometryFilter->SetInputData(attributeFilter->GetOutput());
		centerlineGeometryFilter->SetFrenetBinormalArrayName("FrenetBinormal");
		centerlineGeometryFilter->SetFrenetNormalArrayName("FrenetNormal");
		centerlineGeometryFilter->SetFrenetTangentArrayName("FrenetTangent");
		centerlineGeometryFilter->SetLengthArrayName("Length");
		centerlineGeometryFilter->SetTorsionArrayName("Torsion");
		centerlineGeometryFilter->SetTortuosityArrayName("Tortuosity");
		centerlineGeometryFilter->SetCurvatureArrayName("Curvature");
		centerlineGeometryFilter->SetLineSmoothing(0);
		centerlineGeometryFilter->SetNumberOfSmoothingIterations(100);
		centerlineGeometryFilter->SetSmoothingFactor(0.1);
		centerlineGeometryFilter->SetGlobalWarningDisplay(1);
		centerlineGeometryFilter->Update();

		vtkNew<vtkPolyData> interpolatedLine;
		interpolatedLine->DeepCopy(centerlineGeometryFilter->GetOutput());

		// centerline id
		vtkSmartPointer<vtkIntArray> centerlindIdsArray = vtkSmartPointer<vtkIntArray>::New();
		centerlindIdsArray->SetNumberOfComponents(1);
		centerlindIdsArray->SetNumberOfTuples(interpolatedLine->GetNumberOfCells());
		centerlindIdsArray->SetName("CenterlineIds");
		centerlindIdsArray->FillComponent(0, i);

		interpolatedLine->GetCellData()->AddArray(centerlindIdsArray);

		centerlineAppendFilter->AddInputData(interpolatedLine);

		// the clipped centerline suppose to have only two lines for each centerlineid
		if (connectFilter->GetNumberOfExtractedRegions() != 2)
			continue;

		// get the start and end points
		vtkSmartPointer<vtkThreshold> threshold2 = vtkSmartPointer<vtkThreshold>::New();
		vtkSmartPointer<vtkGeometryFilter> geometryFilter2 = vtkSmartPointer<vtkGeometryFilter>::New();

		threshold2->SetInputData(connectFilter->GetOutput());
		threshold2->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, "RegionId");
			
		// start point
		threshold2->ThresholdBetween(0, 0);
		threshold2->Update();
		geometryFilter2->SetInputData(threshold2->GetOutput());
		geometryFilter2->Update();

		double startPoint[3];
		for (int i = 0; i < 3; i++)
		{
			startPoint[i] = geometryFilter2->GetOutput()->GetPoint(geometryFilter2->GetOutput()->GetNumberOfPoints() - 1)[i];
		}

		// prepare the proximal side dataset
		vtkSmartPointer<vtkPolyData> proximalPoints = vtkSmartPointer<vtkPolyData>::New();
		//std::cout << "clipping proximal points" << std::endl;
		ExtractCylindericInterpolationVoronoiDiagram(geometryFilter2->GetOutput(), proximalPoints, -1, false, 1.5);
		
		if (proximalPoints->GetNumberOfPoints() > 1)
			voronoiAppendFilter->AddInputData(proximalPoints); 

		// end point
		threshold2->ThresholdBetween(1, 1);
		threshold2->Update();
		geometryFilter2->SetInputData(threshold2->GetOutput());
		geometryFilter2->Update();

		double endPoint[3];
		for (int i = 0; i < 3; i++)
		{
			endPoint[i] = geometryFilter2->GetOutput()->GetPoint(0)[i];
		}

		// prepare the distal side dataset
		vtkSmartPointer<vtkPolyData> distalPoints = vtkSmartPointer<vtkPolyData>::New();
		//std::cout << "clipping distal points" << std::endl;
		ExtractCylindericInterpolationVoronoiDiagram(geometryFilter2->GetOutput(), distalPoints, 0, true, 1.5);
		if (distalPoints->GetNumberOfPoints()>1)
			voronoiAppendFilter->AddInputData(distalPoints);

		// locate the start and end point id from the interpolated centerline
		vtkSmartPointer<vtkKdTreePointLocator> locator = vtkSmartPointer<vtkKdTreePointLocator>::New();
		locator->SetDataSet(interpolatedLine);
		locator->BuildLocator();
		int startId = locator->FindClosestPoint(startPoint);
		int endId = locator->FindClosestPoint(endPoint);

		//std::cout << "interpolate centerlineid " << i << " complete" << std::endl;
		//interpolatedLine->GetPointData()->Print(std::cout);

		double startAbsci = interpolatedLine->GetPointData()->GetArray("Abscissas")->GetTuple1(startId);
		double endAbsci = interpolatedLine->GetPointData()->GetArray("Abscissas")->GetTuple1(endId);

		//std::cout << "startId: " << startId << std::endl;
		//std::cout << "endId: " << endId << std::endl;
		//std::cout << "start abscissas: " << startAbsci << std::endl;
		//std::cout << "end abscissas: " << endAbsci << std::endl;

		vtkSmartPointer<vtkCenterOfMass> comProximal = vtkSmartPointer<vtkCenterOfMass>::New();
		comProximal->SetInputData(proximalPoints);
		comProximal->SetUseScalarsAsWeights(false);
		comProximal->Update();

		vtkSmartPointer<vtkCenterOfMass> comDistal = vtkSmartPointer<vtkCenterOfMass>::New();
		comDistal->SetInputData(distalPoints);
		comDistal->SetUseScalarsAsWeights(false);
		comDistal->Update();

		// transform the proximal points along interpolated centerline
		for (int j = startId; j <= endId; j++)
		{
			//std::cout << "=================== pointId: " << j << " ==================="<< std::endl;
			double distFactor = 1.0 - ((j- startId)*1.0) / ((endId - startId)*1.0) * (1-maxVoronoiShrinkFactor);
			//std::cout << "distance factor: " << distFactor << std::endl;

			double point0[3] = { comProximal->GetCenter()[0] ,comProximal->GetCenter()[1] ,comProximal->GetCenter()[2] };
			interpolatedLine->GetPoint(j,point0);
			//std::cout << "point0: " << point0[0] << ", " << point0[1] << ", " << point0[2] << std::endl;

			double point1[3];
			interpolatedLine->GetPoint(j ,point1);
			//std::cout << "point1: " << point1[0] << ", " << point1[1] << ", " << point1[2] << std::endl;

			double tranlateVector[3] = {
				point1[0] - comProximal->GetCenter()[0],
				point1[1] - comProximal->GetCenter()[1],
				point1[2] - comProximal->GetCenter()[2],
			};

			//std::cout << "translation vector: " << tranlateVector[0] << ", " << tranlateVector[1] << ", " << tranlateVector[2] << std::endl;

			double tangent0[3];
			interpolatedLine->GetPointData()->GetArray("FrenetTangent")->GetTuple(startId,tangent0);
			double tangent1[3];
			interpolatedLine->GetPointData()->GetArray("FrenetTangent")->GetTuple(j, tangent1);

			double ptn0[3];
			interpolatedLine->GetPointData()->GetArray("ParallelTransportNormals")->GetTuple(startId, ptn0);
			double ptn1[3];
			interpolatedLine->GetPointData()->GetArray("ParallelTransportNormals")->GetTuple(j, ptn1);

			// rotate to align tangential direction
			double rotateAngleTangent = vtkMath::AngleBetweenVectors(tangent0,tangent1);
			double rodriguesVectorTangent[3];
			vtkMath::Cross(tangent0, tangent1, rodriguesVectorTangent);
			//std::cout << "tangent0: " << tangent0[0] << ", " << tangent0[1] << ", " << tangent0[2] << std::endl;
			//std::cout << "tangent1: " << tangent1[0] << ", " << tangent1[1] << ", " << tangent1[2] << std::endl;

			//std::cout << "rotate angle to align tangential direction: " << rotateAngleTangent*180. / vtkMath::Pi() << std::endl;
			//std::cout << "Rodrigues vector for tengential alginment: " << rodriguesVectorTangent[0] << ", " << rodriguesVectorTangent[1] << ", " << rodriguesVectorTangent[2] << std::endl;

			// rotate about tangent
			double ptn0TangentialAligned[3];
			double q[4] = { rotateAngleTangent ,rodriguesVectorTangent[0],rodriguesVectorTangent[1] ,rodriguesVectorTangent[2] };
			vtkMath::RotateVectorByWXYZ(ptn0, q, ptn0TangentialAligned);
			//std::cout << "Rotated parallel transport normal: " << ptn0TangentialAligned[0] << ", " << ptn0TangentialAligned[1] << ", " << ptn0TangentialAligned[2] << std::endl;
			//std::cout << "Target parallel transport normal: " << ptn1[0] << ", " << ptn1[1] << ", " << ptn1[2] << std::endl;
			double rotateAnglePTN = vtkMath::AngleBetweenVectors(ptn0TangentialAligned, ptn1);
			//std::cout << "rotate angle to align PTN direction: " << rotateAnglePTN*180. / vtkMath::Pi() << std::endl;

			vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
			transform->PostMultiply();
			transform->Translate(-comProximal->GetCenter()[0], -comProximal->GetCenter()[1], -comProximal->GetCenter()[2]);
			transform->Scale(distFactor, distFactor, distFactor);
			transform->RotateWXYZ(rotateAngleTangent*180./vtkMath::Pi(), rodriguesVectorTangent);
			transform->RotateWXYZ(rotateAnglePTN*180./vtkMath::Pi(), tangent1);
			transform->Translate(comProximal->GetCenter()[0], comProximal->GetCenter()[1], comProximal->GetCenter()[2]);
			transform->Translate(tranlateVector[0], tranlateVector[1], tranlateVector[2]);
			transform->Update();

			vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			transformFilter->SetInputData(proximalPoints);
			transformFilter->SetTransform(transform);
			transformFilter->Update();

			voronoiAppendFilter->AddInputData(transformFilter->GetOutput());
		}

		// transform the distal points along interpolated centerline
		for (int j = endId; j >= startId; j--)
		{
			//std::cout << "=================== pointId: " << j << " ===================" << std::endl;
			double distFactor = ((j - startId)*1.0) / ((endId - startId)*1.0)* (1 - maxVoronoiShrinkFactor) + maxVoronoiShrinkFactor;
			//std::cout << "distance factor: " << distFactor << std::endl;

			double point0[3] = { comDistal->GetCenter()[0] ,comDistal->GetCenter()[1] ,comDistal->GetCenter()[2] };
			//std::cout << "point0: " << point0[0] << ", " << point0[1] << ", " << point0[2] << std::endl;

			double point1[3];
			interpolatedLine->GetPoint(j, point1);
			//std::cout << "point1: " << point1[0] << ", " << point1[1] << ", " << point1[2] << std::endl;

			double tranlateVector[3] = {
				point1[0] - comDistal->GetCenter()[0],
				point1[1] - comDistal->GetCenter()[1],
				point1[2] - comDistal->GetCenter()[2],
			};

			//std::cout << "translation vector: " << tranlateVector[0] << ", " << tranlateVector[1] << ", " << tranlateVector[2] << std::endl;

			double tangent0[3];
			interpolatedLine->GetPointData()->GetArray("FrenetTangent")->GetTuple(endId, tangent0);
			double tangent1[3];
			interpolatedLine->GetPointData()->GetArray("FrenetTangent")->GetTuple(j, tangent1);

			double ptn0[3];
			interpolatedLine->GetPointData()->GetArray("ParallelTransportNormals")->GetTuple(endId, ptn0);
			double ptn1[3];
			interpolatedLine->GetPointData()->GetArray("ParallelTransportNormals")->GetTuple(j, ptn1);

			// rotate to align tangential direction
			double rotateAngleTangent = vtkMath::AngleBetweenVectors(tangent0, tangent1);
			double rodriguesVectorTangent[3];
			vtkMath::Cross(tangent0, tangent1, rodriguesVectorTangent);
			//std::cout << "tangent0: " << tangent0[0] << ", " << tangent0[1] << ", " << tangent0[2] << std::endl;
			//std::cout << "tangent1: " << tangent1[0] << ", " << tangent1[1] << ", " << tangent1[2] << std::endl;

			//std::cout << "rotate angle to align tangential direction: " << rotateAngleTangent*180. / vtkMath::Pi() << std::endl;
			//std::cout << "Rodrigues vector for tengential alginment: " << rodriguesVectorTangent[0] << ", " << rodriguesVectorTangent[1] << ", " << rodriguesVectorTangent[2] << std::endl;

			// rotate about tangent
			double ptn0TangentialAligned[3];
			double q[4] = { rotateAngleTangent ,rodriguesVectorTangent[0],rodriguesVectorTangent[1] ,rodriguesVectorTangent[2] };
			vtkMath::RotateVectorByWXYZ(ptn0, q, ptn0TangentialAligned);
			//std::cout << "Rotated parallel transport normal: " << ptn0TangentialAligned[0] << ", " << ptn0TangentialAligned[1] << ", " << ptn0TangentialAligned[2] << std::endl;
			//std::cout << "Target parallel transport normal: " << ptn1[0] << ", " << ptn1[1] << ", " << ptn1[2] << std::endl;
			double rotateAnglePTN = vtkMath::AngleBetweenVectors(ptn0TangentialAligned, ptn1);
			//std::cout << "rotate angle to align PTN direction: " << rotateAnglePTN*180. / vtkMath::Pi() << std::endl;

			vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
			transform->PostMultiply();
			transform->Translate(-comDistal->GetCenter()[0], -comDistal->GetCenter()[1], -comDistal->GetCenter()[2]);
			transform->Scale(distFactor, distFactor, distFactor);
			transform->RotateWXYZ(rotateAngleTangent*180. / vtkMath::Pi(), rodriguesVectorTangent);
			transform->RotateWXYZ(rotateAnglePTN*180. / vtkMath::Pi(), tangent1);
			transform->Translate(comDistal->GetCenter()[0], comDistal->GetCenter()[1], comDistal->GetCenter()[2]);
			transform->Translate(tranlateVector[0], tranlateVector[1], tranlateVector[2]);
			transform->Update();

			vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			transformFilter->SetInputData(distalPoints);
			transformFilter->SetTransform(transform);
			transformFilter->Update();

			voronoiAppendFilter->AddInputData(transformFilter->GetOutput());
		}

		// update progress bar
		m_statusProgressBar->setValue(10 + i*count/(centerlineIds->GetRange()[1] - centerlineIds->GetRange()[0])*90.0);
		count++;
	}

	centerlineAppendFilter->Update();
	voronoiAppendFilter->AddInputData(m_io->GetVoronoiDiagram());
	voronoiAppendFilter->Update();

	std::cout << "interpolate voronoi diagram complete" << std::endl;

	// get radius from interpolated voronoi diagram to centerline
	vtkSmartPointer<vtkPointInterpolator> interpolator = vtkSmartPointer<vtkPointInterpolator>::New();
	interpolator->SetInputData(centerlineAppendFilter->GetOutput());
	interpolator->SetSourceData(voronoiAppendFilter->GetOutput());
	interpolator->SetKernel(vtkSmartPointer<vtkVoronoiKernel>::New());
	interpolator->Update();

	m_io->SetVornoiDiagram(voronoiAppendFilter->GetOutput());
	m_io->GetCenterline()->DeepCopy(interpolator->GetOutput());

	this->renderCenterline();
	this->renderVoronoi();
	this->updateCenterlineDataTable();
	this->updateCenterlinesInfoWidget();
	m_preferences->slotUpdateArrays();

	m_statusLabel->setText("Interpolation complete");
	m_statusProgressBar->setValue(100);
}

void MainWindow::slotResetRecon()
{
	ui->checkBoxReconCenterlineVisisble->setChecked(false);
	this->slotResetCenterline();
	m_io->SetVornoiDiagram(m_io->GetOriginalVoronoiDiagram());
	this->renderVoronoi();
}

void MainWindow::slotReconstruct()
{
	vtkPolyData* voronoi = m_io->GetVoronoiDiagram();
	vtkPolyData* centerline = m_io->GetCenterline();
	if (voronoi->GetNumberOfPoints() == 0 || centerline->GetNumberOfPoints() == 0)
		return;

	// perform smooth
	// create implict function with spheres along clipped centerline
	std::cout << "Computing smoothed radius"<< std::endl;
	vtkSmartPointer<vtkArrayCalculator> cal = vtkSmartPointer<vtkArrayCalculator>::New();
	cal->SetInputData(m_io->GetCenterline());
	cal->AddScalarArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());
	char buffer[999];
	sprintf(buffer, "%s * %f", m_preferences->GetRadiusArrayName().toStdString(), 1. - ui->doubleSpinBoxSmooth->value());
	cal->SetFunction(buffer);
	cal->SetResultArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());
	cal->Update();

	vtkSmartPointer<vtkvmtkPolyBallLine> tubeFunction = vtkSmartPointer<vtkvmtkPolyBallLine>::New();
	tubeFunction->SetInput((vtkPolyData*)cal->GetOutput());
	tubeFunction->SetPolyBallRadiusArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());
	
	vtkSmartPointer<vtkDoubleArray> maskArray = vtkSmartPointer<vtkDoubleArray>::New();
	maskArray->SetNumberOfComponents(1);
	maskArray->SetNumberOfTuples(m_io->GetVoronoiDiagram()->GetNumberOfPoints());
	maskArray->SetName("Mask");
	maskArray->FillComponent(0, 0);
	vtkSmartPointer<vtkImplicitBoolean> compositeFunction = vtkSmartPointer<vtkImplicitBoolean>::New();
	compositeFunction->AddFunction(tubeFunction);
	std::cout << "Evaluating Voronoi diagram to smooth output" << std::endl;
	compositeFunction->EvaluateFunction(m_io->GetVoronoiDiagram()->GetPoints()->GetData(), maskArray);

	m_io->GetVoronoiDiagram()->GetPointData()->AddArray(maskArray);
	m_io->GetVoronoiDiagram()->GetPointData()->SetActiveScalars("Mask");

	vtkSmartPointer<vtkClipPolyData> clipperSmooth = vtkSmartPointer<vtkClipPolyData>::New();
	clipperSmooth->SetValue(0);
	clipperSmooth->SetInsideOut(true);
	clipperSmooth->SetInputData(m_io->GetVoronoiDiagram());
	clipperSmooth->Update();

	m_io->SetVornoiDiagram(clipperSmooth->GetOutput());
	this->renderVoronoi();

	std::cout << "Smooth Voronoi diagram complete" << std::endl;

	// Reconstructing Surface from Voronoi Diagram
	std::cout << "Polyball modeling..." << std::endl;
	std::cout << "Radius array name: " << m_preferences->GetRadiusArrayName().toStdString() << std::endl;
	vtkSmartPointer<vtkvmtkPolyBallModeller> modeller = vtkSmartPointer<vtkvmtkPolyBallModeller>::New();
	modeller->SetInputData(clipperSmooth->GetOutput());
	modeller->SetRadiusArrayName(m_preferences->GetRadiusArrayName().toStdString().c_str());
	//modeller->SetRadiusArrayName("Radius");
	modeller->UsePolyBallLineOff();
	int polyBallImageSize[3] = { 90,90,90 };
	modeller->SetSampleDimensions(polyBallImageSize);
	modeller->Update();

	vtkNew<vtkImageData> polyballImage;
	polyballImage->DeepCopy(modeller->GetOutput());

	std::cout << "Performing marching cube..." << std::endl;

	vtkSmartPointer<vtkMarchingCubes> marchingCube = vtkSmartPointer<vtkMarchingCubes>::New();
	marchingCube->SetInputData(polyballImage);
	marchingCube->SetValue(0, 0.0);
	marchingCube->Update();

	std::cout << "Reconstruct surface complete" << std::endl;

	m_io->SetReconstructedSurface(marchingCube->GetOutput());
	this->renderReconSurface();
}

void MainWindow::slotSliderReconSurfaceOpacityChanged()
{
	double opacity = ui->horizontalSliderReconSurfaceOpacity->value()*1.0 / 100.;
	ui->doubleSpinBoxReconSurfaceOpacity->setValue(opacity);
	m_reconSurfaceActor->GetProperty()->SetOpacity(opacity);

	ui->qvtkWidget->update();
}

void MainWindow::slotSpinBoxReconSurfaceOpacityChanged()
{
	ui->horizontalSliderReconSurfaceOpacity->setValue(ui->doubleSpinBoxReconSurfaceOpacity->value() * 100);
	m_reconSurfaceActor->GetProperty()->SetOpacity(ui->doubleSpinBoxReconSurfaceOpacity->value());

	ui->qvtkWidget->update();
}

void MainWindow::slotBrowseReconCenterline()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open Reconstructed Centerline File"), ui->lineEditCenterline->text(), tr("Reconstructed Centerline Files (*.vtk *.vtp)"));

	if (fileName.isNull())
		return;

	ui->lineEditReconCenterline->setText(fileName);

	m_statusLabel->setText("Loading reconstructed centerline file...");
	m_statusProgressBar->setValue(51);

	this->enableUI(false);
	this->m_io->SetCenterlinePath(fileName);

	// Instantiate the watcher to unlock
	m_ioWatcher = new QFutureWatcher<bool>;
	connect(m_ioWatcher, SIGNAL(finished()), this, SLOT(readReconCenterlineComplete()));

	// use QtConcurrent to run the read file on a new thread;
	QFuture<bool> future = QtConcurrent::run(this->m_io, &IO::ReadCenterline);
	m_ioWatcher->setFuture(future);
}

void MainWindow::slotSaveReconCenterline()
{
	if (m_io->GetReconstructedCenterline()->GetNumberOfPoints() == 0)
		return;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Reconstructed Centerline File"), ui->lineEditReconCenterline->text(), tr("Reconstructed Centerline Files (*.vtk *.vtp)"));

	if (fileName.isNull())
		return;

	m_io->WriteReconCenterline(fileName);
}

void MainWindow::slotBrowseReconSurface()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open Reconstructed Surface"), ui->lineEditReconSurface->text(), tr("Reconstructed Surface Files (*.vtk *.vtp *.stl)"));

	if (fileName.isNull())
		return;

	ui->lineEditReconSurface->setText(fileName);

	m_statusLabel->setText("Loading reconstructed surface file...");
	m_statusProgressBar->setValue(51);

	this->enableUI(false);
	this->m_io->SetReconSurfacePath(fileName);

	// Instantiate the watcher to unlock
	m_ioWatcher = new QFutureWatcher<bool>;
	connect(m_ioWatcher, SIGNAL(finished()), this, SLOT(readReconSurfaceComplete()));

	// use QtConcurrent to run the read file on a new thread;
	QFuture<bool> future = QtConcurrent::run(this->m_io, &IO::ReadReconSurface);
	m_ioWatcher->setFuture(future);
}

void MainWindow::slotSaveReconSurface()
{
	if (m_io->GetReconstructedSurface()->GetNumberOfPoints() == 0)
		return;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Reconstructed Surface File"), ui->lineEditReconSurface->text(), tr("Reconstructed Surface Files (*.vtk *.vtp *.stl)"));

	if (fileName.isNull())
		return;

	m_io->WriteReconSurface(fileName);
}

void MainWindow::slotSurfaceCapping()
{
	// remove all old data
	m_io->RemoveAllBoundaryCaps();

	if (m_io->GetSurface()->GetNumberOfPoints() == 0)
		return;

	// extract feature edges
	vtkSmartPointer<vtkFeatureEdges> boundaryEdges = vtkSmartPointer<vtkFeatureEdges>::New();
	boundaryEdges->SetInputData(m_io->GetSurface());
	boundaryEdges->BoundaryEdgesOn();
	boundaryEdges->FeatureEdgesOff();
	boundaryEdges->NonManifoldEdgesOff();
	boundaryEdges->ManifoldEdgesOff();
	boundaryEdges->Update();

	vtkSmartPointer<vtkStripper> boundaryStrips = vtkSmartPointer<vtkStripper>::New();
	boundaryStrips->SetInputData(boundaryEdges->GetOutput());
	boundaryStrips->Update();

	// change polyline to polygons
	vtkSmartPointer<vtkPolyData> boundaryPoly = vtkSmartPointer<vtkPolyData> ::New();
	boundaryPoly->SetPoints(boundaryStrips->GetOutput()->GetPoints());
	boundaryPoly->SetPolys(boundaryStrips->GetOutput()->GetLines());

	// connectivity to separate surfaces
	vtkSmartPointer<vtkConnectivityFilter> connectFilter = vtkSmartPointer<vtkConnectivityFilter>::New();
	connectFilter->SetExtractionModeToAllRegions();
	connectFilter->SetInputData(boundaryPoly);
	connectFilter->ColorRegionsOn(); // to generate RegionId array
	connectFilter->Update();

	vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
	geomFilter->SetInputConnection(connectFilter->GetOutputPort());
	geomFilter->Update();
	boundaryPoly->DeepCopy(geomFilter->GetOutput());

	// loop over the caps
	for (int i = 0; i < connectFilter->GetNumberOfExtractedRegions(); i++)
	{
		// extract isolated surface
		vtkSmartPointer<vtkThreshold> thresholdFilter = vtkSmartPointer<vtkThreshold>::New();
		thresholdFilter->SetInputData(boundaryPoly);
		thresholdFilter->ThresholdBetween(i, i);
		thresholdFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "RegionId");
		thresholdFilter->Update();
			
		vtkSmartPointer<vtkGeometryFilter> geomFilter2 = vtkSmartPointer<vtkGeometryFilter>::New();
		geomFilter2->SetInputConnection(thresholdFilter->GetOutputPort());
		geomFilter2->Update();

		//// triangulate the boundary caps
		//vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
		//triangleFilter->SetInputData(geomFilter2->GetOutput());
		//triangleFilter->Update();

		//// ui blocking, need to fix
		//// remesh to equal space, this may have small gap between surface and the boundary caps, need to fix
		//vtkSmartPointer<vtkvmtkPolyDataSurfaceRemeshing> surfaceRemeshing = vtkSmartPointer<vtkvmtkPolyDataSurfaceRemeshing>::New();
		//surfaceRemeshing->SetInputData(triangleFilter->GetOutput());
		//surfaceRemeshing->SetMinArea(1e-3);
		//surfaceRemeshing->SetMaxArea(1e-2);
		//surfaceRemeshing->SetNumberOfIterations(10);
		//surfaceRemeshing->Update();

		vtkSmartPointer<vtkPolyData> cap_poly = vtkSmartPointer<vtkPolyData>::New();
		cap_poly->DeepCopy(geomFilter2->GetOutput());

		// inject into io database
		BoundaryCap bc;
		bc.polydata->DeepCopy(cap_poly);

		// compute center of the boundary cap
		vtkSmartPointer<vtkCenterOfMass> com = vtkSmartPointer<vtkCenterOfMass>::New();
		com->SetInputData(cap_poly);
		com->Update();

		// get the center id
		if (m_io->GetOriginalCenterline()->GetNumberOfPoints() > 0)
		{
			vtkSmartPointer<vtkKdTreePointLocator> kdTree = vtkSmartPointer<vtkKdTreePointLocator>::New();
			kdTree->SetDataSet(m_io->GetOriginalCenterline());
			kdTree->Update();
			int id = kdTree->FindClosestPoint(com->GetCenter());

			// set the center
			QVector<double> center(3);
			center[0] = m_io->GetOriginalCenterline()->GetPoint(id)[0];
			center[1] = m_io->GetOriginalCenterline()->GetPoint(id)[1];
			center[2] = m_io->GetOriginalCenterline()->GetPoint(id)[2];
			bc.center = center;

			// set the radius
			if (m_io->GetOriginalCenterline()->GetPointData()->GetArray(
				m_preferences->GetRadiusArrayName().toStdString().c_str()) != nullptr)
				bc.radius = m_io->GetOriginalCenterline()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetComponent(id, 0);

			// set the tangent
			if (m_io->GetOriginalCenterline()->GetPointData()->GetArray(
				m_preferences->GetFrenetTangentArrayName().toStdString().c_str()) != nullptr)
			{
				QVector<double> tangent(3);
				tangent[0] = m_io->GetOriginalCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(id, 0);
				tangent[1] = m_io->GetOriginalCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(id, 1);
				tangent[2] = m_io->GetOriginalCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str())->GetComponent(id, 2);

				bc.tangent = tangent;
			}
		}
		else
		{
			// set the center
			QVector<double> center(3);
			center[0] = com->GetCenter()[0];
			center[1] = com->GetCenter()[1];
			center[2] = com->GetCenter()[2];
			bc.center = center;
		}

		m_io->AddBoundaryCap(bc);
	}

	this->updateBoundaryCapsTable();

	// render
	this->renderBoundaryCaps();
	this->renderBoundaryCapsDirection();
}

void MainWindow::slotSaveDomain()
{
	QDir dir = QFileInfo(ui->lineEditCenterline->text()).absoluteDir();

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Domain"), dir.absolutePath() + "/domain.json", tr("Domain File (*.json)"));

	if (fileName.isNull())
		return;

	m_io->SetWriteDomainPath(fileName);
	m_io->WriteDomain();
}

void MainWindow::slotRemoveCap()
{
	if (ui->tableWidgetDomain->currentRow() == -1)
		return;

	// id before remove
	int idx = ui->tableWidgetDomain->currentRow();

	// remove actors
	m_renderer->RemoveActor(this->m_boundaryCapActors.at(idx));
	m_renderer->RemoveActor(this->m_boundaryCapsDirectionActor.at(idx));
	
	m_boundaryCapActors.removeAt(idx);
	m_boundaryCapsDirectionActor.removeAt(idx);

	vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
	this->m_outlineMapper->SetInputData(polydata);
	this->m_outlineActor->VisibilityOff();

	m_io->RemoveBoundaryCap(idx);

	this->updateBoundaryCapsTable();
	this->renderBoundaryCaps();
	this->renderBoundaryCapsDirection();
	

	if (idx > 1)
		this->ui->tableWidgetDomain->selectRow(idx-1);
}

void MainWindow::slotRemoveAllCaps()
{
	m_io->RemoveAllBoundaryCaps();
	this->renderBoundaryCaps();
	this->renderBoundaryCapsDirection();
	this->updateBoundaryCapsTable();
}

void MainWindow::slotBoundaryCapTypeChange(int index)
{
	for (int i = 0; i < ui->tableWidgetDomain->rowCount(); i++)
	{
		BoundaryCap bc;
		bc.name = m_io->GetBoundaryCaps().at(i).name;
		bc.center = m_io->GetBoundaryCaps().at(i).center;
		bc.polydata->DeepCopy(m_io->GetBoundaryCaps().at(i).polydata);
		bc.radius = m_io->GetBoundaryCaps().at(i).radius;
		bc.tangent = m_io->GetBoundaryCaps().at(i).tangent;

		QComboBox* combo = (QComboBox* )ui->tableWidgetDomain->cellWidget(i, 1);

		switch (combo->currentIndex())
		{
			case 0:
				bc.type = BoundaryCapType::none;
				break;
			case 1:
				bc.type = BoundaryCapType::inlet;
				break;
			case 2:
				bc.type = BoundaryCapType::outlet;
				break;
		}

		m_io->SetBoundaryCap(i,bc);
	}
	this->renderBoundaryCaps();
	this->renderBoundaryCapsDirection();
}

void MainWindow::slotBoundaryCapTableItemChanged(QTableWidgetItem* item)
{
	// Block table signals
	ui->tableWidgetDomain->blockSignals(true);

	// check if first column (name) change
	if (item->column() == 0)
	{
		BoundaryCap bc;
		bc.name = item->text();
		bc.center = m_io->GetBoundaryCaps().at(item->row()).center;
		bc.polydata->DeepCopy(m_io->GetBoundaryCaps().at(item->row()).polydata);
		bc.radius = m_io->GetBoundaryCaps().at(item->row()).radius;
		bc.tangent = m_io->GetBoundaryCaps().at(item->row()).tangent;
		bc.type = m_io->GetBoundaryCaps().at(item->row()).type;

		m_io->SetBoundaryCap(item->row(), bc);
	}

	// Unblock signals
	ui->tableWidgetDomain->blockSignals(false);
}

void MainWindow::slotCurrentBoundaryCap()
{
	if (ui->tableWidgetDomain->currentRow() < 0)
	{
		m_outlineActor->VisibilityOff();
		return;
	}

	m_outlinerFilter->SetInputData(m_io->GetBoundaryCaps().at(ui->tableWidgetDomain->currentRow()).polydata);
	m_outlinerFilter->Update();
	m_outlineMapper->SetInputData(m_outlinerFilter->GetOutput());
	m_outlineMapper->Update();
	m_outlineActor->VisibilityOn();
	ui->qvtkWidget->update();
}

void MainWindow::slotAddCenterlineKeyPoint()
{
	if (m_io->GetSurface()->GetNumberOfPoints() == 0)
		return;

	QVector<double> keyPoint;
	keyPoint.append(m_io->GetSurface()->GetPoint(0)[0]);
	keyPoint.append(m_io->GetSurface()->GetPoint(0)[1]);
	keyPoint.append(m_io->GetSurface()->GetPoint(0)[2]);

	m_io->AddCenterlineKeyPoint(keyPoint, 0);
	this->updateCenterlineKeyPointsTable();
	// select last point
	ui->tableWidgetCenterlineKeyPoints->selectRow(ui->tableWidgetCenterlineKeyPoints->rowCount() - 1);

	this->renderCenterlineKeyPoints();
}

void MainWindow::slotCenterlineKeyPointTypeChanged(int index)
{
	for (int i = 0; i < ui->tableWidgetCenterlineKeyPoints->rowCount(); i++)
	{
		QPair <QVector<double>, bool> keyPoint;
		keyPoint.first.append(m_io->GetCenterlineKeyPoints().at(i).first[0]);
		keyPoint.first.append(m_io->GetCenterlineKeyPoints().at(i).first[1]);
		keyPoint.first.append(m_io->GetCenterlineKeyPoints().at(i).first[2]);

		QComboBox* combo = (QComboBox*)ui->tableWidgetCenterlineKeyPoints->cellWidget(i, 1);

		keyPoint.second = !!combo->currentIndex(); // Marxismic way of casting int to bool

		m_io->SetCenterlineKeyPoint(i, keyPoint);
	}

	this->renderCenterlineKeyPoints();
}

void MainWindow::slotCenterlineKeyPointUpdated()
{
	this->updateCenterlineKeyPointsTable();
	this->renderCenterlineKeyPoints();
}

void MainWindow::slotRemoveCenterlineKeyPoint()
{
	if (ui->tableWidgetCenterlineKeyPoints->currentRow() == -1)
		return;

	// index before remove
	int idx = ui->tableWidgetCenterlineKeyPoints->currentRow();

	// remove actors
	m_renderer->RemoveActor(this->m_centerlineKeyPointActors.at(idx));
	m_centerlineKeyPointActors.removeAt(idx);
	m_io->RemoveCenterlineKeyPoint(idx);

	this->updateCenterlineKeyPointsTable();

	this->renderCenterlineKeyPoints();

	if (idx > 0)
		this->ui->tableWidgetCenterlineKeyPoints->selectRow(idx - 1);
}

void MainWindow::slotRemoveAllCenterlineKeyPoint()
{
	// remove actors
	while (m_centerlineKeyPointActors.size()>0)
	{
		m_renderer->RemoveActor(this->m_centerlineKeyPointActors.at(m_io->GetCenterlineKeyPoints().size()-1));
		m_centerlineKeyPointActors.removeAt(m_io->GetCenterlineKeyPoints().size() - 1);
		m_io->RemoveCenterlineKeyPoint(m_io->GetCenterlineKeyPoints().size() - 1);
	}

	this->updateCenterlineKeyPointsTable();
	this->renderCenterlineKeyPoints();
}

void MainWindow::slotComputeCenterline()
{
	if (m_io->GetSurface()->GetNumberOfPoints() == 0)
		return;

	//QMessageBox *msg = new QMessageBox(this);
	//msg->setText("Computing Centerline");
	//msg->setStandardButtons(0);
	//msg->exec();

	// merge boundary cap to the surface
	vtkSmartPointer<vtkPolyData> surface = vtkSmartPointer<vtkPolyData>::New();
	surface->DeepCopy(m_io->GetSurface());

	// append boundary cap files
	if (m_io->GetBoundaryCaps().size() > 0)
	{
		vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
		appendFilter->SetInputData(surface);

		for (int i = 0; i < m_io->GetBoundaryCaps().size(); i++)
		{
			appendFilter->AddInputData(m_io->GetBoundaryCaps().at(i).polydata);
		}
		appendFilter->Update();

		vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
		cleanFilter->SetInputData(appendFilter->GetOutput());
		cleanFilter->Update();

		surface->DeepCopy(cleanFilter->GetOutput());
	}

	// prepare key points
	vtkSmartPointer<vtkKdTreePointLocator> kDTree = vtkSmartPointer<vtkKdTreePointLocator>::New();
	kDTree->SetDataSet(surface);
	kDTree->BuildLocator();

	vtkSmartPointer<vtkIdList> sourceIds = vtkSmartPointer<vtkIdList>::New();
	vtkSmartPointer<vtkIdList> targetIds = vtkSmartPointer<vtkIdList>::New();

	for (int i = 0; i < m_io->GetCenterlineKeyPoints().size(); i++)
	{
		QVector<double> point = m_io->GetCenterlineKeyPoints().at(i).first;
		double point_[3];
		point_[0] = point[0];
		point_[1] = point[1];
		point_[2] = point[2];
		vtkIdType iD = kDTree->FindClosestPoint(point_);
		
		switch (m_io->GetCenterlineKeyPoints().at(i).second)
		{
		case 0:
			sourceIds->InsertNextId(iD);
			break;
		case 1:
			targetIds->InsertNextId(iD);
			break;
		}
	}

	// error window
	if (sourceIds->GetNumberOfIds() == 0 || sourceIds->GetNumberOfIds() == 0)
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle("Compute Centerline");
		msgBox.setText("Invalid number of centerline sources/ targets.");
		msgBox.setInformativeText("At least one source point and one target point should be setted.");
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setIcon(QMessageBox::Warning);
		int ret = msgBox.exec();

		return;
	}

	// compute centerline
	std::cout << "data check ok, start compute centerline..." << std::endl;

	// centerline 
	vtkSmartPointer<vtkvmtkPolyDataCenterlines> centerlinesFilter = vtkSmartPointer<vtkvmtkPolyDataCenterlines>::New();
	centerlinesFilter->SetInputData(surface);
	centerlinesFilter->SetSourceSeedIds(sourceIds);
	centerlinesFilter->SetTargetSeedIds(targetIds);
	centerlinesFilter->SetAppendEndPointsToCenterlines(false);
	centerlinesFilter->SetCenterlineResampling(true);
	centerlinesFilter->SetResamplingStepLength(1);
	centerlinesFilter->SetRadiusArrayName("Radius");
	centerlinesFilter->SetEdgeArrayName("Edge");
	centerlinesFilter->SetEdgePCoordArrayName("PCoord");
	centerlinesFilter->Update();

	// geometry
	// compute centerline attribute, geometry and branch splitting
	vtkSmartPointer<vtkvmtkCenterlineAttributesFilter> attributeFilter = vtkSmartPointer<vtkvmtkCenterlineAttributesFilter>::New();
	attributeFilter->SetInputData(centerlinesFilter->GetOutput());
	attributeFilter->SetParallelTransportNormalsArrayName("ParallelTransportNormals");
	attributeFilter->SetAbscissasArrayName("Abscissas");
	attributeFilter->Update();

	vtkSmartPointer<vtkvmtkCenterlineGeometry> geometryFilter = vtkSmartPointer<vtkvmtkCenterlineGeometry>::New();
	geometryFilter->SetInputData(attributeFilter->GetOutput());
	geometryFilter->SetFrenetBinormalArrayName("FrenetBinormal");
	geometryFilter->SetFrenetNormalArrayName("FrenetNormal");
	geometryFilter->SetFrenetTangentArrayName("FrenetTangent");
	geometryFilter->SetLengthArrayName("Length");
	geometryFilter->SetTorsionArrayName("Torsion");
	geometryFilter->SetTortuosityArrayName("Tortuosity");
	geometryFilter->SetCurvatureArrayName("Curvature");
	geometryFilter->SetLineSmoothing(0);
	geometryFilter->SetNumberOfSmoothingIterations(100);
	geometryFilter->SetSmoothingFactor(0.1);
	geometryFilter->SetGlobalWarningDisplay(1);
	geometryFilter->Update();

	vtkSmartPointer<vtkvmtkCenterlineBranchExtractor> branchExtractor = vtkSmartPointer<vtkvmtkCenterlineBranchExtractor>::New();
	branchExtractor->SetInputData(geometryFilter->GetOutput());
	branchExtractor->SetRadiusArrayName("Radius");
	branchExtractor->SetCenterlineIdsArrayName("CenterlineIds");
	branchExtractor->SetGroupIdsArrayName("GroupIds");
	branchExtractor->SetBlankingArrayName("Blanking");
	branchExtractor->SetTractIdsArrayName("TractIds");
	branchExtractor->Update();

	//std::cout << "compute centerline ok" << std::endl;

	m_io->GetOriginalCenterline()->DeepCopy(branchExtractor->GetOutput());
	m_io->GetCenterline()->DeepCopy(branchExtractor->GetOutput());
	this->renderCenterline();
	this->updateCenterlineDataTable();
	this->updateCenterlinesInfoWidget();
	this->slotAutoLocateFirstBifurcation();

	// update centerlines plot
	ui->centerlinesInfoWidget->SetCenterlines(m_io->GetCenterline());
	ui->centerlinesInfoWidget->UpdatePlot();

	//msg->close();
}

void MainWindow::slotCurrentCenterlineKeyPoint()
{
	if (ui->tableWidgetCenterlineKeyPoints->currentRow() < 0)
	{
		m_outlineActor->VisibilityOff();
		return;
	}

	this->renderOutlineBoundingBox();

	ui->qvtkWidget->update();
}

void MainWindow::slotSetCenterlineIdsArray(QString name)
{
	//std::cout << name.toStdString() << std::endl;

	// remove old actors
	while (m_centerlineIdsActors.size() > 0)
	{
		m_renderer->RemoveActor(m_centerlineIdsActors.first());
		m_centerlineIdsActors.pop_front();
	}

	vtkDataArray* centerlineIds = m_io->GetCenterline()->GetCellData()->GetArray(name.toStdString().c_str());
	if (centerlineIds == nullptr)
		return;

	vtkSmartPointer<vtkThreshold> thresholdFilter = vtkSmartPointer<vtkThreshold>::New();
	thresholdFilter->SetInputData(m_io->GetCenterline());
	thresholdFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, name.toStdString().c_str());

	for (int i = centerlineIds->GetRange()[0]; i <= centerlineIds->GetRange()[1]; i++)
	{
		thresholdFilter->ThresholdBetween(i, i);
		thresholdFilter->Update();
		vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
		geomFilter->SetInputData((vtkDataObject*)thresholdFilter->GetOutput());
		geomFilter->Update();
		if (geomFilter->GetOutput()->GetNumberOfPoints() < 1)
			continue;

		double* plotPoint = geomFilter->GetOutput()->GetPoint(geomFilter->GetOutput()->GetNumberOfPoints() - 1);

		vtkSmartPointer<vtkBillboardTextActor3D> actor = vtkSmartPointer<vtkBillboardTextActor3D >::New();
		std::string dispText = std::to_string(i);
		actor->SetInput(dispText.c_str());
		actor->SetPosition(plotPoint);
		actor->GetTextProperty()->SetColor(1,1,0.5);

		m_renderer->AddActor(actor);
		m_centerlineIdsActors.push_back(actor);
	}

	// config update
	m_preferences->SetCenterlineIdsArrayName(name);

	ui->qvtkWidget->update();
}

void MainWindow::slotSetAbscissasArray(QString name )
{
	m_preferences->SetAbscissasArrayName(name);
}

void MainWindow::slotCenterlineConfigUpdate()
{
	this->updateCenterlineDataTable();

	// recon tab update
	ui->labelCurrentCenterlineIdsArray->setText(m_preferences->GetCenterlineIdsArrayName());

	ui->listWidgetCenterlineIdsPending->clear();
	ui->listWidgetCenterlineIdsRecon->clear();

	vtkDataArray* centerlineIdsArray = m_io->GetCenterline()->GetCellData()->GetArray(m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
	if (centerlineIdsArray == nullptr)
		return;

	for (int i = centerlineIdsArray->GetRange()[0];i<= centerlineIdsArray->GetRange()[1];i++)
		ui->listWidgetCenterlineIdsPending->addItem(QString::number(i));

	// centerlines info widget update
	ui->centerlinesInfoWidget->SetCenterlineIdsArray(m_preferences->GetCenterlineIdsArrayName());
	ui->centerlinesInfoWidget->SetAbscissasArray(m_preferences->GetAbscissasArrayName());
}

void MainWindow::slotAddFiducial()
{
	int current_row = ui->tableWidgetCenterline->currentRow();
	if (current_row < 0 || m_io->GetCenterline()->GetNumberOfPoints() == 0)
		return;

	QVector<double> point(3);
	point[0] = m_io->GetCenterline()->GetPoint(current_row)[0];
	point[1] = m_io->GetCenterline()->GetPoint(current_row)[1];
	point[2] = m_io->GetCenterline()->GetPoint(current_row)[2];

	m_io->AddFiducial(point,FiducialType::Stenosis);

	this->updateFiducialTable();
	m_currentPickingActor->VisibilityOff();
	this->renderFiducial();
}

void MainWindow::slotFiducialTypeChanged()
{
	if (ui->tableWidgetFiducial->currentRow() < 0)
	{
		m_outlineActor->VisibilityOff();
		return;
	}

	this->renderOutlineBoundingBox();

	ui->qvtkWidget->update();
}

void MainWindow::slotRemoveFiducial()
{
	if (ui->tableWidgetFiducial->currentRow() < 0)
		return;
	m_io->RemoveFiducial(ui->tableWidgetFiducial->currentRow());
	this->updateFiducialTable();
	this->renderFiducial();
}

void MainWindow::slotCurrentFiducial()
{
	if (ui->tableWidgetFiducial->currentRow() < 0)
	{
		m_outlineActor->VisibilityOff();
		return;
	}

	this->renderOutlineBoundingBox();

	ui->qvtkWidget->update();
}

void MainWindow::slotSetStenosisPoint()
{
	m_io->SetStenosisPoint(
		ui->centerlinesInfoWidget->GetStenosisPoint()[0],
		ui->centerlinesInfoWidget->GetStenosisPoint()[1],
		ui->centerlinesInfoWidget->GetStenosisPoint()[2]
		);
}

void MainWindow::slotSetProximalNormalPoint()
{
	m_io->SetProximalNormalPoint(
		ui->centerlinesInfoWidget->GetProximalNormalPoint()[0],
		ui->centerlinesInfoWidget->GetProximalNormalPoint()[1],
		ui->centerlinesInfoWidget->GetProximalNormalPoint()[2]
		);

	vtkSmartPointer<vtkKdTreePointLocator> locator = vtkSmartPointer<vtkKdTreePointLocator>::New();
	locator->SetDataSet(m_io->GetCenterline());
	locator->BuildLocator();
	double pt[3] = {
		ui->centerlinesInfoWidget->GetProximalNormalPoint()[0],
		ui->centerlinesInfoWidget->GetProximalNormalPoint()[1],
		ui->centerlinesInfoWidget->GetProximalNormalPoint()[2]
	};

	int id = locator->FindClosestPoint(pt);

	vtkDataArray* tangentArray = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str());
	vtkDataArray* normalArray = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str());
	vtkDataArray* binormalArray = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str());
	vtkDataArray* radiusArray = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str());
	if (tangentArray == nullptr)
	{
		std::cout << "Point data array \"" << m_preferences->GetFrenetTangentArrayName().toStdString() <<"\" not found in centerline" << std::endl;
		m_proximalNormalActor->SetVisibility(false);
		return;
	}
	if (normalArray == nullptr)
	{
		std::cout << "Point data array \"" << m_preferences->GetFrenetNormalArrayName().toStdString() << "\" not found in centerline" << std::endl;
		m_proximalNormalActor->SetVisibility(false);
		return;
	}
	if (binormalArray == nullptr)
	{
		std::cout << "Point data array \"" << m_preferences->GetFrenetBinormalArrayName().toStdString() << "\" not found in centerline" << std::endl;
		m_proximalNormalActor->SetVisibility(false);
		return;
	}

	double radius;
	if (radiusArray == nullptr)
	{
		std::cout << "Point data array \"" << m_preferences->GetRadiusArrayName().toStdString() << "\" not found in centerline" << std::endl;
		//m_proximalNormalActor->SetVisibility(false);
		radius = 2;
	}
	else
	{
		radius = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetTuple(id)[0];
	}

	double center2origin[3];
	center2origin[0] = -1 * (m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id)[0] + m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id)[0]);
	center2origin[1] = -1 * (m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id)[1] + m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id)[1]);
	center2origin[2] = -1 * (m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id)[2] + m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id)[2]);

	double center2origin_norm = vtkMath::Norm(center2origin);
	double normal_norm = vtkMath::Norm(m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id));
	double binormal_norm = vtkMath::Norm(m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id));

	double origin[3];

	double size_factor = 3.;

	origin[0] = m_io->GetCenterline()->GetPoint(id)[0] + size_factor * radius*center2origin[0] / center2origin_norm;
	origin[1] = m_io->GetCenterline()->GetPoint(id)[1] + size_factor * radius*center2origin[1] / center2origin_norm;
	origin[2] = m_io->GetCenterline()->GetPoint(id)[2] + size_factor * radius*center2origin[2] / center2origin_norm;

	double point1[3];
	point1[0] = origin[0] + sqrt(2)*size_factor * radius*m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id)[0] / normal_norm;
	point1[1] = origin[1] + sqrt(2)*size_factor * radius*m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id)[1] / normal_norm;
	point1[2] = origin[2] + sqrt(2)*size_factor * radius*m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id)[2] / normal_norm;

	double point2[3];
	point2[0] = origin[0] + sqrt(2)*size_factor * radius*m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id)[0] / binormal_norm;
	point2[1] = origin[1] + sqrt(2)*size_factor * radius*m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id)[1] / binormal_norm;
	point2[2] = origin[2] + sqrt(2)*size_factor * radius*m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id)[2] / binormal_norm;

	vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
	plane->SetOrigin(origin);
	plane->SetPoint1(point1);
	plane->SetPoint2(point2);
	plane->Update();

	m_proximalNormalMapper->SetInputData(plane->GetOutput());
	ui->qvtkWidget->update();
}

void MainWindow::slotSetDistalNormalPoint()
{
	m_io->SetDistalNormalPoint(
		ui->centerlinesInfoWidget->GetDistalNormalPoint()[0],
		ui->centerlinesInfoWidget->GetDistalNormalPoint()[1],
		ui->centerlinesInfoWidget->GetDistalNormalPoint()[2]
	);

	vtkSmartPointer<vtkKdTreePointLocator> locator = vtkSmartPointer<vtkKdTreePointLocator>::New();
	locator->SetDataSet(m_io->GetCenterline());
	locator->BuildLocator();
	double pt[3] = {
		ui->centerlinesInfoWidget->GetDistalNormalPoint()[0],
		ui->centerlinesInfoWidget->GetDistalNormalPoint()[1],
		ui->centerlinesInfoWidget->GetDistalNormalPoint()[2]
	};

	int id = locator->FindClosestPoint(pt);

	vtkDataArray* tangentArray = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str());
	vtkDataArray* normalArray = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str());
	vtkDataArray* binormalArray = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str());
	vtkDataArray* radiusArray = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str());
	if (tangentArray == nullptr)
	{
		std::cout << "Point data array \"" << m_preferences->GetFrenetTangentArrayName().toStdString() << "\" not found in centerline" << std::endl;
		m_distalNormalActor->SetVisibility(false);
		return;
	}
	if (normalArray == nullptr)
	{
		std::cout << "Point data array \"" << m_preferences->GetFrenetNormalArrayName().toStdString() << "\" not found in centerline" << std::endl;
		m_distalNormalActor->SetVisibility(false);
		return;
	}
	if (binormalArray == nullptr)
	{
		std::cout << "Point data array \"" << m_preferences->GetFrenetBinormalArrayName().toStdString() << "\" not found in centerline" << std::endl;
		m_distalNormalActor->SetVisibility(false);
		return;
	}
	double radius;
	if (radiusArray == nullptr)
	{
		std::cout << "Point data array \"" << m_preferences->GetRadiusArrayName().toStdString() << "\" not found in centerline" << std::endl;
		//m_proximalNormalActor->SetVisibility(false);
		radius = 2;
	}
	else
	{
		radius = m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str())->GetTuple(id)[0];
	}

	double center2origin[3];
	center2origin[0] = -1 * (m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id)[0] + m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id)[0]);
	center2origin[1] = -1 * (m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id)[1] + m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id)[1]);
	center2origin[2] = -1 * (m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id)[2] + m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id)[2]);

	double center2origin_norm = vtkMath::Norm(center2origin);
	double normal_norm = vtkMath::Norm(m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id));
	double binormal_norm = vtkMath::Norm(m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id));

	double origin[3];

	double size_factor = 3.;

	origin[0] = m_io->GetCenterline()->GetPoint(id)[0] + size_factor * radius*center2origin[0] / center2origin_norm;
	origin[1] = m_io->GetCenterline()->GetPoint(id)[1] + size_factor * radius*center2origin[1] / center2origin_norm;
	origin[2] = m_io->GetCenterline()->GetPoint(id)[2] + size_factor * radius*center2origin[2] / center2origin_norm;

	double point1[3];
	point1[0] = origin[0] + sqrt(2)*size_factor * radius*m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id)[0] / normal_norm;
	point1[1] = origin[1] + sqrt(2)*size_factor * radius*m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id)[1] / normal_norm;
	point1[2] = origin[2] + sqrt(2)*size_factor * radius*m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetNormalArrayName().toStdString().c_str())->GetTuple(id)[2] / normal_norm;

	double point2[3];
	point2[0] = origin[0] + sqrt(2)*size_factor * radius*m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id)[0] / binormal_norm;
	point2[1] = origin[1] + sqrt(2)*size_factor * radius*m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id)[1] / binormal_norm;
	point2[2] = origin[2] + sqrt(2)*size_factor * radius*m_io->GetCenterline()->GetPointData()->GetArray(m_preferences->GetFrenetBinormalArrayName().toStdString().c_str())->GetTuple(id)[2] / binormal_norm;
	vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
	plane->SetOrigin(origin);
	plane->SetPoint1(point1);
	plane->SetPoint2(point2);
	plane->Update();

	m_distalNormalMapper->SetInputData(plane->GetOutput());
	ui->qvtkWidget->update();
}

void MainWindow::slotActionBranch()
{
	m_branchOperation->show();
	m_preferences->raise();
	m_preferences->activateWindow();
}

void MainWindow::slotActionPreferences()
{
	m_preferences->show();
	m_preferences->raise();
	m_preferences->activateWindow();
}

void MainWindow::slotCenterlineConfigure()
{
	m_preferences->SetCurrentTab(0);
	m_preferences->show();
	m_preferences->raise();
	m_preferences->activateWindow();
}

void MainWindow::slotActionMeasurements()
{
	m_measurements->show();
	m_measurements->raise();
	m_measurements->activateWindow();
}

void MainWindow::slotExit()
{
	qApp->exit();
}

void MainWindow::enableUI(bool enable)
{
	ui->pushButtonSurface->setEnabled(enable);
	ui->pushButtonCenterline->setEnabled(enable);
}

void MainWindow::renderSurface()
{
	if (!(m_io->GetSurface()->GetNumberOfCells() > 0 ||
		m_io->GetSurface()->GetNumberOfPoints() > 0))
	{
		return;
	}

	m_surfaceMapper->SetInputData(m_io->GetSurface());
	m_surfaceMapper->ScalarVisibilityOff();
	m_surfaceActor->GetProperty()->SetOpacity(ui->doubleSpinBoxOpacity->value());

	ui->qvtkWidget->update();
}

void MainWindow::renderCenterline()
{
	if (!(m_io->GetCenterline()->GetNumberOfCells() > 0 ||
		m_io->GetCenterline()->GetNumberOfPoints() > 0))
	{
		return;
	}

	m_centerlineActor->SetVisibility(ui->checkBoxCenterlineVisisble->isChecked());
	m_centerlineMapper->SetScalarVisibility(false);
	m_centerlineMapper->SetInputData(m_io->GetCenterline());

	ui->qvtkWidget->update();
}

void MainWindow::renderVoronoi()
{
	if (!(m_io->GetVoronoiDiagram()->GetNumberOfCells() > 0 ||
		m_io->GetVoronoiDiagram()->GetNumberOfPoints() > 0))
	{
		return;
	}

	m_io->GetVoronoiDiagram()->GetPointData()->SetActiveScalars("Radius");

	m_voronoiMapper->SetInputData(m_io->GetVoronoiDiagram());
	m_voronoiMapper->SetScalarRange(m_io->GetVoronoiDiagram()->GetScalarRange());

	ui->qvtkWidget->update();
}

void MainWindow::renderReconSurface()
{
	if (!(m_io->GetReconstructedSurface()->GetNumberOfCells() > 0 ||
		m_io->GetReconstructedSurface()->GetNumberOfPoints() > 0))
	{
		return;
	}

	m_reconSurfaceMapper->SetInputData(m_io->GetReconstructedSurface());
	m_reconSurfaceMapper->SetScalarVisibility(false);

	ui->qvtkWidget->update();
}

void MainWindow::renderReconCenterline()
{
	if (!(m_io->GetReconstructedCenterline()->GetNumberOfCells() > 0 ||
		m_io->GetReconstructedCenterline()->GetNumberOfPoints() > 0))
	{
		return;
	}

	m_io->GetReconstructedCenterline()->Print(std::cout);

	m_reconCenterlineActor->SetVisibility(ui->checkBoxReconCenterlineVisisble->isChecked());
	m_reconCenterlineMapper->SetScalarVisibility(false);
	m_reconCenterlineMapper->SetInputData(m_io->GetReconstructedCenterline());

	ui->qvtkWidget->update();
}

void MainWindow::renderFirstBifurcationPoint()
{
	QVector<double> point = m_io->GetCenterlineFirstBifurcationPoint();

	m_firstBifurcationSphereSource->SetCenter(point[0], point[1], point[2]);
	m_firstBifurcationActor->VisibilityOn();
	ui->qvtkWidget->update();
}

void MainWindow::renderCenterlineKeyPoints()
{
	// remove all previous actors from renderer first
	for (int i = 0; i < m_centerlineKeyPointActors.size(); i++)
	{
		m_renderer->RemoveActor(m_centerlineKeyPointActors.at(i));
	}
	m_centerlineKeyPointActors.clear();

	for (int i = 0; i < m_io->GetCenterlineKeyPoints().size(); i++)
	{
		vtkSmartPointer<vtkSphereSource> source = vtkSmartPointer<vtkSphereSource>::New();
		source->SetCenter(
			m_io->GetCenterlineKeyPoints().at(i).first[0],
			m_io->GetCenterlineKeyPoints().at(i).first[1],
			m_io->GetCenterlineKeyPoints().at(i).first[2]);

		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputConnection(source->GetOutputPort());

		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		switch (m_io->GetCenterlineKeyPoints().at(i).second)
		{
		case 0:
			actor->GetProperty()->SetColor(255 / 255.0, 0 / 255.0, 0 / 255.0);
			break;
		case 1:
			actor->GetProperty()->SetColor(0 / 255.0, 255 / 255.0, 0 / 255.0);
			break;
		}

		m_centerlineKeyPointActors.append(actor);
		m_renderer->AddActor(actor);
	}

	this->renderOutlineBoundingBox();

	ui->qvtkWidget->update();
}

void MainWindow::updateCenterlineKeyPointsTable()
{
	// clear table
	ui->tableWidgetCenterlineKeyPoints->setRowCount(0);

	for (int i = 0; i < m_io->GetCenterlineKeyPoints().size(); i++)
	{
		ui->tableWidgetCenterlineKeyPoints->insertRow(ui->tableWidgetCenterlineKeyPoints->rowCount());

		// point
		ui->tableWidgetCenterlineKeyPoints->setItem(
			ui->tableWidgetCenterlineKeyPoints->rowCount() - 1,
			0,
			new QTableWidgetItem(
				QString::number(m_io->GetCenterlineKeyPoints().at(i).first[0]) + ", " +
				QString::number(m_io->GetCenterlineKeyPoints().at(i).first[1]) + ", " +
				QString::number(m_io->GetCenterlineKeyPoints().at(i).first[2])
			));

		// disable edit function
		ui->tableWidgetCenterlineKeyPoints->item(i, 0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		// type
		QComboBox* combo = new QComboBox(ui->tableWidgetCenterlineKeyPoints);
		combo->addItem("Source");
		combo->addItem("Target");
		combo->setCurrentIndex(m_io->GetCenterlineKeyPoints().at(i).second);
		ui->tableWidgetCenterlineKeyPoints->setCellWidget(
			ui->tableWidgetCenterlineKeyPoints->rowCount() - 1,
			1,
			combo);
		// combobox change
		connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::slotCenterlineKeyPointTypeChanged);	
	}
}

void MainWindow::updateCenterlineDataTable()
{
	// clear table
	ui->tableWidgetCenterline->setRowCount(0);
	
	vtkSmartPointer<vtkPolyData> centerline = m_io->GetCenterline();
	for (int i = 0; i < centerline->GetNumberOfPoints(); i++)
	{
		ui->tableWidgetCenterline->insertRow(ui->tableWidgetCenterline->rowCount());
		// id
		ui->tableWidgetCenterline->setItem(
			ui->tableWidgetCenterline->rowCount() - 1,
			0, 
			new QTableWidgetItem(QString::number(i)));
		// abscissas

		vtkDataArray* abscissas = centerline->GetPointData()->GetArray(
			m_preferences->GetAbscissasArrayName().toStdString().c_str());
		if (abscissas != nullptr)
		{
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				1,
				new QTableWidgetItem(QString::number(abscissas->GetComponent(i, 0))));

			// normalized abscissas
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				2,
				new QTableWidgetItem(
					QString::number(abscissas->GetComponent(i, 0)-
					abscissas->GetComponent(m_io->GetCenterlineFirstBifurcationPointId(),0))));
		}
		else
		{
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				1,
				new QTableWidgetItem(QString::number(0)));

			// normalized abscissas
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				2,
				new QTableWidgetItem(QString::number(0)));
		}

		// point
		ui->tableWidgetCenterline->setItem(
			ui->tableWidgetCenterline->rowCount() - 1,
			3,
			new QTableWidgetItem(
				QString::number(centerline->GetPoint(i)[0]) + ", " + 
				QString::number(centerline->GetPoint(i)[1]) + ", "+
				QString::number(centerline->GetPoint(i)[2])
			));

		// binormal
		vtkDataArray* binormal = centerline->GetPointData()->GetArray(
			m_preferences->GetFrenetBinormalArrayName().toStdString().c_str());
		if (binormal != nullptr)
		{
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				4,
				new QTableWidgetItem(
					QString::number(binormal->GetComponent(i, 0)) + ", "+
					QString::number(binormal->GetComponent(i, 1)) + ", " +
					QString::number(binormal->GetComponent(i, 2))
				));
		}
		else
		{
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				4,
				new QTableWidgetItem(
					QString::number(0) + ", " +
					QString::number(0) + ", " +
					QString::number(0)
				));
		}

		// normal
		vtkDataArray* normal = centerline->GetPointData()->GetArray(
			m_preferences->GetFrenetNormalArrayName().toStdString().c_str());
		if (normal != nullptr)
		{
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				5,
				new QTableWidgetItem(
					QString::number(normal->GetComponent(i, 0)) + ", " +
					QString::number(normal->GetComponent(i, 1)) + ", " +
					QString::number(normal->GetComponent(i, 2))
				));
		}
		else
		{
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				5,
				new QTableWidgetItem(
					QString::number(0) + ", " +
					QString::number(0) + ", " +
					QString::number(0)
				));
		}

		// tangent
		vtkDataArray* tangent = centerline->GetPointData()->GetArray(
			m_preferences->GetFrenetTangentArrayName().toStdString().c_str());
		if (tangent != nullptr)
		{
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				6,
				new QTableWidgetItem(
					QString::number(tangent->GetComponent(i, 0)) + ", " +
					QString::number(tangent->GetComponent(i, 1)) + ", " +
					QString::number(tangent->GetComponent(i, 2))
				));
		}
		else
		{
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				6,
				new QTableWidgetItem(
					QString::number(0) + ", " +
					QString::number(0) + ", " +
					QString::number(0)
				));
		}

		// radius
		vtkDataArray* radius = centerline->GetPointData()->GetArray(
			m_preferences->GetRadiusArrayName().toStdString().c_str());
		if (radius != nullptr)
		{
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				7,
				new QTableWidgetItem(QString::number(radius->GetComponent(i, 0))));
		}
		else
		{
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				7,
				new QTableWidgetItem(QString::number(0)));

		}

		// disable edit function
		for (int j = 0; j < 8; j++)
		{
			ui->tableWidgetCenterline->item(ui->tableWidgetCenterline->rowCount() - 1, j)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		}
		
		// set row color
		if (ui->tableWidgetCenterline->item(ui->tableWidgetCenterline->rowCount() - 1, 2)->text().toDouble() < -1.*ui->doubleSpinBoxProximalBound->value() ||
			ui->tableWidgetCenterline->item(ui->tableWidgetCenterline->rowCount() - 1, 2)->text().toDouble() > ui->doubleSpinBoxDistalBound->value())
		{
			for (int j = 0; j<8; j++)
				ui->tableWidgetCenterline->item(ui->tableWidgetCenterline->rowCount() - 1, j)->setData(Qt::BackgroundRole, QColor(255, 110, 110));
		}
		else
		{
			for (int j = 0; j<8; j++)
				ui->tableWidgetCenterline->item(ui->tableWidgetCenterline->rowCount() - 1, j)->setData(Qt::BackgroundRole, QColor(255, 255, 255));
		}
	}
}

void MainWindow::createFirstBifurationPoint()
{
	// Create a sphere
	m_firstBifurcationSphereSource->SetCenter(0.0, 0.0, 0.0);
	m_firstBifurcationSphereSource->SetRadius(0.5);
	// Make the surface smooth.
	m_firstBifurcationSphereSource->SetPhiResolution(100);
	m_firstBifurcationSphereSource->SetThetaResolution(100);

	m_firstBifurcationMapper->SetInputConnection(m_firstBifurcationSphereSource->GetOutputPort());
	m_firstBifurcationActor->SetMapper(m_firstBifurcationMapper);
	m_firstBifurcationActor->GetProperty()->SetColor(58*1.0/255,52 * 1.0 / 255,235 * 1.0 / 255);
	m_firstBifurcationActor->VisibilityOff();

	m_renderer->AddActor(m_firstBifurcationActor);
}

void MainWindow::createCurrentPickingPoint()
{
	// Create a sphere
	m_currentPickingSphereSource->SetCenter(0.0, 0.0, 0.0);
	m_currentPickingSphereSource->SetRadius(0.5);
	// Make the surface smooth.
	m_currentPickingSphereSource->SetPhiResolution(100);
	m_currentPickingSphereSource->SetThetaResolution(100);

	m_currentPickingMapper->SetInputConnection(m_currentPickingSphereSource->GetOutputPort());
	m_currentPickingActor->SetMapper(m_currentPickingMapper);
	m_currentPickingActor->GetProperty()->SetColor(235 * 1.0 / 255, 255 * 1.0 / 255, 20 * 1.0 / 255);
	m_currentPickingActor->VisibilityOff();

	m_renderer->AddActor(m_currentPickingActor);
}

void MainWindow::createOutlineBoundingBox()
{
	m_outlineActor->SetMapper(m_outlineMapper);
	m_outlineActor->GetProperty()->SetColor(255 * 1.0 / 255, 255 * 1.0 / 255, 255 * 1.0 / 255);
	m_outlineActor->VisibilityOff();

	m_renderer->AddActor(m_outlineActor);
}

void MainWindow::createClipper()
{
	m_clipperActor->SetMapper(m_clipperMapper);
	m_clipperActor->GetProperty()->SetColor(161 * 1.0 / 255, 255 * 1.0 / 255, 20 * 1.0 / 255);
	m_clipperActor->GetProperty()->SetOpacity(1.0);
	m_clipperActor->VisibilityOff();

	m_renderer->AddActor(m_clipperActor);
}

void MainWindow::clip(int direction)
{
	//std::cout << "=======================================" << std::endl;
	//std::cout << "number of centerline points before clip: " << m_io->GetCenterline()->GetNumberOfPoints() << std::endl;
	//std::cout << "Bifurcation point before clip: " <<
	//	m_io->GetCenterlineFirstBifurcationPoint()[0] << ", " <<
	//	m_io->GetCenterlineFirstBifurcationPoint()[1] << ", " <<
	//	m_io->GetCenterlineFirstBifurcationPoint()[2] << std::endl;
	//std::cout << "clipper transform before clip: " << std::endl;
	//m_clipTransform->GetMatrix()->Print(std::cout);
	//std::cout << "*********" << std::endl;


	/**
	* the direction must be -1 (remove distal) or 1 (remove proximal)
	*/

	if (m_clppingPointId > m_io->GetCenterline()->GetNumberOfPoints() - 1)
		return;

	// calculate the pick point (1mm from centerline normal direction
	// centerline tangent
	double* tangent = m_io->GetCenterline()->GetPointData()->GetArray("FrenetTangent")->GetTuple(m_clppingPointId);
	double* point = m_io->GetCenterline()->GetPoint(m_clppingPointId);

	double pickPoint[3] = { point[0] + direction*tangent[0],point[1] + direction*tangent[1] ,point[2] + direction*tangent[2] };

	if (tangent == nullptr || point == nullptr)
		return;

	// lcc filter
	vtkSmartPointer<vtkConnectivityFilter> connectedFilter = vtkSmartPointer<vtkConnectivityFilter>::New();
	connectedFilter->SetExtractionModeToClosestPointRegion();
	connectedFilter->SetClosestPoint(pickPoint);

	vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
	clipper->GenerateClippedOutputOn();
	clipper->SetValue(0);

	if (ui->comboBoxClipperStyle->currentText() == "Box")
	{
		vtkSmartPointer<vtkBoxWidget> clipWidget = vtkSmartPointer<vtkBoxWidget>::New();
		clipWidget->SetTransform(m_clipTransform);
		vtkSmartPointer<vtkPlanes> clipFunction = vtkSmartPointer<vtkPlanes>::New();
		// transfer the box widget planes to clip function
		clipWidget->GetPlanes(clipFunction);
		clipper->SetClipFunction(clipFunction);
	}
	else if (ui->comboBoxClipperStyle->currentText() == "Clyinder")
	{
		vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
		cylinder->SetResolution(100);

		vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
		transformFilter->SetInputData(cylinder->GetOutput());
		transformFilter->SetTransform(m_clipTransform);
		transformFilter->Update();

		// Implicit function that will be used to slice the mesh 
		vtkSmartPointer<vtkImplicitPolyDataDistance> implicitPolyDataDistance =
			vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
		implicitPolyDataDistance->SetInput(transformFilter->GetOutput());
		implicitPolyDataDistance->Modified();

		clipper->SetClipFunction(implicitPolyDataDistance);
	}

	//m_clipTransform->Print(std::cout);

	// clip surface
	clipper->SetInputData(m_io->GetSurface());
	clipper->Update();
	connectedFilter->SetInputData(clipper->GetOutput());
	connectedFilter->Update();

#if VTK_MAJOR_VERSION >= 8
	m_io->GetSurface()->DeepCopy(connectedFilter->GetOutput());
#else
	vtkSmartPointer<vtkGeometryFilter> geomFilter1 = vtkSmartPointer<vtkGeometryFilter>::New();
	geomFilter1->SetInputConnection(connectedFilter->GetOutputPort());
	geomFilter1->Update();
	m_io->GetSurface()->DeepCopy(geomFilter1->GetOutput());
#endif

	// hold bifurcation point before centerline clip
	QVector<double> bif_point(3);
	bif_point[0] = m_io->GetCenterlineFirstBifurcationPoint()[0];
	bif_point[1] = m_io->GetCenterlineFirstBifurcationPoint()[1];
	bif_point[2] = m_io->GetCenterlineFirstBifurcationPoint()[2];
	
	//std::cout << "before clip: " <<
	//	bif_point[0] << ", " <<
	//	bif_point[1] << ", " <<
	//	bif_point[2] << ", " <<
	//	std::endl;

	// clip centerline
	clipper->SetInputData(m_io->GetCenterline());
	clipper->Update();
	connectedFilter->SetInputData(clipper->GetOutput());
	connectedFilter->Update();

#if VTK_MAJOR_VERSION >= 8
	m_io->GetCenterline()->DeepCopy(connectedFilter->GetOutput());
#else
	vtkSmartPointer<vtkGeometryFilter> geomFilter2 = vtkSmartPointer<vtkGeometryFilter>::New();
	geomFilter2->SetInputConnection(connectedFilter->GetOutputPort());
	geomFilter2->Update();
	m_io->GetCenterline()->DeepCopy(geomFilter2->GetOutput());
#endif

	//std::cout << "after clip: " <<
	//	bif_point[0] << ", " <<
	//	bif_point[1] << ", " <<
	//	bif_point[2] << ", " <<
	//	std::endl;

	m_io->SetCenterlineFirstBifurcationPoint(bif_point);
	this->renderFirstBifurcationPoint();

	this->updateCenterlineDataTable();

	// set clipper invisible
	this->m_clipperActor->VisibilityOff();
	ui->qvtkWidget->update();

	//std::cout << "number of centerline points after clip: " << m_io->GetCenterline()->GetNumberOfPoints() << std::endl;
	//std::cout << "Bifurcation point after clip: " << 
	//	m_io->GetCenterlineFirstBifurcationPoint()[0] << ", " << 
	//	m_io->GetCenterlineFirstBifurcationPoint()[1] << ", " <<
	//	m_io->GetCenterlineFirstBifurcationPoint()[2] << std::endl;
	//std::cout << "clipper transform after clip: " << std::endl;
	//m_clipTransform->GetMatrix()->Print(std::cout);

}

void MainWindow::renderBoundaryCaps()
{
	// remove all previous actors from renderer first
	for (int i = 0; i < m_boundaryCapActors.size(); i++)
	{
		m_renderer->RemoveActor(m_boundaryCapActors.at(i));
	}
	m_boundaryCapActors.clear();

	for (int i =0; i < m_io->GetBoundaryCaps().size();i++)
	{
		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputData(m_io->GetBoundaryCaps().at(i).polydata);
		mapper->SetScalarVisibility(0);
		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		//actor->GetProperty()->SetRepresentationToWireframe();
		switch (m_io->GetBoundaryCaps().at(i).type)
		{
		case BoundaryCapType::none:
			actor->GetProperty()->SetColor(52 / 255.0, 20 / 255.0, 255 / 255.0);
			break;
		case BoundaryCapType::inlet:
			actor->GetProperty()->SetColor(94 / 255.0, 255 / 255.0, 145 / 255.0);
			break;
		case BoundaryCapType::outlet:
			actor->GetProperty()->SetColor(255 / 255.0, 90 / 255.0, 61 / 255.0);
			break;
		}
		actor->GetProperty()->SetOpacity(ui->doubleSpinBoxOpacity->value());

		if (ui->checkBoxWireframe->checkState() == Qt::CheckState::Checked)
		{
			actor->GetProperty()->SetRepresentationToWireframe();
		}
		else if (ui->checkBoxWireframe->checkState() == Qt::CheckState::Unchecked)
		{
			actor->GetProperty()->SetRepresentationToSurface();
		}

		m_boundaryCapActors.append(actor);
		m_renderer->AddActor(actor);
	}

	// set capper to invisible
	m_clipperActor->VisibilityOff();

	ui->qvtkWidget->update();
}

void MainWindow::updateBoundaryCapsTable()
{
	// Block table signals
	ui->tableWidgetDomain->blockSignals(true);

	// clear table first
	ui->tableWidgetDomain->setRowCount(0);

	for (int i = 0; i < m_io->GetBoundaryCaps().size(); i++)
	{
		BoundaryCap bc = m_io->GetBoundaryCaps().at(i);
		ui->tableWidgetDomain->insertRow(ui->tableWidgetDomain->rowCount());
		
		// name
		ui->tableWidgetDomain->setItem(
			ui->tableWidgetDomain->rowCount() - 1,
			0,
			new QTableWidgetItem(m_io->GetBoundaryCaps().at(i).name));

		// type
		QComboBox* combo = new QComboBox(ui->tableWidgetDomain);
		combo->addItem("None");
		combo->addItem("Inlet");
		combo->addItem("Outlet");
		combo->setCurrentIndex(bc.type);
		ui->tableWidgetDomain->setCellWidget(
			ui->tableWidgetDomain->rowCount() - 1,
			1,
			combo);
		// combobox change
		connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::slotBoundaryCapTypeChange);

		// center
		QString center = 
			QString::number(bc.center[0]) + ", " + 
			QString::number(bc.center[1]) + ", " + 
			QString::number(bc.center[2]);
			 
		ui->tableWidgetDomain->setItem(
			ui->tableWidgetDomain->rowCount() - 1,
			2,
			new QTableWidgetItem(center));

		// tangent
		QString tangent =
			QString::number(bc.tangent[0]) + ", " +
			QString::number(bc.tangent[1]) + ", " +
			QString::number(bc.tangent[2]);

		ui->tableWidgetDomain->setItem(
			ui->tableWidgetDomain->rowCount() - 1,
			3,
			new QTableWidgetItem(tangent));

		// radius
		ui->tableWidgetDomain->setItem(
			ui->tableWidgetDomain->rowCount() - 1,
			4,
			new QTableWidgetItem(QString::number(bc.radius)));

		// disable edit function
		for (int j = 2; j<4; j++)
			ui->tableWidgetDomain->item(i, j)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}

	// Unblock signals
	ui->tableWidgetDomain->blockSignals(false);
}

void MainWindow::renderBoundaryCapsDirection()
{
	// remove all previous actors from renderer first
	for (int i = 0; i < m_boundaryCapsDirectionActor.size(); i++)
	{
		m_renderer->RemoveActor(m_boundaryCapsDirectionActor.at(i));
	}
	m_boundaryCapsDirectionActor.clear();

	for (int i = 0; i < m_io->GetBoundaryCaps().size(); i++)
	{
		BoundaryCap bc = m_io->GetBoundaryCaps().at(i);

		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		
		// create arrow to indication direction
		vtkSmartPointer<vtkArrowSource> arrowSource = vtkSmartPointer<vtkArrowSource>::New();
		arrowSource->SetShaftResolution(100);
		arrowSource->SetTipResolution(100);
		arrowSource->Update();
		
		// create transform
		vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
		transform->Translate(bc.center[0],bc.center[1],bc.center[2]);
		double w = atan(sqrt(pow(bc.tangent[0], 2) + pow(bc.tangent[1], 2)) / bc.tangent[2]) * 180 / 3.14;
		transform->RotateWXYZ(w, -bc.tangent[1], bc.tangent[0], 0);
		if (bc.tangent[2] >= 0)
			transform->RotateY(-90);
		else
			transform->RotateY(90);
		switch (bc.type)
		{
			case BoundaryCapType::none:
				transform->Scale(0, 0, 0);
				break;
			case BoundaryCapType::inlet:
				transform->Scale(10,10,10);
				transform->Translate(-1, 0, 0);
				break;
			case BoundaryCapType::outlet:
				transform->Scale(10, 10, 10);
				break;
		}
		
		vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
		transformFilter->SetInputData(arrowSource->GetOutput());
		transformFilter->SetTransform(transform);
		transformFilter->Update();

		mapper->SetInputData(transformFilter->GetOutput());
		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		switch (bc.type)
		{
		case BoundaryCapType::none:
			break;
		case BoundaryCapType::inlet:
			actor->GetProperty()->SetColor(94 / 255.0, 255 / 255.0, 145 / 255.0);
			break;
		case BoundaryCapType::outlet:
			actor->GetProperty()->SetColor(255 / 255.0, 90 / 255.0, 61 / 255.0);
			break;
		}
		actor->GetProperty()->SetOpacity(ui->doubleSpinBoxOpacity->value());
		m_boundaryCapsDirectionActor.append(actor);
		m_renderer->AddActor(actor);
	}

	ui->qvtkWidget->update();

}

void MainWindow::renderOutlineBoundingBox()
{
	// bounding box
	if (ui->tableWidgetCenterlineKeyPoints->currentRow() >= 0)
	{
		// create input poly data
		QVector<double> point = m_io->GetCenterlineKeyPoints().at(ui->tableWidgetCenterlineKeyPoints->currentRow()).first;

		vtkSmartPointer<vtkSphereSource> source = vtkSmartPointer<vtkSphereSource>::New();
		source->SetCenter(point[0], point[1], point[2]);
		source->Update();

		m_outlinerFilter->SetInputData(source->GetOutput());
		m_outlinerFilter->Update();
		m_outlineMapper->SetInputData(m_outlinerFilter->GetOutput());
		m_outlineMapper->Update();
		m_outlineActor->VisibilityOn();
	}
	else if ((ui->tableWidgetFiducial->currentRow() >= 0))
	{
		// create input poly data
		QVector<double> point = m_io->GetFiducial().at(ui->tableWidgetFiducial->currentRow()).first;

		vtkSmartPointer<vtkSphereSource> source = vtkSmartPointer<vtkSphereSource>::New();
		source->SetCenter(point[0], point[1], point[2]);
		source->Update();

		m_outlinerFilter->SetInputData(source->GetOutput());
		m_outlinerFilter->Update();
		m_outlineMapper->SetInputData(m_outlinerFilter->GetOutput());
		m_outlineMapper->Update();
		m_outlineActor->VisibilityOn();
	}
	else
	{
		m_outlineActor->VisibilityOff();
	}
}

void MainWindow::updateFiducialTable()
{
	// clear table
	ui->tableWidgetFiducial->setRowCount(0);

	for (int i = 0; i < m_io->GetFiducial().size(); i++)
	{
		ui->tableWidgetFiducial->insertRow(ui->tableWidgetFiducial->rowCount());

		// point
		ui->tableWidgetFiducial->setItem(
			ui->tableWidgetFiducial->rowCount() - 1,
			0,
			new QTableWidgetItem(
				QString::number(m_io->GetFiducial().at(i).first[0]) + ", " +
				QString::number(m_io->GetFiducial().at(i).first[1]) + ", " +
				QString::number(m_io->GetFiducial().at(i).first[2])
			));

		// disable edit function
		ui->tableWidgetFiducial->item(i, 0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		// type
		QComboBox* combo = new QComboBox(ui->tableWidgetFiducial);
		combo->addItem("Stenosis");
		combo->addItem("Bifurcation");
		combo->addItem("DoS Ref");
		combo->addItem("Proximal Normal");
		combo->addItem("Distal Normal");
		combo->addItem("Others");
		combo->setCurrentIndex(m_io->GetFiducial().at(i).second);
		ui->tableWidgetFiducial->setCellWidget(
			ui->tableWidgetFiducial->rowCount() - 1,
			1,
			combo);
		// combobox change
		connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::slotFiducialTypeChanged);
	}
}

void MainWindow::renderFiducial()
{
	// remove all previous actors from renderer first
	for (int i = 0; i < m_fiducialActors.size(); i++)
	{
		m_renderer->RemoveActor(m_fiducialActors.at(i));
	}
	m_fiducialActors.clear();

	for (int i = 0; i < m_io->GetFiducial().size(); i++)
	{
		vtkSmartPointer<vtkSphereSource> source = vtkSmartPointer<vtkSphereSource>::New();
		source->SetCenter(
			m_io->GetFiducial().at(i).first[0],
			m_io->GetFiducial().at(i).first[1],
			m_io->GetFiducial().at(i).first[2]);

		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputConnection(source->GetOutputPort());

		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		//switch (m_io->GetCenterlineKeyPoints().at(i).second)
		//{
		//case 0:
		//	actor->GetProperty()->SetColor(255 / 255.0, 0 / 255.0, 0 / 255.0);
		//	break;
		//case 1:
		//	actor->GetProperty()->SetColor(0 / 255.0, 255 / 255.0, 0 / 255.0);
		//	break;
		//}
		actor->GetProperty()->SetColor(113 / 255.0, 0 / 255.0, 125 / 255.0);
		m_fiducialActors.append(actor);
		m_renderer->AddActor(actor);
	}

	this->renderOutlineBoundingBox();

	ui->qvtkWidget->update();
}

void MainWindow::updateCenterlinesInfoWidget()
{
	ui->centerlinesInfoWidget->SetCenterlines(m_io->GetCenterline());
}

void MainWindow::ExtractCylindericInterpolationVoronoiDiagram(vtkPolyData * clippedCenterline, vtkPolyData* outputData, int idx, bool dir, float len)
{
	//std::cout << "idx: " << idx << std::endl;

	if (m_io->GetVoronoiDiagram()->GetNumberOfPoints() == 0)
	{
		std::cout << "No point in Voronoi diagram" << std::endl;
		return;
	}

	if (outputData == nullptr)
		outputData = vtkSmartPointer<vtkPolyData>::New();

	//std::cout << "extract interpolation data from clipped voronoi diagram" << std::endl;

	vtkDataArray* tangentArray = clippedCenterline->GetPointData()->GetArray(m_preferences->GetFrenetTangentArrayName().toStdString().c_str());
	vtkDataArray* parallelTransportNormalsArray = clippedCenterline->GetPointData()->GetArray(m_preferences->GetParallelTransportNormalsName().toStdString().c_str());
	vtkDataArray* radiusArray = clippedCenterline->GetPointData()->GetArray(m_preferences->GetRadiusArrayName().toStdString().c_str());

	if (idx == -1)
		idx = clippedCenterline->GetNumberOfPoints() - 1;

	//std::cout << "index: " << idx << std::endl;
	//std::cout << "direction: " << dir << std::endl;
	//std::cout << "length: " << len << std::endl;

	if (tangentArray == nullptr || parallelTransportNormalsArray == nullptr || radiusArray == nullptr)
		return;

	if (tangentArray->GetNumberOfComponents() != 3 || parallelTransportNormalsArray->GetNumberOfComponents() != 3 ||
		tangentArray->GetTuple(idx) == nullptr || parallelTransportNormalsArray->GetTuple(idx) == nullptr)
		return;

	double* tangent = tangentArray->GetTuple(idx);;
	double* parallelTransportNormal = parallelTransportNormalsArray->GetTuple(idx);
	double tangentNorm = vtkMath::Norm(tangent);
	double parallelTransportNormalNorm = vtkMath::Norm(parallelTransportNormal);

	for (int i = 0; i < 3; i++)
	{
		tangent[i] = tangent[i] / tangentNorm;
		parallelTransportNormal[i] = parallelTransportNormal[i] / parallelTransportNormalNorm;
	}

	double* point = clippedCenterline->GetPoint(idx);
	double radius = radiusArray->GetTuple1(idx);

	//std::cout << "center: " << point[0] <<", " << point[1] << ", " << point[2] << std::endl;
	//std::cout << "radius: " << radius << std::endl;
	//std::cout << "tangent: " << tangent[0] << ", " << tangent[1] << ", " << tangent[2] << std::endl;

	// create cylinder function
	vtkSmartPointer<vtkCylinder> cylinderFunction = vtkSmartPointer<vtkCylinder>::New();
	cylinderFunction->SetCenter(point);
	cylinderFunction->SetRadius(radius*1.5);
	cylinderFunction->SetAxis(tangent);

	// clip planes
	vtkSmartPointer<vtkPlane> planeFunction1 = vtkSmartPointer<vtkPlane>::New();
	vtkSmartPointer<vtkPlane> planeFunction2 = vtkSmartPointer<vtkPlane>::New();
	double plane1origin[3];
	double plane2origin[3];

	for (int i = 0; i < 3; i++)
	{
		if (dir == true)
		{
			// proximal to distal
			plane1origin[i] = point[i];
			plane2origin[i] = point[i] + tangent[i] * len;
		}
		else
		{
			// distal to proximal
			plane1origin[i] = point[i] - tangent[i] * len;
			plane2origin[i] = point[i];
		}
	}

	//std::cout << "plane1origin: " << plane1origin[0] << ", " << plane1origin[1] << ", " << plane1origin[2] << std::endl;
	//std::cout << "plane2origin: " << plane2origin[0] << ", " << plane2origin[1] << ", " << plane2origin[2] << std::endl;

	planeFunction1->SetOrigin(plane1origin);
	planeFunction1->SetNormal(tangent[0]*-1, tangent[1] * -1, tangent[2] * -1);
	planeFunction2->SetOrigin(plane2origin);
	planeFunction2->SetNormal(tangent[0]*1, tangent[1] * 1, tangent[2] * 1);

	// create mask array
	vtkSmartPointer<vtkDoubleArray> mask = vtkSmartPointer<vtkDoubleArray>::New();
	mask->SetNumberOfComponents(1);
	mask->SetNumberOfTuples(m_io->GetVoronoiDiagram()->GetNumberOfPoints());
	mask->SetName("Mask");
	mask->FillComponent(0, 0);

	// implicit clip function composed by cylinder, plane 1 and plane 2
	vtkSmartPointer<vtkImplicitBoolean> compositeFunction = vtkSmartPointer<vtkImplicitBoolean>::New();
	compositeFunction->AddFunction(cylinderFunction);
	compositeFunction->AddFunction(planeFunction1);
	compositeFunction->AddFunction(planeFunction2);
	compositeFunction->SetOperationTypeToIntersection();
	compositeFunction->EvaluateFunction(m_io->GetVoronoiDiagram()->GetPoints()->GetData(), mask);

	outputData->DeepCopy(m_io->GetVoronoiDiagram());
	outputData->GetPointData()->AddArray(mask);
	outputData->GetPointData()->SetActiveScalars("Mask");

	vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
	clipper->SetValue(1e-6);
	clipper->SetInsideOut(true);
	clipper->SetInputData(outputData);
	clipper->Update();

	outputData->DeepCopy(clipper->GetOutput());
	outputData->GetPointData()->SetActiveScalars("Radius");
}

void MainWindow::readSurfaceFileComplete()
{
	if (!m_ioWatcher->future().result())
	{
		m_statusLabel->setText("Loading surface file complete");
		this->renderSurface();

		m_renderer->ResetCamera();
		this->updateCenterlineDataTable();
		ui->qvtkWidget->update();

	}
	else
	{
		m_statusLabel->setText("Loading surface file fail");
	}

	m_statusProgressBar->setValue(100);

	this->m_outlineActor->SetVisibility(0);
	this->m_clipperActor->SetVisibility(0);

	// unlock ui
	this->enableUI(true);

	delete m_ioWatcher;
}

void MainWindow::readCenterlineFileComplete()
{
	if (!m_ioWatcher->future().result())
	{
		m_statusLabel->setText("Loading centerline file complete");
		this->renderCenterline();
		this->updateCenterlinesInfoWidget();
		this->renderFirstBifurcationPoint();
		m_preferences->slotUpdateArrays();

		m_renderer->ResetCamera();
		this->updateCenterlineDataTable();
		ui->qvtkWidget->update();
	}
	else
	{
		m_statusLabel->setText("Loading centerline file fail");
	}

	this->updateCenterlinesInfoWidget();

	m_statusProgressBar->setValue(100);

	// unlock ui
	this->enableUI(true);

	delete m_ioWatcher;
}

void MainWindow::readVoronoiFileComplete()
{
	if (!m_ioWatcher->future().result())
	{
		m_statusLabel->setText("Loading Voronoi file complete");
		this->renderVoronoi();

		m_renderer->ResetCamera();
		ui->qvtkWidget->update();
	}
	else
	{
		m_statusLabel->setText("Loading Voronoi file fail");
	}

	m_statusProgressBar->setValue(100);

	// unlock ui
	this->enableUI(true);

	delete m_ioWatcher;
}

void MainWindow::readReconCenterlineComplete()
{
	if (!m_ioWatcher->future().result())
	{
		m_statusLabel->setText("Loading reconstructed centerline file complete");
		this->renderReconCenterline();
		//this->updateCenterlinesInfoWidget();
		m_preferences->slotUpdateArrays();

		m_renderer->ResetCamera();
		//this->updateCenterlineDataTable();
		ui->qvtkWidget->update();
	}
	else
	{
		m_statusLabel->setText("Loading reconstructed centerline file fail");
	}

	//this->updateCenterlinesInfoWidget();

	m_statusProgressBar->setValue(100);

	// unlock ui
	this->enableUI(true);

	delete m_ioWatcher;
}

void MainWindow::readReconSurfaceComplete()
{
	if (!m_ioWatcher->future().result())
	{
		m_statusLabel->setText("Loading reconstructed surface file complete");
		this->renderReconSurface();

		m_renderer->ResetCamera();
		ui->qvtkWidget->update();
	}
	else
	{
		m_statusLabel->setText("Loading reconstructed surface file fail");
	}

	m_statusProgressBar->setValue(100);

	// unlock ui
	this->enableUI(true);

	delete m_ioWatcher;
}
