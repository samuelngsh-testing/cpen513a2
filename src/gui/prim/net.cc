// @file:     net.cc
// @author:   Samuel Ng
// @created:  2021-02-04
// @license:  GNU LGPL v3
//
// @desc:     Net class implementation

#include "net.h"
#include "gui/settings.h"

using namespace gui;

Net::Net(int net_id, int n_nets, int nx, int ny, 
    const QList<QPair<int,int>> &t_coords)
  : net_id(net_id), nx(nx), ny(ny)
{
  net_col = st::Settings::colorGenerator(net_id, n_nets);
  lines.resize(t_coords.size() - 1);
  updateCoords(t_coords);
}

void Net::updateCoords(const QList<QPair<int,int>> &t_coords)
{
  // update params
  if (coords.size() == 0) {
    coords.resize(t_coords.size());
  }
  for (int i=0; i<t_coords.size(); i++) {
    coords[i] = QPoint(t_coords[i].first, t_coords[i].second);
  }

  // update top left position
  setPos(st::Settings::sf/2,st::Settings::sf/2);  // slight offset for centering
}

QRectF Net::boundingRect() const
{
  return QRectF(QPointF(0,0), QPointF(nx,ny)*st::Settings::sf);
}

void Net::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  painter->setPen(net_col);
  QPointF base_coord = QPointF(coords[0]) * st::Settings::sf;
  base_coord.setY(base_coord.y()*2);  // scale for routing tracks
  for (int i=1; i<coords.size(); i++) {
    QPointF t_coord = QPointF(coords[i])*st::Settings::sf;
    t_coord.setY(t_coord.y()*2);  // scale for routing tracks
    painter->drawLine(QLineF(base_coord, t_coord));
  }
}
