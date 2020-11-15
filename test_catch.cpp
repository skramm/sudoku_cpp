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


#include "algorithms.h"

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
		std::vector<pos_vcand> v;
		v.emplace_back( 3, std::vector<value_t>{4,5} );
		v.emplace_back( 4, std::vector<value_t>{1,2,3} );
		v.emplace_back( 5, std::vector<value_t>{1,2,3} );
		v.emplace_back( 6, std::vector<value_t>{1,2,3} );

		auto res = SearchTriplesPattern( v );
		REQUIRE( res.found_NT == true );
		REQUIRE( res.cand_values == trip_val );
		REQUIRE( res.cand_pos == trip_pos );
	}
	{
		INFO( "test case B" )
		std::vector<pos_vcand> v;
		v.emplace_back( 3, std::vector<value_t>{4,5} );
		v.emplace_back( 4, std::vector<value_t>{1,2} );
		v.emplace_back( 5, std::vector<value_t>{1,2,3} );
		v.emplace_back( 6, std::vector<value_t>{1,2,3} );

		auto res = SearchTriplesPattern( v );
		REQUIRE( res.found_NT == true );
		REQUIRE( res.cand_values == trip_val );
		REQUIRE( res.cand_pos == trip_pos );
	}
	{
		INFO( "test case C - no good" )
		std::vector<pos_vcand> v;
		v.emplace_back( 3, std::vector<value_t>{4,5} );
		v.emplace_back( 4, std::vector<value_t>{1,2} );
		v.emplace_back( 5, std::vector<value_t>{1,2} );
		v.emplace_back( 6, std::vector<value_t>{1,2,3} );

		auto res = SearchTriplesPattern( v );
		REQUIRE( res.found_NT == false );
	}
	{
		INFO( "test case C - good" )
		std::vector<value_t> vC{2,3};

		std::vector<pos_vcand> v;
		v.emplace_back( 3, std::vector<value_t>{4,5} );
		v.emplace_back( 4, std::vector<value_t>{1,2} );
		v.emplace_back( 5, std::vector<value_t>{2,3} );
		v.emplace_back( 6, std::vector<value_t>{1,2,3} );

		auto res = SearchTriplesPattern( v );
		REQUIRE( res.found_NT == true );
		REQUIRE( res.cand_values == trip_val );
		REQUIRE( res.cand_pos == trip_pos );
	}
	{
		INFO( "test case D - no good" )
		std::cout << "test case D - no good\n";
		std::vector<pos_vcand> v;
		v.emplace_back( 3, std::vector<value_t>{4,5} );
		v.emplace_back( 4, std::vector<value_t>{1,6} );
		v.emplace_back( 5, std::vector<value_t>{1,2} );
		v.emplace_back( 6, std::vector<value_t>{7,8} );

		auto res = SearchTriplesPattern( v );
		REQUIRE( res.found_NT == false );
	}
	{
		INFO( "test case D - good" )
		std::cout << "test case D - good\n";
		std::vector<pos_vcand> v;
		v.emplace_back( 2, std::vector<value_t>{1,4} );
		v.emplace_back( 3, std::vector<value_t>{4,5} );
		v.emplace_back( 4, std::vector<value_t>{1,3} );
		v.emplace_back( 5, std::vector<value_t>{1,2} );
		v.emplace_back( 6, std::vector<value_t>{2,3} );

		auto res = SearchTriplesPattern( v );
		REQUIRE( res.found_NT == true );
		REQUIRE( res.cand_values == trip_val );
		REQUIRE( res.cand_pos == trip_pos );
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
CheckCycle( const Cycle& c, En_CycleType ct, int pos=0 )
{
	std::cout << "CheckCycle: " << c;
	Cycle c2(c);                      // copy, 'coz original is const
	for( int i=0; i<c.size(); i++ )
	{
		std::rotate( std::begin( c2.data() ), std::begin( c2.data() )+1, std::end( c2.data() ) );
		if( GetCycleType( c ).first != ct )
		{
			std::cout << "- Failure for cycle: " << c << ": Is not of required type\n";
		}
	}
	if( ct != CT_Invalid )
	{
		if( GetCycleType( c ).second != pos )
		{
			std::cout << "- Failure for cycle: " << c << ": position not correct\n";
		}
	}
}

TEST_CASE( "test of cycle detection", "[cycles]" )
{
	CheckCycle( BuildCycle( "W-S-W-W-W-S" ), CT_Invalid );
	CheckCycle( BuildCycle( "W-S-W-S-W-S" ), CT_Continuous );
	CheckCycle( BuildCycle( "W-S-W-W-S-W-S" ), CT_Discont_2WL, 2 );
	CheckCycle( BuildCycle( "W-S-W-S-S-W-S" ), CT_Discont_2SL, 3 );

	CheckCycle( BuildCycle( "W-S-W-S-S-W-S-S" ), CT_Invalid ); // twice 2 strong links
	CheckCycle( BuildCycle( "W-W-S-W-S-W-W-S" ), CT_Invalid ); // twice 2 weak links
}

