/**
\file
\brief man header file, holds some common stuff

This file is part of https://github.com/skramm/sudoku_cpp
Licence: GPLv3
*/

/**
\mainpage sudoku_cpp refence pages

See https://github.com/skramm/sudoku_cpp
*/
#include "grid.h"

#ifdef DEBUGMODE
	#define DEBUG if(1) std::cout
#else
	#define DEBUG if(0) std::cout
#endif

#define COUT(a) { if( g_data.Verbose ) std::cout << a; }

#define PRINT_MAIN_IDX( o ) \
	{ \
		if( g_data.Verbose ) {\
			std::cout << " -" << (o==OR_ROW ? "row" : (o==OR_COL?"col":"block") ) << '='; \
			if( o==OR_ROW ) \
				std::cout << GetRowLetter(i); \
			else \
				std::cout << (int)i+1; \
			std::cout << '\n'; \
		} \
	}

void TestCycleType();

