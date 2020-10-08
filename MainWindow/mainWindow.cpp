#include "mainWindow.h"

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

MainWindow::MainWindow(QMainWindow *parent) : ui(new Ui::MainWindow)
{
	vtkObject::GlobalWarningDisplayOff();
	ui->setupUi(this);

	// io object
	m_io = new IO;

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

	// qvtk widget start
	ui->qvtkWidget->GetRenderWindow()->AddRenderer(m_renderer);

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
	connect(ui->pushButtonSaveCenterline, &QPushButton::clicked, this, &MainWindow::slotSaveCenterline);

	// shortcut, remove for release
	ui->lineEditSurface->setText("Z:/data/intracranial/data_ESASIS_followup/medical/ChanPitChuen/baseline");
	ui->lineEditCenterline->setText("Z:/data/intracranial/data_ESASIS_followup/medical/ChanPitChuen/baseline");
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
	connect(m_ioWatcher, SIGNAL(finished()), this, SLOT(readFileComplete()));

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
	connect(m_ioWatcher, SIGNAL(finished()), this, SLOT(readFileComplete()));

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
		return;

	m_clppingPointId = currentPickingId;

	// centerline tangent
	double* tangent = m_io->GetCenterline()->GetPointData()->GetArray("FrenetTangent")->GetTuple(currentPickingId);

	m_clipTransform->Identity();
	m_clipTransform->Translate(m_io->GetCenterline()->GetPoint(currentPickingId));
	m_clipTransform->RotateWXYZ(ui->doubleSpinBoxRotate->value(), tangent[0], tangent[1], tangent[2]);
	double w = atan(sqrt(pow(tangent[0], 2) + pow(tangent[1], 2)) / tangent[2]) * 180 / 3.14;
	m_clipTransform->RotateWXYZ(w, -tangent[1], tangent[0], 0);
	m_clipTransform->Scale(ui->doubleSpinBoxSize->value(), ui->doubleSpinBoxSize->value(), ui->doubleSpinBoxThickness->value());

	vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	transformFilter->SetTransform(m_clipTransform);

	if (ui->comboBoxClipperStyle->currentText() == "Box")
	{
		vtkSmartPointer<vtkCubeSource> clipBox = vtkSmartPointer<vtkCubeSource>::New();
		transformFilter->SetInputConnection(clipBox->GetOutputPort());
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
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Surface File"), ui->lineEditSurface->text(), tr("Surface Files (*.stl *.vtk *.vtp)"));

	if (fileName.isNull())
		return;

	m_io->WriteSurface(fileName);
}

void MainWindow::slotSaveCenterline()
{
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Centerline File"), ui->lineEditCenterline->text(), tr("Centerline Files (*.vtk *.vtp)"));

	if (fileName.isNull())
		return;

	m_io->WriteSurface(fileName);
}

void MainWindow::slotExit()
{
	qApp->exit();
}

void MainWindow::enableUI(bool enable)
{
	ui->pushButtonSurface->setEnabled(enable);
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

void MainWindow::createClipper()
{
	m_clipperActor->SetMapper(m_clipperMapper);
	m_clipperActor->GetProperty()->SetColor(161 * 1.0 / 161, 255 * 1.0 / 161, 20 * 1.0 / 255);
	m_clipperActor->GetProperty()->SetOpacity(1.0);
	m_clipperActor->VisibilityOff();

	m_renderer->AddActor(m_clipperActor);
}

void MainWindow::clip(int direction)
{
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

	m_clipTransform->Print(std::cout);

	// clip surface
	clipper->SetInputData(m_io->GetSurface());
	clipper->Update();
	connectedFilter->SetInputData(clipper->GetOutput());
	connectedFilter->Update();
	m_io->GetSurface()->DeepCopy(connectedFilter->GetOutput());

	// hold bifurcation point before centerline clip
	QVector<double> bif_point(3);
	bif_point[0] = m_io->GetCenterlineFirstBifurcationPoint()[0];
	bif_point[1] = m_io->GetCenterlineFirstBifurcationPoint()[1];
	bif_point[2] = m_io->GetCenterlineFirstBifurcationPoint()[2];
	
	std::cout << "before clip: " <<
		bif_point[0] << ", " <<
		bif_point[1] << ", " <<
		bif_point[2] << ", " <<
		std::endl;

	// clip centerline
	clipper->SetInputData(m_io->GetCenterline());
	clipper->Update();
	connectedFilter->SetInputData(clipper->GetOutput());
	connectedFilter->Update();
	m_io->GetCenterline()->DeepCopy(connectedFilter->GetOutput());

	std::cout << "after clip: " <<
		bif_point[0] << ", " <<
		bif_point[1] << ", " <<
		bif_point[2] << ", " <<
		std::endl;

	m_io->SetCenterlineFirstBifurcationPoint(bif_point);
	this->renderFirstBifurcationPoint();

	this->updateCenterlineDataTable();
	ui->qvtkWidget->update();
}

void MainWindow::readFileComplete()
{
	if (!m_ioWatcher->future().result())
	{
		m_statusLabel->setText("Loading file complete");
		this->renderSurface();
		this->renderCenterline();
		this->renderFirstBifurcationPoint();

		m_renderer->ResetCamera();
		this->updateCenterlineDataTable();
	}
	else
	{
		m_statusLabel->setText("Loading file fail");
	}

	m_statusProgressBar->setValue(100);

	// unlock ui
	this->enableUI(true);

	delete m_ioWatcher;
}