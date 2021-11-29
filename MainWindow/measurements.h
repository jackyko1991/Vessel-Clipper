#ifndef __MEASUREMENTS_H__
#define __MEASUREMENTS_H__

// qt
#include <QWidget>

namespace Ui {
	class Measurements;
}

class IO;
class Preferences;

class Measurements : public QWidget
{
	Q_OBJECT

public:
	Measurements(QWidget* parent = nullptr);
	~Measurements();
	void SetPreference(Preferences*);
	void SetDataIo(IO*);

public slots:
	void slotUpdate();
	void slotClose();
	void slotCenterlineConfigUpdate();
	void slotReconAddAll();
	void slotReconAddCurrent();
	void slotReconRemoveAll();
	void slotReconRemoveCurrent();

private slots :

private:
	Ui::Measurements *ui;
	IO* m_io = nullptr;
	Preferences* m_preferences = nullptr;
};

#endif