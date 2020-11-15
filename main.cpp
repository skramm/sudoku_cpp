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
#include <fstream>

using namespace std;


/// sudoku solver program
int main( int argc, const char** argv )
{
	Grid grid;
	if( argc == 1 )
	{
		cout << "usage:\n $ sudoku [-s] [-v] <-f file>: load grid file"
			<< "\n $ sudoku [-s] [-v] [grid]: read grid from command line (or use default grid)\n";
		return 0;
	}

	auto nbFlags = 0;
	bool hasFileFlag = false;
	bool saveGridToFile = false;
	for( int i=1; i<argc; i++ )
	{
		if( std::string( argv[i] ) == std::string( "-f" ) )
		{
			if( i+1 < argc )
			{
				if( !grid.loadFromFile( argv[i+1] ) )
				{
					cout << "Error: unable to read input file " << argv[i+1] << "\n";
					return 2;
				}
				hasFileFlag = true;
				cout << "succesful input file reading\n";
			}
			else
			{
				cout << "Error: no input file provided after -f\n";
				return 2;
			}
		}

		if( std::string( argv[i] ) == std::string( "-v" ) )
		{
			nbFlags++;
			g_data.Verbose = true;
			g_data.LogSteps = true;
			cout << " -Option -v (Verbose) activated\n";
		}
		if( std::string( argv[i] ) == std::string( "-l" ) )
		{
			nbFlags++;
			g_data.LogSteps = true;
			cout << " -Option -l (log steps) activated\n";
		}
		if( std::string( argv[i] ) == std::string( "-s" ) )
		{
			nbFlags++;
			saveGridToFile = true;
			cout << " -Option -s (save grid) activated\n";
		}
	}

	if( !hasFileFlag && nbFlags+1 < argc )
	{
		if( !grid.buildFromString( argv[argc-1] ) )
		{
			cout << "Error: invalid grid string given\n";
			return 4;
		}
	}

	grid.InitCandidates();
	if( saveGridToFile )
		grid.saveToFile( "current.sud" );

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
