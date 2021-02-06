// @file:     spatial.h
// @author:   Samuel Ng
// @created:  2021-02-02
// @license:  GNU LGPL v3
//
// @desc:     Spatial definitions (chip and the blocks that go into it).

#ifndef _SP_SPATIAL_H_
#define _SP_SPATIAL_H_

#include <QtWidgets>

namespace sp {

  //! Graph-like data structure with nodes denoting blocks. This class has no 
  //! knowledge about the actual spatial placement of the blocks.
  class Graph
  {
  public:
    //! Constructor taking the number of blocks and nets expected.
    Graph(int n_blocks, int n_nets);

    //! Set the connected blocks for the specified net ID.
    void setNet(int net_id, const QList<int> &conn_blocks);

    //! Check check all blocks have some connection.
    bool allBlocksConnected() const;

    //! Return all nets.
    const QVector<QList<int>> &getNets() {return nets;}

    //! Return the net with the specified ID.
    const QList<int> &getNet(int id) {return nets[id];}

    //! Return block net connectivity records.
    const QVector<QList<int>> &allBlockNets() {return all_block_net_ids;}
    
    //! Return the net connectivity of a single net.
    const QList<int> &blockNets(int id) {return all_block_net_ids[id];}

  private:

    //! List of nets where each net consists of a list of block IDs.
    QVector<QList<int>> nets;
    //! For each block, store a list of associated net IDs.
    QVector<QList<int>> all_block_net_ids;
  };


  //! A chip containing certain numbers of rows and columns for Blocks to be
  //! placed onto.
  class Chip
  {
  public:
    //! Constructor taking the problem file path to be read.
    Chip(const QString &f_path);

    //! Destructor.
    ~Chip();

    //! Return whether this chip has been successfully initialized.
    bool isInitialized() {return initialized;}

    //! Return nx.
    int dimX() const {return nx;}

    //! Return ny.
    int dimY() const {return ny;}

    //! Return the number of blocks.
    int numBlocks() const {return n_blocks;}

    //! Return the number of nets.
    int numNets() const {return n_nets;}

    //! Return block IDs associated with a net
    QList<int> netBlockIds(int net_id) const;

    //! Return coordinates associated with a net
    QList<QPair<int,int>> netCoords(int net_id) const;

    //! Set the cell coordinates of a block.
    void setLocBlock(const QPair<int,int> &loc, int block_id);

    //! Return the block id at the specified cell coordinates.
    int blockIdAt(int x, int y) {return grid[x][y];}

    //! Overrided function taking a pair that represents the cell coordinates.
    int blockIdAt(QPair<int,int> coord) {return grid[coord.first][coord.second];}

    //! Return the cell coordinates of the specified block as a pair.
    QPair<int,int> blockLoc(int block_id) {return block_locs[block_id];}

    //! Re-compute the cost of the current placement from scratch.
    //! Optional bool to update the internal cost value as well.
    int calcCost(bool update_internal=false);

    //! Compute the cost delta for executing a swap between two coordinates.
    //! Does not update the internal cost.
    //! TODO delete this later, let the placement algorithm deal with this.
    int calcSwapCostDelta(int x1, int y1, int x2, int y2);

    //! Update the cost by the provided delta (without checking whether it's right).
    void addCostDelta(int delta);

    //! Return the current cost of the placement, -1 if no placement.
    int getCost() const {return cost;}

    //! Set the grid to the provided 2D matrix.
    void setGrid(const QVector<QVector<int>> &t_grid, bool skip_validation=false);
    
  private:

    //! Calculate and return the cost of the specified net ID.
    int costOfNet(int net_id) const;

    // Private variables
    Graph *graph=nullptr;   //!< Graph object that holds the connectivities.
    bool initialized=false; //!< Indication of whether this chip is initialized.
    int cost=-1;            //!< Current cost of the placement, -1 if no placement.
    int nx=0;               //!< Max cell count in the x direction.
    int ny=0;               //!< Max cell count in the y direction.
    int n_blocks=0;         //!< Number of blocks in the problem.
    int n_nets=0;           //!< Number of nets in the problem.
    QVector<QVector<int>> grid; //!< A grid storing the block ID associated to each cell. -1 if empty.
    QVector<QPair<int,int>> block_locs; //!< Store all block locations.

  };

}

#endif
