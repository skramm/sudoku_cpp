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
\file algorithms.cpp
*/

#include "algorithms.h"
#include "header.h"
#include <numeric> // for std::accumulate()

//----------------------------------------------------------------------------
/// Box reduction. See http://www.sudokuwiki.org/Intersection_Removal#LBR
bool
BoxReduction( EN_ORIENTATION orient, Grid& g )
{
	PRINT_ALGO_START;

	for( index_t idx=0; idx<9; idx++ )  // for each row/col
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_nc v1d = g.GetView( orient, idx );
		for( value_t val=1; val<10; val++ )          // for each candidate value
		{
			std::vector<pos_t> v_cand;
			for( index_t j=0; j<9; j++ )   //
			{
				const Cell& c = v1d.GetCell( j );
				if( c.HasCandidate( val ) )
					v_cand.push_back( c.GetPos() );
			}
			if( v_cand.size()>0 && v_cand.size()<4 ) // if 2 or 3 candidates
			{
//				std::cout << "CAND=" << (int)val << " nb=" << v_cand.size() << '\n';
				bool sameBlock( true );
				auto block_index = GetBlockIndex( v_cand[0] );
				for( auto cand_pos: v_cand )
				{
					if( GetBlockIndex( cand_pos ) != block_index )
						sameBlock = false;
				}
				if( sameBlock ) // if all the candidates are in the same block, we can remove them from the other lines/col of this block
				{
//					std::cout << "all in block " << (int)block_index << '\n';
					View_1Dim_nc block = g.GetView( OR_BLK, block_index );
					bool doneRemoval(false);
					for( index_t j=0; j<9; j++ )   //
					{
						Cell& c = block.GetCell( j );
						if(
							( orient == OR_ROW && c.GetPos().first != idx )
							||
							( orient == OR_COL && c.GetPos().second != idx )
						)
						{
							if( c.RemoveCandidate( val ) )
								doneRemoval = true;
						}
					}
					if( doneRemoval )
						return true;
				}
			}
		}
	}
	return false;
}
//----------------------------------------------------------------------------
/// Box reduction. See http://www.sudokuwiki.org/Intersection_Removal#LBR
bool
Algo_BoxReduction( Grid& g )
{
	if( !BoxReduction( OR_ROW, g ) )
		if( ! BoxReduction( OR_COL, g ) )
			return false;
	return true;

}
//----------------------------------------------------------------------------
void
AddToCandidateCount( std::map<value_t,index_t>& m_count, const Cell& cell )
{
	auto v_cand = cell.GetCandidates();
	for( auto c: v_cand )
		m_count[c]++;
}
//----------------------------------------------------------------------------
/// Counts the number of times each candidate is present in a given row/block/col
std::map<value_t,index_t>
CoundCandidates( Grid& g, EN_ORIENTATION orient, index_t idx )
{
	View_1Dim_nc v1d = g.GetView( orient, idx );

	std::map<value_t,index_t> cand_count; // counts the number of times each candidate is present on the view
	for( value_t i=0; i<9; i++ )
		cand_count[i+1]=0;

	for( index_t j=0; j<9; j++ ) // for each cell in the view, count the candidates
		AddToCandidateCount( cand_count, v1d.GetCell(j) );

	return cand_count;
}
//----------------------------------------------------------------------------
/// Pointing pairs/triples. See http://www.sudokuwiki.org/Intersection_Removal
bool
PointingPairsTriples( Grid& g, EN_ORIENTATION orient )
{
	PRINT_ALGO_START;

	assert( orient == OR_ROW || orient == OR_COL );

	bool ret_val(false);
	for( index_t idx=0; idx<9; idx++ )  // for each row/col
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_nc v1d = g.GetView( orient, idx );

		for( index_t b=0; b<3; b++ ) // for each of the 3 blocks dividing the view
		{
			DEBUG << " block=" << (int)b+1 << '\n';
			const Cell& first = v1d.GetCell(b*3); // get first cell of block
			auto bl_idx = GetBlockIndex( first.GetPos() );
			auto cc1 = CoundCandidates( g, OR_BLK, bl_idx ); // number of candidates in that block
			std::map<value_t,index_t> cc2;
			for( index_t c=0; c<3; c++ )  // count candidates in the 3 cells of that block limit
				AddToCandidateCount( cc2, v1d.GetCell( b*3+c ) );

			for( value_t v=1; v<10; v++ )          // for each candidate value,
				if( cc2[v] > 1 && cc1[v] == cc2[v] )  // if we have 2 or 3 identical candidates, AND no other in the others cells of the same block
				{
					COUT( " - value: " << (int)v << " : found " << (cc2[v]==2 ? '2' : '3') << " alone in block " << (int)b+1)
					for( index_t j=0; j<9; j++ )  // for each cell of the view
					{
						Cell& cell = v1d.GetCell(j);
						if( j/3 != b )
							if( cell.RemoveCandidate( v, Because( B_PointingPairsTriples, orient ) ) )
								ret_val = true;
					}
				}
		}
	}
	return ret_val;
}
//----------------------------------------------------------------------------
/// Pointing pairs/triples. See http://www.sudokuwiki.org/Intersection_Removal
/** \warning This Algorithm operates on rows or cols only, not blocks */
bool
Algo_PointingPairsTriples( Grid& g )
{
	if( !PointingPairsTriples( g, OR_ROW ) )
		if( !PointingPairsTriples( g, OR_COL ) )
			return false;
	return true;
}
//----------------------------------------------------------------------------
/// Remove candidates on rows/cols/blocks that have a value in another cell
bool
RemoveCandidates( Grid& g, EN_ORIENTATION orient )
{
	PRINT_ALGO_START;

	bool res = false;
	for( index_t idx=0; idx<9; idx++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_nc v1d = g.GetView( orient, idx );

		for( index_t j=0; j<9; j++ ) // for each cell in the view
		{
			Cell& cell = v1d.GetCell(j);
			auto currentValue = cell.GetValue();
			if( currentValue != 0 )              // if a value is assigned to that cell,
			{
//				std::cout << "pos=" << cell.GetPos() << " : currentValue=" << (int)currentValue << '\n';
				for( index_t k=0; k<9; k++ ) // for each other cell in the view
					if( v1d.GetCell(k).HasCandidate( currentValue ) )
						res = v1d.GetCell(k).RemoveCandidate( currentValue, Because( B_ValuePresent, idx, j, orient ) );
			}
		}
	}
	return res;
}
//----------------------------------------------------------------------------
/// Remove candidates on rows/cols/blocks that have a value in another cell
bool
Algo_RemoveCandidates( Grid& g )
{
	if( !RemoveCandidates( g, OR_ROW ) )
		if( !RemoveCandidates( g, OR_COL ) )
			if( !RemoveCandidates( g, OR_BLK ) )
				return false;
	return true;
}
//----------------------------------------------------------------------------
/// Check in a 1d view if there is only one value missing. If so, then we can assign the missing value to that cell
bool
SearchSingleMissing( Grid& g, EN_ORIENTATION orient )
{
	PRINT_ALGO_START;

	for( index_t idx=0; idx<9; idx++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_nc v1d = g.GetView( orient, idx );

		std::vector<index_t>   v_zero;

		std::map<index_t,bool> m_zero; // a map with all
		for( index_t j=0; j<9; j++ )   // values from 1 to 9 initialized to 'true'
			m_zero[j+1] = true;

		for( index_t j=0; j<9; j++ ) // for each cell in the view
		{
			Cell& cell = v1d.GetCell(j);

			if( cell.GetValue() == 0 )
				v_zero.push_back( j ); // store position of cell
			else
				m_zero[cell.GetValue()] = false;
		}
		if( v_zero.size() == 1 ) // only one cell unknown
		{
			auto it = std::find_if(
				m_zero.begin(),
				m_zero.end(),
				[&]( const std::pair<index_t,bool>& p ){ return p.second==true;  } // lambda
			);
			assert( it != m_zero.end() );
			Cell& cell = v1d.GetCell( v_zero[0] );
			cell.RemoveAllCandidates();
			cell.SetValue( it->first );
			return true;
		}
	}
	return false;
}
//----------------------------------------------------------------------------
/// Check in a view if there is only one value missing. If so, then we can assign a value to it
bool
Algo_SearchSingleMissing( Grid& g )
{
	if( !SearchSingleMissing( g, OR_ROW ) )
		if( !SearchSingleMissing( g, OR_COL ) )
			if( !SearchSingleMissing( g, OR_BLK ) )
				return false;
	return true;
}
//----------------------------------------------------------------------------
/// Search for cells in a row/col/block where a candidate appears in only one cell.
/// If so, then it has to be the value in that cell
bool
SearchSingleCand( Grid& g, EN_ORIENTATION orient )
{
	PRINT_ALGO_START;

	bool res = false;
	bool stop = false;
	for( index_t idx=0; idx<9 && stop==false; idx++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_nc v1d = g.GetView( orient, idx );

		auto cand_count = CoundCandidates( g, orient, idx );

		for( index_t val=1; val<10; val++ )  // analyse results
		{
			auto c = cand_count[val];
			if( c == 1 )
			{
				COUT( "found single: " << (int)val );
                for( index_t j=0; j<9; j++ ) // for each cell in the view
				{
					Cell& cell = v1d.GetCell(j);
					auto v_cand = cell.GetCandidates();
					auto it = std::find( v_cand.begin(), v_cand.end(), val );
					if( it != v_cand.end() ) // then, we found it
					{
						cell.RemoveAllCandidatesBut( val );
						res = true;
					}
				}
				stop = true;
			}
		}
	}
	return res;
}
//----------------------------------------------------------------------------
/// Search for cells in a row/col/block where a candidates appears in only one cell
bool
Algo_SearchSingleCand( Grid& g )
{
	if( !SearchSingleCand( g, OR_ROW ) )
		if( !SearchSingleCand( g, OR_COL ) )
			if( !SearchSingleCand( g, OR_BLK ) )
				return false;
	return true;
}
//----------------------------------------------------------------------------
/// Naked pairs algorithm, called by Algo_SearchNakedPairs()
bool
SearchNakedPairs( Grid& g, EN_ORIENTATION orient )
{
	PRINT_ALGO_START;

	bool res = false;
	for( index_t idx=0; idx<9; idx++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX(orient);
		std::vector<index_t> v_pos(1);
		View_1Dim_nc v1d = g.GetView( orient, idx );
		std::vector<value_t> v_cand_1;

		for( index_t j=0; j<8 && (v_pos.size()!=2); j++ ) // for each cell in the view (and stop if found 'n' matches)
		{
			Cell& cell_1 = v1d.GetCell(j);
			if( cell_1.NbCandidates() == 2 )     // if cell has 2 candidates
			{
				v_pos[0] = j;
				v_cand_1 = cell_1.GetCandidates();

				for( index_t k=j+1; k<9 && (v_pos.size()!=2); k++ ) // for each other cell in the view
				{
					Cell& cell_2 = v1d.GetCell(k);
					if( cell_2.NbCandidates() == 2 )    // if OTHER cell has also 2 candidates
					{
						if( v_cand_1 == cell_2.GetCandidates() ) // then, if the candidates are the same, then we can remove these from the others cells of the view
						{
//							if( g_data.Verbose )
//								std::cout << "  -found match of pos " << (int)j+1 << " at pos " << (int)k+1 << '\n';
							v_pos.push_back( k );
						}
					}
				}
			}
		}
		assert( v_pos.size() <= 2 );
		if( v_pos.size() == 2 )
		{
			uint8_t Nb(0);
			for( index_t j=0; j<9; j++ ) // for each cell in the view
			{
				Cell& cell = v1d.GetCell(j);
				bool dontremove( false );
				for( uint8_t p=0; p<2; p++ )
					if( j == v_pos[p] )
						dontremove = true;
				if( !dontremove )
					if( cell.RemoveCellCandidates( v_cand_1, Because( B_NakedPair, v_cand_1, orient ) ) )
					{
						res = true;
						Nb++;
					}
			}
//			if( g_data.Verbose && Nb )
//				std::cout << " - found a pair, removed " << (int)Nb << " candidates from others cells in same view\n";
		}
	}
	return res;
}
//----------------------------------------------------------------------------
/// "Naked pair" algorithm
bool
Algo_SearchNakedPairs( Grid& g )
{
	if( !SearchNakedPairs( g, OR_ROW ) )
		if( !SearchNakedPairs( g, OR_COL ) )
			if( !SearchNakedPairs( g, OR_BLK ) )
				return false;
	return true;
}
//----------------------------------------------------------------------------
/// Returns true if the values in \c v1 are in one of the tree values of \c v2.
/// Helper function for CheckForTriplePattern_D()
/**
Example:
\verbatim
v1={ 4, 6 }
v2={ 4, 6, 8 }
=> true

v1={ 4, 5 }
v2={ 4, 6, 8 }
=> false
\endverbatim
*/
bool
isSameValues( const std::vector<value_t>& v1, const	std::array<value_t,3>& v2 )
{
	bool found1 = false;
	bool found2 = false;
	assert( v1.size() == 2 );
	assert( v1[0] != v1[1] );
	if( v1[0] == v2[0] || v1[0] == v2[1] || v1[0] == v2[2] )
		found1 = true;
	if( v1[1] == v2[0] || v1[1] == v2[1] || v1[1] == v2[2] )
		found2 = true;
	return found1 & found2;
}
//----------------------------------------------------------------------------
/// Helper function for SearchTriplesPattern(), search for "pattern D"
/// \todo 20201221: this seems buggy, see "make test"
void
CheckForTriplePattern_D( const std::vector<Pos_vcand>& v_cand, NakedTriple& retval )
{
//	std::cout << " case D !\n";

	std::vector<index_t> v_pairs;
	for( index_t i=0; i<v_cand.size(); i++ )
	{
		if( v_cand[i]._values.size() == 2 )  // if 2 candidates
			v_pairs.push_back(i);
	}

//	PrintVector( v_pairs, "vpairs");

	if( v_pairs.size() < 3 )       // if less than 3 cells with 2 candidates found,
	{                              // then, no pattern!
		retval.found_NT = false;
		return;
	}

	for( index_t i=0; i<v_pairs.size()-1; i++ )
	{
		auto idx1 = v_pairs[i];
		bool done(false);
		auto v1a = v_cand[idx1]._values[0];
		auto v1b = v_cand[idx1]._values[1];
//		std::cout << "i=" << (int)i << " idx1=" << (int)idx1 << " pair=" << (int)v_cand[i]._values[0] << '-' << (int)v_cand[i]._values[1] << '\n';
//		std::cout << " v1=" << (int)v1a << '\n';
		for( index_t j=i+1; j<v_pairs.size() && !done; j++ )
		{
			auto idx2 = v_pairs[j];
//			std::cout << "  j=" << (int)j << " idx2=" << (int)idx2 << '\n';
			if( v1a == v_cand[idx2]._values[0] || v1a == v_cand[idx2]._values[1] )
			{
				auto v2 = v_cand[idx2]._values[1];     // if a match is found, we take the complementary one as 'v2'
				if( v1a == v_cand[idx2]._values[1] )
					v2 = v_cand[idx2]._values[0];

//				std::cout << "   -chain with j=" << (int)j << ", pair=" << (int)v_cand[idx2]._values[0] << '-' << (int)v_cand[idx2]._values[1] << '\n';
//				std::cout << "   -v2=" << (int)v2 << '\n';
				for( index_t k=0; k<v_pairs.size() && !done; k++ )
				{
					auto idx3 = v_pairs[k];

					if( idx3 != idx1 && idx3 != idx2 )
					{
//						std::cout << "      k=" << (int)k << '\n';
						if( v2 == v_cand[idx3]._values[0] || v2 == v_cand[idx3]._values[1] )
						{
							auto v3 = v_cand[idx3]._values[1];     // if a match is found, we take the complementary one as 'v2'
							if( v2 == v_cand[idx3]._values[1] )
								v3 = v_cand[idx3]._values[0];
//							std::cout << "     -chain with k=" << (int)k << " idx3=" << (int)idx3<< ", pair=" << (int)v_cand[idx3]._values[0] << '-' << (int)v_cand[idx3]._values[1] << '\n';
//							std::cout << "     -v3=" << (int)v3 << '\n';
							if( v3 == v1b ) // loop found !
							{
//								std::cout << "     FOUND D PATTERN\n";
								retval.foundPattern();
								retval.cand_values[0] = v1a;
								retval.cand_values[1] = v2;
								retval.cand_values[2] = v3;

								retval.cand_pos[0] = v_cand[idx1].pos_index;
								retval.cand_pos[1] = v_cand[idx2].pos_index;
								retval.cand_pos[2] = v_cand[idx3].pos_index;

								return;
							}
							else
								done=true;
						}
					}
				}
			}
		}
	}
}
//----------------------------------------------------------------------------
/// Helper function for SearchTriplesPattern(), search for "type C" pattern
void
CheckForTriplePattern_C(
	const std::vector<Pos_vcand>& v_cand,
	const std::vector<index_t>&   v_pairs,
	const std::vector<index_t>&   v_triples,
	NakedTriple&                  ret_val
)
{
	assert( v_pairs.size() == 2 );
	assert( v_triples.size() > 0 );
	assert( ret_val.cand_values.size() == 3 );

//	std::cout << " CheckForTriplePattern_C()\n";

	std::vector<index_t> value_count(3,0);
	for( int k=0; k<3; k++ )                              // for each value of the triplet,
	{
//		std::cout << k << ": searching for value " << (int)ret_val.cand_values[k] << "\n";
		if(
			ret_val.cand_values[k] == v_cand[v_pairs[0]]._values[0]
			||
			ret_val.cand_values[k] == v_cand[v_pairs[0]]._values[1]
		)
			value_count[k]++;
		if(
			ret_val.cand_values[k] == v_cand[v_pairs[1]]._values[0]
			||
			ret_val.cand_values[k] == v_cand[v_pairs[1]]._values[1]
		)
			value_count[k]++;
	}
//	PrintVector( value_count, "value_count");
	if(                                                                       // this is to test is
		std::accumulate( value_count.begin(), value_count.end(), 0) == 4      // we have 1,1,2 or 1,2,1 or 2,1,1
		&&
		*std::min_element( value_count.begin(), value_count.end() ) == 1
	)
	{
		ret_val.foundPattern();
		ret_val.cand_pos[0] = v_triples[0];
		ret_val.cand_pos[1] = v_cand[v_pairs[0]].pos_index;
		ret_val.cand_pos[2] = v_cand[v_pairs[1]].pos_index;
		std::sort( std::begin(ret_val.cand_pos), std::end(ret_val.cand_pos) );
	}
}
//----------------------------------------------------------------------------
/// Naked triples, see http://www.sudokuwiki.org/Naked_Candidates#NP
/**
\verbatim
The combinations of candidates for a Naked Triple will be one of the following:

- case A: (123) (123) (123) - {3/3/3} (in terms of candidates per cell)
- case B: (123) (123) (12) - {3/3/2} (or some combination thereof)
- case C: (123) (12) (23) - {3/2/2/}
- case D: (12) (23) (13) - {2/2/2}
\endverbatim

- Input: a set of candidates, described with \ref Pos_vcand
- output: a NakedTriple object, holding a boolean, the triple pattern values and positions
*/
NakedTriple
SearchTriplesPattern( const std::vector<Pos_vcand>& v_cand )
{
	NakedTriple return_value;
	if( v_cand.size() < 3 )
			return return_value;

	COUT( "v_cand size=" << v_cand.size() );
	for( index_t i=0; i<v_cand.size(); i++ )
	{
		COUT( "i=" << (int)i << ": " << v_cand[i] );
//		std::cout << "* main pos=" << i << " size=" << v_cand[i].values.size() << '\n';
		if( v_cand[i]._values.size() == 3 )  // if 3 candidates (useful for patterns A,B,C)
		{
			std::vector<index_t> v_pairsPos;
			std::vector<index_t> v_triplesPos(1);
			v_triplesPos[0] = v_cand[i].pos_index;

			for( index_t j=0; j<v_cand.size(); j++ )  // iterate over other candidates
				if( i != j )
				{
					if( v_cand[i]._values == v_cand[j]._values )                    // if found two triples (case A and B)
						v_triplesPos.push_back( v_cand[j].pos_index );

					if( v_cand[j]._values.size() == 2 )                                // if 2 candidates
						if( VectorsOverlap( v_cand[i]._values, v_cand[j]._values, 2 ) ) // AND if the 2 values are in the triplet
							v_pairsPos.push_back( j );
				}
//			PrintVector( v_triples, "v_triples" );
//			PrintVector( v_pairs,   "v_pairs" );

			for( int k=0; k<3; k++)                                        // we pre-fill the return value with the candidates that are in the
				return_value.cand_values[k] = v_cand[i]._values.at( k );    // triplet we found. If it's not a "naked triple" pattern, then it just gets lost

// step 2: we determine what type the triplets are
			switch( v_triplesPos.size() )
			{
				case 3:                    // case A, for sure
				{
					COUT( "-Naked triple pattern type A:" << return_value );
					return_value.foundPattern();
					for( int k=0; k<3; k++ )
						return_value.cand_pos[k] = v_triplesPos[k];
					return return_value;
				}
				break;

				case 2:                 // maybe case B, lets check!
					if( !v_pairsPos.empty() )
					{
//						std::cout << " case B !\n";
						assert( v_pairsPos.size() == 1 );  // should be, I think. If not, triggering this assert will be a good way to find out ;-)
						return_value.foundPattern();
						for( int k=0; k<2; k++ )
							return_value.cand_pos[k] = v_triplesPos[k];
						return_value.cand_pos[2] = v_cand[v_pairsPos[0]].pos_index;
						std::sort( std::begin(return_value.cand_pos), std::end(return_value.cand_pos) );
						COUT( "-Naked triple pattern type B:" << return_value );
						return return_value;
					}
				break;

				case 1:                 // maybe case C, lets check!
					if( v_pairsPos.size() == 2 )
					{
//						std::cout << " case C !\n";
						CheckForTriplePattern_C( v_cand, v_pairsPos, v_triplesPos, return_value );
						if( return_value.found_NT )
						{
							COUT( "-Naked triple pattern type C:" << return_value );
							return return_value;
						}
					}
				break;

				default: assert(0);
			}
		}
	}
	CheckForTriplePattern_D( v_cand, return_value );
	if( return_value.found_NT )
		COUT( "-Naked triple pattern type D:" << return_value );

	return return_value;
}

//----------------------------------------------------------------------------
/// Search for Naked Triples, and remove candidates accordingly
/**
- Returns true if a candidate has been removed
- Related: \c Pos_vcand
*/
bool
SearchNakedTriples( Grid& g, EN_ORIENTATION orient )
{
	PRINT_ALGO_START;

	bool retval = false;
	for( index_t idx=0; idx<9; idx++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_nc v1d = g.GetView( orient, idx );
		std::vector<Pos_vcand> v_cand;

		for( index_t j=0; j<9; j++ ) // for each cell in the view (and stop if found 'n' matches)
		{
			Cell& cell = v1d.GetCell(j);
			if( cell.NbCandidates() == 2 || cell.NbCandidates() == 3 )     // if cell has 2 or 3 candidates,
				v_cand.emplace_back( j, cell.GetCandidates() );            // then, store its index and the set of candidates.
		}

		auto trp = SearchTriplesPattern(v_cand); // search for triple pattern
		if( trp.found_NT )                                // if a naked triple was found, then:
			for( index_t i=0; i<9; i++ )                  // for all the other positions of the view, remove the candidates found
			{
				if( std::find( std::begin(trp.cand_pos), std::end(trp.cand_pos), i ) == std::end(trp.cand_pos) ) // remove candidate in OTHER cells
					for( int k=0; k<3; k++ ) // 3, because "Naked Triples" always implies 3 values
						if( v1d.GetCell(i).RemoveCandidate( trp.cand_values[k], Because( B_NakedTriples, orient, trp ) ) )
							retval = true;
			}
	}
	return retval;
}
//----------------------------------------------------------------------------
/// "Naked triple" algorithm
bool
Algo_SearchNakedTriples( Grid& g )
{
	if( !SearchNakedTriples( g, OR_ROW ) )
		if( !SearchNakedTriples( g, OR_COL ) )
			if( !SearchNakedTriples( g, OR_BLK ) )
				return false;
	return true;
}
//----------------------------------------------------------------------------
/// Filter vector \c v_pos so that it holds only positions of cells holding only one of the candidates that are in \c v_cand.
/**
Type \c T is a container holding elements of type \c pos_t (example: \c std::vector<pos_t> )
*/
template<typename T>
void
FilterByCand( Grid& g, const std::vector<value_t>& v_cand, T& v_pos )
{
	T v_out;
	assert( v_cand.size() == 2 );
	for( auto& pos : v_pos )
	{
		const Cell& c = g.GetCellByPos( pos );
		if(
			( c.HasCandidate( v_cand[0] ) && !c.HasCandidate( v_cand[1] ) )
			||
			( c.HasCandidate( v_cand[1] ) && !c.HasCandidate( v_cand[0] ) )
		)
			v_out.push_back( pos );
	}
	std::swap( v_out, v_pos );
}
//----------------------------------------------------------------------------
/// Helper function for FindSymmetricalMatches()
/** Finds indexes of correspondences between 2 vectors holding 2 elements.
Returned value holds \c first: index on first vector, \c second: index on second vector
*/
std::pair<index_t,index_t>
FindMatch( const std::vector<value_t>& v_key, const std::vector<value_t>& v_cand )
{
	assert( v_key.size()  == 2 );
	assert( v_cand.size() == 2 );

	std::pair<index_t,index_t> p_out(0,0);
	if( v_key[0] == v_cand[0] )
		return std::pair<index_t,index_t>(0,0);
	if( v_key[0] == v_cand[1] )
		return std::pair<index_t,index_t>(0,1);
	if( v_key[1] == v_cand[0] )
		return std::pair<index_t,index_t>(1,0);
	if( v_key[1] == v_cand[1] )
		return std::pair<index_t,index_t>(1,1);

	assert(0);
	return std::pair<index_t,index_t>(0,0); // just there to avoid a compilation warning
}
//----------------------------------------------------------------------------
/// Holds results of FindSymmetricalMatches()
struct SymMatches
{
	pos_t pA;
	pos_t pB;
	value_t value;
	SymMatches( pos_t _pA, pos_t _pB, value_t _v ) : pA(_pA),pB(_pB),value(_v) {}
	friend std::ostream& operator << ( std::ostream& s, const SymMatches& sm )
	{
		s << "(cells:" << sm.pA << ',' << sm.pB << " val=" << (int)sm.value << ") ";
		return s;
	}
};
//----------------------------------------------------------------------------
/// Searches in the two vectors for a symmetric match (related to XY_Wing)
/**
see http://www.sudokuwiki.org/Y_Wing_Strategy

Say the two candidates are 1,4
and the set of cells holds these:
\beginverbatim
0: 4,6
2: 1,5
3: 1,2
4: 2,4
\endverbatim
then we see that we have a match with elems 3 and 4, and the common candidates (that will get removed in further steps) is 2
*/
std::vector<SymMatches>
FindSymmetricalMatches(
	const Grid& grid,
	const std::vector<value_t>& v_key_cand,  ///< input: vector of 2 values (candidates of key cell)
	const std::vector<pos_t>&   v_cells      ///< input: vector of cell positions that are in the range and hold 2 candidates
)
{
	assert( v_key_cand.size() == 2 );

	std::vector<SymMatches> v_out;
//	PrintVector( v_cells, "v_cells" ); std::cout << "Key cand: " << v_key_cand[0] << '-' << v_key_cand[1] << '\n';

	for( size_t i=0; i<v_cells.size()-1; i++ )
	{
		const auto& pA = v_cells[i];
		const Cell& cA = grid.GetCellByPos( pA );
		const auto& v_cand_A = cA.GetCandidates();
		auto match_idx_A = FindMatch( v_key_cand, v_cand_A );
		for( size_t j=i+1; j<v_cells.size(); j++ )
		{
			const auto& pB = v_cells[j];
//			std::cout <<"i="<<i << " j=" << j << " pos: pA=" << pA << " pB=" << pB << '\n';
			assert( pA != pB );
			const Cell& cB = grid.GetCellByPos( pB );
			const auto& v_cand_B = cB.GetCandidates();
			if( v_cand_A != v_cand_B )
			{
//				PrintVector( v_cand_A, "v_cand_A" ); PrintVector( v_cand_B, "v_cand_B" );

				auto match_idx_B = FindMatch( v_key_cand, v_cand_B );

//				std::cout << "match A: " << match_idx_A.first <<  '-' << match_idx_A.second << '\n';
//				std::cout << "match B: " << match_idx_B.first <<  '-' << match_idx_A.second << '\n';

				if( v_cand_A[!match_idx_A.second] == v_cand_B[!match_idx_B.second] )
				{
					std::pair<pos_t,pos_t> p_out;
					p_out.first  = pA;
					p_out.second = pB;
					value_t common_value = v_cand_A[!match_idx_A.second];
//					std::cout << "FOUND: pA=" << pA << " pB=" << pB <<  " common value=" << common_value << '\n';
					v_out.push_back( SymMatches( pA,pB,common_value) );
				}
			}
		}
	}
//	std::cout << "SymMatches size=" << v_out.size() << '\n';
	return v_out;
}
//----------------------------------------------------------------------------
/// Remove from all the cells in \c v_region the candidate \c cand, EXCEPT in (source) cell \c key
bool
RemoveCandidatesFromRegion( Grid& grid, const std::vector<pos_t>& v_region, value_t cand, const Cell& key )
{
	bool retval(false);
	for( auto& pos: v_region )
		if( pos != key.GetPos() )
			if( grid.GetCellByPos(pos).RemoveCandidate( cand ) )
				retval = true;
	return retval;
}
//----------------------------------------------------------------------------
/// Find common region covered by \c p1 and \c p2
/**
\todo probably a lot of optimizations over here...
*/
std::vector<pos_t>
FindCommonRegion( pos_t p1, pos_t p2 )
{
	assert( p1 != p2 );

//	assert( p1.first != p2.first || p1.second != p2.second ); // must be on different rows OR different cols

	std::vector<pos_t> v_out;
	if( p1.first != p2.first && p1.second != p2.second ) // if they are NOT on same row or same col
	{
//		std::cout << "adding intersections\n";
		v_out.push_back( pos_t(p1.first,p2.second) ); // add the 2 intersection cells
		v_out.push_back( pos_t(p2.first,p1.second) );
	}

	if( p1.first/3 == p2.first/3 )          // if blocks are on the same row (0,1, or 2)
    {
//		std::cout <<" - same block row !\n";
		index_t col1 = p1.second/3*3;
		index_t col2 = p2.second/3*3;
		for( index_t i=0; i<3; i++ )
		{
			pos_t p_r1(p1.first, col2+i);
			if( p_r1 != p1 && p_r1 != p2 )
				AddToVector( v_out, p_r1 );

			pos_t p_r2(p2.first, col1+i);
			if( p_r2 != p1 && p_r2 != p2 )
				AddToVector( v_out, p_r2 );
		}
    }
    if( p1.second/3 == p2.second/3 )          // if blocks are on the same col (0,1, or 2)
    {
//		std::cout <<" - same block col !\n";
		index_t row1 = p1.first/3*3;
		index_t row2 = p2.first/3*3;
		for( index_t i=0; i<3; i++ )
		{
			pos_t p_c1(row2+i, p1.second);
			if( p_c1 != p1 && p_c1 != p2 )
				AddToVector( v_out, p_c1 );

			pos_t p_c2(row1+i, p2.second);
			if( p_c2 != p1 && p_c2 != p2 )
				AddToVector( v_out, p_c2 );
		}
    }
    if( g_data.Verbose )
		PrintVector( v_out, "Common region" );
	return v_out;
}
//----------------------------------------------------------------------------
/// XY Wing algorithm
bool
Algo_XY_Wing( Grid& g )
{
	PRINT_ALGO_START_2;

	bool retval(false);
	for( index_t idx=0; idx<9; idx++ )  // for each row
	{
		PRINT_MAIN_IDX(OR_ROW);
		View_1Dim_nc v1d = g.GetView( OR_ROW, idx );
		for( index_t col=0; col<9; col++ )   // for each col
		{
			Cell& key = v1d.GetCell(col);
			auto v_cand = key.GetCandidates();
			if( v_cand.size() == 2 )
			{
				DEBUG << " col=" << (int)col+1 <<  " cell has " << v_cand.size() << " candidates\n";
				auto v_cells   = g.GetOtherCells_nbc( key, 2, OR_ROW );  // get cells on same row that have 2 candidates
				auto v_cells_b = g.GetOtherCells_nbc( key, 2, OR_COL );  // get cells on same col that have 2 candidates
				auto v_cells_c = g.GetOtherCells_nbc( key, 2, OR_BLK );  // get cells in same block that have 2 candidates
				AddToVector( v_cells, v_cells_b );
				AddToVector( v_cells, v_cells_c );

				if( v_cells.size() > 1 )
				{
					FilterByCand( g, v_cand, v_cells );                 // remove the ones that don't use any of the 2 candidates
					if( v_cells.size() > 1 )
					{
						std::vector<SymMatches> v_matches = FindSymmetricalMatches( g, v_cand, v_cells );
						if( g_data.Verbose && v_matches.size() > 0 )
						{
							std::cout << "key cell: " << key.GetPos() << '\n';
							PrintVector( v_matches, "Symmetric matches" );
						}
						for( const auto& p_match: v_matches )
						{
							auto v_region = FindCommonRegion( p_match.pA, p_match.pB );
							if( RemoveCandidatesFromRegion( g, v_region, p_match.value, key ) )
								retval = true;
							else
								COUT( "No removals" );
						}
					}
				}
			}
		}
	}
	return retval;
}
//----------------------------------------------------------------------------
