// @file:     placer.cc
// @author:   Samuel Ng
// @created:  2021-02-03
// @license:  GNU LGPL v3
//
// @desc:     Implementation of the placer.

#include <algorithm>
#include <math.h>
#include "placer.h"

#define coord_ind(x,y,nx) x+y*nx;
#define ind_coord(ind,nx) qMakePair(ind%nx, (ind-ind%nx)/nx);

using namespace pc;

Placer::Placer(sp::Chip *t_chip)
  : chip(t_chip), mt(rd())
{
  if (!t_chip->isInitialized()) {
    qWarning() << "Uninitialized chip received in constructor, placement will "
      "not be possible.";
  }
  prob_dist = std::uniform_real_distribution<float>(0.0, 1.0);
}

SAResults Placer::runPlacer(const SASettings &t_sa_settings)
{
  // refuse to run if not initialized
  if (!chip->isInitialized()) {
    qWarning() << "Chip is uninitialized. Aborting placement.";
    return SAResults();
  }

  // initialize the block positions and get the initial cost
  chip->initEmptyPlacements();  // clear all previous costs and placements
  initBlockPos();
  if (chip->numBlocks() == 1) {
    // on the off-chance that there is only one block to be placed, just return
    return SAResults();
  }

  // set RNG distribution
  ind_dist = std::uniform_int_distribution<int>(0, chip->dimX()*chip->dimY()-1);
  bid_dist = std::uniform_int_distribution<int>(0, chip->numBlocks()-1);
  // TODO verify that RNG implementation is sound (doesn't always produce the 
  // same random vars)

  // TODO initialize annealing schedule

  // flags and variables
  sa_settings = t_sa_settings;
  sa_settings.min_rw_dim = std::min(sa_settings.min_rw_dim, 
      std::min(chip->dimX(), chip->dimY()));
  bool exit_cond = false;   // exit conditions met
  int cycle_attempts = sa_settings.swap_fact * pow(chip->numBlocks(), (4./3));
  int iterations = 0;
  cycle_attempts = std::max(cycle_attempts, 1); // at least 1 attempt per cycle
  QPair<int,int> coord_a, coord_b;  // coordinates to be swapped
  int bid_a, bid_b;                 // block IDs a and b for the swap
  int update_x=0;

  emit sig_updateGui(chip);

  // start the loop with an initial temperature
  float T = initTempSV(50, 20); // this must come before the first calcCost
  int cost = chip->calcCost();
  chip->setCost(cost);
  int rw_dim = std::max(chip->dimX(), chip->dimY());  // initialize range window
  while (!exit_cond) {
    // variables that renew at every point in the schedule
    int attempts = cycle_attempts;
    int n_swaps = 0;
    long cost_accum = 0;
    long cost_accum_sq = 0;
    float p_accept_accum = 0;
    while (attempts--) {
      // pick random locs to swap
      pickLocsToSwap(coord_a, coord_b, bid_a, bid_b, rw_dim);

      // compute cost delta for the swap
      int cost_delta = chip->calcSwapCostDelta(coord_a.first, coord_a.second,
          coord_b.first, coord_b.second);

      // evaluate swap acceptance
      if (acceptCostDelta(cost_delta, T, p_accept_accum)) {
        // perform swap and update cost
        swapLocs(coord_a, coord_b);
        cost += cost_delta;
        chip->setCost(cost);
        // update std dev calculation stats
        n_swaps++;
        cost_accum += cost;
        cost_accum_sq += pow(cost, 2);
      }

      // emit signal for GUI update
      if (sa_settings.gui_up == GuiEachSwap) {
        emit sig_updateGui(chip);
        emit sig_updateChart(cost, T, -1, -1);
        if (sa_settings.show_stdout) {
          qDebug() << tr("Curr stored cost=%1,  Next T=%3, iteration=%4").arg(cost).arg(T).arg(iterations);
        }
      }
    }

    // update annealing schedule
    iterations++;
    attempts = cycle_attempts;
    if (sa_settings.use_rw) {
      updateRangeWindow(rw_dim, p_accept_accum/cycle_attempts);
    }
    if (iterations == sa_settings.max_its - 1) {
      // set T to 0 for last iteration
      T = 0;
    } else {
      // update T depending on selected schedule
      switch (sa_settings.t_schd) {
        case StdDevTUpdate:
        {
          double std_dev = sqrt(cost_accum_sq/n_swaps - pow(cost_accum/n_swaps, 2));
          T = T * exp(-0.7 * T / std_dev);
          break;
        }
        case ExpDecayTUpdate:
          T *= sa_settings.decay_b;
          break;
      }
    }

    // sanity check
    if (sa_settings.sanity_check) {
      int calc_cost = chip->calcCost();
      if (cost != calc_cost) {
        qWarning() << tr("Conflicting costs: recorded %1, calculated %2").arg(cost).arg(calc_cost);
      }
    }

    if (sa_settings.show_stdout) {
      qDebug() << tr("Curr stored cost=%1, Next T=%2, iterations=%3, avg P accept=%4, range window dim=%5").arg(cost).arg(T).arg(iterations).arg(p_accept_accum/cycle_attempts).arg(rw_dim);
    }
    if (sa_settings.gui_up <= GuiEachAnnealUpdate) {
      emit sig_updateGui(chip);
      emit sig_updateChart(cost, T, p_accept_accum/cycle_attempts, rw_dim);
    }
    update_x++;

    // evaluate exit conditions
    // TODO implement something more proper
    if (iterations == sa_settings.max_its - 1 || isnan(T)) {
      exit_cond = true;
    }
  }

  if (sa_settings.show_stdout) {
    qDebug() << "End of Simulated Annealing";
  }

  if (sa_settings.gui_up <= GuiFinalOnly) {
    emit sig_updateGui(chip);
  }

  SAResults results;
  results.cost = cost;
  results.iterations = iterations;
  return results;
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
  long cost_accum = 0;
  long cost_accum_sq = 0;
  QPair<int,int> coord_a, coord_b;  // coordinates to be swapped
  int bid_a, bid_b;                 // block IDs a and b for the swap
  for (int i=0; i<rand_moves; i++) {
    // pick random locs to swap
    pickLocsToSwap(coord_a, coord_b, bid_a, bid_b, std::max(chip->dimX(), chip->dimY()));
    // TODO switch to delta cost calc
    int cost_i = chip->calcCost();
    // perform the swap
    swapLocs(coord_a, coord_b);
    // calc difference
    int cost_f = chip->calcCost();
    // TODO remove costs[i] = cost_f - cost_i;

    cost_accum += cost_f - cost_i;
    cost_accum_sq += pow(cost_f - cost_i, 2);
  }
  float std_dev = sqrt(cost_accum_sq/rand_moves - pow(cost_accum/rand_moves, 2));
  return std_dev * T_fact;
}

void Placer::pickLocsToSwap(QPair<int,int> &coord_a, QPair<int,int> &coord_b,
    int &bid_a, int &bid_b, int rw_dim)
{
  bool chosen = false;
  while (!chosen) {
    // choose random block ID as a and any location as b, eligible if not equal
    bid_a = bid_dist(mt);
    coord_a = chip->blockLoc(bid_a);
    // TODO remove coord_b = ind_coord(ind_dist(mt), chip->dimX());
    pickCoordFromRangeWindow(coord_a, coord_b, rw_dim);
    chosen = (coord_a != coord_b);
  }
  bid_b = chip->blockIdAt(coord_b);
}

void Placer::pickCoordFromRangeWindow(const QPair<int,int> &coord_center,
    QPair<int,int> &picked_coord, int rw_dim)
{
  // if not using range window, or if the window covers entire chip, pick anywhere
  if (!sa_settings.use_rw || rw_dim == std::max(chip->dimX(), chip->dimY())) {
    picked_coord = ind_coord(ind_dist(mt), chip->dimX());
    return;
  }

  // otherwise, find the area of coverage
  QRect rw_rect(coord_center.first - std::floor(rw_dim/2), coord_center.second - std::floor(rw_dim/2),
      std::min(rw_dim, chip->dimX()), std::min(rw_dim, chip->dimY()));
  if (rw_rect.top() < 0) {
    rw_rect.moveTop(0);
  }
  if (rw_rect.left() < 0) {
    rw_rect.moveLeft(0);
  }
  if (rw_rect.right() >= chip->dimX()) {
    rw_rect.moveRight(chip->dimX()-1);
  }
  if (rw_rect.bottom() >= chip->dimY()) {
    rw_rect.moveBottom(chip->dimY()-1);
  }
  // sanity check that the range window is fully contained in the chip
  if (sa_settings.sanity_check) {
    QRect chip_rect(0, 0, chip->dimX(), chip->dimY());
    if (!chip_rect.contains(rw_rect)) {
      qWarning() << "Range window rect " << rw_rect << " not completely "
        "contained in chip rect " << chip_rect;
    }
  }

  // pick a location in the range window, retry if overlapped with coord_center
  std::uniform_int_distribution<int> rw_dist(0, rw_rect.width()*rw_rect.height()-1);
  bool eligible = false;
  int rw_ind = 0;
  while (!eligible) {
    rw_ind = rw_dist(mt);
    picked_coord = ind_coord(rw_ind, rw_rect.width());
    eligible = (picked_coord != coord_center);
  }
  // add the range window top left offset to the chosen coordinates
  picked_coord.first += rw_rect.left();
  picked_coord.second += rw_rect.top();

  // sanity check that the chosen coordinates fall within the chip
  if (sa_settings.sanity_check) {
    QRect chip_rect(0, 0, chip->dimX(), chip->dimY());
    if (!chip_rect.contains(QPoint(picked_coord.first, picked_coord.second))) {
      qWarning() << "Coordinates that fall outside the chip have been chosen.";
    }
  }

}

void Placer::swapLocs(const QPair<int,int> &coord_a, const QPair<int,int> &coord_b)
{
  int bid_a = chip->blockIdAt(coord_a);
  chip->setLocBlock(coord_a, chip->blockIdAt(coord_b));
  chip->setLocBlock(coord_b, bid_a);
}

bool Placer::acceptCostDelta(int delta, float T, float &p_accept_accum)
{
  // always accept if lower cost
  if (delta <= 0) {
    return true;
  }
  // accept with some probability according to the annealing temperature
  float prob = std::exp(- (float)delta / T);
  p_accept_accum += prob;
  return prob_dist(mt) < prob;
}

void Placer::updateRangeWindow(int &rw_dim, float p_accept)
{
  int max_dim = std::max(chip->dimX(), chip->dimY());
  if (p_accept > sa_settings.p_upper) {
    // acceptance rate too high, enlarge range window
    if (rw_dim == max_dim) {
      // range window already as big as the chip, can't expand further
      return;
    }
    rw_dim = std::min(rw_dim + sa_settings.rw_dim_delta, max_dim);
  } else if (p_accept < sa_settings.p_lower) {
    // acceptance rate too low, shrink range window
    if (rw_dim == sa_settings.min_rw_dim) {
      // range window already at minimum, can't reduce further
      return;
    }
    rw_dim = std::max(rw_dim - sa_settings.rw_dim_delta, sa_settings.min_rw_dim);
  }
  // prefer range windows with odd side-lengths
  if (rw_dim % 2 != 1) {
    rw_dim -= 1;
  }
}
