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
\file main.cpp
\brief main

This file is part of https://github.com/skramm/sudoku_cpp
*/

#include "header.h"
#include "algorithms.h"

using namespace std;


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
				g_data.LogSteps = true;
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
