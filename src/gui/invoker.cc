// @file:     invoker.cc
// @author:   Samuel Ng
// @created:  2021-02-15
// @license:  GNU LGPL v3
//
// @desc:     Net class implementation

#include "invoker.h"

using namespace gui;

Invoker::Invoker(QWidget *parent)
  : QWidget(parent)
{
  initGui();
}

void Invoker::invokePlacement()
{
  // take settings from GUI options
  pc::SASettings sa_set;
  sa_set.gui_up = static_cast<pc::GuiUpdate>(cbb_gui_up->currentIndex());
  sa_set.t_schd = static_cast<pc::TSchd>(cbb_t_schd->currentIndex());
  sa_set.decay_b = sb_decay_b->value();
  sa_set.swap_fact = sb_swap_fact->value();
  sa_set.max_its = sb_max_its->value();
  sa_set.max_its_cost_unchanged = sb_max_its_cost_unchanged->value();
  sa_set.use_rw = gb_use_rw->isChecked();
  sa_set.p_upper = sb_p_upper->value();
  sa_set.p_lower = sb_p_lower->value();
  sa_set.min_rw_dim = sb_min_rw_dim->value();
  sa_set.rw_dim_delta = sb_rw_dim_delta->value();
  sa_set.sanity_check = cb_sanity_check->isChecked();
  sa_set.show_stdout = cb_show_stdout->isChecked();

  emit sig_runPlacement(sa_set);
}

void Invoker::initGui()
{
  // get an instance of SASettings with default settings
  pc::SASettings sa_set;

  // init gui elements

  // temperature schedule
  cbb_t_schd = new QComboBox();
  cbb_t_schd->addItem("Exponential decay");
  cbb_t_schd->addItem("Dynamic standard deviation update");
  cbb_t_schd->setCurrentIndex(sa_set.t_schd);
  cbb_t_schd->setToolTip("Temperature schedules:\n"
      "Dynamic: T_new = T_old e^{-0.7 T_old / sigma}\n"
      "Exp Decay: T_new = beta * T_old");

  // temperature decay factor if exponential decay
  sb_decay_b = new QDoubleSpinBox();
  sb_decay_b->setDecimals(3);
  sb_decay_b->setSingleStep(0.001);
  sb_decay_b->setRange(0.001, 0.999);
  sb_decay_b->setValue(sa_set.decay_b);
  sb_decay_b->setEnabled(cbb_t_schd->currentIndex() == pc::ExpDecayTUpdate);

  // swap multiplier
  sb_swap_fact = new QSpinBox();
  sb_swap_fact->setSingleStep(10);
  sb_swap_fact->setRange(0, 1000);
  sb_swap_fact->setValue(sa_set.swap_fact);

  // max iterations
  sb_max_its = new QSpinBox();
  sb_max_its->setSingleStep(100);
  sb_max_its->setRange(1000, 1000000);
  sb_max_its->setValue(sa_set.max_its);

  // max iterations cost unchanged
  sb_max_its_cost_unchanged = new QSpinBox();
  sb_max_its_cost_unchanged->setSingleStep(100);
  sb_max_its_cost_unchanged->setRange(100, 100000);
  sb_max_its_cost_unchanged->setValue(sa_set.max_its_cost_unchanged);

  // range window settings
  gb_use_rw = new QGroupBox("Range Window");
  QFormLayout *fl_rw = new QFormLayout();
  gb_use_rw->setLayout(fl_rw);
  gb_use_rw->setCheckable(true);
  gb_use_rw->setChecked(sa_set.use_rw);

  // upper P_accept bound to trigger range window enlargement
  sb_p_upper = new QDoubleSpinBox();
  sb_p_upper->setSingleStep(0.01);
  sb_p_upper->setRange(0, 1);
  sb_p_upper->setValue(sa_set.p_upper);
  fl_rw->addRow("P_accept_upper_bound", sb_p_upper);

  // lower P_accept bound to trigger range window shrinkage
  sb_p_lower = new QDoubleSpinBox();
  sb_p_lower->setSingleStep(0.01);
  sb_p_lower->setRange(0, 1);
  sb_p_lower->setValue(sa_set.p_lower);
  fl_rw->addRow("P_accept lower bound", sb_p_lower);

  // minimum range window dimensions (don't shrink further than this)
  sb_min_rw_dim = new QSpinBox();
  sb_min_rw_dim->setMinimum(3);
  sb_min_rw_dim->setValue(sa_set.min_rw_dim);
  fl_rw->addRow("Minimum side length", sb_min_rw_dim);

  // range window dimension update delta
  sb_rw_dim_delta = new QSpinBox();
  sb_rw_dim_delta->setMinimum(1);
  sb_rw_dim_delta->setValue(sa_set.rw_dim_delta);
  fl_rw->addRow("Side length step size", sb_rw_dim_delta);

  // GUI update frequency
  cbb_gui_up = new QComboBox();
  cbb_gui_up->addItem("Every swap action");
  cbb_gui_up->addItem("End of every iteration");
  cbb_gui_up->addItem("Final result only");
  cbb_gui_up->setCurrentIndex(sa_set.gui_up);

  // sanity check
  cb_sanity_check = new QCheckBox("Run sanity checks");
  cb_sanity_check->setChecked(sa_set.sanity_check);

  // show terminal output
  cb_show_stdout = new QCheckBox("Show terminal output");
  cb_show_stdout->setChecked(sa_set.show_stdout);

  // submit button
  QPushButton *pb_run_placement = new QPushButton("Run Placement");
  pb_run_placement->setShortcut(tr("CTRL+R"));

  // connect signals
  connect(cbb_t_schd, QOverload<int>::of(&QComboBox::currentIndexChanged),
      [this]() {sb_decay_b->setEnabled(cbb_t_schd->currentIndex() == pc::ExpDecayTUpdate);});
  connect(pb_run_placement, &QAbstractButton::released, this, &Invoker::invokePlacement);

  // add items to layout
  QFormLayout *fl_gen = new QFormLayout();
  fl_gen->addRow("GUI updates", cbb_gui_up);
  fl_gen->addRow("Schedule", cbb_t_schd);
  fl_gen->addRow("Decay factor", sb_decay_b);
  fl_gen->addRow("Num moves factor", sb_swap_fact);
  fl_gen->addRow("Max iterations", sb_max_its);
  fl_gen->addRow("Exit if cost unchanged for iters", sb_max_its_cost_unchanged);

  QVBoxLayout *vl_main = new QVBoxLayout();
  vl_main->addLayout(fl_gen);
  vl_main->addWidget(cb_sanity_check);
  vl_main->addWidget(cb_show_stdout);
  vl_main->addWidget(gb_use_rw);
  vl_main->addWidget(pb_run_placement);

  setLayout(vl_main);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
}
