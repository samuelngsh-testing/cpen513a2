// @file:     main.cc
// @author:   Samuel Ng
// @created:  2021-02-02
// @license:  GNU LGPL v3
//
// @desc:     Main function for the placer.

#include <QApplication>
#include <QCommandLineParser>
#include <QMainWindow>
#include <QDebug>

#include "benchmarker.h"
#include "gui/mainwindow.h"

int main(int argc, char **argv) {
  // initialize QApplication
  QApplication app(argc, argv);
  app.setApplicationName("Standard Cell Placement Application");

  // specify possible command line inputs
  QCommandLineParser parser;
  parser.setApplicationDescription("Standard cell placement tool for CPEN 513 "
      "by Samuel Ng.");
  parser.addHelpOption();
  parser.addPositionalArgument("in_file", "Input file specifying the problem to"
      " be placed (optional, can be selected from the GUI).");
  parser.addOption({"benchmark", "Benchmark mode. Run each sample problem "
      "multiple times using default presets and return relevant statistics."});
  parser.addOption({"bench_settings_in", "JSON input file for benchmark settings",
      "path"});
  parser.addOption({"json_out", "Write generated data into <path>. Simply"
      " writes to out.json if unspecified.", "path"});
  parser.addOption({"repeat", "Repeat each benchmark for the specified number "
      "of times. Defaults to 10 if unspecified.", "repeat"});
  parser.process(app);

  // benchmark mode routine (don't show GUI if benchmarking)
  bool benchmark_mode = parser.isSet("benchmark");
  if (benchmark_mode) {
    QString out_name = parser.isSet("json_out") ? parser.value("json_out") : "out.json";
    QString set_name = parser.isSet("bench_settings_in") ? 
      parser.value("bench_settings_in") : "";
    int repeat = parser.isSet("repeat") ? parser.value("repeat").toInt() : 10;
    // run the benchmarks and output to the specified JSON path
    cli::Benchmarker bm(out_name, repeat, set_name);
    bm.runBenchmarks();

    // no need to show GUI after benchmarks are complete
    return 0; 
  }

  // get input file path
  const QStringList args = parser.positionalArguments();
  QString in_path;
  if (!args.empty()) {
    in_path = args[0];
    qDebug() << QObject::tr("Input file path: %1").arg(in_path);
  }

  // show the main GUI
  gui::MainWindow mw(in_path);
  mw.show();

  // run app
  return app.exec();
}
