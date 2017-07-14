
#include "grid.h"
#ifndef _ALGORITHMS_H_
#define _ALGORITHMS_H_

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


#endif
