/*!
  \file net.h
  \brief Net class for showing net connectivities on the chip.
  \author Samuel Ng
  \date 2021-02-04 created
  \copyright GNU LGPL v3
  */

#ifndef _GUI_CELL_H_
#define _GUI_CELL_H_

#include <QtWidgets>
#include "spatial.h"

namespace gui {

  //! A primitive graphical element that displays all connectivities related to a net.
  class Net : public QGraphicsItem
  {
  public:
    //! Constructor taking a list of QPoints, each holding the x and y position 
    //! that the net connects. The first value is the "root" where all of the 
    //! other lines connect to.
    Net(int net_id, int n_nets, int nx, int ny, 
        const QList<QPair<int,int>> &t_coords);

    //! Update the coordinates list.
    void updateCoords(const QList<QPair<int,int>> &t_coords);

    //! Overriden method to return the proper bounding rectangle of this cell.
    virtual QRectF boundingRect() const override;

    //! Overriden method to paint this cell on scene.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

  private:

    int net_id;                     //!< The assciated net ID.
    int nx;                         //!< x elements.
    int ny;                         //!< y elements.
    QColor net_col;                 //!< The color to use for this net.
    QVector<QPoint> coords;         //!< A list of all coordinates to connect.
    QRect coord_bounding_rect;      //!< Bounding rect in cell coordinate space.
    QVector<QLineF> lines;          //!< Lines to be drawn.

  };

}

#endif
