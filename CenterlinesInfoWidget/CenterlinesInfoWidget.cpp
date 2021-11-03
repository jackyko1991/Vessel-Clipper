#include "CenterlinesInfoWidget.h"

// vtk
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkThreshold.h"
#include "vtkSmartPointer.h"
#include "vtkDataArray.h"
#include "vtkGeometryFilter.h"
#include "vtkDoubleArray.h"
#include "vtkStructuredGrid.h"

// qt
#include "ui_CenterlinesInfoWidget.h"
#include "qcustomplot.h"
#include <QComboBox>

CenterlinesInfoWidget::CenterlinesInfoWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CenterlinesInfoWidget)
{
	ui->setupUi(this);
	this->ui->customPlotCenterlines->xAxis->setLabel("Abscissas");
	this->ui->customPlotCenterlines->yAxis->setLabel("Y");
	this->ui->customPlotCenterlines->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

	// signal slot connections
	connect(ui->comboBoxCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::updateCenterlineIdsComboBox);
	connect(ui->comboBoxCenterlineIds, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);
	connect(ui->comboBoxAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);
	connect(ui->comboBoxYArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);
}

CenterlinesInfoWidget::~CenterlinesInfoWidget()
{

}

void CenterlinesInfoWidget::SetCenterlines(vtkPolyData *centerlines)
{
	m_centerlines = centerlines;

	// block combobox signal
	disconnect(ui->comboBoxCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::updateCenterlineIdsComboBox);
	disconnect(ui->comboBoxAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);
	disconnect(ui->comboBoxYArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);

	ui->comboBoxCenterlineIdsArray->clear();
	ui->comboBoxCenterlineIds->clear();
	ui->comboBoxAbscissasArray->clear();
	ui->comboBoxYArray->clear();

	if (m_centerlines == nullptr)
		return;

	// ============== centerlineids array ==============
	// get all array names from cell data
	int centerlineids_idx = 0;
	for (int i = 0; i < m_centerlines->GetCellData()->GetNumberOfArrays(); i++)
	{
		ui->comboBoxCenterlineIdsArray->addItem(m_centerlines->GetCellData()->GetArrayName(i));
		if (QString(m_centerlines->GetCellData()->GetArrayName(i)).contains("CenterlineIds"))
		{
			centerlineids_idx = i;
		}
	}

	// ============== abscissas and Y array ==============
	// get all array names from point data
	int abscissas_idx = 0;
	for (int i = 0; i < m_centerlines->GetPointData()->GetNumberOfArrays(); i++)
	{
		ui->comboBoxAbscissasArray->addItem(m_centerlines->GetPointData()->GetArrayName(i));
		ui->comboBoxYArray->addItem(m_centerlines->GetPointData()->GetArrayName(i));
		if (QString(m_centerlines->GetPointData()->GetArrayName(i)).contains("Abscissas"))
		{
			abscissas_idx = i;
		}
	}

	// reconnect signal slots
	connect(ui->comboBoxCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::updateCenterlineIdsComboBox);
	connect(ui->comboBoxAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);
	connect(ui->comboBoxYArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);

	// auto select centerlineids and abscissas array
	ui->comboBoxCenterlineIdsArray->setCurrentIndex(centerlineids_idx);
	ui->comboBoxAbscissasArray->setCurrentIndex(abscissas_idx);

	// ============== update plot ==============
	//this->UpdatePlot();
}

void CenterlinesInfoWidget::updateCenterlineIdsComboBox()
{
	ui->comboBoxCenterlineIds->clear();
	ui->comboBoxCenterlineIds->addItem("All");
	for (int i = m_centerlines->GetCellData()->GetArray(ui->comboBoxCenterlineIdsArray->currentIndex())->GetRange()[0];
		i < m_centerlines->GetCellData()->GetArray(ui->comboBoxCenterlineIdsArray->currentIndex())->GetRange()[1];
		i++)
	{
		ui->comboBoxCenterlineIds->addItem(QString::number(i));
	}
}

void CenterlinesInfoWidget::UpdatePlot()
{
	if (m_centerlines == nullptr)
		return;

	// clear old plots
	this->ui->customPlotCenterlines->clearGraphs();
	this->ui->customPlotCenterlines->yAxis->setLabel(ui->comboBoxYArray->currentText());

	vtkSmartPointer<vtkThreshold> thresholdFilter = vtkSmartPointer<vtkThreshold>::New();
	thresholdFilter->SetInputData(m_centerlines);
	thresholdFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, this->ui->comboBoxCenterlineIdsArray->currentText().toStdString().c_str());

	if (ui->comboBoxCenterlineIds->currentText() == QString("All"))
	{
		for (int i = m_centerlines->GetCellData()->GetArray(ui->comboBoxCenterlineIdsArray->currentIndex())->GetRange()[0];
			i < m_centerlines->GetCellData()->GetArray(ui->comboBoxCenterlineIdsArray->currentIndex())->GetRange()[1];
			i++)
		{
			thresholdFilter->ThresholdBetween(i, i);
			thresholdFilter->Update();

			vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
			geomFilter->SetInputData((vtkDataObject*) thresholdFilter->GetOutput());
			geomFilter->Update();

			QVector<double> x;
			QVector<double> y;

			// get abscissas array
			std::string abscissas_array_name = this->ui->comboBoxAbscissasArray->currentText().toStdString();
			std::string y_array_name = this->ui->comboBoxYArray->currentText().toStdString();

			vtkDataArray* abscissasArray = geomFilter->GetOutput()->GetPointData()->GetArray(abscissas_array_name.c_str());
			
			for (int j = 0; j < abscissasArray->GetNumberOfTuples(); j++)
			{
				x.append(abscissasArray->GetTuple(j)[0]);
			}

			vtkDataArray* yArray = geomFilter->GetOutput()->GetPointData()->GetArray(y_array_name.c_str());

			for (int j = 0; j < yArray->GetNumberOfTuples(); j++)
			{
				if (yArray->GetNumberOfComponents() == 3)
				{
					y.append(sqrt(
						std::pow(yArray->GetTuple(j)[0], 2) +
						std::pow(yArray->GetTuple(j)[1], 2) +
						std::pow(yArray->GetTuple(j)[2], 2)));
				}
				else
				{
					y.append(yArray->GetTuple(j)[0]);
				}
			}

			if (this->ui->checkBoxRaw->isChecked())
			{
				this->ui->customPlotCenterlines->addGraph();
				this->ui->customPlotCenterlines->graph(i)->setPen(QPen(Qt::blue));
				this->ui->customPlotCenterlines->graph(i)->setData(x, y);
				this->ui->customPlotCenterlines->graph(i)->rescaleAxes();
			}
			if (this->ui->checkBoxDerivative->isChecked())
			{
				// derivative plot
			}
		}
	}
	//else
	//{
	//	thresholdFilter->ThresholdBetween(
	//		ui->comboBoxCenterlineIds->currentText().toDouble(), 
	//		ui->comboBoxCenterlineIds->currentText().toDouble());
	//	thresholdFilter->Update();

	//	vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
	//	geomFilter->SetInputData((vtkDataObject*)thresholdFilter->GetOutput());
	//	geomFilter->Update();

	//	QVector<double> x;
	//	QVector<double> y;

	//	// get abscissas array
	//	std::string abscissas_array_name = this->ui->comboBoxAbscissasArray->currentText().toStdString();
	//	std::string y_array_name = this->ui->comboBoxYArray->currentText().toStdString();

	//	vtkDataArray* abscissasArray = geomFilter->GetOutput()->GetPointData()->GetArray(abscissas_array_name.c_str());

	//	for (int j = 0; j < abscissasArray->GetNumberOfTuples(); j++)
	//	{
	//		x.append(abscissasArray->GetTuple(j)[0]);
	//	}

	//	vtkDataArray* yArray = geomFilter->GetOutput()->GetPointData()->GetArray(y_array_name.c_str());

	//	for (int j = 0; j < yArray->GetNumberOfTuples(); j++)
	//	{
	//		if (yArray->GetNumberOfComponents() == 3)
	//		{
	//			y.append(sqrt(
	//				std::pow(yArray->GetTuple(j)[0], 2) +
	//				std::pow(yArray->GetTuple(j)[1], 2) +
	//				std::pow(yArray->GetTuple(j)[2], 2)));
	//		}
	//		else
	//		{
	//			y.append(yArray->GetTuple(j)[0]);
	//		}
	//	}

	//	if (this->ui->checkBoxRaw->isChecked())
	//	{
	//		this->ui->customPlotCenterlines->addGraph();
	//		this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setPen(QPen(Qt::blue));
	//		this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setData(x, y);
	//		this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->rescaleAxes();
	//	}
	//	if (this->ui->checkBoxDerivative->isChecked())
	//	{
	//		// derivative plot
	//	}
	//}

	this->ui->customPlotCenterlines->replot();
}