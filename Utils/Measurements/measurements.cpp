#include "measurements.h"
#include "ui_measurements.h"

#include "io.h"

Measurements::Measurements(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Measurements)
{
	ui->setupUi(this);

	// connections
	connect(ui->pushButtonUpdate, &QPushButton::clicked, this, &Measurements::slotUpdate);
	connect(ui->pushButtonClose, &QPushButton::clicked, this, &Measurements::slotClose);

}

Measurements::~Measurements()
{

}

void Measurements::SetDataIo(IO *io)
{
	m_io = io;
}

void Measurements::slotClose()
{
	this->close();
}

void Measurements::slotUpdate()
{
	// check data ok

}