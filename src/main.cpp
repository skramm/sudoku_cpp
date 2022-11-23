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

\todo add generation of final report, giving initial nb of empty cells, nb of steps used, algorithms used, etc.
*/

#include "header.h"
#include "algorithms.h"

#include <iomanip>

using namespace std;

enum ReturnValues: int
{
	RV_success
	,RV_missingFile
	,RV_missingFileName
	,RV_missingCells
	,RV_invalidGrid
	,RV_solvingFailure
	,RV_invalidSwitch
};

/// sudoku solver program
int main( int argc, const char** argv )
{
	Grid grid;
	if( argc == 1 )
	{
		cout << "A sudoku solver, see https://github.com/skramm/sudoku_cpp\n"
			<< "-usage:\n sudoku [-s] [-v] <-f file>: load grid file"
			<< "\n sudoku [-s] [-v] grid: read grid from command line\n"
			<< "-switches:\n -s: save grid to file (human readable), and can be loaded with -f"
			<< "\n -v: verbose\n -l: log steps"
			<< "\n -t: list implmented algorithms and stop"
			<< "\n -p: stop after first cell found"
			<< "\n-return value:\n "
			<< RV_success         << ": success (solved puzzle)\n "
			<< RV_missingFile     << ": unable to read given filename (missing or format error)\n "
			<< RV_missingFileName << ": missing filename after -f\n "
			<< RV_missingCells    << ": invalid grid given (must be 81 characters, only digits or '.')\n "
			<< RV_invalidGrid     << ": invalid grid\n "
			<< RV_solvingFailure  << ": unable to solve\n "
			<< RV_invalidSwitch   << ": invalid switch\n";

		return RV_success;
	}

	auto nbFlags = 0;
	bool hasFileFlag = false;
	bool saveGridToFile = false;

	for( int i=1; i<argc; i++ )
	{
		auto arg = std::string( argv[i] );
		if( arg == "-f" )
		{
			if( i+1 < argc )
			{
				if( !grid.loadFromFile( argv[i+1] ) )
				{
					cout << "Error: unable to read input file " << argv[i+1] << "\n";
					return RV_missingFile;
				}
				hasFileFlag = true;
				cout << "succesful input file reading\n";
			}
			else
			{
				cout << "Error: no input file provided after -f\n";
				return RV_missingFileName;
			}
		}

		if( arg == "-v" )
		{
			nbFlags++;
			g_data.Verbose = true;
			g_data.LogSteps = 3;
			cout << " -Option -v (Verbose) activated\n";
		}

		if( arg == "-c" )      // checking at every step
		{
			nbFlags++;
			g_data.doChecking = true;
			cout << " -Option -c (Checking) activated\n";
		}

		if( arg.substr(0,2) == "-l" )     // logging options
		{
			nbFlags++;
			g_data.LogSteps = 1;
			cout << " -Option -l (log steps) activated\n";
			if( arg.size() > 2 )
				switch( arg.back() )
				{
					case '1': break;
					case '2': g_data.LogSteps = 2; break;
					case '3': g_data.LogSteps = 3; break;
					default: std::cerr << "invalid switch !\n"; exit(RV_invalidSwitch);
				}
		}

		if( arg == "-p" )
		{
			cout << " -Option -p (stop after first step) activated\n";
			g_data.stopAfterFirstFound = true;
		}

		if( arg == "-s" )
		{
			nbFlags++;
			saveGridToFile = true;
			cout << " -Option -s (save grid) activated\n";
		}

		if( arg == "-t" )
		{
			std::cout << "Implemented algorithms: " << (int)ALG_END << '\n';
			for( auto i=0; i<ALG_END; i++ )
				std::cout << i+1 << ": " << GetString( static_cast<EN_ALGO>(i) ) << '\n';
			return 0;
		}
	}

	if( !hasFileFlag && nbFlags+1 < argc )
	{
		if( !grid.buildFromString( argv[argc-1] ) )
		{
			cout << "Error: invalid grid string given\n";
			return RV_missingCells;
		}
	}

	grid.initCandidates();
	if( saveGridToFile )
	{
		grid.saveToFile( "grid_start.sud" );
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		std::ostringstream oss;
		oss << "grid_start_"
			<< std::put_time( &tm, "%Y%m%d_%H%M" )
			<< ".sud";
		grid.saveToFile( oss.str() );
	}

    cout << "Starting grid:\n" << grid << endl;
    if( g_data.Verbose )
		grid.PrintCandidates( cout, "start" );
    if( !grid.Check() )
    {
		std::cout << "Grid is invalid\n";
		return RV_invalidGrid;
    }
    auto ret = RV_success;
    if( grid.Solve() )
	{
		cout << "-solved with " << g_data.NbSteps << " steps\n";
		if( saveGridToFile )
		{
			grid.saveToFile( "grid_solved.sud" );
		}
	}
	else
	{
		cout << "failure, used " << g_data.NbSteps << " steps\n";
		grid.PrintCandidates( cout, "final" );
		ret = RV_solvingFailure;
	}
	cout << grid;
    return ret;
}
