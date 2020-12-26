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
#include "header.h"

#include <boost/graph/adjacency_list.hpp>

bool Algo_XY_Chains( Grid& g );

//----------------------------------------------------------------------------
enum En_ChainRole
{
	CR_Start,
	CR_End,
	CR_Used,
	CR_Unused
};
using ValuePair = std::pair<value_t, value_t>;


//----------------------------------------------------------------------------
/// Vertex datatype, with BGL. Holds a cell position
struct GraphNode_B
{
	index_t cell_idx;   ///< index in the set of cells having two candidates
};

/// Edge datatype
struct GraphEdge_B
{
	value_t commonVal = 0;
};

/// A graph datatype, with BGL
typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS,
//	boost::directedS,
	GraphNode_B,
	GraphEdge_B
	> graph2_t;

using vertex2_t = typename boost::graph_traits<graph2_t>::vertex_descriptor;
//typedef typename boost::graph_traits<graph2_t>::edge_descriptor   edge2_t;

//----------------------------------------------------------------------------
/// A cell holding 2 candidates, hold as additional attribute its role in the chain (see \c En_ChainRole)
struct Cell2
{
	En_ChainRole  _chainRole = CR_Unused;
	int           _graphIdx = -1;
	Pos           _pos;
//	pos_t         _pos;
	ValuePair     _candidValues; ///< the 2 candidates of the cell
	vertex2_t     _vertex = 0;

	Pos pos() const { return Pos(_pos); }

	value_t get(index_t i) const
	{
		assert( i==0 || i==1 );
		return i ? _candidValues.first : _candidValues.second;
	}

/// Return the other value
	value_t getOtherVal( value_t v ) const
	{
		return _candidValues.first==v ? _candidValues.second : _candidValues.first;
	}

/// Constructor
	Cell2( const Cell& c )
		: _pos( c.GetPos() )
	{
		const auto& vcand = c.GetCandidates();
		assert( vcand.size() == 2 );
		assert( vcand[0] != vcand[1] );
		_candidValues.first  = std::min( vcand[0],vcand[1] );
		_candidValues.second = std::max( vcand[0],vcand[1] );
	}

/// sorting operator, based on position
/*	friend bool operator < ( const Cell2& c1, const Cell2& c2 )
	{
		auto p1 = c1._pos;
		auto p2 = c2._pos;
		if( p1.first < p2.first )
			return true;
		if( p1.first == p2.first )
			return p1.second < p2.second;
		return false;
	}
*/
#ifdef TESTMODE
/// This constructor is only useful for unit-testing
/// \todo replace when \c Pos is fine
/*	Cell2( std::string spos )
	{
		_pos.first  = spos[0] != 'J' ? spos[0] - 'A': 8;
		_pos.second = spos[1] - '1';
	}*/

/// This constructor is only useful for unit-testing
//	Cell2( std::string spos, value_t c1, value_t c2  ) : Cell2( spos )
	Cell2( std::string spos, value_t c1, value_t c2  ) : _pos( spos )
	{
		_candidValues = std::make_pair(c1,c2);
//		_pos.first  = spos[0] != 'J' ? spos[0] - 'A': 8;
//		_pos.second = spos[1] - '1';
	}
#endif // TESTMODE

	friend std::ostream& operator << ( std::ostream& s, const Cell2& c )
	{
		s << c._pos << ": used:" << c._graphIdx << ", values=(" << (int)c._candidValues.first << "-" << (int)c._candidValues.second << ')';
		return s;
	}
};


//-------------------------------------------------------------------
/// Holds the intersection of the two final cells. Related to XY-Chains.
struct XYC_area
{
	value_t        _commonValue;
	std::set<Pos>  _sPos;

	XYC_area( value_t val ) : _commonValue(val)
	{}
};


#ifdef TESTMODE
	std::vector<graph2_t> buildGraphsFrom( index_t start,  std::vector<Cell2>& );
	XYC_area getArea( const Cell2&, const Cell2& );

void addToPosSet( std::set<Pos>&, EN_ORIENTATION, Pos, Pos );

#endif // TESTMODE

using Pgrvalset = std::pair<graph2_t,std::set<value_t>>;

std::vector<Pgrvalset> buildGraphs( std::vector<Cell2>& );


//----------------------------------------------------------------------------
#endif // HG_XY_CHAINS_H

