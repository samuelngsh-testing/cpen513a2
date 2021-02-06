// @file:     placer.h
// @author:   Samuel Ng
// @created:  2021-02-03
// @license:  GNU LGPL v3
//
// @desc:     The simulated annealing block placement class.

#ifndef _PC_PLACER_H_
#define _PC_PLACER_H_

#include <QObject>
#include <random>
#include "spatial.h"

// placer namespace
namespace pc{

  enum TSchd {ExpDecayT};
  enum GuiUpdate {GuiEachSwap, GuiEachAnnealUpdate, GuiFinalOnly};

  //! Simulated annealer settings.
  struct SASettings
  {
    GuiUpdate gui_up=GuiEachSwap; //!< GUI update frequency.
    TSchd t_schd=ExpDecayT;       //!< Temperature schedule.
    float decay_b=0.998;          //!< Base factor for exponential decay T.
    // TODO initial temperature schemes (week 4 slide 16)
    // TODO vars for compliecated temperature update (week 4 slide 17)
    float swap_fact=1;         // TODO replace with sensible
    //float swap_fact=10;         //!< swap_fact * n_blocks^(4/3) moves are made per cycle
  };

  //! The block placer class using simulated annealing to minimize placement
  //! costs.
  class Placer : public QObject
  {
  Q_OBJECT

  public:
    //! Constructor taking a Chip pointer.
    Placer(sp::Chip *);

    //! Destructor.
    ~Placer() {};

    //! Run the placer.
    void runPlacer(const SASettings &sa_settings);

  signals:
    //! Signal for updating GUI with the current chip state.
    void sig_updateGui(sp::Chip *);

    //! Signal for updating GUI chart.
    void sig_updateChart(int cost, float T);

  private:

    //! Place blocks onto grid in sequence.
    void initBlockPos();

    //! Pick random blocks to swap. Directly write to the provided refs.
    void pickLocsToSwap(QPair<int,int> &coord_a, QPair<int,int> &coord_b,
        int &bid_a, int &bid_b);

    //! Swap the two provided locations.
    void swapLocs(const QPair<int,int> &coord_a, const QPair<int,int> &coord_b);

    //! Decide whether to accept a given cost difference.
    bool acceptCostDelta(int delta, float T);

    // Private variables
    sp::Chip *chip;         //!< Pointer to the chip.
    SASettings sa_settings; //!< Simulated annealer settings.
    std::random_device rd;  //!< Random device.
    std::mt19937 mt;        //!< Use the Mersenne Twister PRNG.
    std::uniform_int_distribution<int> ind_dist;      //! Random distribution for indices.
    std::uniform_real_distribution<float> prob_dist;  //! Random distribution for probabilities.
  };

}


#endif
