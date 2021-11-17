#include "measurements.h"

#include "ui_measurements.h"

Measurements::Measurements(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Measurements)
{
	ui->setupUi(this);

}

Measurements::~Measurements()
{

}