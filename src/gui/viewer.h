// @file:     viewer.h
// @author:   Samuel Ng
// @created:  2021-02-02
// @license:  GNU LGPL v3
//
// @desc:     Show the problem grid and the inter-cell connectivities

#ifndef _GUI_VIEWER_H_
#define _GUI_VIEWER_H_

#include <QtWidgets>
#include "spatial.h"
#include "prim/cell.h"
#include "prim/net.h"

namespace gui {

  class Viewer : public QGraphicsView
  {
    Q_OBJECT

  public:

    //! Constructor.
    Viewer(QWidget *parent=nullptr);

    //! Destructor.
    ~Viewer();

    //! Instruct viewer to show the provided problem.
    void showChip(sp::Chip *t_chip);

    //! Instruct viewer to clear any existing problems.
    void clearProblem();

    //! Fit problem in viewport.
    void fitProblemInView();

  private:

    //! Initialize the viewer's GUI elements.
    void initViewer();

    //! Update cell states.
    void updateCells();

    //! Update nets according to the current chip state.
    void updateNets();

    // Private variables
    QGraphicsScene *scene=nullptr;  //!< Pointer to the scene object.
    sp::Chip *chip=nullptr;         //!< Pointer to the chip currently shown.
    QVector<QVector<Cell*>> cells;  //!< Grid of cell pointers.
    QVector<Net*> nets;             //!< List of net pointers.

  };
}

#endif
