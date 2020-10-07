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

// vtk
#include "QVTKWidget.h"
#include "vtkRenderWindow.h"
#include "vtkProperty.h"

MainWindow::MainWindow(QMainWindow *parent) : ui(new Ui::MainWindow)
{
	vtkObject::GlobalWarningDisplayOff();
	ui->setupUi(this);

	// io object
	m_io = new IO;

	// create objects for the label and progress bar
	m_statusLabel = new QLabel(this);
	m_statusProgressBar = new QProgressBar(this);

	// set text for the label
	m_statusLabel->setText("Ready");

	// make progress bar text invisible
	m_statusProgressBar->setTextVisible(true);

	// add the two controls to the status bar
	ui->statusBar->addPermanentWidget(m_statusLabel);
	ui->statusBar->addPermanentWidget(m_statusProgressBar, 1);

	// qvtk widget start
	ui->qvtkWidget->GetRenderWindow()->AddRenderer(m_renderer);

	m_surfaceActor->SetMapper(m_surfaceMapper);
	m_surfaceActor->GetProperty()->SetColor(1, 1, 1);
	m_surfaceActor->GetProperty()->SetOpacity(0.5);

	m_centerlineActor->SetMapper(m_centerlineMapper);
	m_centerlineActor->GetProperty()->SetColor(1, 1, 1);
	
	m_renderer->AddActor(m_surfaceActor);
	m_renderer->AddActor(m_centerlineActor);

	// signal slot connections
	connect(ui->pushButtonSurface, &QPushButton::clicked, this, &MainWindow::slotBrowseSurface);
	connect(ui->pushButtonCenterline, &QPushButton::clicked, this, &MainWindow::slotBrowseCenterline);
	connect(ui->horizontalSliderOpacity, &QSlider::valueChanged, this, &MainWindow::slotSliderOpacityChanged);
	connect(ui->doubleSpinBoxOpacity, QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,&MainWindow::slotSpinBoxOpacityChanged);

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
		tr("Open Centerline File"), ui->lineEditCenterline->text(), tr("Centerline Files (*.stl *.vtk *.vtp)"));

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

void MainWindow::readFileComplete()
{
	if (!m_ioWatcher->future().result())
	{
		m_statusLabel->setText("Loading file complete");
		this->renderSurface();
		this->renderCenterline();
		m_renderer->ResetCamera();
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