// @file:     placer.cc
// @author:   Samuel Ng
// @created:  2021-02-03
// @license:  GNU LGPL v3
//
// @desc:     Implementation of the placer.

#include <algorithm>
#include "placer.h"

#define coord_ind(x,y,nx) x+y*nx;
#define ind_coord(ind,nx) qMakePair(ind%nx, (ind-ind%nx)/nx);

using namespace pc;

Placer::Placer(sp::Chip *t_chip)
  : chip(t_chip), mt(rd())
{
  prob_dist = std::uniform_real_distribution<float>(0.0, 1.0);
  // TODO implement
}

void Placer::runPlacer(const SASettings &t_sa_settings)
{
  // initialize the block positions and get the initial cost
  initBlockPos();
  if (chip->numBlocks() == 1) {
    // on the off-chance that there is only one block to be placed, just return
    return;
  }

  // set RNG distribution
  ind_dist = std::uniform_int_distribution<int>(0, chip->dimX()*chip->dimY()-1);
  bid_dist = std::uniform_int_distribution<int>(0, chip->numBlocks()-1);
  // TODO verify that RNG implementation is sound (doesn't always produce the 
  // same random vars)

  // TODO initialize annealing schedule

  // flags and variables
  sa_settings = t_sa_settings;
  bool exit_cond = false;   // exit conditions met
  int cycle_attempts = sa_settings.swap_fact * pow(chip->numBlocks(), (4./3));
  int step_until_exit = 2000; // TODO change this to some dynamic thing
  cycle_attempts = std::max(cycle_attempts, 1); // at least 1 attempt per cycle
  //float T_init = 500; // TODO make configurable
  float T_init = initTempSV(50, 20);
  QPair<int,int> coord_a, coord_b;  // coordinates to be swapped
  int bid_a, bid_b;                 // block IDs a and b for the swap
  int update_x=0;

  emit sig_updateGui(chip);

  // start the loop with an initial temperature
  int cost = chip->calcCost();
  chip->setCost(cost);
  float T = T_init;
  int attempts = cycle_attempts;
  while (!exit_cond) {
    while (attempts > 0) {
      // pick random locs to swap
      pickLocsToSwap(coord_a, coord_b, bid_a, bid_b);

      // compute cost delta for the swap
      int cost_delta = chip->calcSwapCostDelta(coord_a.first, coord_a.second,
          coord_b.first, coord_b.second);

      // evaluate swap acceptance
      if (acceptCostDelta(cost_delta, T)) {
        // perform swap and update cost
        swapLocs(coord_a, coord_b);
        cost += cost_delta;
        chip->setCost(cost);
      }

      // emit signal for GUI update
      // TODO remove hard-coded step_until_exit cond
      if (sa_settings.gui_up == GuiEachSwap) {
        emit sig_updateGui(chip);
        qDebug() << tr("Curr stored cost=%1,  Next T=%3, till_exit=%4").arg(cost).arg(T).arg(step_until_exit);
      }

      attempts--;
    }

    // update annealing schedule
    T *= sa_settings.decay_b;
    attempts = cycle_attempts;
    step_until_exit--;
    if (sa_settings.crunch && step_until_exit==1) {
      T = 0;
    }
    //qDebug() << tr("Curr stored cost=%1, calculated cost=%2, Next T=%3, till_exit=%4").arg(cost).arg(chip->calcCost()).arg(T).arg(step_until_exit);
    qDebug() << tr("Curr stored cost=%1, Next T=%3, till_exit=%4").arg(cost).arg(T).arg(step_until_exit);
    if (sa_settings.gui_up <= GuiEachAnnealUpdate) {
      emit sig_updateGui(chip);
      emit sig_updateChart(cost, T);
    }
    update_x++;

    // evaluate exit conditions
    // TODO implement something more proper
    if (step_until_exit == 0) {
      exit_cond = true;
    }

    // TODO extra SA optimizations if needed
  }

  qDebug() << "End of Simulated Annealing";

  if (sa_settings.gui_up <= GuiFinalOnly) {
    emit sig_updateGui(chip);
  }
}

void Placer::initBlockPos()
{
  int nx = chip->dimX();
  int ny = chip->dimY();
  // list of unoccupied grid indices
  QList<int> grid_inds;
  for (int gid=0; gid<nx*ny; gid++) {
    grid_inds.append(gid);
  }
  // place block by block
  for (int bid=0; bid<chip->numBlocks(); bid++) {
    std::uniform_int_distribution<int> dis(0, grid_inds.size()-1);
    int rand_ind = dis(mt);
    QPair<int,int> loc = ind_coord(grid_inds[rand_ind], nx);
    chip->setLocBlock(loc, bid);
    grid_inds.removeAt(rand_ind);
  }
}

float Placer::initTempSV(int rand_moves, float T_fact)
{
  QVector<int> costs(rand_moves);
  QPair<int,int> coord_a, coord_b;  // coordinates to be swapped
  int bid_a, bid_b;                 // block IDs a and b for the swap
  for (int i=0; i<rand_moves; i++) {
    // pick random locs to swap
    pickLocsToSwap(coord_a, coord_b, bid_a, bid_b);
    int cost_i = chip->calcCost();
    // perform the swap
    swapLocs(coord_a, coord_b);
    // calc difference
    int cost_f = chip->calcCost();
    costs[i] = cost_f - cost_i;
  }
  float sum = std::accumulate(costs.begin(), costs.end(), 0.0);
  float mean = sum / costs.size();
  float sq_sum = std::inner_product(costs.begin(), costs.end(), costs.begin(), 0.0);
  float stdev = std::sqrt(sq_sum / costs.size() - mean * mean);
  return stdev * T_fact;
}

void Placer::pickLocsToSwap(QPair<int,int> &coord_a, QPair<int,int> &coord_b,
    int &bid_a, int &bid_b)
{
  bool chosen = false;
  while (!chosen) {
    // choose random block ID as a and any location as b, eligible if not equal
    bid_a = bid_dist(mt);
    coord_a = chip->blockLoc(bid_a);
    coord_b = ind_coord(ind_dist(mt), chip->dimX());
    chosen = (coord_a != coord_b);
  }
  bid_b = chip->blockIdAt(coord_b);
}

void Placer::swapLocs(const QPair<int,int> &coord_a, const QPair<int,int> &coord_b)
{
  int bid_a = chip->blockIdAt(coord_a);
  chip->setLocBlock(coord_a, chip->blockIdAt(coord_b));
  chip->setLocBlock(coord_b, bid_a);
}

bool Placer::acceptCostDelta(int delta, float T)
{
  // always accept if lower cost
  if (delta <= 0) {
    return true;
  }
  // accept with some probability according to the annealing temperature
  float prob = std::exp(- (float)delta / T);
  return prob_dist(mt) < prob;
}
