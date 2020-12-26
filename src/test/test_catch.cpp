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


#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "../xy_chains.h"

#include "../algorithms.h"

TEST_CASE( "test of position/index conversions", "tposi" )
{
	pos_t p(3,3);
	CHECK( GetBlockIndex( pos_t(0,0) ) == 0 );
	CHECK( GetBlockIndex( pos_t(0,1) ) == 0 );
	CHECK( GetBlockIndex( pos_t(0,2) ) == 0 );
	CHECK( GetBlockIndex( pos_t(1,2) ) == 0 );

	CHECK( GetBlockIndex( pos_t(3,0) ) == 3 );
	CHECK( GetBlockIndex( pos_t(4,1) ) == 3 );

	CHECK( GetBlockIndex( pos_t(4,4) ) == 4 );

	CHECK( GetBlockIndex( pos_t(8,8) ) == 8 );


	CHECK( getPosFromBlockIndex( 0, 0 ) == pos_t(0,0) );
	CHECK( getPosFromBlockIndex( 0, 1 ) == pos_t(0,1) );
	CHECK( getPosFromBlockIndex( 0, 2 ) == pos_t(0,2) );
	CHECK( getPosFromBlockIndex( 0, 3 ) == pos_t(1,0) );
	CHECK( getPosFromBlockIndex( 0, 4 ) == pos_t(1,1) );
	CHECK( getPosFromBlockIndex( 0, 5 ) == pos_t(1,2) );
	CHECK( getPosFromBlockIndex( 0, 6 ) == pos_t(2,0) );
	CHECK( getPosFromBlockIndex( 0, 7 ) == pos_t(2,1) );
	CHECK( getPosFromBlockIndex( 0, 8 ) == pos_t(2,2) );

	CHECK( getPosFromBlockIndex( 1, 0 ) == pos_t(0,3) );
	CHECK( getPosFromBlockIndex( 1, 1 ) == pos_t(0,4) );

	CHECK( getPosFromBlockIndex( 3, 0 ) == pos_t(3,0) );
	CHECK( getPosFromBlockIndex( 3, 1 ) == pos_t(3,1) );

}

TEST_CASE( "test of reading grid from string", "[readstring]" )
{
	Grid g;
	std::string s1( "777111222...777...111888999777111222...777...111888999777111222...777...111888999" );
	CHECK( g.buildFromString( s1 ) );
	std::string s2( "00111222...777...111888999777111222...777...111888999777111222...777...111888999" );
	CHECK( !g.buildFromString( s2 ) );
	std::string s3( " 0111222...777...111888999777111222...777...111888999777111222...777...111888999" );
	CHECK( !g.buildFromString( s3 ) );
}

TEST_CASE( "test of overlap", "[overlap1]" )
{
	std::vector<int> v1{1,2,3};
	{
		std::vector<int> v2{1,2};

		REQUIRE( VectorsOverlap(v1,v2,2) == true );
		REQUIRE( VectorsOverlap(v2,v1,2) == true );
		REQUIRE( VectorsOverlap(v1,v2,1) == true );
		REQUIRE( VectorsOverlap(v2,v1,1) == true );
		REQUIRE( VectorsOverlap(v1,v2,3) == false );
		REQUIRE( VectorsOverlap(v2,v1,3) == false );
	}
	{
		std::vector<int> v2{1,4};

		REQUIRE( VectorsOverlap(v1,v2,2) == false );
		REQUIRE( VectorsOverlap(v2,v1,2) == false );
		REQUIRE( VectorsOverlap(v1,v2,1) == true );
		REQUIRE( VectorsOverlap(v2,v1,1) == true );
		REQUIRE( VectorsOverlap(v1,v2,3) == false );
		REQUIRE( VectorsOverlap(v2,v1,3) == false );
	}
}

TEST_CASE( "test of naked triple search", "[triple]" )
{
	std::array<value_t,3> trip_val{1,2,3} ;
	std::array<index_t,3> trip_pos{4,5,6} ;
	{
		INFO( "test case A" )
		std::vector<Pos_vcand> v;
		v.emplace_back( 3, std::vector<value_t>{4,5} );
		v.emplace_back( 4, std::vector<value_t>{1,2,3} );
		v.emplace_back( 5, std::vector<value_t>{1,2,3} );
		v.emplace_back( 6, std::vector<value_t>{1,2,3} );

		auto res = SearchTriplesPattern( v );
		CHECK( res.found_NT == true );
		CHECK( res.cand_values == trip_val );
		CHECK( res.cand_pos == trip_pos );
	}
	{
		INFO( "test case B" )
		std::vector<Pos_vcand> v;
		v.emplace_back( 3, std::vector<value_t>{4,5} );
		v.emplace_back( 4, std::vector<value_t>{1,2} );
		v.emplace_back( 5, std::vector<value_t>{1,2,3} );
		v.emplace_back( 6, std::vector<value_t>{1,2,3} );

		auto res = SearchTriplesPattern( v );
		CHECK( res.found_NT == true );
		CHECK( res.cand_values == trip_val );
		CHECK( res.cand_pos == trip_pos );
	}
	{
		INFO( "test case C - no good" )
		std::vector<Pos_vcand> v;
		v.emplace_back( 3, std::vector<value_t>{4,5} );
		v.emplace_back( 4, std::vector<value_t>{1,2} );
		v.emplace_back( 5, std::vector<value_t>{1,2} );
		v.emplace_back( 6, std::vector<value_t>{1,2,3} );

		auto res = SearchTriplesPattern( v );
		CHECK( res.found_NT == false );
	}
	{
		INFO( "test case C - good" )
		std::vector<value_t> vC{2,3};

		std::vector<Pos_vcand> v;
		v.emplace_back( 3, std::vector<value_t>{4,5} );
		v.emplace_back( 4, std::vector<value_t>{1,2} );
		v.emplace_back( 5, std::vector<value_t>{2,3} );
		v.emplace_back( 6, std::vector<value_t>{1,2,3} );

		auto res = SearchTriplesPattern( v );
		CHECK( res.found_NT == true );
		CHECK( res.cand_values == trip_val );
		CHECK( res.cand_pos == trip_pos );
	}
	{
		INFO( "test case D1 - no good" )
		std::cout << "case D1 - no good\n";
		std::vector<Pos_vcand> v;
		v.emplace_back( 3, std::vector<value_t>{4,5} );
		v.emplace_back( 4, std::vector<value_t>{1,6} );
		v.emplace_back( 5, std::vector<value_t>{1,2} );
		v.emplace_back( 6, std::vector<value_t>{7,8} );

		auto res = SearchTriplesPattern( v );
		CHECK( res.found_NT == false );
	}
	{
		INFO( "test case D2 - good" )
		std::cout << "case D2 - good\n";
		std::vector<Pos_vcand> v;
		v.emplace_back( 2, std::vector<value_t>{1,4} );
		v.emplace_back( 3, std::vector<value_t>{4,5} );
		v.emplace_back( 4, std::vector<value_t>{1,3} );
		v.emplace_back( 5, std::vector<value_t>{1,2} );
		v.emplace_back( 6, std::vector<value_t>{2,3} );

		auto res = SearchTriplesPattern( v );
		CHECK( res.found_NT == true );
		CHECK( res.cand_values == trip_val );
		CHECK( res.cand_pos == trip_pos );
	}

	{
		INFO( "test case D3 - no good" )
		std::cout << "case D3 - no good\n";
		std::vector<Pos_vcand> v;
		v.emplace_back( 3, std::vector<value_t>{3,9} );
		v.emplace_back( 4, std::vector<value_t>{6,9} );
		v.emplace_back( 5, std::vector<value_t>{3,6,8} );
		v.emplace_back( 6, std::vector<value_t>{3,5} );

		auto res = SearchTriplesPattern( v );
		CHECK( res.found_NT == false );
	}

}

/// used only for unit testing
Cycle
BuildCycle( const std::string& s )
{
	assert( s.size()%2 ); // length must be odd
	Cycle c;
	for( size_t i=0; i<s.size(); i+=2 )
	{
		assert( s.at(i)=='W' || s.at(i)=='S' );
		Link link( s.at(i)=='W' ? LT_Weak :  LT_Strong );
		c.AddElem(link);
	}
	return c;
}

void
CheckCycle( int n, const Cycle& c, En_CycleType ct, int pos=0 )
{
	bool fail = false;
	std::cout << n << ": CheckCycle: " << c;
	Cycle c2(c);                      // copy, 'coz original is const
	for( decltype(c.size()) i=0; i<c.size(); i++ )
	{
		std::rotate( std::begin( c2.data() ), std::begin( c2.data() )+1, std::end( c2.data() ) );
		if( GetCycleType( c )._ctype != ct )
		{
			std::cout << "- Failure for cycle: " << c << ": Is not of required type\n";
			fail = true;
		}
	}
	CHECK( !fail );
	fail = false;
	if( ct != CT_Invalid && ct != CT_Continuous )   // position makes no sense for invalid or continuous cycles
	{
		auto computed_pos = GetCycleType( c )._idx;
		if( computed_pos != pos )
		{
			std::cout << "- Failure: position not correct, should be " << pos
				<< " but is " << computed_pos << "\n";
			fail = true;
		}
	}
	CHECK( !fail );
}

TEST_CASE( "test of cycle detection", "[cycles]" )
{
	int n=0;
	CheckCycle( n++, BuildCycle( "W-W-S-S" ), CT_Invalid );

	CheckCycle( n++, BuildCycle( "W-S-W-W-W-S" ), CT_Invalid );
	CheckCycle( n++, BuildCycle( "W-S-W-S-W-S" ), CT_Continuous );
	CheckCycle( n++, BuildCycle( "W-S-W-W-S-W-S" ), CT_Discont_2WL, 2 );
	CheckCycle( n++, BuildCycle( "W-S-W-S-S-W-S" ), CT_Discont_2SL, 3 );

	CheckCycle( n++, BuildCycle( "W-S-W-S-S-W-S-S" ), CT_Invalid ); // twice 2 strong links
	CheckCycle( n++, BuildCycle( "W-W-S-W-S-W-W-S" ), CT_Invalid ); // twice 2 weak links
}

#if 1
TEST_CASE( "XY-chains test 1", "[XY-chains-1]" )
{
	{
		std::vector<Cell2> v;
		v.push_back( Cell2( "A7", 6,9 ) );
		v.push_back( Cell2( "A5", 2,9 ) );
		v.push_back( Cell2( "A1", 2,6 ) );
		v.push_back( Cell2( "C2", 5,6 ) );
		v.push_back( Cell2( "C8", 5,8 ) );

		auto vout = buildGraphs( v );
		CHECK( vout.size() == 1 );
		const auto& graph = vout[0].first;
		const auto& vset  = vout[0].second;
		CHECK( vset.count(1) == 0 );
		CHECK( vset.count(2) == 1 );
		CHECK( vset.count(3) == 0 );
		CHECK( vset.count(4) == 0 );
		CHECK( vset.count(5) == 1 );
		CHECK( vset.count(6) == 1 );
		CHECK( vset.count(7) == 0 );
		CHECK( vset.count(8) == 1 );
		CHECK( vset.count(9) == 1 );
	}



// Example from https://www.sudokuwiki.org/sudoku.htm?bd=003001000800000000051009060080000290000700080200040503600900000002084000410050600
// see doc/AS_XY_chain_example1.png
	{
		std::vector<Cell2> v;
		v.push_back( Cell2( "B3", 4,6 ) );

		v.push_back( Cell2( "E3", 4,5 ) );
		v.push_back( Cell2( "G3", 8,5 ) );
		v.push_back( Cell2( "J3", 8,9 ) );

		v.push_back( Cell2( "E1", 5,3 ) );
		v.push_back( Cell2( "H1", 5,3 ) );

		v.push_back( Cell2( "C5", 2,3 ) );
		v.push_back( Cell2( "C9", 2,8 ) );
		v.push_back( Cell2( "J9", 9,8 ) );
		v.push_back( Cell2( "D5", 3,6 ) );

		v.push_back( Cell2( "E2", 3,4 ) );
		v.push_back( Cell2( "F2", 9,6 ) );
		v.push_back( Cell2( "F3", 9,6 ) );
		v.push_back( Cell2( "B6", 5,6 ) );
		v.push_back( Cell2( "D6", 5,6 ) );
		v.push_back( Cell2( "D4", 5,3 ) );
		v.push_back( Cell2( "G7", 4,8 ) );
		v.push_back( Cell2( "H7", 7,9 ) );

		auto vout = buildGraphs( v );
		CHECK( vout.size() == 2 );

		const auto& graph = vout[0].first;
		const auto& vset  = vout[0].second;
	}

}
#endif

TEST_CASE( "Grid::getCellsPos", "Grid::getCellsPos" )
{
	{
		std::set<pos_t> v1{ {0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8} };
		auto v2 = getCellsPos( OR_ROW, 0 );
		CHECK( v1 == v2 );
	}
	{
		std::set<pos_t> v1{ {0,2}, {1,2}, {2,2}, {3,2}, {4,2}, {5,2}, {6,2}, {7,2}, {8,2} };
		auto v2 = getCellsPos( OR_COL, 2 );
		CHECK( v1 == v2 );
	}
	{
		std::set<pos_t> v1{ {0,0}, {0,1}, {0,2}, {1,0}, {1,1}, {1,2}, {2,0}, {2,1}, {2,2} };
		auto v2 = getCellsPos( OR_BLK, 0 );
		CHECK( v1 == v2 );
	}
}

TEST_CASE( "findRowColBlkIntersect()", "FRCLI" )
{

	Cell2 c1( "B3", 4, 6 ); // pos=1,2
	Cell2 c2( "B9", 3, 6 ); // pos=1,8

	auto res = findRowColBlkIntersect( c1, c2 );

	std::set<pos_t> s1{ {1,0}, {1,1}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7} };

	CHECK( res._sPos  == s1 );
}

TEST_CASE( "addToPosSet", "addToPosSet" )
{

/*	addToPosSet()
	std::set<pos_t>& posSet,   ///< output set, we add values here
	EN_ORIENTATION   orient,
	pos_t            p1,
	pos_t            p2*/
)
