// @file:     block.cc
// @author:   Samuel Ng
// @created:  2021-02-04
// @license:  GNU LGPL v3
//
// @desc:     Implementation of the Cell primitive viewer object.

#include "cell.h"

using namespace gui;

Cell::Cell(int x, int y, int block_id)
  : x(x), y(y), block_id(block_id)
{
  setPos(x*st::Settings::sf, 2*y*st::Settings::sf);
}

QRectF Cell::boundingRect() const
{
  // the unit size is defined by the scale factor in settings
  return QRectF(0, 0, st::Settings::sf, st::Settings::sf);
}

void Cell::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  QColor fill_col;
  if (block_id == -1) {
    fill_col = QColor("#999999");  // some shade of gray for empty cells
  } else {
    fill_col = QColor("#FFFFFF");  // white for occupied cells
    // draw block ID if occupied
    painter->setBrush(QColor("#000000"));
    painter->drawText(boundingRect(), Qt::TopLeftCorner, QObject::tr("S%1").arg(block_id));
  }
  // draw rectangle
  painter->setBrush(fill_col);
  painter->drawRect(boundingRect());
}
