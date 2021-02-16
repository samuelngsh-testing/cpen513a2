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

void TelemetryChart::addTelemetry(int cost, float T, float p_accept, int rw_dim)
{
  int x_step = std::max(cost_series->count(), T_series->count());
  // update values
  if (cost >= 0) {
    cost_series->append(x_step, cost);
    max_cost = std::max(max_cost, cost);
    l_curr_cost->setText(QString("%1").arg(cost));
  }
  if (T >= 0) {
    T_series->append(x_step, T);
    max_T = std::max(max_T, T);
    l_curr_T->setText(QString("%1").arg(T));
  }
  if (p_accept >= 0) {
    p_accept_series->append(x_step, p_accept);
    max_p_accept = std::max(max_p_accept, p_accept);
  }
  if (rw_dim >= 0) {
    rw_series->append(x_step, rw_dim);
    max_rw_dim = std::max(max_rw_dim, rw_dim);
  }
  // update ticks and ranges
  axis_x->setRange(0, cost_series->count()-1);
  axis_x_rw->setRange(0, p_accept_series->count()-1);
  axis_y_cost->setRange(0, max_cost);
  axis_y_T->setRange(0, max_T);
  axis_y_pa->setRange(0, max_p_accept);
  axis_y_rw->setRange(0, max_rw_dim);
  axis_y_cost->applyNiceNumbers();
  axis_y_T->applyNiceNumbers();
  axis_y_pa->applyNiceNumbers();
  axis_y_rw->applyNiceNumbers();
}

void TelemetryChart::clearTelemetries()
{
  max_cost = -1;
  max_T = -1;
  cost_series->clear();
  T_series->clear();
  p_accept_series->clear();
  rw_series->clear();
}

void TelemetryChart::initGui()
{
  // initialize private pointers for chart
  chart = new QChart();
  chart_view = new QChartView(chart);
  rw_chart = new QChart();
  rw_view = new QChartView(rw_chart);
  cost_series = new QLineSeries();
  T_series = new QLineSeries();
  p_accept_series = new QLineSeries();
  rw_series = new QLineSeries();
  axis_x = new QValueAxis();
  axis_x_rw = new QValueAxis();
  axis_y_cost = new QValueAxis();
  axis_y_T = new QValueAxis();
  axis_y_pa = new QValueAxis();
  axis_y_rw = new QValueAxis();

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
  axis_y_cost->setTickCount(10);

  // add temperature series to chart
  T_series->setName("Temperature");
  chart->addAxis(axis_y_T, Qt::AlignRight);
  chart->addSeries(T_series);
  T_series->attachAxis(axis_x);
  T_series->attachAxis(axis_y_T);
  axis_y_T->setLinePenColor(T_series->pen().color());
  axis_y_T->setTickCount(10);

  // range window telemetry
  rw_chart->setTitle("Range Window Telemetry");
  rw_chart->addAxis(axis_x_rw, Qt::AlignBottom);

  // add acceptance probability series to chart
  p_accept_series->setName("Average Acceptance Probability");
  rw_chart->addAxis(axis_y_pa, Qt::AlignLeft);
  rw_chart->addSeries(p_accept_series);
  p_accept_series->attachAxis(axis_x_rw);
  p_accept_series->attachAxis(axis_y_pa);
  axis_y_pa->setLinePenColor(p_accept_series->pen().color());
  axis_y_pa->setTickCount(10);

  // range window dimension
  rw_series->setName("Range Window Size");
  rw_chart->addAxis(axis_y_rw, Qt::AlignRight);
  rw_chart->addSeries(rw_series);
  rw_series->attachAxis(axis_x_rw);
  rw_series->attachAxis(axis_y_rw);
  axis_y_rw->setLinePenColor(rw_series->pen().color());
  axis_y_pa->setTickCount(10);

  // initialize status form
  l_curr_T = new QLabel();
  l_curr_cost = new QLabel();
  QFormLayout *fl_status = new QFormLayout();
  fl_status->addRow("Temperature", l_curr_T);
  fl_status->addRow("Cost", l_curr_cost);

  // set layout
  QVBoxLayout *vb = new QVBoxLayout();
  vb->addWidget(chart_view);
  vb->addWidget(rw_view);
  vb->addLayout(fl_status);
  setLayout(vb);

  // size settings
  setMinimumSize(600, 600);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
}
