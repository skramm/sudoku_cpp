/**************************************************************************

    This file is part of sudoku_cpp.
    homepage: https://github.com/skramm/sudoku_cpp

    Author & Copyright 2017 Sebastien Kramm

    Contact: firstname.lastname@univ-rouen.fr

    Licence: LGPL v3

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

	See included lgpl.txt and gpl.txt files.

**************************************************************************/

/**
\file
\brief Implementation for XY-chains algorithm
*/


#include "xy_chains.h"

#include "grid.h"
#include "header.h"
#include "algorithms.h"

#ifndef TESTMODE

#include <boost/graph/adjacency_list.hpp>

//----------------------------------------------------------------------------
/// Vertex datatype, with BGL. Holds a cell position
/// \todo Checkout if it can be merged with \c GraphNode
struct GraphNode2
{
	index_t idx;   ///< index in the set of cells having two candidates
};

/// Edge datatype
/// \todo Checkout if it can be merged with \c GraphEdge
struct GraphEdge2
{
//	En_LinkType link_type;
//	EN_ORIENTATION link_orient;
};

/// A graph datatype, with BGL
typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS,
	GraphNode2,
	GraphEdge2
	> graph2_t;

#endif // TESTMODE

typedef typename boost::graph_traits<graph2_t>::vertex_descriptor vertex2_t;
typedef typename boost::graph_traits<graph2_t>::edge_descriptor   edge2_t;

//-------------------------------------------------------------------
/// Returns true if cell are "linkable", that is they are:
/**
- either on same row
- either on same column
- either in same block
*/
bool
areLinkable( const Cell2& c1, const Cell2& c2 )
{
	if( c1._pos.first == c2._pos.first )
		return true;
	if( c1._pos.second == c2._pos.second )
		return true;
	if( GetBlockIndex(c1._pos) == GetBlockIndex(c2._pos) )
		return true;
	return false;
}
//-------------------------------------------------------------------
/// Explore set of Cells having 2 candidates and add them to the graph, recursively
/**
Stop condition: when we can't add any more nodes
*/
void
buildGraphRecursive(
	graph2_t&          graph,     ///< graph
	vertex2_t          startV,    ///< graph starting vertex
	vertex2_t          currV,     ///< current vertex
	index_t            whichOne,  ///< 0 or 1, the index on the value of the origin vertex that we are considering
	std::vector<Cell2>& v_cells   ///< the set of cells holding 2 candidates.
)
{
	static int iter;
	auto curr_idx = graph[currV].idx;           // index on the cell we are considering
	auto& curr_cell = v_cells.at(curr_idx);     // we fetch the cell
	COUT( "iter " << ++iter
		<< " curr_idx=" << (int)curr_idx
		<< " pos=" << curr_cell._pos
		<< " graph: nb_nodes=" << boost::num_vertices(graph) );

	value_t val1 = curr_cell.get(whichOne);
	value_t val2 = curr_cell.get(whichOne?0:1);

	for( index_t idx=0; idx<v_cells.size(); idx++ )
	{
		if( idx != curr_idx )          // if not the same cell !
		{
			auto& cell = v_cells[idx];
			if(
				cell._chainRole == CR_Unused  // if cell is not used yet
				&&
				cell._chainRole != CR_Start  // and not the starting cell
			)
			{
				if( areLinkable( cell, curr_cell ) )  // if cells are on same row/col/block
				{
					value_t vc1 = cell.get(0);
					value_t vc2 = cell.get(1);
					COUT( "vc1=" << (int)vc1 << " vc2=" << (int)vc2 );
					if( val2 == vc1 || val2 == vc2 )   // if the cell holds the OTHER value of the starting cell,
						if( val1 != vc1 && val1 != vc2 )  // and it does NOT hold as other value the considered value
						{                                 // THEN, its a new node in the graph !
							auto newV = boost::add_vertex( graph );
							graph[newV].idx = idx;
							cell._chainRole = CR_Used;
							boost::add_edge( currV, newV, graph );

							if( 0 /*todo */)                      // if cell can be joined to starting cell
							{
								cell._chainRole = CR_End;
								auto finalEdge = boost::add_edge( startV, newV, graph ).first;
								graph[finalEdge].isFinalEdge = true;
							}
							else
								buildGraphRecursive( graph, startV, newV, (whichOne?0:1), v_cells );
						}
				}
				else
					COUT( "NOT linkable" );
			}
		}
	}
//	COUT( "END iter " << iter );
}

//-------------------------------------------------------------------
graph2_t
buildGraphFrom(
	index_t             idxCell,   ///< the index on the cell we are starting from, holds 2 candidates
	index_t             whichOne,  ///< 0 or 1
	std::vector<Cell2>& v_cells    ///< the set of cells holding 2 candidates. Not const, because each cell may get tagged as 'used' in graph
)
{
	graph2_t graph;
	auto v = boost::add_vertex( graph );  // add initial vertex
	graph[v].idx = idxCell;
	v_cells.at( idxCell )._chainRole = CR_Start;

	buildGraphRecursive( graph, v, v, whichOne, v_cells );


	return graph;
}
//-------------------------------------------------------------------
bool
Algo_XY_Chains( Grid& g )
{

// step 1 - build a set of cells having two candidates
	std::vector<Cell2> v_cells;
	for( index_t i=0; i<81; i++ )
	{
		auto cell = g.getCell(i);
		if( cell.NbCandidates() == 2 )
		{
			Cell2 c2( cell );
			v_cells.push_back( c2 );
		}
	}
// for each of these, build a non-oriented graph of cells
	for( index_t idx=0; idx<v_cells.size(); idx++ )
	{
//		auto vc = cell.GetCandidates();
		auto gr0 = buildGraphFrom( idx, 0, v_cells );
		auto gr1 = buildGraphFrom( idx, 1, v_cells );
	}

// step 2 - iterate through these to see if we can build a chain



	return false;
}
//-------------------------------------------------------------------
