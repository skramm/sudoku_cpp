/**
\file algorithms.h
*/

#ifndef _ALGORITHMS_H_
#define _ALGORITHMS_H_

#include "grid.h"
//#include <tuple>

bool X_Cycles( Grid& g );
bool Algo_PointingPairsTriples(  Grid& g );
bool Algo_BoxReduction(          Grid& g );
bool Algo_RemoveCandidates(      Grid& g );
bool Algo_SeachForSingleMissing( Grid& g );
bool Algo_SearchSingleCand(      Grid& g );
bool Algo_SearchPairs(           Grid& g );
bool Algo_SearchTriples(         Grid& g );
bool Algo_XY_Wing(               Grid& g );


inline
const char*
GetString( EN_ALGO algo )
{
	switch( algo )
	{
		case ALG_REMOVE_CAND: return "RemoveCand"; break;
		case ALG_SEARCH_PAIRS: return "SearchPairs"; break;
		case ALG_SEARCH_TRIPLES: return "SearchTriples"; break;
		case ALG_SEARCH_SINGLE_CAND: return "SearchSingleCand"; break;
		case ALG_SEARCH_MISSING_SINGLE: return "MissingSingle"; break;
		case ALG_POINTING_PT: return "PointingPairs/Triples"; break;
		case ALG_BOX_RED: return "BoxReduction"; break;
		case ALG_XY_WING: return "XY_Wing"; break;
		case ALG_X_CYCLES: return "X_cycles"; break;
		default: assert(0);
	}
}

#define PRINT_ALGO_START \
	{ \
		if( g_data.Verbose ) \
			std::cout << "START ALGO: " << __FUNCTION__ << ", orient=" << GetString( orient ) << '\n'; \
	}

#define PRINT_ALGO_START_2 \
	{ \
		if( g_data.Verbose ) \
			std::cout << "START ALGO: " << __FUNCTION__ << '\n'; \
	}

//----------------------------------------------------------------------------
/// Used in the "naked triples" algorithm. See SearchTriplesPattern() and SearchNakedTriples()
struct pos_vcand
{
	index_t pos_index;
	std::vector<value_t> values;
	pos_vcand( size_t p, const std::vector<value_t>& vect ) : pos_index(p), values(vect) {}
};

//----------------------------------------------------------------------------
/// return type of SearchTriplesPattern(). Holds
struct NakedTriple
{
	bool found = false;
	std::array<value_t,3> cand_values;
	std::array<index_t,3> cand_pos;
//	NakedTriple(bool b) : found(b) {}
	void foundPattern() { found = true; }
};

NakedTriple SearchTriplesPattern( const std::vector<pos_vcand>& v_cand );

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
