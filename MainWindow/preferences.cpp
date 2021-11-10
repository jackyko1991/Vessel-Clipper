#include "preferences.h"
#include "ui_preferences.h"

// qt
#include <QComboBox>

// vtk
#include "vtkCellData.h"
#include "vtkPointData.h"

Preferences::Preferences(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Preferences)
{
	ui->setupUi(this);

	// signal slot connection
	connect(this->ui->comboBoxCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxFrenetBinormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxFrenetNormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxFrenetTangentArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxRadiusArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
}

Preferences::~Preferences()
{

}

void Preferences::SetIO(IO *io)
{
	m_io = io;
}

void Preferences::SetCurrentTab(int id)
{
	ui->tabWidget->setCurrentIndex(id);
}

void Preferences::SetCenterlineIdsArrayName(QString name)
{
	int idx = ui->comboBoxCenterlineIdsArray->findText(name);
	ui->comboBoxCenterlineIdsArray->setCurrentIndex(idx);
}

void Preferences::SetAbscissasArrayName(QString name)
{
	std::cout << name.toStdString() << std::endl;

	int idx = ui->comboBoxAbscissasArray->findText(name);
	ui->comboBoxAbscissasArray->setCurrentIndex(idx);
}

QString Preferences::GetCenterlineIdsArrayName()
{
	return ui->comboBoxCenterlineIdsArray->currentText();
}

QString Preferences::GetAbscissasArrayName()
{
	return ui->comboBoxAbscissasArray->currentText();
}

QString Preferences::GetFrenetTangentArrayName()
{
	return ui->comboBoxFrenetTangentArray->currentText();
}

QString Preferences::GetFrenetNormalArrayName()
{
	return ui->comboBoxFrenetNormalArray->currentText();
}

QString Preferences::GetFrenetBinormalArrayName()
{
	return ui->comboBoxFrenetBinormalArray->currentText();
}

QString Preferences::GetRadiusArrayName()
{
	return ui->comboBoxRadiusArray->currentText();
}

void Preferences::slotConfigurationSet()
{
	emit signalComboBoxesUpdated();
}

void Preferences::slotUpdateArrays()
{
	if (m_io == nullptr)
		return;

	// disconnect all comboboxes
	disconnect(this->ui->comboBoxCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	disconnect(this->ui->comboBoxAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	disconnect(this->ui->comboBoxFrenetBinormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	disconnect(this->ui->comboBoxFrenetNormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	disconnect(this->ui->comboBoxFrenetTangentArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	disconnect(this->ui->comboBoxRadiusArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);

	// remove all combobox items
	ui->comboBoxCenterlineIdsArray->clear();
	ui->comboBoxAbscissasArray->clear();
	ui->comboBoxFrenetTangentArray->clear();
	ui->comboBoxFrenetNormalArray->clear();
	ui->comboBoxFrenetBinormalArray->clear();
	ui->comboBoxRadiusArray->clear();

	int centerlineidsIdx = 0;
	int abscissasIdx = 0;
	int frenetTangentIdx = 0;
	int frenetNormalIdx = 0;
	int frenetBinormalIdx = 0;
	int radiusIdx = 0;

	ui->comboBoxCenterlineIdsArray->addItem("");
	ui->comboBoxAbscissasArray->addItem("");
	ui->comboBoxFrenetTangentArray->addItem("");
	ui->comboBoxFrenetNormalArray->addItem("");
	ui->comboBoxFrenetBinormalArray->addItem("");
	ui->comboBoxRadiusArray->addItem("");

	for (int i = 0; i < m_io->GetCenterline()->GetCellData()->GetNumberOfArrays(); i++)
	{
		ui->comboBoxCenterlineIdsArray->addItem(m_io->GetCenterline()->GetCellData()->GetArrayName(i));
		
		if (QString(m_io->GetCenterline()->GetCellData()->GetArrayName(i)).contains("CenterlineIds"))
			centerlineidsIdx = i+1;
	}

	for (int i = 0; i< m_io->GetCenterline()->GetPointData()->GetNumberOfArrays(); i++)
	{
		ui->comboBoxAbscissasArray->addItem(m_io->GetCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxFrenetTangentArray->addItem(m_io->GetCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxFrenetNormalArray->addItem(m_io->GetCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxFrenetBinormalArray->addItem(m_io->GetCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxRadiusArray->addItem(m_io->GetCenterline()->GetPointData()->GetArrayName(i));

		if (QString(m_io->GetCenterline()->GetPointData()->GetArrayName(i)).contains("Abscissas"))
			abscissasIdx = i + 1;
		if (QString(m_io->GetCenterline()->GetPointData()->GetArrayName(i)).contains("FrenetTangent"))
			frenetTangentIdx = i + 1;
		if (QString(m_io->GetCenterline()->GetPointData()->GetArrayName(i)).contains("FrenetNormal"))
			frenetNormalIdx = i + 1;
		if (QString(m_io->GetCenterline()->GetPointData()->GetArrayName(i)).contains("FrenetBinormal"))
			frenetBinormalIdx = i + 1;
		if (QString(m_io->GetCenterline()->GetPointData()->GetArrayName(i)).contains("Radius"))
			radiusIdx = i + 1;
	}

	ui->comboBoxCenterlineIdsArray->setCurrentIndex(centerlineidsIdx);
	ui->comboBoxAbscissasArray->setCurrentIndex(abscissasIdx);
	ui->comboBoxFrenetTangentArray->setCurrentIndex(frenetTangentIdx );
	ui->comboBoxFrenetNormalArray->setCurrentIndex(frenetNormalIdx);
	ui->comboBoxFrenetBinormalArray->setCurrentIndex(frenetBinormalIdx);
	ui->comboBoxRadiusArray->setCurrentIndex(radiusIdx);

	// reconnect signal slots
	connect(this->ui->comboBoxCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxFrenetBinormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxFrenetNormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxFrenetTangentArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxRadiusArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);

	this->slotConfigurationSet();
}