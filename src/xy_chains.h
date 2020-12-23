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

/// Return the other value
	value_t getOther( value_t v ) const
	{
		return _candidValues.first==v ? _candidValues.second : _candidValues.first;
	}

/// Constructor
	Cell2( const Cell& c )
		: _pos( c.GetPos() )
	{
		auto vcand = c.GetCandidates();
		assert( vcand.size() == 2 );
		assert( vcand[0] != vcand[1] );
		_candidValues.first  = std::min( vcand[0],vcand[1] );
		_candidValues.second = std::max( vcand[0],vcand[1] );
	}

#ifdef TESTMODE
/// This constructor is only useful for unit-testing
	Cell2( std::string spos, value_t c1, value_t c2  )
	{
		_candidValues = std::make_pair(c1,c2);
		_pos.first  = spos[0] != 'J' ? spos[0] - 'A': 8;
		_pos.second = spos[1] - '1';
		COUT( "creating cell2 at " << _pos );
	}


#endif // TESTMODE
};


//-------------------------------------------------------------------
struct LinkXY
{
	pos_t _p1, _p2;
	value_t _commonVal;
	LinkXY( const Cell2& c1, const Cell2& c2, value_t val )
		: _p1(c1._pos), _p2(c2._pos), _commonVal(val)
	{}

	friend bool operator == ( const LinkXY& lA, const LinkXY& lB )
	{
		if( lA._commonVal != lB._commonVal )
			return false;

		if( lA._p1 == lB._p1 )
			if( lA._p2 == lB._p2 )
				return true;
		if( lA._p1 == lB._p2 )
			if( lA._p2 == lB._p1 )
				return true;
		return false;
	}

	friend std::ostream& operator << ( std::ostream& s, const LinkXY& l )
	{
		s << '{' << l._p1 << "-" << l._p2 << ";C=" << (int)l._commonVal << "}\n"; // '}';
		return s;
	}
};

//----------------------------------------------------------------------------
/// Vertex datatype, with BGL. Holds a cell position
struct GraphNode_B
{
	index_t idx;   ///< index in the set of cells having two candidates
	std::pair<value_t, value_t> colorValues;

	GraphNode_B()
	{}
	GraphNode_B( index_t i, value_t v1, value_t v2 ) : idx(i)
	{
		colorValues.first  = v1;
		colorValues.second = v2;
	}
};

/// Edge datatype
struct GraphEdge_B
{
	bool isFinalEdge = false;
};

/// A graph datatype, with BGL
typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS,
	GraphNode_B,
	GraphEdge_B
	> graph2_t;

#ifdef TESTMODE
	graph2_t buildGraphFrom( index_t start, index_t whichOne, std::vector<Cell2>& );
#endif // TESTMODE

bool areLinkable( const Cell2& c1, const Cell2& c2 );
std::vector<LinkXY> buildSetOfLinks( const std::vector<Cell2>& v_cells );

//----------------------------------------------------------------------------
#endif // HG_XY_CHAINS_H

