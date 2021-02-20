/*!
  \file mainwindow.h
  \brief Main window (frame) of the application.
  \author Samuel Ng
  \date 2021-02-02 created
  \copyright GNU LGPL v3
  */

#ifndef _GUI_MAINWINDOW_H_
#define _GUI_MAINWINDOW_H_

#include <QtWidgets>
#include "spatial.h"
#include "viewer.h"
#include "invoker.h"
#include "telemetrychart.h"

namespace gui {

  //! Main window widget housing all GUI elements.
  class MainWindow : public QMainWindow
  {
    Q_OBJECT

  public:

    //! Constructor taking the input file path describing the placement problem.
    explicit MainWindow(const QString &in_path, QWidget *parent=nullptr);

    //! Describtor.
    ~MainWindow();

    //! Override the resize event to fit the placement problem in the viewport
    //! when user resizes the main window.
    virtual void resizeEvent(QResizeEvent *) override;

    //! Read a problem file and show it in the viewer.
    void readAndShowProblem(const QString &in_path);

    //! Run placement on the current problem.
    void runPlacement(pc::SASettings sa_set);

  private:

    //! Initialize the GUI.
    void initGui();

    //! Initialize the top menu bar.
    void initMenuBar();

    //! Load problem from file dialog.
    void loadProblemFromFileDialog();

    // Private variables
    sp::Chip *chip=nullptr;   //!< Pointer to the chip.
    Viewer *viewer=nullptr;   //!< Pointer to the GUI viewer.
    Invoker *invoker=nullptr; //!< Pointer to the Invoker widget.
    QDockWidget *dw_invoker=nullptr;//!< Dockwidget for the invoker.
    TelemetryChart *tchart=nullptr; //!< Pointer to the telemetry chart.
    QDockWidget *dw_tchart=nullptr; //!< Dockwidget for telemetry chart.

  };

}

#endif
