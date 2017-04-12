/**
\file main.cpp
\brief main

This file is part of https://github.com/skramm/sudoku_cpp
Licence: GPLv3
*/

//#include <iostream>

#include "grid.h"


using namespace std;

int main( int argc, const char** argv )
{
	Grid grid;
	if( argc > 1 )
	{
		cout << "Reading file " << argv[argc-1];
		if( !grid.Load( argv[argc-1] ) )
		{
			cout << ": failure\n";
			return 2;
		}
		cout << ": success\n";

		for( int i=0; i<argc-1; i++ )
		{
			if( std::string( argv[i+1] ) == std::string( "-v" ) )
				g_data.Verbose = true;
			if( std::string( argv[i+1] ) == std::string( "-s" ) )
				g_data.LogSteps = true;
		}
	}
	grid.InitCandidates();

    cout << "Starting grid:\n" << grid << endl;
    if( g_data.Verbose )
		grid.PrintCandidates( cout, "start" );
    if( !grid.Check() )
    {
		std::cout << "Grid is invalid\n";
		return 3;
    }
    int ret = 0;
    if( grid.Solve() )
		cout << "-solved with " << g_data.NbSteps << " steps\n";
	else
	{
		cout << "failure, used " << g_data.NbSteps << " steps\n";
		grid.PrintCandidates( cout, "final" );
		ret = 1;
	}
	cout << grid;
    return ret;
}
