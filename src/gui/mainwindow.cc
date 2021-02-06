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

  // show the problem
  viewer->showChip(chip);
  tchart->clearTelemetries();
}

void MainWindow::runPlacement()
{
  if (chip == nullptr) {
    qWarning() << "runPlacement invoked when no Chip is present. Ignoring.";
    return;
  }
  pc::Placer placer(chip);
  pc::SASettings sa_settings; // TODO read from some GUI to change settings

  sa_settings.gui_up = pc::GuiEachAnnealUpdate;

  // connect signals
  connect(&placer, &pc::Placer::sig_updateGui,
      [this](sp::Chip *chip)
      {
        viewer->showChip(chip);
        qApp->processEvents();
      });
  connect(&placer, &pc::Placer::sig_updateChart, tchart, &TelemetryChart::addTelemetry);

  // run the placement
  placer.runPlacer(sa_settings);
}

void MainWindow::initGui()
{
  initMenuBar();

  // init GUI elements
  viewer = new Viewer(this);
  tchart = new TelemetryChart(this);

  QHBoxLayout *hbl = new QHBoxLayout(); // main vertical layout
  hbl->addWidget(viewer);
  hbl->addWidget(tchart);
  // TODO add a widget for showing current step properties (most importantly the cost)

  QWidget *w_main = new QWidget(this);  // main widget that holds everything
  w_main->setLayout(hbl);
  setCentralWidget(w_main);
}

void MainWindow::initMenuBar()
{
  // initialize menus
  QMenu *file = menuBar()->addMenu(tr("&File"));

  // file menu actions
  QAction *open_file = new QAction(tr("&Open..."), this);
  QMenu *open_sample_problem = new QMenu(tr("Open Sample Problem"), this);
  QAction *run_placement = new QAction(tr("&Run Placement"), this);
  QAction *quit = new QAction(tr("&Quit"), this);

  // TODO populate sample problems list

  // assign keyboard shortcuts
  open_file->setShortcut(tr("CTRL+O"));
  quit->setShortcut(tr("CTRL+Q"));

  // connection action signals
  connect(open_file, &QAction::triggered,
      [this](){
        // open file dialog and load the specified file path
        QFileDialog fd;
        fd.setDefaultSuffix("txt");
        QString open_path = fd.getOpenFileName(this, tr("Open File"),
            "", tr("Text Files (*.txt);;All files (*.*)"));
        if (!open_path.isNull()) {
          readAndShowProblem(open_path);
        }
      });
  connect(run_placement, &QAction::triggered, this, &MainWindow::runPlacement);
  connect(quit, &QAction::triggered, this, &QWidget::close);

  // add actions to the appropriate menus
  file->addAction(open_file);
  file->addMenu(open_sample_problem);
  file->addSeparator();
  file->addAction(run_placement);
  file->addSeparator();
  file->addAction(quit);
}
