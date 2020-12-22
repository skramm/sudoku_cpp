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

#include <boost/graph/adjacency_list.hpp>

//----------------------------------------------------------------------------
enum En_ChainRole
{
	CR_Start,
	CR_End,
	CR_Used,
	CR_Unused
};

//----------------------------------------------------------------------------
/// A cell hold as additional attribute its role in the chain (see \c En_ChainRole)
struct Cell2
{
	En_ChainRole               _chainRole = CR_Unused;
	pos_t                      _pos;
	std::pair<value_t,value_t> _candidValues;
	value_t get(index_t i) const
	{
		assert( i==0 || i==1 );
		return i ? _candidValues.first : _candidValues.second;
	}
	Cell2( const Cell& c )
		: _pos( c.GetPos() )
	{
		auto vcand = c.GetCandidates();
		assert( vcand.size() == 2 );
		_candidValues.first  = vcand[0];
		_candidValues.second = vcand[1];
	}
};

//----------------------------------------------------------------------------
/// Vertex datatype, with BGL. Holds a cell position
/// \todo Checkout if it can be merged with \c GraphNode
struct GraphNode2
{
//	pos_t pos;
	index_t idx;   ///< index in the set of cells having two candidates
};

/// Edge datatype
/// \todo Checkout if it can be merged with \c GraphEdge
struct GraphEdge2
{
	En_LinkType link_type;
	EN_ORIENTATION link_orient;
};

/// A graph datatype, with BGL
typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS,
	GraphNode2,
	GraphEdge2
	> graph2_t;

typedef typename boost::graph_traits<graph2_t>::vertex_descriptor vertex2_t;
typedef typename boost::graph_traits<graph2_t>::edge_descriptor   edge2_t;

//-------------------------------------------------------------------
/// Explore set of Cells having 2 candidates and add them to the graph, recursively
/**
Stop condition: when we can't add any more nodes
*/
void
buildGraphRecursive(
	graph2_t&          graph,   ///< graph
	vertex2_t          v,       ///< origin vertex
	index_t            whichOne,  ///< 0 or 1, the index on the value of the origin vertex that we are considering
	std::vector<Cell2>& v_cells  ///< the set of cells holding 2 candidates.
)
{
	auto curr_idx = graph[v].idx;          // index on the cell we are considering
	auto& curr_cell = v_cells.at(curr_idx);      // we fetch the cell


	value_t val1 = curr_cell.get(whichOne);
	value_t val2 = curr_cell.get(whichOne?0:1);

	for( auto cell: v_cells )
	{
		if( cell._chainRole == CR_Unused )
		{
			value_t vc1 = cell.get(0);
			value_t vc2 = cell.get(1);

			if( val2 == vc1 || val2 == vc2 )   // if the cell holds the OTHER value of the starting cell,
				if( val1 != vc1 && val1 != vc2 )  // and it does NOT hold as other value the considered value
			{
;
			}

		}
	}
//	value_t v1 =
}

//-------------------------------------------------------------------
graph2_t
buildGraphFrom(
	index_t             idxCell, ///< the index on the cell we are starting from, holds 2 candidates
	index_t             whichOne,       ///< 0 or 1
	std::vector<Cell2>& v_cells ///< the set of cells holding 2 candidates. Not const, because each cell may get tagged as 'used' in graph
)
{
	graph2_t graph;
	auto v = boost::add_vertex( graph );  // add initial vertex
	graph[v].idx = idxCell;
	v_cells.at( idxCell )._chainRole = CR_Start;

	buildGraphRecursive( graph, v, whichOne, v_cells );


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
