#ifndef __MEASUREMENTS_H__
#define __MEASUREMENTS_H__

// qt
#include <QWidget>

namespace Ui {
	class Measurements;
}

class Measurements : public QWidget
{
	Q_OBJECT

public:
	Measurements(QWidget* parent = nullptr);
	~Measurements();

public slots:

private slots :

private:
	Ui::Measurements *ui;
};

#endif