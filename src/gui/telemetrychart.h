// @file:     telemetrychart.h
// @author:   Samuel Ng
// @created:  2021-02-05
// @license:  GNU LGPL v3
//
// @desc:     Widget that charts current placement telemetries such as cost, 
//            annealing temp, etc.

#ifndef _GUI_TELEMCHART_H_
#define _GUI_TELEMCHART_H_

#include <QtWidgets>
#include <QtCharts>

namespace gui {

  using namespace QtCharts;

  //! A widget that plots telemetry relevant to the problem.
  class TelemetryChart : public QWidget
  {
    Q_OBJECT

  public:
    //! Constructor.
    TelemetryChart(QWidget *parent=nullptr);

    //! Destructor.
    ~TelemetryChart();

    //! Add telemetry info to chart (assume invalid if negative).
    void addTelemetry(int cost, float T, float p_accept, int rw_dim);

    //! Clear telemetries.
    void clearTelemetries();

  private:

    //! Initialize the widget.
    void initGui();

    // Private variables
    QChart *chart;            //!< Chart to draw on.
    QChartView *chart_view;   //!< Qt Chart view containing the chart.
    QChart *rw_chart;         //!< Range window chart.
    QChartView *rw_view;      //!< Chart showing range window related parameters.
    QLineSeries *cost_series; //!< Line series storing the cost.
    QLineSeries *T_series;    //!< Line series storing the temperature.
    QLineSeries *p_accept_series; //!< Acceptance probability series.
    QLineSeries *rw_series;       //!< Range window dimensions series.
    QValueAxis *axis_x;       //!< x axis pointer.
    QValueAxis *axis_x_rw;    //!< x axis pointer.
    QValueAxis *axis_y_cost;  //!< y axis pointer for cost.
    QValueAxis *axis_y_T;     //!< y axis pointer for temperature.
    QValueAxis *axis_y_pa;    //!< Acceptance probability axis.
    QValueAxis *axis_y_rw;    //!< Range window dimension axis.
    QLabel *l_curr_T;         //!< Label of current temperature.
    QLabel *l_curr_cost;      //!< Label of current cost.
    float y_max_buf=1.1;      //!< Percentage buffer to add at the top of y axes.
    int max_cost=-1;          //!< Maximum cost seen.
    float max_T=-1;           //!< Maximum temperature seen.
    float max_p_accept=-1;    //!< Maximum acceptance probability seen.
    int max_rw_dim=-1;        //!< Maximum range window dimension seen.
  };

}

#endif
