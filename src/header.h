/**
\file
\brief main header file, holds some common stuff

This file is part of https://github.com/skramm/sudoku_cpp
Licence: GPLv3
*/

/**
\mainpage sudoku_cpp refence pages

See https://github.com/skramm/sudoku_cpp
and README.md
*/
#ifndef HG_HEADER_H
#define HG_HEADER_H

#ifdef DEBUGMODE
	#define DEBUG if(1) std::cout
#else
	#define DEBUG if(0) std::cout
#endif

#define ASSERT_1(a,b) \
	if( !(a) ) \
	{ \
		std::cout << "assert failure, exiting...\n" \
			<< "file:" << __FILE__ << " line:" << __LINE__ \
			<< "\nexpression: \"" << #a << "\" value=" << (a) << "\n"; \
		exit(1); \
	}

#define COUT(a) { if( g_data.Verbose ) std::cout << a << '\n'; }

#define PRINT_MAIN_IDX( o ) \
	{ \
		if( g_data.Verbose ) {\
			std::cout << " -" << (o==OR_ROW ? "row" : (o==OR_COL?"col":"block") ) << '='; \
			if( o==OR_ROW ) \
				std::cout << GetRowLetter(idx); \
			else \
				std::cout << (int)idx+1; \
			std::cout << '\n'; \
		} \
	}

void TestCycleType();

#endif // HG_HEADER_H

