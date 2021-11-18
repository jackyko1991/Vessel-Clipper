#ifndef __MEASUREMENTS_H__
#define __MEASUREMENTS_H__

// qt
#include <QWidget>

namespace Ui {
	class Measurements;
}

class IO;

class Measurements : public QWidget
{
	Q_OBJECT

public:
	Measurements(QWidget* parent = nullptr);
	~Measurements();
	void SetDataIo(IO*);

public slots:
	void slotUpdate();
	void slotClose();

private slots :

private:
	Ui::Measurements *ui;
	IO* m_io = nullptr;
};

#endif