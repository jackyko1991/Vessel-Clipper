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

// vmtk
#include "vtkvmtkPolyDataCenterlines.h"

MainWindow::MainWindow(QMainWindow *parent) : ui(new Ui::MainWindow)
{
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

	// actors
	m_surfaceActor->SetMapper(m_surfaceMapper);
	m_surfaceActor->GetProperty()->SetColor(1, 1, 1);
	m_surfaceActor->GetProperty()->SetOpacity(0.5);

	m_centerlineActor->SetMapper(m_centerlineMapper);
	m_centerlineActor->GetProperty()->SetColor(1, 1, 1);
	
	m_renderer->AddActor(m_surfaceActor);
	m_renderer->AddActor(m_centerlineActor);

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
	connect(ui->pushButtonSaveDomain, &QPushButton::clicked, this, &MainWindow::slotSaveDomain);

	// centerline 
	connect(ui->pushButtonAddCenterlineKeyPoint, &QPushButton::clicked, this, &MainWindow::slotAddCenterlineKeyPoint);
	connect(ui->pushButtonSurfaceCappingCenterline, &QPushButton::clicked, this, &MainWindow::slotSurfaceCapping);
	connect(m_io, SIGNAL(centerlineKeyPointUpdated()), this, SLOT(slotCenterlineKeyPointUpdated()));
	connect(ui->pushButtonRemoveCenterlineKeyPoint, &QPushButton::clicked, this, &MainWindow::slotRemoveCenterlineKeyPoint);
	connect(ui->pushButtonSaveCenterline_3, &QPushButton::clicked, this, &MainWindow::slotSaveCenterline);
	connect(ui->pushButtonComputeCenterline, &QPushButton::clicked, this, &MainWindow::slotComputCenterline);
	connect(ui->tableWidgetCenterlineKeyPoints, &QTableWidget::currentCellChanged, this, &MainWindow::slotCurrentCenterlineKeyPoint);

	// clipping
	connect(ui->pushButtonSaveCenterline, &QPushButton::clicked, this, &MainWindow::slotSaveCenterline);

	// extend

	// domain

	// domain table text change
	connect(ui->tableWidgetDomain, &QTableWidget::itemChanged, this, &MainWindow::slotBoundaryCapTableItemChanged);
	connect(ui->tableWidgetDomain, &QTableWidget::currentCellChanged, this, &MainWindow::slotCurrentBoundaryCap);

	// shortcut, remove for release
	//ui->lineEditSurface->setText("Z:/data/intracranial");
	//ui->lineEditCenterline->setText("Z:/data/intracranial");

	ui->lineEditSurface->setText("Z:/data/intracranial/data_ESASIS_followup/medical/ChanPitChuen/baseline");
	ui->lineEditCenterline->setText("Z:/data/intracranial/data_ESASIS_followup/medical/ChanPitChuen/baseline");
	//ui->lineEditSurface->setText("D:/Projects/Vessel-Clipper/Data");
	//ui->lineEditCenterline->setText("D:/Projects/Vessel-Clipper/Data");
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
	ui->qvtkWidget->update();
}

void MainWindow::slotSpinBoxOpacityChanged()
{
	ui->horizontalSliderOpacity->setValue(ui->doubleSpinBoxOpacity->value()* 100);
	m_surfaceActor->GetProperty()->SetOpacity(ui->doubleSpinBoxOpacity->value());
	ui->qvtkWidget->update();
}

void MainWindow::slotCurrentPickingPoint()
{
	if (ui->tableWidgetCenterline->currentRow() < 0)
		return;
	m_currentPickingSphereSource->SetCenter(m_io->GetCenterline()->GetPoint(ui->tableWidgetCenterline->currentRow()));
	m_currentPickingActor->VisibilityOn();
	ui->qvtkWidget->update();
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
	m_io->GetCenterline()->DeepCopy(m_io->GetOriginalCenterline());
	this->updateCenterlineDataTable();
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
			if (m_io->GetOriginalCenterline()->GetPointData()->GetArray("Radius") != nullptr)
				bc.radius = m_io->GetOriginalCenterline()->GetPointData()->GetArray("Radius")->GetComponent(id, 0);

			// set the tangent
			if (m_io->GetOriginalCenterline()->GetPointData()->GetArray("FrenetTangent") != nullptr)
			{
				QVector<double> tangent(3);
				tangent[0] = m_io->GetOriginalCenterline()->GetPointData()->GetArray("FrenetTangent")->GetComponent(id, 0);
				tangent[1] = m_io->GetOriginalCenterline()->GetPointData()->GetArray("FrenetTangent")->GetComponent(id, 1);
				tangent[2] = m_io->GetOriginalCenterline()->GetPointData()->GetArray("FrenetTangent")->GetComponent(id, 2);

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

void MainWindow::slotComputCenterline()
{
	if (m_io->GetSurface()->GetNumberOfPoints() == 0)
		return;

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

		surface->DeepCopy(appendFilter->GetOutput());
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
	m_centerlineActor->GetProperty()->SetOpacity(ui->doubleSpinBoxOpacity->value());

	ui->qvtkWidget->update();
}

void MainWindow::renderCenterline()
{
	if (!(m_io->GetCenterline()->GetNumberOfCells() > 0 ||
		m_io->GetCenterline()->GetNumberOfPoints() > 0))
	{
		return;
	}

	m_centerlineMapper->SetInputData(m_io->GetCenterline());

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
		vtkDataArray* abscissas = centerline->GetPointData()->GetArray("Abscissas");
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
		vtkDataArray* binormal = centerline->GetPointData()->GetArray("FrenetBinormal");
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
		// normal
		vtkDataArray* normal = centerline->GetPointData()->GetArray("FrenetNormal");
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
		// tangent
		vtkDataArray* tangent = centerline->GetPointData()->GetArray("FrenetTangent");
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
		// radius
		vtkDataArray* radius = centerline->GetPointData()->GetArray("Radius");
		if (radius != nullptr)
		{
			ui->tableWidgetCenterline->setItem(
				ui->tableWidgetCenterline->rowCount() - 1,
				7,
				new QTableWidgetItem(QString::number(radius->GetComponent(i, 0))));
		}

		// disable edit function
		for (int j=0;j<8;j++)
			ui->tableWidgetCenterline->item(ui->tableWidgetCenterline->rowCount() - 1, j)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		
		// set row color
		if (ui->tableWidgetCenterline->item(ui->tableWidgetCenterline->rowCount() - 1, 2)->text().toDouble() < -1.*ui->doubleSpinBoxProximalBound->value() ||
			ui->tableWidgetCenterline->item(ui->tableWidgetCenterline->rowCount() - 1, 2)->text().toDouble() > ui->doubleSpinBoxDistalBound->value())
		{
			for (int j = 0; j<8; j++)
				ui->tableWidgetCenterline->item(ui->tableWidgetCenterline->rowCount() - 1, j)->setData(Qt::BackgroundRole, QColor(255, 110, 110));
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
		transform->RotateY(-90);
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
		//std::cout << "slotCurrentCenterlineKeyPoint: "
		//	<< ui->tableWidgetCenterlineKeyPoints->currentRow() << " " <<
		//	point[0] << ", " << point[1] << ", " << point[2] << std::endl;

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
		this->renderFirstBifurcationPoint();

		m_renderer->ResetCamera();
		this->updateCenterlineDataTable();
		ui->qvtkWidget->update();
	}
	else
	{
		m_statusLabel->setText("Loading centerline file fail");
	}

	m_statusProgressBar->setValue(100);

	// unlock ui
	this->enableUI(true);

	delete m_ioWatcher;
}