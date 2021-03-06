/*! 
  \file placer.h
  \brief The simulated annealing block placement class.
  \author Samuel Ng
  \date 2021-02-03 created
  \copyright GNU LGPL v3
  */

#ifndef _PC_PLACER_H_
#define _PC_PLACER_H_

#include <QObject>
#include <random>
#include "spatial.h"

// placer namespace
namespace pc{

  //! The annealing temperature schedule.
  enum class TSchd {
    //! Exponential decay temperature.
    ExpDecayTUpdate,
    //! Dynamic temperature update.
    StdDevTUpdate
  };
  enum GuiUpdate {GuiEachSwap, GuiEachAnnealUpdate, GuiFinalOnly};

  //! Simulated annealer settings.
  struct SASettings
  {
    // GUI settings
    GuiUpdate gui_up=GuiEachAnnealUpdate; //!< GUI update frequency.

    // annealing schedule settings
    TSchd t_schd=TSchd::StdDevTUpdate;  //!< Temperature schedule.
    float decay_b=0.995;            //!< Base factor for exponential decay T.
    float swap_fact=25;             //!< swap_fact * n_blocks^(4/3) moves are made per cycle
    int max_its=3000;               //!< maximum iterations
    int max_its_cost_unchanged=200; //!< exit main loop if cost unchanged for this many cycles

    // range window params
    bool use_rw=true;     //!< Specify whether range window should be used.
    float p_upper=0.55;   //!< Upper P_accept bound to trigger range window enlargement.
    float p_lower=0.35;   //!< Lower P_accept bount to trigger range window shrinkage.
    int min_rw_dim=5;     //!< Do not reduce range window dimensions below this dim.
    int rw_dim_delta=10;  //!< Increase or reduce range window dimensions by this much.

    // other runtime params
    bool sanity_check=false;  //!< Run additional sanity checks to help find bugs.
    bool show_stdout=false;   //!< Whether to show terminal output
  };

  //! Results to return.
  struct SAResults
  {
    int cost=-1;              //!< Final cost of the layout.
    int iterations=-1;        //!< Total iterations used.
  };

  //! Simulated annealing placement algorithm.
  class Placer : public QObject
  {
  Q_OBJECT

  public:
    //! Constructor taking a Chip pointer.
    Placer(sp::Chip *);

    //! Destructor.
    ~Placer() {};

    //! Run the placer.
    SAResults runPlacer(const SASettings &sa_settings);

    //! Place blocks onto random grid locations.
    void initBlockPos();

  signals:
    //! Signal for updating GUI with the current chip state.
    void sig_updateGui(sp::Chip *);

    //! Signal for updating GUI chart.
    void sig_updateChart(int cost, float T, float p_accept, int rw_dim);

  private:

    //! Decide on initial temperature with Sangiovanni-Vincentelli approach.
    float initTempSV(int rand_moves, float T_fact);

    //! Pick random blocks to swap. Directly write to the provided refs.
    void pickLocsToSwap(QPair<int,int> &coord_a, QPair<int,int> &coord_b,
        int &bid_a, int &bid_b, int rw_dim);

    //! Pick coord from range window centered around a cell. If the centering 
    //! point causes the range window to go out of bound, then shift the window 
    //! until fitting is possible.
    void pickCoordFromRangeWindow(const QPair<int,int> &coord_center, 
        QPair<int,int> &picked_coord, int rw_dim);

    //! Swap the two provided locations.
    void swapLocs(const QPair<int,int> &coord_a, const QPair<int,int> &coord_b);

    //! Decide whether to accept a given cost difference. Adds the computed 
    //! acceptance probability to the provided p_accept_accum.
    bool acceptCostDelta(int delta, float T, float &p_accept_accum);

    //! Update range window size according to the given acceptance probability.
    void updateRangeWindow(int &rw_dim, float p_accept);

    // Private variables
    sp::Chip *chip;         //!< Pointer to the chip.
    SASettings sa_settings; //!< Simulated annealer settings.
    std::random_device rd;  //!< Random device.
    std::mt19937 mt;        //!< Use the Mersenne Twister PRNG.
    std::uniform_int_distribution<int> ind_dist;      //!< Random distribution for indices.
    std::uniform_int_distribution<int> bid_dist;      //!< Random distribution for block IDs.
    std::uniform_real_distribution<float> prob_dist;  //!< Random distribution for probabilities.
  };

}


#endif
