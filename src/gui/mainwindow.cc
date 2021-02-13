// @file:     mainwindow.cc
// @author:   Samuel Ng
// @created:  2021-02-02
// @license:  GNU LGPL v3
//
// @desc:     Implementation of the MainWindow class.

#include "mainwindow.h"
#include "placer/placer.h"

using namespace gui;

MainWindow::MainWindow(const QString &in_path, QWidget *parent)
  : QMainWindow(parent)
{
  initGui();
  if (!in_path.isEmpty()) {
    // show the problem if the input path is not empty
    readAndShowProblem(in_path);
  }
}

MainWindow::~MainWindow()
{
  delete chip;
  chip = nullptr;
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
  viewer->fitProblemInView();
  QMainWindow::resizeEvent(e);
}

void MainWindow::readAndShowProblem(const QString &in_path)
{
  setWindowTitle(tr("%1 - %2").arg(QCoreApplication::applicationName())
      .arg(QFileInfo(in_path).fileName()));

  // read the problem onto the class chip pointer
  chip = new sp::Chip(in_path);
  if (!chip->isInitialized()) {
    qWarning() << "Chip was not successfully initialized.";
    QMessageBox::warning(this, "Chip initialization failed", "A chip "
        "construction was unsuccessful. Is the provided file valid?");
    return;
  }

  // show the problem
  viewer->showChip(chip);
  tchart->clearTelemetries();
}

void MainWindow::runPlacement()
{
  if (chip == nullptr) {
    qWarning() << "runPlacement invoked when no Chip is present. Aborting.";
    QMessageBox::warning(this, "No Problem Present", "An attempt to run "
        "placement with no loaded problem has been halted.");
    return;
  }
  tchart->clearTelemetries();
  pc::Placer placer(chip);
  pc::SASettings sa_settings; // TODO read from some GUI to change settings

  sa_settings.gui_up = pc::GuiEachAnnealUpdate;

  // connect signals
  connect(&placer, &pc::Placer::sig_updateGui,
      [this](sp::Chip *chip)
      {
        // update the chip upon instructions from the placer
        viewer->showChip(chip);
        qApp->processEvents();
      });
  connect(&placer, &pc::Placer::sig_updateChart, tchart, &TelemetryChart::addTelemetry);

  // run the placement
  placer.runPlacer(sa_settings);
}

void MainWindow::initGui()
{
  // init GUI elements
  viewer = new Viewer(this);
  tchart = new TelemetryChart(this);

  QHBoxLayout *hbl = new QHBoxLayout(); // main layout
  hbl->addWidget(viewer);

  QWidget *w_main = new QWidget(this);  // main widget that holds everything
  w_main->setLayout(hbl);
  setCentralWidget(w_main);

  // set dock widgets
  dw_tchart = new QDockWidget("Placement Telemetry", this);
  dw_tchart->setWidget(tchart);
  addDockWidget(Qt::RightDockWidgetArea, dw_tchart);

  // initiate menu bars
  initMenuBar();
}

void MainWindow::initMenuBar()
{
  // initialize menus
  QMenu *file = menuBar()->addMenu(tr("&File"));
  QMenu *view = menuBar()->addMenu(tr("&View"));

  // file menu actions
  QAction *open_file = new QAction(tr("&Open..."), this);
  QAction *run_placement = new QAction(tr("&Run Placement"), this);
  QAction *quit = new QAction(tr("&Quit"), this);

  // assign keyboard shortcuts
  open_file->setShortcut(tr("CTRL+O"));
  run_placement->setShortcut(tr("CTRL+R"));
  quit->setShortcut(tr("CTRL+Q"));

  // connection action signals
  connect(open_file, &QAction::triggered, this, &MainWindow::loadProblemFromFileDialog);
  connect(run_placement, &QAction::triggered, this, &MainWindow::runPlacement);
  connect(quit, &QAction::triggered, this, &QWidget::close);

  // add actions to the appropriate menus
  file->addAction(open_file);
  file->addAction(run_placement);
  file->addSeparator();
  file->addAction(quit);
  view->addAction(dw_tchart->toggleViewAction());
}

void MainWindow::loadProblemFromFileDialog()
{
  // open file dialog and load the specified file path
  QFileDialog fd;
  fd.setDefaultSuffix("txt");
  QString open_path = fd.getOpenFileName(this, tr("Open File"),
      "", tr("Text Files (*.txt);;All files (*.*)"));
  if (!open_path.isNull()) {
    readAndShowProblem(open_path);
  }
}
