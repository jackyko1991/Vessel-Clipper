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

private slots :
	void updateCenterlineIdsComboBox();

private:
	Ui::CenterlinesInfoWidget *ui;
	vtkPolyData* m_centerlines = nullptr;
};

#endif