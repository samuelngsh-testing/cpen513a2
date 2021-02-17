// @file:     benchmarker.cc
// @author:   Samuel Ng
// @created:  2021-02-16
// @license:  GNU LGPL v3
//
// @desc:     Implementation of the benchmarking routines.

#include <QJsonDocument>
#include <QJsonObject>
#include "benchmarker.h"

using namespace cli;

// static declarations
QMap<QPair<QString, int>, pc::SAResults> Benchmarker::bench_results;
std::mutex Benchmarker::result_store_mutex;

Benchmarker::Benchmarker(const QString &json_out_path, int repeat_count,
    const QString &settings_path)
  : json_out_path(json_out_path), repeat_count(repeat_count)
{
  if (!settings_path.isEmpty()) {
    readSettings(settings_path);
  }
  bench_names << "alu2" << "apex1" << "apex4" << "C880" << "cm138a" << "cm150a"
    << "cm151a" << "cm162a" << "cps" << "e64" << "paira" << "pairb";
}

void Benchmarker::runBenchmarks()
{
  // try to open file for writing before actually running the benchmark, avoid 
  // write failure after running a bunch of benches.
  QFile f_out(json_out_path);
  if (!f_out.open(QIODevice::WriteOnly)) {
    qDebug() << "Failed to open " << f_out << " for writing.";
    exit(1);
  }

  // run each benchmark
  for (const QString &bench_name : bench_names) {
    qDebug() << "Adding thread for benchmark" << bench_name << "...";
    // keep track of costs and iteration counts
    int repeat = repeat_count;
    while (repeat--) {
      QString f_path = ":/benchmarks/" + bench_name + ".txt";
      BenchmarkTask task(bench_name, repeat, f_path, sa_settings);
      std::thread th(&BenchmarkTask::runBenchmark, task);
      threads.push_back(std::move(th));
    }
  }

  qDebug() << "Waiting for all benchmark threads to complete...";
  for (auto &th : threads) {
    th.join();
  }

  // construct QVariantMap for exportation to JSON
  qDebug() << "All benchmarks have finished. Preparing export...";
  QVariantMap result_map;
  for (const QString &bench_name : bench_names) {
    QList<QVariant> costs;
    QList<QVariant> its;
    for (int i=0; i<repeat_count; i++) {
      pc::SAResults r = bench_results.value(qMakePair(bench_name, i));
      costs.append(r.cost);
      its.append(r.iterations);
    }
    QVariantMap bench_map;
    bench_map["costs"] = costs;
    bench_map["iterations"] = its;
    result_map.insert(bench_name, bench_map);
  }

  // export the recorded statistics to the JSON output path
  QJsonObject json_obj = QJsonObject::fromVariantMap(result_map);
  QJsonDocument json_doc(json_obj);
  QString json_str = json_doc.toJson();
  f_out.write(json_str.toLocal8Bit());
  f_out.close();
  qDebug() << "Results written to " << json_out_path;
}

void Benchmarker::storeResults(const QString &bench_name, int bench_id,
    pc::SAResults results)
{
  result_store_mutex.lock();
  bench_results.insert(qMakePair(bench_name, bench_id), results);
  result_store_mutex.unlock();
}

void Benchmarker::readSettings(const QString &settings_path)
{
  qDebug() << "Reading benchmark settings from" << settings_path;

  // try to open file for reading
  QFile in_file(settings_path);
  if (!in_file.open(QIODevice::ReadOnly)) {
    qFatal("Unable to read benchmark settings file.");
  }

  // read text stream and create JSON doc
  QTextStream f_text(&in_file);
  QByteArray json_bytes = f_text.readAll().toLocal8Bit();
  QJsonDocument json_doc = QJsonDocument::fromJson(json_bytes);
  in_file.close();

  // check for read errors
  if (json_doc.isNull()) {
    qFatal("Failed to create JSON doc");
  }
  if (!json_doc.isObject()) {
    qFatal("JSON is not an object in unit test.");
  }

  QJsonObject json_obj = json_doc.object();
  if (json_obj.isEmpty()) {
    qFatal("JSON object is empty");
  }

  // iterate through all key value pairs and make appropriate settings
  for (auto json_it=json_obj.constBegin(); json_it!=json_obj.constEnd(); json_it++) {
    if (json_it.key() == "t_schd") {
      sa_settings.t_schd = static_cast<pc::TSchd>(json_it.value().toInt());
    } else if (json_it.key() == "decay_b") {
      sa_settings.decay_b = json_it.value().toDouble();
    } else if (json_it.key() == "swap_fact") {
      sa_settings.swap_fact = json_it.value().toDouble();
    } else if (json_it.key() == "max_its") {
      sa_settings.max_its = json_it.value().toInt();
    } else if (json_it.key() == "use_rw") {
      sa_settings.use_rw = json_it.value().toBool();
    } else if (json_it.key() == "p_upper") {
      sa_settings.p_upper = json_it.value().toDouble();
    } else if (json_it.key() == "p_lower") {
      sa_settings.p_lower = json_it.value().toDouble();
    } else if (json_it.key() == "min_rw_dim") {
      sa_settings.min_rw_dim = json_it.value().toInt();
    } else if (json_it.key() == "rw_dim_delta") {
      sa_settings.rw_dim_delta = json_it.value().toInt();
    } else if (json_it.key() == "sanity_check") {
      sa_settings.sanity_check = json_it.value().toBool();
    } else if (json_it.key() == "show_stdout") {
      sa_settings.show_stdout = json_it.value().toBool();
    } else {
      qWarning() << "Unidentified setting encountered with key:" << json_it.key();
    }
  }
}


// BenchmarkTask class implementation

BenchmarkTask::BenchmarkTask(const QString &bench_name, int bench_id,
    const QString &f_path, const pc::SASettings &sa_settings)
  : bench_name(bench_name), bench_id(bench_id), f_path(f_path),
    sa_settings(sa_settings)
{}

void BenchmarkTask::runBenchmark()
{
  sp::Chip chip(f_path);
  pc::Placer placer(&chip);
  pc::SAResults results = placer.runPlacer(sa_settings);
  Benchmarker::storeResults(bench_name, bench_id, results);
}
