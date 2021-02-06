// @file:     spatial.cc
// @author:   Samuel Ng
// @created:  2021-02-03
// @license:  GNU LGPL v3
//
// @desc:     Implementation of spatial objects.

#include "spatial.h"
#include <algorithm>

using namespace sp;


// Graph class implementations

Graph::Graph(int n_blocks, int n_nets)
{
  all_block_net_ids.resize(n_blocks);
  nets.resize(n_nets);
}

void Graph::setNet(int net_id, const QList<int> &conn_blocks)
{
  nets[net_id] = conn_blocks;
  for (int b_id : conn_blocks) {
    all_block_net_ids[b_id].append(net_id);
  }
}

bool Graph::allBlocksConnected() const
{
  for (const QList<int> &block_net_ids : all_block_net_ids) {
    if (block_net_ids.isEmpty()) {
      return false;
    }
  }
  return true;
}



// Chip class implementations

Chip::Chip(const QString &f_path)
{
  QFile in_file(f_path);
  if (!in_file.open(QFile::ReadOnly | QFile::Text)) {
    qWarning() << "Unable to open file for reading.";
    return;
  }

  // parse the file
  bool reading_nets = false;  // first line is problem def, rest are nets
  int net_id = -1;
  while (!in_file.atEnd() && !(net_id >= n_nets)) {
    QString line = in_file.readLine().trimmed();
    QStringList line_items = line.split(" ");

    if (net_id == -1) {
      // if the net ID is -1 then read the problem definition
      if (line_items.size() != 4) {
        qWarning("First line of the input file must contain 4 values. File invalid.");
        return;
      }
      // read the values and assign Graph pointer
      n_blocks = line_items[0].toInt();
      n_nets = line_items[1].toInt();
      ny = line_items[2].toInt();
      nx = line_items[3].toInt();
      graph = new Graph(n_blocks, n_nets);
      reading_nets = true;
    } else {
      // read net definitions and add to Graph
      QList<int> conn_blocks;
      for (const QString &b_id_str : line_items) {
        conn_blocks.append(b_id_str.toInt());
      }
      graph->setNet(net_id, conn_blocks);
    }
    net_id++;
  }
  in_file.close();

  // sanity check on the produced Graph
  if (!graph->allBlocksConnected()) {
    qWarning() << "There are blocks on the produced Graph that aren't connected"
      " to anything";
  }

  // initialize 2D grid
  grid.clear();
  grid.resize(nx);
  for (int x=0; x<nx; x++) {
    grid[x].resize(ny);
    for (int y=0; y<ny; y++) {
      grid[x][y] = -1;  // initialize to be empty
    }
  }

  // initialize blocks list
  block_locs.resize(n_blocks);
  for (auto &block_loc : block_locs) {
    block_loc = qMakePair(-1, -1);
  }

  // initial cost set to be -1 (invalid) since nothing is placed
  cost = -1;

  initialized = true;
}

Chip::~Chip()
{
  delete graph;
}

QList<int> Chip::netBlockIds(int net_id) const
{
  return graph->getNet(net_id);
}

QList<QPair<int,int>> Chip::netCoords(int net_id) const
{
  QList<QPair<int,int>> coords;
  for (int bid : graph->getNet(net_id)) {
    coords.append(block_locs[bid]);
  }
  return coords;
}

void Chip::setLocBlock(const QPair<int,int> &loc, int block_id)
{
  grid[loc.first][loc.second] = block_id;
  if (block_id >= 0) {
    block_locs[block_id] = loc;
  }
}

int Chip::calcCost(bool update_internal)
{
  // TODO current calculation ignores the inter-row gap. Shouldn't affect the 
  // algorithm implementation but need to add that back in later for accurate 
  // reporting.
  if (block_locs.isEmpty()) {
    qWarning() << "Should not call calcCost before block locations have been "
      "initialized.";
    return -1;
  }
  int calc_cost = 0;
  for (int net_id=0; net_id<n_nets; net_id++) {
    calc_cost += costOfNet(net_id);
  }
  if (update_internal) {
    // update stored cost if instructed to do so
    cost = calc_cost;
  }
  return cost;
}

int Chip::calcSwapCostDelta(int x1, int y1, int x2, int y2)
{
  if (block_locs.isEmpty()) {
    qWarning() << "Should not call calcSwapCostDelta before block locations "
      "have been initialized.";
    return -1;
  }

  int bid_1 = grid[x1][y1];
  int bid_2 = grid[x2][y2];

  // TODO this routine can be sped up by having each net store its original
  // cost. This optimization can be implemented later.

  // if swapping between two empty blocks, no change
  if (bid_1 == -1 && bid_2 == -1) {
    return 0;
  }

  auto associatedNetCosts = [this, bid_1, bid_2]() {
    int cost = 0;
    for (int bid : {bid_1, bid_2}) {
      if (bid == -1) {
        continue;
      }
      for (int net_id : graph->blockNets(bid)) {
        cost += costOfNet(net_id);
      }
    }
    return cost;
  };

  // compute the cost of associated nets before the change
  int cost_i = associatedNetCosts();

  // perform the swap
  QPair<int,int> bcoord_swap = block_locs[bid_2];
  block_locs[bid_2] = qMakePair(block_locs[bid_1].first, block_locs[bid_1].second);
  block_locs[bid_1] = bcoord_swap;

  int cost_f = associatedNetCosts();

  // swap back
  bcoord_swap = block_locs[bid_2];
  block_locs[bid_2] = qMakePair(block_locs[bid_1].first, block_locs[bid_1].second);
  block_locs[bid_1] = bcoord_swap;

  return cost_f - cost_i;
}

void Chip::addCostDelta(int delta)
{
  if (cost < 0) {
    qWarning() << "Attempted to add delta to invalid cost (<0).";
  }
  cost += delta;
  if (cost < 0) {
    qWarning() << "Cost becomes invalid after adding a delta (<0).";
  }
}

void Chip::setGrid(const QVector<QVector<int>> &t_grid, bool skip_validation)
{
  if (!skip_validation) {
    // validate x dim
    if (t_grid.size() != nx) {
      qWarning() << "Provided grid has x dimension that differs from the chip.";
      return;
    }
    for (const QVector<int> &row : t_grid) {
      // validate y dim
      if (row.size() != ny) {
        qWarning() << "Provided grid has y dimension that differs from the chip.";
        return;
      }
      // validate that all block ID falls within expected range
      for (int b_id : row) {
        if (b_id < 0 || b_id > n_blocks) {
          qWarning() << "Provided grid contains blocks with ID beyond supposed "
            "number of blocks.";
          return;
        }
      }
    }
  }
  grid = t_grid;
  calcCost();
}


int Chip::costOfNet(int net_id) const
{
  int x_min=nx;
  int x_max=0;
  int y_min=ny;
  int y_max=0;
  for (int b_id : graph->getNet(net_id)) {
    x_min = std::min(x_min, block_locs[b_id].first);
    x_max = std::max(x_max, block_locs[b_id].first);
    y_min = std::min(y_min, block_locs[b_id].second);
    y_max = std::max(y_max, block_locs[b_id].second);
  }
  return (x_max - x_min) + 2 * (y_max - y_min);
}