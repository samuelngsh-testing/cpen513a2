// @file:     viewer.cc
// @author:   Samuel Ng
// @created:  2021-02-02
// @license:  GNU LGPL v3
//
// @desc:     Implementation of the MainWindow class.

#include "viewer.h"

using namespace gui;

Viewer::Viewer(QWidget *parent)
  : QGraphicsView(parent)
{
  initViewer();
}

Viewer::~Viewer()
{
  clearProblem();
}

void Viewer::showChip(sp::Chip *t_chip)
{
  if (!t_chip->isInitialized()) {
    qDebug() << "Provided chip is not initialized, aborting.";
    return;
  }
  // create GUI objects if new chip
  if (chip != t_chip) {
    // clear the viewer
    clearProblem();

    // create graphical objects from scratch
    chip = t_chip;
    cells.resize(chip->dimX());
    for (int x=0; x<chip->dimX(); x++) {
      cells[x].resize(chip->dimY());
      for (int y=0; y<chip->dimY(); y++) {
        Cell *cell = new Cell(x, y, chip->blockIdAt(x, y));
        cells[x][y] = cell;
        scene->addItem(cell);
      }
    }
  } else {
    updateCells();
  }

  // update nets if there exists any
  if (chip->numNets() > 0) {
    updateNets();
  }

  fitProblemInView();
}

void Viewer::clearProblem()
{
  // clear GUI objects
  scene->clear();

  // clear relevant vars
  cells.clear();
  nets.clear();
  chip = nullptr;
}

void Viewer::fitProblemInView()
{
  if (chip != nullptr && chip->isInitialized()) {
    qreal sf = st::Settings::sf;  // scaling factor
    QRectF rect(0, 0, chip->dimX()*sf, chip->dimY()*sf*2);
    setSceneRect(rect);
    fitInView(rect, Qt::KeepAspectRatio);
  }
}

void Viewer::initViewer()
{
  scene = new QGraphicsScene(this);
  setScene(scene);
  setMinimumSize(300,300);
}

void Viewer::updateCells()
{
  // update the graphical block ID at each location
  for (int x=0; x<chip->dimX(); x++) {
    for (int y=0; y<chip->dimY(); y++) {
      cells[x][y]->setBlockId(chip->blockIdAt(x,y));
    }
  }
}

void Viewer::updateNets()
{
  // if net list is empty, recreate from scratch
  if (nets.isEmpty()) {
    nets.resize(chip->numNets());
    for (int net_id=0; net_id<nets.size(); net_id++) {
      nets[net_id] = new Net(net_id, nets.size(), chip->dimX(), chip->dimY(), 
          chip->netCoords(net_id));
      scene->addItem(nets[net_id]);
    }
  } else {
    for (int net_id=0; net_id<nets.size(); net_id++) {
      nets[net_id]->updateCoords(chip->netCoords(net_id));
    }
  }
}
