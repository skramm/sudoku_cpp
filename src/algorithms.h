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
\file algorithms.h
*/

#ifndef _ALGORITHMS_H_
#define _ALGORITHMS_H_

#include "grid.h"
#include "x_cycles.h"
#include "xy_chains.h"

bool Algo_PointingPairsTriples(  Grid& g );
bool Algo_BoxReduction(          Grid& g );
bool Algo_RemoveCandidates(      Grid& g );
bool Algo_SearchSingleMissing(   Grid& g );
bool Algo_SearchSingleCand(      Grid& g );
bool Algo_SearchNakedPairs(      Grid& g );
bool Algo_SearchNakedTriples(    Grid& g );
bool Algo_XY_Wing(               Grid& g );


inline
const char*
GetString( EN_ALGO algo )
{
	switch( algo )
	{
		case ALG_REMOVE_CAND: return "RemoveCand"; break;
		case ALG_SEARCH_PAIRS: return "SearchNakedPairs"; break;
		case ALG_SEARCH_TRIPLES: return "SearchNakedTriples"; break;
		case ALG_SEARCH_SINGLE_CAND: return "SearchSingleCand"; break;
		case ALG_SEARCH_MISSING_SINGLE: return "MissingSingle"; break;
		case ALG_POINTING_PT: return "PointingPairs/Triples"; break;
		case ALG_BOX_RED: return "BoxReduction"; break;
		case ALG_XY_WING: return "XY_Wing"; break;
		case ALG_XY_CHAINS: return "XY_Chains"; break;
#ifndef BUILD_WITHOUT_UDGCD
		case ALG_X_CYCLES: return "X_cycles"; break;
#endif
		default: assert(0);
	}
}

#define PRINT_ALGO_START \
	{ \
		if( g_data.LogSteps > 2 ) \
			std::cout << "START ALGO: " << __FUNCTION__ << ", orient=" << GetString( orient ) << '\n'; \
	}

#define PRINT_ALGO_START_2 \
	{ \
		if( g_data.LogSteps > 2 ) \
			std::cout << "START ALGO: " << __FUNCTION__ << '\n'; \
	}

//----------------------------------------------------------------------------
/// Used in the "naked triples" algorithm. See SearchTriplesPattern() and SearchNakedTriples()
struct Pos_vcand
{
	index_t pos_index;            ///< Cell index in the row/col/block
	std::vector<value_t> _values;  ///< Candidates for that cell
	Pos_vcand( size_t p, const std::vector<value_t>& vect )
		: pos_index(p), _values(vect)
	{}
	friend std::ostream& operator << ( std::ostream& s, const Pos_vcand& pvc )
	{
		s << "Pos_vcand: idx=" << (int)pvc.pos_index << ", values=";
		for( auto v: pvc._values)
			s << (int)v << '-';
		s << ' ';
		return s;
	}
};


NakedTriple SearchTriplesPattern( const std::vector<Pos_vcand>& v_cand );

//----------------------------------------------------------------------------
template<typename T>
bool
VectorsOverlap( const std::vector<T>& v1, const std::vector<T>& v2, size_t n )
{
	const std::vector<T>* pv1 = &v1;
	const std::vector<T>* pv2 = &v2;
	if( v1.size() < v2.size() )
	{
		pv1 = &v2;
		pv2 = &v1;
	}
	assert( n <= pv1->size() );

	size_t c=0;
	for( const auto& e1: *pv1 )           // for each element of v1,
		if( std::find(                    // if we find one
				std::begin( *pv2 ),       // of the elements of v2
				std::end( *pv2 ),
				e1
			) != std::end( *pv2 )
		)
			c++;                        // then we increment the counter

	if( c >= n )                        // if more than 2 elements,
		return true;                    // then, the vectors do overlap
	return false;
}
//----------------------------------------------------------------------------


#endif
