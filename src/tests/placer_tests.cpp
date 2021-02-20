/*!
  \file placer_tests.cpp
  \brief Unit tests for the placer program.
  \author Samuel Ng
  \date 2021-02-15 created
  \copyright GNU LGPL v3
  */

#include <QtTest/QtTest>
#include <QJsonObject>
#include "placer/placer.h"
#include "gui/settings.h"

class PlacerTests : public QObject
{
  Q_OBJECT

  public:

    /*! \brief Read test problem properties from JSON.
     *
     * Read the test problem properties from the provided path and return a 
     * QVariantMap.
     */
    QVariantMap readTestProps(const QString &f_path)
    {
      QFile f(f_path);
      if (!f.open(QIODevice::ReadOnly)) {
        qFatal("Unable to read test problem properties in unit test.");
      }

      QTextStream f_text(&f);
      QString json_str;
      json_str = f_text.readAll();
      f.close();
      QByteArray json_bytes = json_str.toLocal8Bit();

      QJsonDocument json_doc = QJsonDocument::fromJson(json_bytes);

      if (json_doc.isNull()) {
        qFatal("Failed to create JSON doc in unit test.");
      }
      if (!json_doc.isObject()) {
        qFatal("JSON is not an object in unit test.");
      }

      QJsonObject json_obj = json_doc.object();

      if (json_obj.isEmpty()) {
        qFatal("JSON object is empty in unit test.");
      }

      return json_obj.toVariantMap();
    }


  // functions in these slots are automatically called after compilation
  private slots:

    //! Test that placement problems can be read successfully.
    void testProblemRead()
    {
      using namespace sp;

      // All test problems have an associated manually filled properties JSON 
      // file. The formats are always "name.txt" for the problem and 
      // "name_props.json" for the properties. Populate p_names by the list of
      // names. The actual files are located under src/qrc/test_problems.
      QStringList p_names;
      p_names << "mini" << "mini_2" << "alu2" << "apex1" << "paira";

      for (QString p_name : p_names) {
        QString base_name = ":/test_problems/" + p_name;
        Chip chip(base_name + ".txt");
        QVariantMap expected_props = readTestProps(base_name + "_props.json");
        // check that basic properties were read correctly
        QCOMPARE(chip.isInitialized(), true);
        QCOMPARE(chip.numBlocks(), expected_props["num_blocks"].value<int>());
        QCOMPARE(chip.numNets(), expected_props["num_nets"].value<int>());
        QCOMPARE(chip.getCost() < 0, true);  // uninitialized cost is -1
        // check the generated data structures
        Graph *graph = chip.getGraph();
        QCOMPARE(graph->allBlocksConnected(), true);
        QCOMPARE(graph->getNets().size(), expected_props["num_nets"].value<int>());
      }
    }

    //! Validate cost calculation (both for initial calc and swap delta calc).
    void testCostCalculation()
    {
      using namespace sp;

      QString p_path = ":/test_problems/mini_2.txt";

      // Manually set the placement of a known problem and see if the calculated
      // cost aligns with the expected cost.
      // Net 0: 0 1 2 3
      // Net 1: 0 4
      // Locations layout 1:
      // x x x x
      // 0 1 2 3
      // 4 x x x
      // x x x x
      Chip chip(p_path);
      chip.setLocBlock(qMakePair(0,1), 0);
      chip.setLocBlock(qMakePair(1,1), 1);
      chip.setLocBlock(qMakePair(2,1), 2);
      chip.setLocBlock(qMakePair(3,1), 3);
      chip.setLocBlock(qMakePair(0,2), 4);
      QCOMPARE(chip.costOfNet(0), 3);
      QCOMPARE(chip.costOfNet(1), 2);
      QCOMPARE(chip.calcCost(), 5);

      // Locations layout 2:
      // Net 0: 0 1 2 3
      // Net 1: 0 4
      // x x x x
      // x 4 0 1
      // x x 2 3
      // x x x x
      chip.initEmptyPlacements();
      chip.setLocBlock(qMakePair(2,1), 0);
      chip.setLocBlock(qMakePair(3,1), 1);
      chip.setLocBlock(qMakePair(2,2), 2);
      chip.setLocBlock(qMakePair(3,2), 3);
      chip.setLocBlock(qMakePair(1,1), 4);
      QCOMPARE(chip.costOfNet(0), 3);
      QCOMPARE(chip.costOfNet(1), 1);
      QCOMPARE(chip.calcCost(), 4);

      // Keep previous locations but verify the change in cost of potential 
      // swaps:

      // From layout 2, swap blocks 2 and 3 (no change in cost)
      QCOMPARE(chip.calcSwapCostDelta(2, 2, 3, 2), 0);

      // From layout 2, move block 0 to (1, 0)
      // x 0 x x
      // x 4 x 1
      // x x 2 3
      // x x x x
      QCOMPARE(chip.calcSwapCostDelta(1, 0, 2, 1), 4);

      // From layout 2, swap blocks 0 and 4
      // x x x x
      // x 0 4 1
      // x x 2 3
      // x x x x
      QCOMPARE(chip.calcSwapCostDelta(1, 1, 2, 1), 1);
    }

    /*! \brief Check random block placement initialization.
     *
     * Check that random block placement initialization successfully places 
     * all blocks using the ALU2 problem.
     */
    void testRandomPlacementInit()
    {
      QString p_path = ":/test_problems/alu2.txt";
      sp::Chip chip(p_path);
      pc::Placer placer(&chip);
      // init block positions randomly, manipulates the chip pointer directly
      placer.initBlockPos();
      // iterate through all blocks to see whether they have a uniqute location
      QSet<QPair<int,int>> coord_set;
      for (int bid=0; bid<chip.numBlocks(); bid++) {
        QPair<int,int> loc = chip.blockLoc(bid);
        QCOMPARE(coord_set.contains(loc), false);
        coord_set.insert(loc);
      }
    }

    //! Validate that placement of a very trivial problem is successful.
    void testTrivialPlacementProblem()
    {
      // problem only contains two blocks, should be able to reach minimum cost
      // very quickly
      QString p_path = ":/test_problems/mini.txt";
      sp::Chip chip(p_path);
      pc::Placer placer(&chip);
      pc::SASettings sa_settings;
      sa_settings.t_schd = pc::TSchd::StdDevTUpdate;
      sa_settings.max_its = 500;
      pc::SAResults results = placer.runPlacer(sa_settings);
      QCOMPARE(results.cost, 1);
    }

};

QTEST_MAIN(PlacerTests)
#include "placer_tests.moc"  // generated at compile time
