/*!
  \file cell.h
  \brief Cell class for showing blocks and empty space on the viewer.
  \author Samuel Ng
  \date 2021-02-04
  \copyright GNU LGPL v3
  */

#ifndef _GUI_BLOCK_H_
#define _GUI_BLOCK_H_

#include <QtWidgets>
#include "gui/settings.h"
#include "spatial.h"

namespace gui {

  //! A primitive graphical element that displays individual cells on the chip grid.
  class Cell : public QGraphicsItem
  {
  public:
    //! Constructor taking the x and y coordinates of the Cell. The block ID can
    //! optionally be provided, set to -1 if empty cell.
    Cell(int x, int y, int block_id=-1);

    //! Set a new block ID.
    void setBlockId(int t_block_id) {block_id = t_block_id;}

    //! Overriden method to return the proper bounding rectangle of this cell.
    virtual QRectF boundingRect() const override;

    //! Overriden method to paint this cell on scene.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

  private:

    int x;        //!< x position on the chip.
    int y;        //!< y position on the chip (neglecting inter-row spaces).
    int block_id; //!< Block ID or -1 if empty.


  };

}

#endif
