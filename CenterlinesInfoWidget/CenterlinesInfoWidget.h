#ifndef __CENTERLINESINFOWIDGET_H__
#define __CENTERLINESINFOWIDGET_H__

// qt
#include <QWidget>

// vtk
#include "vtkPolyData.h"

namespace Ui {
	class CenterlinesInfoWidget;
}

class CenterlinesInfoWidget : public QWidget
{
	Q_OBJECT

public:
	CenterlinesInfoWidget(QWidget* parent = nullptr);
	~CenterlinesInfoWidget();
	void SetCenterlines(vtkPolyData*);

public slots:
	void UpdatePlot();
	void SetCursorPosition(double x, double y, double z);
	void SetStenosisPoint();
	void SetProximalNormalPoint();
	void SetDistalNormalPoint();
	QVector<double> GetStenosisPoint();
	QVector<double> GetProximalNormalPoint();
	QVector<double> GetDistalNormalPoint();

private slots :
	void updateCenterlineIdsComboBox();

signals:
	void signalSetStenosis();
	void signalSetProximalNormal();
	void signalSetDistalNormal();
	void signalSetCenterlineIdsArray(QString);

private:
	Ui::CenterlinesInfoWidget *ui;
	vtkPolyData* m_centerlines = nullptr;
	bool m_resetPlotRange = true;

	QVector<double> m_cursorPosition;
	QVector<double> m_stenosisPoint;
	QVector<double> m_proximalNormalPoint;
	QVector<double> m_distalNormalPoint;
};

#endif