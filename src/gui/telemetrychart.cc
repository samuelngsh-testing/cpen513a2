// @file:     telemetrychart.cc
// @author:   Samuel Ng
// @created:  2021-02-05
// @license:  GNU LGPL v3
//
// @desc:     Implementation of TelemetryChart.

#include "telemetrychart.h"

using namespace gui;

TelemetryChart::TelemetryChart(QWidget *parent)
  : QWidget(parent)
{
  initGui();
}

TelemetryChart::~TelemetryChart()
{
  // TODO implement
}

void TelemetryChart::addTelemetry(int cost, float T)
{
  int x_step = std::max(cost_series->count(), T_series->count());
  if (cost >= 0) {
    cost_series->append(x_step, cost);
    max_cost = std::max(max_cost, cost);
  }
  if (T >= 0) {
    T_series->append(x_step, T);
    max_T = std::max(max_T, T);
  }
  axis_x->setRange(0, cost_series->count()-1);
  axis_y_cost->setRange(0, max_cost*y_max_buf);
  axis_y_T->setRange(0, max_T*y_max_buf);
}

void TelemetryChart::clearTelemetries()
{
  max_cost = -1;
  max_T = -1;
  cost_series->clear();
  T_series->clear();
}

void TelemetryChart::initGui()
{
  // initialize private pointers
  chart = new QChart();
  chart_view = new QChartView(chart);
  cost_series = new QLineSeries();
  T_series = new QLineSeries();
  axis_x = new QValueAxis();
  axis_y_cost = new QValueAxis();
  axis_y_T = new QValueAxis();

  // set chart props
  chart->setTitle("Placement Telemetry");
  chart->addAxis(axis_x, Qt::AlignBottom);

  // add cost series to chart
  cost_series->setName("Cost");
  chart->addAxis(axis_y_cost, Qt::AlignLeft);
  chart->addSeries(cost_series);
  cost_series->attachAxis(axis_x);
  cost_series->attachAxis(axis_y_cost);
  axis_y_cost->setLinePenColor(cost_series->pen().color());

  // add temperature series to chart
  T_series->setName("Temperature");
  chart->addAxis(axis_y_T, Qt::AlignRight);
  chart->addSeries(T_series);
  T_series->attachAxis(axis_x);
  T_series->attachAxis(axis_y_T);
  axis_y_T->setLinePenColor(T_series->pen().color());

  QVBoxLayout *vb = new QVBoxLayout();
  vb->addWidget(chart_view);
  setLayout(vb);
}
