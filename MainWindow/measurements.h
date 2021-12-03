#ifndef __MEASUREMENTS_H__
#define __MEASUREMENTS_H__

// qt
#include <QWidget>

namespace Ui {
	class Measurements;
}

class IO;
class Preferences;
class CenterlinesInfoWidget;
class vtkPolyData;

enum DataType{
	Original,
	Recon
};

enum Section {
	NTN,
	FWHM
};

class Measurements : public QWidget
{
	Q_OBJECT

public:
	Measurements(QWidget* parent = nullptr);
	~Measurements();
	void SetPreference(Preferences*);
	void SetDataIo(IO*);
	void SetCenterlinesInfoWidget(CenterlinesInfoWidget*);

public slots:
	void slotUpdate();
	void slotClose();
	void slotCenterlineConfigUpdate();
	void slotUpdatePointDataArray();
	//void slotReconAddAll();
	//void slotReconAddCurrent();
	//void slotReconRemoveAll();
	//void slotReconRemoveCurrent();

private slots :

private:
	Ui::Measurements *ui;
	IO* m_io = nullptr;
	Preferences* m_preferences = nullptr;
	CenterlinesInfoWidget* m_centerlinesInfoWidget = nullptr;
	QString m_currentPointDataArrayName;

	void clipCenterline(double* proximalPt, double* distalPt, DataType dataType, vtkPolyData* clippedCenterline);
	void clipSurface(vtkPolyData* surface, vtkPolyData* clippedCenterline, DataType dataType, vtkPolyData* clippedSurface);
	void resetResults();

};

#endif