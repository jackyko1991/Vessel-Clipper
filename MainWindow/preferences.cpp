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
	// centerline
	connect(this->ui->comboBoxCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxFrenetBinormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxFrenetNormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxFrenetTangentArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxRadiusArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxParallelTransportNormalsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);

	// recon centerline
	connect(this->ui->comboBoxReconCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxReconAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxReconFrenetBinormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxReconFrenetNormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxReconFrenetTangentArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxReconRadiusArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxReconParallelTransportNormalsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
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

QString Preferences::GetParallelTransportNormalsName()
{
	return ui->comboBoxParallelTransportNormalsArray->currentText();
}

void Preferences::SetReconCenterlineIdsArrayName(QString name)
{
	int idx = ui->comboBoxReconCenterlineIdsArray->findText(name);
	ui->comboBoxCenterlineIdsArray->setCurrentIndex(idx);
}

void Preferences::SetReconAbscissasArrayName(QString name)
{
	std::cout << name.toStdString() << std::endl;

	int idx = ui->comboBoxReconAbscissasArray->findText(name);
	ui->comboBoxReconAbscissasArray->setCurrentIndex(idx);
}

QString Preferences::GetReconCenterlineIdsArrayName()
{
	return ui->comboBoxReconCenterlineIdsArray->currentText();
}

QString Preferences::GetReconAbscissasArrayName()
{
	return ui->comboBoxReconAbscissasArray->currentText();
}

QString Preferences::GetReconFrenetTangentArrayName()
{
	return ui->comboBoxReconFrenetTangentArray->currentText();
}

QString Preferences::GetReconFrenetNormalArrayName()
{
	return ui->comboBoxReconFrenetNormalArray->currentText();
}

QString Preferences::GetReconFrenetBinormalArrayName()
{
	return ui->comboBoxReconFrenetBinormalArray->currentText();
}

QString Preferences::GetReconRadiusArrayName()
{
	return ui->comboBoxReconRadiusArray->currentText();
}

QString Preferences::GetReconParallelTransportNormalsName()
{
	return ui->comboBoxReconParallelTransportNormalsArray->currentText();
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
	disconnect(this->ui->comboBoxParallelTransportNormalsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);

	disconnect(this->ui->comboBoxReconCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	disconnect(this->ui->comboBoxReconAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	disconnect(this->ui->comboBoxReconFrenetBinormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	disconnect(this->ui->comboBoxReconFrenetNormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	disconnect(this->ui->comboBoxReconFrenetTangentArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	disconnect(this->ui->comboBoxReconRadiusArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	disconnect(this->ui->comboBoxReconParallelTransportNormalsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);

	// remove all combobox items
	ui->comboBoxCenterlineIdsArray->clear();
	ui->comboBoxAbscissasArray->clear();
	ui->comboBoxFrenetTangentArray->clear();
	ui->comboBoxFrenetNormalArray->clear();
	ui->comboBoxFrenetBinormalArray->clear();
	ui->comboBoxRadiusArray->clear();
	ui->comboBoxParallelTransportNormalsArray->clear();

	ui->comboBoxReconCenterlineIdsArray->clear();
	ui->comboBoxReconAbscissasArray->clear();
	ui->comboBoxReconFrenetTangentArray->clear();
	ui->comboBoxReconFrenetNormalArray->clear();
	ui->comboBoxReconFrenetBinormalArray->clear();
	ui->comboBoxReconRadiusArray->clear();
	ui->comboBoxReconParallelTransportNormalsArray->clear();

	int centerlineidsIdx = 0;
	int abscissasIdx = 0;
	int frenetTangentIdx = 0;
	int frenetNormalIdx = 0;
	int frenetBinormalIdx = 0;
	int radiusIdx = 0;
	int parallelTransportNomralsIdx = 0;

	int centerlineidsIdxRecon = 0;
	int abscissasIdxRecon = 0;
	int frenetTangentIdxRecon = 0;
	int frenetNormalIdxRecon = 0;
	int frenetBinormalIdxRecon = 0;
	int radiusIdxRecon = 0;
	int parallelTransportNomralsIdxRecon = 0;

	ui->comboBoxCenterlineIdsArray->addItem("");
	ui->comboBoxAbscissasArray->addItem("");
	ui->comboBoxFrenetTangentArray->addItem("");
	ui->comboBoxFrenetNormalArray->addItem("");
	ui->comboBoxFrenetBinormalArray->addItem("");
	ui->comboBoxRadiusArray->addItem("");
	ui->comboBoxParallelTransportNormalsArray->addItem("");

	ui->comboBoxReconCenterlineIdsArray->addItem("");
	ui->comboBoxReconAbscissasArray->addItem("");
	ui->comboBoxReconFrenetTangentArray->addItem("");
	ui->comboBoxReconFrenetNormalArray->addItem("");
	ui->comboBoxReconFrenetBinormalArray->addItem("");
	ui->comboBoxReconRadiusArray->addItem("");
	ui->comboBoxReconParallelTransportNormalsArray->addItem("");

	for (int i = 0; i < m_io->GetCenterline()->GetCellData()->GetNumberOfArrays(); i++)
	{
		ui->comboBoxCenterlineIdsArray->addItem(m_io->GetCenterline()->GetCellData()->GetArrayName(i));
		
		if (QString(m_io->GetCenterline()->GetCellData()->GetArrayName(i)).contains("CenterlineIds"))
			centerlineidsIdx = i+1;
	}

	for (int i = 0; i < m_io->GetReconstructedCenterline()->GetCellData()->GetNumberOfArrays(); i++)
	{
		ui->comboBoxReconCenterlineIdsArray->addItem(m_io->GetReconstructedCenterline()->GetCellData()->GetArrayName(i));

		if (QString(m_io->GetReconstructedCenterline()->GetCellData()->GetArrayName(i)).contains("CenterlineIds"))
			centerlineidsIdxRecon = i + 1;
	}

	for (int i = 0; i< m_io->GetCenterline()->GetPointData()->GetNumberOfArrays(); i++)
	{
		ui->comboBoxAbscissasArray->addItem(m_io->GetCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxFrenetTangentArray->addItem(m_io->GetCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxFrenetNormalArray->addItem(m_io->GetCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxFrenetBinormalArray->addItem(m_io->GetCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxRadiusArray->addItem(m_io->GetCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxParallelTransportNormalsArray->addItem(m_io->GetCenterline()->GetPointData()->GetArrayName(i));

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
		if (QString(m_io->GetCenterline()->GetPointData()->GetArrayName(i)).contains("ParallelTransportNormals"))
			parallelTransportNomralsIdx = i + 1;
	}

	for (int i = 0; i< m_io->GetReconstructedCenterline()->GetPointData()->GetNumberOfArrays(); i++)
	{
		ui->comboBoxReconAbscissasArray->addItem(m_io->GetReconstructedCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxReconFrenetTangentArray->addItem(m_io->GetReconstructedCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxReconFrenetNormalArray->addItem(m_io->GetReconstructedCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxReconFrenetBinormalArray->addItem(m_io->GetReconstructedCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxReconRadiusArray->addItem(m_io->GetReconstructedCenterline()->GetPointData()->GetArrayName(i));
		ui->comboBoxReconParallelTransportNormalsArray->addItem(m_io->GetReconstructedCenterline()->GetPointData()->GetArrayName(i));

		if (QString(m_io->GetReconstructedCenterline()->GetPointData()->GetArrayName(i)).contains("Abscissas"))
			abscissasIdxRecon = i + 1;
		if (QString(m_io->GetReconstructedCenterline()->GetPointData()->GetArrayName(i)).contains("FrenetTangent"))
			frenetTangentIdxRecon = i + 1;
		if (QString(m_io->GetReconstructedCenterline()->GetPointData()->GetArrayName(i)).contains("FrenetNormal"))
			frenetNormalIdxRecon = i + 1;
		if (QString(m_io->GetReconstructedCenterline()->GetPointData()->GetArrayName(i)).contains("FrenetBinormal"))
			frenetBinormalIdxRecon = i + 1;
		if (QString(m_io->GetReconstructedCenterline()->GetPointData()->GetArrayName(i)).contains("Radius"))
			radiusIdxRecon = i + 1;
		if (QString(m_io->GetReconstructedCenterline()->GetPointData()->GetArrayName(i)).contains("ParallelTransportNormals"))
			parallelTransportNomralsIdxRecon = i + 1;
	}

	ui->comboBoxCenterlineIdsArray->setCurrentIndex(centerlineidsIdx);
	ui->comboBoxAbscissasArray->setCurrentIndex(abscissasIdx);
	ui->comboBoxFrenetTangentArray->setCurrentIndex(frenetTangentIdx );
	ui->comboBoxFrenetNormalArray->setCurrentIndex(frenetNormalIdx);
	ui->comboBoxFrenetBinormalArray->setCurrentIndex(frenetBinormalIdx);
	ui->comboBoxRadiusArray->setCurrentIndex(radiusIdx);
	ui->comboBoxParallelTransportNormalsArray->setCurrentIndex(parallelTransportNomralsIdx);

	ui->comboBoxReconCenterlineIdsArray->setCurrentIndex(centerlineidsIdxRecon);
	ui->comboBoxReconAbscissasArray->setCurrentIndex(abscissasIdxRecon);
	ui->comboBoxReconFrenetTangentArray->setCurrentIndex(frenetTangentIdxRecon);
	ui->comboBoxReconFrenetNormalArray->setCurrentIndex(frenetNormalIdxRecon);
	ui->comboBoxReconFrenetBinormalArray->setCurrentIndex(frenetBinormalIdxRecon);
	ui->comboBoxReconRadiusArray->setCurrentIndex(radiusIdxRecon);
	ui->comboBoxReconParallelTransportNormalsArray->setCurrentIndex(parallelTransportNomralsIdxRecon);

	// reconnect signal slots
	connect(this->ui->comboBoxCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxFrenetBinormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxFrenetNormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxFrenetTangentArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxRadiusArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxParallelTransportNormalsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);

	connect(this->ui->comboBoxReconCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxReconAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxReconFrenetBinormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxReconFrenetNormalArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxReconFrenetTangentArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxReconRadiusArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);
	connect(this->ui->comboBoxReconParallelTransportNormalsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Preferences::slotConfigurationSet);

	this->slotConfigurationSet();
}