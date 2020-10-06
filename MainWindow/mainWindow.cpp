#include "mainWindow.h"

// qt
#include "QPushButton"
#include "QFileDialog"
#include "QProgressBar"
#include "QStatusBar"
#include "QtConcurrent"
#include <QtConcurrent/qtconcurrentrun.h>

MainWindow::MainWindow(QMainWindow *parent) : ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// io object
	m_io = new IO;

	// create objects for the label and progress bar
	statusLabel = new QLabel(this);
	statusProgressBar = new QProgressBar(this);

	// set text for the label
	statusLabel->setText("Ready");

	// make progress bar text invisible
	statusProgressBar->setTextVisible(false);

	// add the two controls to the status bar
	ui->statusBar->addPermanentWidget(statusLabel);
	ui->statusBar->addPermanentWidget(statusProgressBar, 1);

	connect(ui->pushButtonSurface, &QPushButton::clicked, this, &MainWindow::slotBrowseSurface);
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

	ui->statusBar->showMessage("Loading surface file...");
	statusProgressBar->setValue(51);

	this->enableUI(false);
	this->m_io->SetSurfacePath(fileName);

	// Instantiate the watcher to unlock
	connect(m_ioWatcher, SIGNAL(finished()), this, SLOT(readFileComplete()));

	// use QtConcurrent to run the read file on a new thread;
	QFuture<bool> future = QtConcurrent::run(this->m_io, &IO::ReadSurface);

	// future watchers
	m_ioWatcher = new QFutureWatcher<bool>;
	m_ioWatcher->setFuture(future);
}

void MainWindow::slotExit()
{
	qApp->exit();
}

void MainWindow::enableUI(bool enable)
{
	ui->pushButtonSurface->setEnabled(enable);
}

void MainWindow::readFileComplete()
{
	if (!m_ioWatcher->future().result())
	{
		ui->statusBar->showMessage("Loading file complete");
		//this->renderSource();
		//this->renderTarget();
		//m_renderer->ResetCamera();
	}
	else
	{
		ui->statusBar->showMessage("Loading file fail");
	}

	
	statusProgressBar->setValue(100);

	// unlock ui
	this->enableUI(true);

	delete m_ioWatcher;
}