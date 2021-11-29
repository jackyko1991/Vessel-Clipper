#include "measurements.h"
#include "ui_measurements.h"

#include "vtkPolyData.h"
#include "vtkCellData.h"

#include "io.h"
#include "preferences.h"

Measurements::Measurements(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Measurements)
{
	ui->setupUi(this);

	// connections
	connect(ui->pushButtonUpdate, &QPushButton::clicked, this, &Measurements::slotUpdate);
	connect(ui->pushButtonClose, &QPushButton::clicked, this, &Measurements::slotClose);
	connect(ui->pushButtonAddAllRecon, &QPushButton::clicked, this, &Measurements::slotReconAddAll);
	connect(ui->pushButtonAddRecon, &QPushButton::clicked, this, &Measurements::slotReconAddCurrent);
	connect(ui->pushButtonRemoveAllRecon, &QPushButton::clicked, this, &Measurements::slotReconRemoveAll);
	connect(ui->pushButtonRemoveRecon, &QPushButton::clicked, this, &Measurements::slotReconRemoveCurrent);


}

Measurements::~Measurements()
{

}

void Measurements::SetPreference(Preferences *preferences)
{
	m_preferences = preferences;
}

void Measurements::SetDataIo(IO *io)
{
	m_io = io;
}

void Measurements::slotClose()
{
	this->close();
}

void Measurements::slotCenterlineConfigUpdate()
{
	if (m_preferences == nullptr)
		return;

	ui->labelCenterlineIdsArray->setText(m_preferences->GetCenterlineIdsArrayName());
	ui->labelRadiusArray->setText(m_preferences->GetRadiusArrayName());

	// centerlineids table update
	ui->listWidgetCenterlineIdsPending->clear();
	ui->listWidgetCenterlineIdsRecon->clear();

	vtkDataArray* centerlineIdsArray = m_io->GetCenterline()->GetCellData()->GetArray(m_preferences->GetCenterlineIdsArrayName().toStdString().c_str());
	if (centerlineIdsArray == nullptr)
		return;

	for (int i = centerlineIdsArray->GetRange()[0]; i <= centerlineIdsArray->GetRange()[1]; i++)
		ui->listWidgetCenterlineIdsPending->addItem(QString::number(i));
}

void Measurements::slotUpdate()
{
	// check data ok

}

void Measurements::slotReconAddAll()
{
	while (ui->listWidgetCenterlineIdsPending->count() > 0)
	{
		QListWidgetItem* currentItem = ui->listWidgetCenterlineIdsPending->item(0);
		ui->listWidgetCenterlineIdsRecon->addItem(currentItem->text());
		ui->listWidgetCenterlineIdsPending->takeItem(ui->listWidgetCenterlineIdsPending->row(currentItem));
	}
}

void Measurements::slotReconAddCurrent()
{
	QListWidgetItem* currentItem = ui->listWidgetCenterlineIdsPending->currentItem();

	if (currentItem == nullptr)
		return;

	ui->listWidgetCenterlineIdsRecon->addItem(currentItem->text());
	ui->listWidgetCenterlineIdsPending->takeItem(ui->listWidgetCenterlineIdsPending->row(currentItem));
}

void Measurements::slotReconRemoveAll()
{
	while (ui->listWidgetCenterlineIdsRecon->count() > 0)
	{
		QListWidgetItem* currentItem = ui->listWidgetCenterlineIdsRecon->item(0);
		ui->listWidgetCenterlineIdsPending->addItem(currentItem->text());
		ui->listWidgetCenterlineIdsRecon->takeItem(ui->listWidgetCenterlineIdsRecon->row(currentItem));
	}
}

void Measurements::slotReconRemoveCurrent()
{
	QListWidgetItem* currentItem = ui->listWidgetCenterlineIdsRecon->currentItem();

	if (currentItem == nullptr)
		return;
	if (currentItem == nullptr)
		return;
	ui->listWidgetCenterlineIdsPending->addItem(currentItem->text());
	ui->listWidgetCenterlineIdsRecon->takeItem(ui->listWidgetCenterlineIdsRecon->row(currentItem));
}