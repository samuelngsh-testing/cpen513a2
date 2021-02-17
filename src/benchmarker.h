// @file:     benchmarker.h
// @author:   Samuel Ng
// @created:  2021-02-16
// @license:  GNU LGPL v3
//
// @desc:     Benchmarking routines.

#ifndef _CLI_BENCHMARKER_H_
#define _CLI_BENCHMARKER_H_

#include <QtWidgets>
#include "placer/placer.h"

namespace cli {

  //! Run benchmarks in multiple threads.
  class Benchmarker
  {
  public:
    //! Constructor taking the output JSON path.
    Benchmarker(const QString &json_out_path, int repeat_count,
        const QString &settings_path="");

    //! Destructor.
    ~Benchmarker() {};

    //! Run benchmarks.
    void runBenchmarks();

    //! Store results.
    static void storeResults(const QString &bench_name, int bench_id, 
        pc::SAResults results);

  private:

    //! Read and store settings if path specified.
    void readSettings(const QString &settings_path);

    // Private variables
    QString json_out_path;          //!< Output path to write to.
    int repeat_count;               //!< Repeat each benchmark for this many times.
    QStringList bench_names;        //!< File names of the benchmarks (excluding txt).
    pc::SASettings sa_settings;     //!< Placement settings.
    std::vector<std::thread> threads; //!< Benchmarking threads.
    static QMap<QPair<QString, int>, pc::SAResults> bench_results;

    static std::mutex result_store_mutex; //!< Mutex for result write-in.
  };

  //! Individual benchmark thread.
  class BenchmarkTask
  {
  public:
    //! Constructor taking the benchmark problem file path.
    BenchmarkTask(const QString &bench_name, int bench_id, 
        const QString &f_path, const pc::SASettings &sa_settings);

    //! Run the benchmark at the given path and store the result to Benchmarker.
    void runBenchmark();

  private:

    QString bench_name;
    int bench_id;
    QString f_path;
    pc::SASettings sa_settings;
  };

}

#endif
