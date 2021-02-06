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

  class TelemetryChart : public QWidget
  {
    Q_OBJECT

  public:
    //! Constructor.
    TelemetryChart(QWidget *parent=nullptr);

    //! Destructor.
    ~TelemetryChart();

    //! Add telemetry info to chart (assume invalid if negative).
    void addTelemetry(int cost, float T);

    //! Clear telemetries.
    void clearTelemetries();

  private:

    //! Initialize the widget.
    void initGui();

    // Private variables
    QChart *chart;            //!< Chart to draw on.
    QChartView *chart_view;   //!< Qt Chart view containing the chart.
    QLineSeries *cost_series; //!< Line series storing the cost.
    QLineSeries *T_series;    //!< Line series storing the temperature.
    QValueAxis *axis_x;       //!< x axis pointer.
    QValueAxis *axis_y_cost;  //!< y axis pointer for cost.
    QValueAxis *axis_y_T;     //!< y axis pointer for temperature.
    float y_max_buf=1.1;      //!< Percentage buffer to add at the top of y axes.
    int max_cost=-1;          //!< Maximum cost seen.
    float max_T=-1;           //!< Maximum temperature seen.
  };

}

#endif
