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
#include "vtkKdTreePointLocator.h"
#include "vtkImplicitPolyDataDistance.h"

// qt
#include "ui_CenterlinesInfoWidget.h"
#include "qcustomplot.h"
#include <QComboBox>

#include <limits>

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
	connect(ui->checkBoxRaw, &QCheckBox::stateChanged, this, &CenterlinesInfoWidget::UpdatePlot);
	connect(ui->checkBoxDerivative1, &QCheckBox::stateChanged, this, &CenterlinesInfoWidget::UpdatePlot);
	connect(ui->checkBoxDerivative2, &QCheckBox::stateChanged, this, &CenterlinesInfoWidget::UpdatePlot);

	connect(ui->pushButtonManualLocateStenosis, &QPushButton::clicked, this, &CenterlinesInfoWidget::SetStenosisPoint);
	connect(ui->pushButtonManualLocateProximalNormal, &QPushButton::clicked, this, &CenterlinesInfoWidget::SetProximalNormalPoint);
	connect(ui->pushButtonManualLocateDistalNormal, &QPushButton::clicked, this, &CenterlinesInfoWidget::SetDistalNormalPoint);
}

CenterlinesInfoWidget::~CenterlinesInfoWidget()
{

}

void CenterlinesInfoWidget::SetCenterlines(vtkPolyData *centerlines)
{
	m_centerlines = centerlines;

	// block combobox signal
	disconnect(ui->comboBoxCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::updateCenterlineIdsComboBox);
	disconnect(ui->comboBoxCenterlineIds, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);
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
	connect(ui->comboBoxCenterlineIds, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);
	connect(ui->comboBoxAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);
	connect(ui->comboBoxYArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);

	// auto select centerlineids and abscissas array
	ui->comboBoxCenterlineIdsArray->setCurrentIndex(centerlineids_idx);
	ui->comboBoxAbscissasArray->setCurrentIndex(abscissas_idx);
}

void CenterlinesInfoWidget::SetCursorPosition(double x, double y, double z)
{
	m_cursorPosition.clear();
	m_cursorPosition.append(x);
	m_cursorPosition.append(y);
	m_cursorPosition.append(z);

	this->m_resetPlotRange = false;
	this->UpdatePlot();
	this->m_resetPlotRange = true;
}

void CenterlinesInfoWidget::SetStenosisPoint()
{
	m_stenosisPoint.clear();
	m_stenosisPoint.append(m_cursorPosition[0]);
	m_stenosisPoint.append(m_cursorPosition[1]);
	m_stenosisPoint.append(m_cursorPosition[2]);

	this->m_resetPlotRange = false;
	this->UpdatePlot();
	this->m_resetPlotRange = true;

	emit signalSetStenosis();
}

void CenterlinesInfoWidget::SetProximalNormalPoint()
{
	m_proximalNormalPoint.clear();
	m_proximalNormalPoint.append(m_cursorPosition[0]);
	m_proximalNormalPoint.append(m_cursorPosition[1]);
	m_proximalNormalPoint.append(m_cursorPosition[2]);

	this->m_resetPlotRange = false;
	this->UpdatePlot();
	this->m_resetPlotRange = true;

	emit signalSetProximalNormal();
}

void CenterlinesInfoWidget::SetDistalNormalPoint()
{
	m_distalNormalPoint.clear();
	m_distalNormalPoint.append(m_cursorPosition[0]);
	m_distalNormalPoint.append(m_cursorPosition[1]);
	m_distalNormalPoint.append(m_cursorPosition[2]);

	this->m_resetPlotRange = false;
	this->UpdatePlot();
	this->m_resetPlotRange = true;

	emit signalSetDistalNormal();
}

QVector<double> CenterlinesInfoWidget::GetStenosisPoint()
{
	return m_stenosisPoint;
}

QVector<double> CenterlinesInfoWidget::GetProximalNormalPoint()
{
	return m_proximalNormalPoint;
}

QVector<double> CenterlinesInfoWidget::GetDistalNormalPoint()
{
	return m_distalNormalPoint;
}

void CenterlinesInfoWidget::updateCenterlineIdsComboBox()
{
	// block combobox signal
	disconnect(ui->comboBoxCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::updateCenterlineIdsComboBox);
	disconnect(ui->comboBoxCenterlineIds, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);
	disconnect(ui->comboBoxAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);
	disconnect(ui->comboBoxYArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);

	ui->comboBoxCenterlineIds->clear();
	ui->comboBoxCenterlineIds->addItem("All");

	for (int i = m_centerlines->GetCellData()->GetArray(ui->comboBoxCenterlineIdsArray->currentIndex())->GetRange()[0];
		i < m_centerlines->GetCellData()->GetArray(ui->comboBoxCenterlineIdsArray->currentIndex())->GetRange()[1];
		i++)
	{
		ui->comboBoxCenterlineIds->addItem(QString::number(i));
	}

	// connect combobox signal
	connect(ui->comboBoxCenterlineIdsArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::updateCenterlineIdsComboBox);
	connect(ui->comboBoxCenterlineIds, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);
	connect(ui->comboBoxAbscissasArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);
	connect(ui->comboBoxYArray, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CenterlinesInfoWidget::UpdatePlot);

	emit signalSetCenterlineIdsArray(this->ui->comboBoxCenterlineIdsArray->currentText());
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

	double y_max = std::numeric_limits<double>::max()*-1;
	double y_min = std::numeric_limits<double>::max();

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
				this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount()-1)->setPen(QPen(Qt::blue));
				this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setData(x, y);
				if (m_resetPlotRange)
					this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->rescaleAxes();
			}

			// derivative plot
			QVector<double> y_grad;
			y_grad.append(0);
			for (int i = 1; i < y.length() - 1; i++)
			{
				y_grad.append((y[i + 1] - y[i - 1]) / 2);
			}
			if (y_grad.length() > 1)
				y_grad[0] = y_grad[1];
			y_grad.append(y_grad[y_grad.length() - 1]);
			if (this->ui->checkBoxDerivative1->isChecked())
			{
				this->ui->customPlotCenterlines->addGraph();
				this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setPen(QPen(Qt::green));
				this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setData(x, y_grad);
				if (m_resetPlotRange)
					this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->rescaleAxes();
			}

			QVector<double> y_grad2;
			if (this->ui->checkBoxDerivative2->isChecked())
			{
				y_grad2.append(0);
				for (int i = 1; i < y_grad.length() - 1; i++)
				{
					y_grad2.append((y_grad[i + 1] - y_grad[i - 1]) / 2);
				}
				if (y_grad2.length() > 1)
					y_grad2[0] = y_grad2[1];
				y_grad2.append(y_grad2[y_grad2.length() - 1]);

				this->ui->customPlotCenterlines->addGraph();
				this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setPen(QPen(Qt::cyan));
				this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setData(x, y_grad2);
				if (m_resetPlotRange)
					this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->rescaleAxes();
			}

			// plot range
			for (int j = 0; j < y.length(); j++)
			{
				if (this->ui->checkBoxRaw->isChecked())
				{
					if (y[j] < y_min)
						y_min = y[j];
					if (y[j] > y_max)
						y_max = y[j];
				}
				if (this->ui->checkBoxDerivative1->isChecked())
				{
					if (y_grad[j] < y_min)
						y_min = y_grad[j];
					if (y_grad[j] > y_max)
						y_max = y_grad[j];
				}
				if (this->ui->checkBoxDerivative2->isChecked())
				{
					if (y_grad2[j] < y_min)
						y_min = y_grad2[j];
					if (y_grad2[j] > y_max)
						y_max = y_grad2[j];
				}
			}
		}
	}
	else
	{
		thresholdFilter->ThresholdBetween(
			ui->comboBoxCenterlineIds->currentText().toDouble(), 
			ui->comboBoxCenterlineIds->currentText().toDouble());
		thresholdFilter->Update();

		vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
		geomFilter->SetInputData((vtkDataObject*)thresholdFilter->GetOutput());
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
			this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setPen(QPen(Qt::blue));
			this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setData(x, y);
			if (m_resetPlotRange)
				this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->rescaleAxes();
		}

		// derivative plot
		QVector<double> y_grad;
		y_grad.append(0);
		for (int i = 1; i < y.length() - 1; i++)
		{
			y_grad.append((y[i + 1] - y[i - 1]) / 2);
		}
		if (y_grad.length() > 1)
			y_grad[0] = y_grad[1];
		y_grad.append(y_grad[y_grad.length() - 1]);

		if (this->ui->checkBoxDerivative1->isChecked())
		{
			this->ui->customPlotCenterlines->addGraph();
			this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setPen(QPen(Qt::green));
			this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setData(x, y_grad);
			if (m_resetPlotRange)
				this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->rescaleAxes();
		}

		QVector<double> y_grad2;
		if (this->ui->checkBoxDerivative2->isChecked())
		{
			y_grad2.append(0);
			for (int i = 1; i < y_grad.length() - 1; i++)
			{
				y_grad2.append((y_grad[i + 1] - y_grad[i - 1]) / 2);
			}
			if (y_grad2.length() > 1)
				y_grad2[0] = y_grad2[1];
			y_grad2.append(y_grad2[y_grad2.length() - 1]);

			this->ui->customPlotCenterlines->addGraph();
			this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setPen(QPen(Qt::cyan));
			this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setData(x, y_grad2);
			if (m_resetPlotRange)
				this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->rescaleAxes();
		}

		// plot range
		for (int j = 0; j < y.length(); j++)
		{
			if (this->ui->checkBoxRaw->isChecked())
			{
				if (y[j] < y_min)
					y_min = y[j];
				if (y[j] > y_max)
					y_max = y[j];
			}
			if (this->ui->checkBoxDerivative1->isChecked())
			{
				if (y_grad[j] < y_min)
					y_min = y_grad[j];
				if (y_grad[j] > y_max)
					y_max = y_grad[j];
			}
			if (this->ui->checkBoxDerivative2->isChecked())
			{
				if (y_grad2[j] < y_min)
					y_min = y_grad2[j];
				if (y_grad2[j] > y_max)
					y_max = y_grad2[j];
			}
		}
	}

	// set y axis range
	if (m_resetPlotRange)
	{
		if (y_max < 0)
			y_max = 0;
		if (y_min > 0)
			y_min = 0;
		this->ui->customPlotCenterlines->yAxis->setRange(y_min*1.2, y_max*1.2);
	}

	// cursor
	if (m_cursorPosition.length()==3)
	{
		std::string abscissas_array_name = this->ui->comboBoxAbscissasArray->currentText().toStdString();
		vtkDataArray* abscissasArray = nullptr;

		vtkSmartPointer<vtkKdTreePointLocator> locator = vtkSmartPointer<vtkKdTreePointLocator>::New();

		if (this->ui->comboBoxCenterlineIds->currentText() == QString("All"))
		{
			locator->SetDataSet(m_centerlines);
			abscissasArray = m_centerlines->GetPointData()->GetArray(abscissas_array_name.c_str());
		}
		else
		{
			vtkSmartPointer<vtkThreshold> thresholdFilter = vtkSmartPointer<vtkThreshold>::New();
			thresholdFilter->SetInputData(m_centerlines);
			thresholdFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, this->ui->comboBoxCenterlineIdsArray->currentText().toStdString().c_str());
			thresholdFilter->ThresholdBetween(
				this->ui->comboBoxCenterlineIds->currentText().toDouble(),
				this->ui->comboBoxCenterlineIds->currentText().toDouble());
			thresholdFilter->Update();

			vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
			geomFilter->SetInputData((vtkDataObject*)thresholdFilter->GetOutput());
			geomFilter->Update();

			locator->SetDataSet(geomFilter->GetOutput());
			abscissasArray = geomFilter->GetOutput()->GetPointData()->GetArray(abscissas_array_name.c_str());
		}
		locator->BuildLocator();

		if (abscissasArray == nullptr || abscissasArray->GetNumberOfComponents() != 1)
			return;

		// locate the neareast point
		double cursorPt[3] = { m_cursorPosition[0], m_cursorPosition[1], m_cursorPosition[2] };
		int iD = locator->FindClosestPoint(cursorPt);
		// problem to obtain correct id for "all" case
		double abscissas = abscissasArray->GetTuple(iD)[0];

		QVector<double> x_cursor = { abscissas,abscissas };
		QVector<double> y_cursor = { ui->customPlotCenterlines->yAxis->range().lower*10, ui->customPlotCenterlines->yAxis->range().upper*10 };

		//std::cout << "point: " << m_cursorPosition[0] << ", " << m_cursorPosition[1] << ", " << m_cursorPosition[2] << std::endl;
		//std::cout << "id:" << iD << std::endl;
		//std::cout << "x_cursor: " << x_cursor[0] << ", " << x_cursor[1] << std::endl;
		//std::cout << "y_cursor: "<< y_cursor[0] << ", "<< y_cursor[1] << std::endl;

		// plot 
		this->ui->customPlotCenterlines->addGraph();
		QPen cursorPen;
		cursorPen.setColor(Qt::black);
		cursorPen.setStyle(Qt::DotLine);
		cursorPen.setWidthF(2);
		this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setPen(cursorPen);
		this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setData(x_cursor, y_cursor);
	}

	// stenosis
	if (m_stenosisPoint.length() == 3)
	{
		std::string abscissas_array_name = this->ui->comboBoxAbscissasArray->currentText().toStdString();
		vtkDataArray* abscissasArray = nullptr;

		vtkSmartPointer<vtkKdTreePointLocator> locator = vtkSmartPointer<vtkKdTreePointLocator>::New();

		if (this->ui->comboBoxCenterlineIds->currentText() == QString("All"))
		{
			locator->SetDataSet(m_centerlines);
			abscissasArray = m_centerlines->GetPointData()->GetArray(abscissas_array_name.c_str());
		}
		else
		{
			vtkSmartPointer<vtkThreshold> thresholdFilter = vtkSmartPointer<vtkThreshold>::New();
			thresholdFilter->SetInputData(m_centerlines);
			thresholdFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, this->ui->comboBoxCenterlineIdsArray->currentText().toStdString().c_str());
			thresholdFilter->ThresholdBetween(
				this->ui->comboBoxCenterlineIds->currentText().toDouble(),
				this->ui->comboBoxCenterlineIds->currentText().toDouble());
			thresholdFilter->Update();

			vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
			geomFilter->SetInputData((vtkDataObject*)thresholdFilter->GetOutput());
			geomFilter->Update();

			locator->SetDataSet(geomFilter->GetOutput());
			abscissasArray = geomFilter->GetOutput()->GetPointData()->GetArray(abscissas_array_name.c_str());
		}
		locator->BuildLocator();

		if (abscissasArray == nullptr || abscissasArray->GetNumberOfComponents() != 1)
			return;

		// locate the neareast point
		double cursorPt[3] = { m_stenosisPoint[0], m_stenosisPoint[1], m_stenosisPoint[2] };
		int iD = locator->FindClosestPoint(cursorPt);
		// problem to obtain correct id for "all" case
		double abscissas = abscissasArray->GetTuple(iD)[0];

		QVector<double> x_cursor = { abscissas,abscissas };
		QVector<double> y_cursor = { ui->customPlotCenterlines->yAxis->range().lower * 10, ui->customPlotCenterlines->yAxis->range().upper * 10 };

		//std::cout << "point: " << m_cursorPosition[0] << ", " << m_cursorPosition[1] << ", " << m_cursorPosition[2] << std::endl;
		//std::cout << "id:" << iD << std::endl;
		//std::cout << "x_cursor: " << x_cursor[0] << ", " << x_cursor[1] << std::endl;
		//std::cout << "y_cursor: "<< y_cursor[0] << ", "<< y_cursor[1] << std::endl;

		// plot 
		this->ui->customPlotCenterlines->addGraph();
		QPen cursorPen;
		cursorPen.setColor(QColor(30, 40, 255, 150));
		//cursorPen.setStyle(Qt::DotLine);
		cursorPen.setWidthF(2);
		this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setPen(cursorPen);
		this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setData(x_cursor, y_cursor);
	}

	// proximal normal
	if (m_proximalNormalPoint.length() == 3)
	{
		std::string abscissas_array_name = this->ui->comboBoxAbscissasArray->currentText().toStdString();
		vtkDataArray* abscissasArray = nullptr;

		vtkSmartPointer<vtkKdTreePointLocator> locator = vtkSmartPointer<vtkKdTreePointLocator>::New();

		if (this->ui->comboBoxCenterlineIds->currentText() == QString("All"))
		{
			locator->SetDataSet(m_centerlines);
			abscissasArray = m_centerlines->GetPointData()->GetArray(abscissas_array_name.c_str());
		}
		else
		{
			vtkSmartPointer<vtkThreshold> thresholdFilter = vtkSmartPointer<vtkThreshold>::New();
			thresholdFilter->SetInputData(m_centerlines);
			thresholdFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, this->ui->comboBoxCenterlineIdsArray->currentText().toStdString().c_str());
			thresholdFilter->ThresholdBetween(
				this->ui->comboBoxCenterlineIds->currentText().toDouble(),
				this->ui->comboBoxCenterlineIds->currentText().toDouble());
			thresholdFilter->Update();

			vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
			geomFilter->SetInputData((vtkDataObject*)thresholdFilter->GetOutput());
			geomFilter->Update();

			locator->SetDataSet(geomFilter->GetOutput());
			abscissasArray = geomFilter->GetOutput()->GetPointData()->GetArray(abscissas_array_name.c_str());
		}
		locator->BuildLocator();

		if (abscissasArray == nullptr || abscissasArray->GetNumberOfComponents() != 1)
			return;

		// locate the neareast point
		double cursorPt[3] = { m_proximalNormalPoint[0], m_proximalNormalPoint[1], m_proximalNormalPoint[2] };
		int iD = locator->FindClosestPoint(cursorPt);
		// problem to obtain correct id for "all" case
		double abscissas = abscissasArray->GetTuple(iD)[0];

		QVector<double> x_cursor = { abscissas,abscissas };
		QVector<double> y_cursor = { ui->customPlotCenterlines->yAxis->range().lower * 10, ui->customPlotCenterlines->yAxis->range().upper * 10 };

		//std::cout << "point: " << m_cursorPosition[0] << ", " << m_cursorPosition[1] << ", " << m_cursorPosition[2] << std::endl;
		//std::cout << "id:" << iD << std::endl;
		//std::cout << "x_cursor: " << x_cursor[0] << ", " << x_cursor[1] << std::endl;
		//std::cout << "y_cursor: "<< y_cursor[0] << ", "<< y_cursor[1] << std::endl;

		// plot 
		this->ui->customPlotCenterlines->addGraph();
		QPen cursorPen;
		cursorPen.setColor(Qt::red);
		//cursorPen.setStyle(Qt::DotLine);
		cursorPen.setWidthF(2);
		this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setPen(cursorPen);
		this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setData(x_cursor, y_cursor);
	}

	// distal normal
	if (m_distalNormalPoint.length() == 3)
	{
		std::string abscissas_array_name = this->ui->comboBoxAbscissasArray->currentText().toStdString();
		vtkDataArray* abscissasArray = nullptr;

		vtkSmartPointer<vtkKdTreePointLocator> locator = vtkSmartPointer<vtkKdTreePointLocator>::New();

		if (this->ui->comboBoxCenterlineIds->currentText() == QString("All"))
		{
			locator->SetDataSet(m_centerlines);
			abscissasArray = m_centerlines->GetPointData()->GetArray(abscissas_array_name.c_str());
		}
		else
		{
			vtkSmartPointer<vtkThreshold> thresholdFilter = vtkSmartPointer<vtkThreshold>::New();
			thresholdFilter->SetInputData(m_centerlines);
			thresholdFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, this->ui->comboBoxCenterlineIdsArray->currentText().toStdString().c_str());
			thresholdFilter->ThresholdBetween(
				this->ui->comboBoxCenterlineIds->currentText().toDouble(),
				this->ui->comboBoxCenterlineIds->currentText().toDouble());
			thresholdFilter->Update();

			vtkSmartPointer<vtkGeometryFilter> geomFilter = vtkSmartPointer<vtkGeometryFilter>::New();
			geomFilter->SetInputData((vtkDataObject*)thresholdFilter->GetOutput());
			geomFilter->Update();

			locator->SetDataSet(geomFilter->GetOutput());
			abscissasArray = geomFilter->GetOutput()->GetPointData()->GetArray(abscissas_array_name.c_str());
		}
		locator->BuildLocator();

		if (abscissasArray == nullptr || abscissasArray->GetNumberOfComponents() != 1)
			return;

		// locate the neareast point
		double cursorPt[3] = { m_distalNormalPoint[0], m_distalNormalPoint[1], m_distalNormalPoint[2] };
		int iD = locator->FindClosestPoint(cursorPt);
		// problem to obtain correct id for "all" case
		double abscissas = abscissasArray->GetTuple(iD)[0];

		QVector<double> x_cursor = { abscissas,abscissas };
		QVector<double> y_cursor = { ui->customPlotCenterlines->yAxis->range().lower * 10, ui->customPlotCenterlines->yAxis->range().upper * 10 };

		//std::cout << "point: " << m_cursorPosition[0] << ", " << m_cursorPosition[1] << ", " << m_cursorPosition[2] << std::endl;
		//std::cout << "id:" << iD << std::endl;
		//std::cout << "x_cursor: " << x_cursor[0] << ", " << x_cursor[1] << std::endl;
		//std::cout << "y_cursor: "<< y_cursor[0] << ", "<< y_cursor[1] << std::endl;

		// plot 
		this->ui->customPlotCenterlines->addGraph();
		QPen cursorPen;
		cursorPen.setColor(Qt::red);
		//cursorPen.setStyle(Qt::DotLine);
		cursorPen.setWidthF(2);
		this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setPen(cursorPen);
		this->ui->customPlotCenterlines->graph(this->ui->customPlotCenterlines->graphCount() - 1)->setData(x_cursor, y_cursor);
	}

	// replot
	this->ui->customPlotCenterlines->replot();
}