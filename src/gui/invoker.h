// @file:     invoker.h
// @author:   Samuel Ng
// @created:  2021-02-15
// @license:  GNU LGPL v3
//
// @desc:     GUI for tweaking invocation parameters.

#ifndef _GUI_INVOKER_H_
#define _GUI_INVOKER_H_

#include <QtWidgets>
#include "placer/placer.h"

namespace gui {

  class Invoker : public QWidget
  {
    Q_OBJECT

  public:
    //! Constructor.
    Invoker(QWidget *parent=nullptr);

    //! Destructor.
    ~Invoker() {};

    //! Invoke placement with the current GUI settings.
    void invokePlacement();

  signals:
    //! Emit SASettings for invocation.
    void sig_runPlacement(pc::SASettings sa_set);

  private:

    //! Initialize the widget.
    void initGui();

    // Private variables, the names basically correspond to pc::SASettings.
    QComboBox *cbb_t_schd;
    QDoubleSpinBox *sb_decay_b;
    QSpinBox *sb_swap_fact;
    QGroupBox *gb_use_rw;
    QDoubleSpinBox *sb_p_upper;
    QDoubleSpinBox *sb_p_lower;
    QSpinBox *sb_min_rw_dim;
    QSpinBox * sb_rw_dim_delta;
    QCheckBox *cb_sanity_check;
    QComboBox *cbb_gui_up;
  };


}


#endif
