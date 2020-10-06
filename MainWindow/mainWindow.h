#ifndef MainWindow_H
#define MainWindow_H

#include <QMainWindow>
#include <QFutureWatcher>
#include "ui_mainWindow.h"
#include "io.h"

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

private slots:
	void readFileComplete();

private:
	Ui::MainWindow *ui;
	void enableUI(bool);

	IO* m_io;

	// add references to Label and ProgressBar
	QLabel *statusLabel;
	QProgressBar *statusProgressBar;

	QFutureWatcher<bool>* m_ioWatcher;
	QFutureWatcher<void>* m_executeWatcher;
	QMutex* m_mutex;
};

#endif