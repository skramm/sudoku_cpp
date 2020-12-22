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
\brief header file for XY-chains algorithm
*/
#ifndef HG_XY_CHAINS_H
#define HG_XY_CHAINS_H

#include "grid.h"
//struct Grid;

#ifdef TESTMODE
	#include <boost/graph/adjacency_list.hpp>
#endif

bool Algo_XY_Chains( Grid& g );

//----------------------------------------------------------------------------
enum En_ChainRole
{
	CR_Start,
	CR_End,
	CR_Used,
	CR_Unused
};

//----------------------------------------------------------------------------
/// A cell holding 2 candidates, hold as additional attribute its role in the chain (see \c En_ChainRole)
struct Cell2
{
	En_ChainRole               _chainRole = CR_Unused;
	pos_t                      _pos;
	std::pair<value_t,value_t> _candidValues; ///< the 2 candidates of the cell

	value_t get(index_t i) const
	{
		assert( i==0 || i==1 );
		return i ? _candidValues.first : _candidValues.second;
	}

/// Constructor
	Cell2( const Cell& c )
		: _pos( c.GetPos() )
	{
		auto vcand = c.GetCandidates();
		assert( vcand.size() == 2 );
		_candidValues.first  = vcand[0];
		_candidValues.second = vcand[1];
	}

#ifdef TESTMODE
	Cell2( std::string spos, value_t c1, value_t c2  )
	{
		_candidValues = std::make_pair(c1,c2);
		_pos.first  = spos[0] != 'J' ? spos[0] - 'A': 8;
		_pos.second = spos[1] - '0';
	}


#endif // TESTMODE
};

#ifdef TESTMODE

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


graph2_t buildGraphFrom( index_t, index_t, std::vector<Cell2>& );

#endif // TESTMODE

//----------------------------------------------------------------------------
#endif // HG_XY_CHAINS_H

