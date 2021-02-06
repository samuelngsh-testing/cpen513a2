// @file:     mainwindow.h
// @author:   Samuel Ng
// @created:  2021-02-02
// @license:  GNU LGPL v3
//
// @desc:     Main window (frame) of the application.

#ifndef _GUI_MAINWINDOW_H_
#define _GUI_MAINWINDOW_H_

#include <QtWidgets>
#include "spatial.h"
#include "viewer.h"
#include "telemetrychart.h"

namespace gui {

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
    void runPlacement();

  private:

    //! Initialize the GUI.
    void initGui();

    //! Initialize the top menu bar.
    void initMenuBar();

    // Private variables
    sp::Chip *chip=nullptr; //!< Pointer to the chip.
    Viewer *viewer=nullptr; //!< Pointer to the GUI viewer.
    TelemetryChart *tchart=nullptr; //!< Pointer to the telemetry chart.

  };

}

#endif
