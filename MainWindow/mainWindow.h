#ifndef MainWindow_H
#define MainWindow_H

#include "ui_mainWindow.h"
#include "io.h"

// qt
#include <QMainWindow>
#include <QFutureWatcher>

// vtk
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>

namespace Ui {
	class MainWindow;
}

class QProgressBar;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:

	explicit MainWindow(QMainWindow *parent = nullptr);
	~MainWindow();

public slots:
	virtual void slotExit();
	void slotBrowseSurface();
	void slotBrowseCenterline();

private slots:
	void readFileComplete();

private:
	Ui::MainWindow *ui;
	void enableUI(bool);
	void renderSurface();
	void renderCenterline();
	void updateCenterlineDataTable();

	IO* m_io;

	// add references to Label and ProgressBar
	QLabel *m_statusLabel;
	QProgressBar *m_statusProgressBar;
	QFutureWatcher<bool>* m_ioWatcher;
	QFutureWatcher<void>* m_executeWatcher;
	QMutex* m_mutex;

	vtkSmartPointer<vtkRenderer> m_renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkPolyDataMapper> m_surfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper> m_centerlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> m_surfaceActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> m_centerlineActor = vtkSmartPointer<vtkActor>::New();
};

#endif