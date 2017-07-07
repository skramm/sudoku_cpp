/**
\file main.cpp
\brief main

This file is part of https://github.com/skramm/sudoku_cpp
Licence: GPLv3
*/

//#include <iostream>

#include "header.h"


using namespace std;

#ifndef TESTMODE

/// sudoku solver program
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
			{
				g_data.Verbose = true;
				cout << " -Option -v (Verbose) activated\n";
			}
			if( std::string( argv[i+1] ) == std::string( "-s" ) )
			{
				g_data.LogSteps = true;
				cout << " -Option -s (log Steps) activated\n";
			}
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
#else
/// Test program
int main( int argc, const char** argv )
{
	std::cout << "Test program\n";
	TestCycleType();
}
#endif
