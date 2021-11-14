#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

// qt
#include <QWidget>

// me 
#include "io.h"

namespace Ui {
	class Preferences;
}

class Preferences : public QWidget
{
	Q_OBJECT

public:
	Preferences(QWidget* parent = nullptr);
	~Preferences();
	void SetIO(IO*);
	void SetCurrentTab(int);
	void SetCenterlineIdsArrayName(QString);
	void SetAbscissasArrayName(QString);
	QString GetCenterlineIdsArrayName();
	QString GetAbscissasArrayName();
	QString GetFrenetTangentArrayName();
	QString GetFrenetNormalArrayName();
	QString GetFrenetBinormalArrayName();
	QString GetRadiusArrayName();
	QString GetParallelTransportNormalsName();

public slots:
	void slotUpdateArrays();
	void slotConfigurationSet();

signals:
	void signalComboBoxesUpdated();

private:
	Ui::Preferences *ui;
	IO* m_io = nullptr;
};

#endif