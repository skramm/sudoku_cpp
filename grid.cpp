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
\file grid.cpp

This file is part of https://github.com/skramm/sudoku_cpp
Licence: GPLv3
*/



// some optionnal symbols ('til I write a makefile...)
//#define DEBUGMODE



#include "grid.h"
#include "algorithms.h"

#include "header.h"
#include <fstream>


GlobData g_data;

//----------------------------------------------------------------------------
void
LogStep( const Cell& cell, std::string msg )
{
	++g_data.NbSteps;
	if( g_data.LogSteps )
		std::cout << "*** step " << g_data.NbSteps << ": CELL " << cell._pos << ": " << msg << '\n';
}
//----------------------------------------------------------------------------
std::ostream&
operator << ( std::ostream& s, const Viewtable& vt )
{
	return s;

	for( index_t i=0; i<9; i++ ) // rows
	{
		s << " | ";
		for( index_t j=0; j<9; j++ ) // cols
		{
			s << vt << " ";
			if( !j%3 )
				s << " | ";
		}
		s << '\n';
	}

}
//----------------------------------------------------------------------------
void
DrawLine( std::ostream& s, char ch, bool PrintValues )
{
	s << "  ";

	int c = 17;
	if( PrintValues )
		c = 11;
	for( int i=0; i<3; i++ )
	{
		s << '+';
		for( int k=0; k<c;k++ )
			s << ch;
		s << '+';
	}
	s << '\n';
}

//----------------------------------------------------------------------------
void
PrintSpaces( std::ostream& s, int n )
{
	for( int i=0; i<n; i++ )
		s << ' ';
}

void
PrintLineNumbers( std::ostream& s, int before, int after )
{
	s << ' ';
	for( int i=0; i<9; i++ )
	{
		PrintSpaces( s, before );
		s << i+1;
		PrintSpaces( s, after );
		if( !((i+1)%3) )
			s << ' ';
	}
	s << '\n';
}
//----------------------------------------------------------------------------
std::ostream&
operator << ( std::ostream& s, const Grid& g )
{
	s << "nb unknowns cells " << g.NbUnknows() << "\n";
//	s << "nb unknowns (values) " << g.NbUnknows2() << "\n";
//	s << "res:\n";

	PrintLineNumbers( s, 3, 0 );

	DrawLine( s, '-', true );
	for( index_t i=0; i<9; i++ )
	{
		s << GetRowLetter(i) << " | ";
		for( index_t j=0; j<9; j++ )
		{
			if( g._data[i][j].GetValue() == 0 )
				s << ' ';
			else
				s << (char)('0' + g._data[i][j].GetValue());
			s << " |";
			if( !((j+1)%3) && j!=8 )
				s << '|';
			s << ' ';
		}
		s << '\n';
		if( !((i+1)%3) )
			DrawLine( s, '-', true );
	}
	return s;
}
//----------------------------------------------------------------------------
void
Grid::PrintCandidates( std::ostream& s, std::string txt ) const
{
	s << "Candidates: " << txt << '\n';
	s << " -nb unknowns (cand) " << NbUnknows() << "\n";
//	s << " -nb unknowns (values) " << NbUnknows2() << "\n";

	Viewtable vt = BuildViewtable();

	PrintLineNumbers( s, 4, 1 );

	DrawLine( s, '-', false );
	for( index_t i=0; i<3*9; i++ )
	{
		if( !((i+2)%3) )
			s << GetRowLetter(i/3) << " |";
		else
			s << "  |";

		for( index_t j=0; j<3*9; j++ )
		{
			s << vt[i][j];
			if( ((j+1)%3) )
				s << ' ';
			if( !((j+1)%3) )
				s << '|';
			if( !((j+1)%9) && j!=26 )
				s << '|';
//			s << ' ';
		}
		s << '\n';
		if( !((i+1)%9) && i != 26 )
			DrawLine( s, '=', false );
		else
			if( !((i+1)%3) )
				DrawLine( s, '-', false );

	}
}

//----------------------------------------------------------------------------
/// Constructor, fills with a simple sample grid
Grid::Grid()
{
	std::array<std::array<int,9>,9> s = {{
	{ 9,6,0, 0,0,0, 7,0,0 },
	{ 0,5,0, 7,0,9, 2,4,6 },
	{ 0,2,7, 0,0,4, 8,0,0 },

	{ 0,0,0, 0,5,7, 3,1,4 },
	{ 0,0,0, 9,0,3, 0,0,0 },
	{ 4,3,6, 1,8,0, 0,0,0 },

	{ 0,0,1, 4,0,0, 9,2,0 },
	{ 3,4,9, 2,0,5, 0,6,0 },
	{ 0,0,2, 0,0,0, 0,3,5 }
	}};

	for( index_t i=0; i<9; i++ )
		for( index_t j=0; j<9; j++ )
			_data[i][j].SetValue( s[i][j] );

	for( index_t i=0; i<9; i++ )
		for( index_t j=0; j<9; j++ )
		{
			_data[i][j].SetPos( i, j );
		}
}
//----------------------------------------------------------------------------
/// returns false if puzzle is inconsistent
bool
Grid::Check( EN_ORIENTATION orient ) const
{
	for( index_t i=0; i<9; i++ )  // for each row/col/block
	{
		View_1Dim_c v1d = GetView( orient, i );

		for( index_t j=0; j<9; j++ ) // for each cell in the view
		{
			const Cell& cell_1 = v1d.GetCell(j);
			auto val_1 = cell_1.GetValue();
			if( val_1 != 0 )
			{
				for( index_t k=0; k<9; k++ ) // for each cell in the view
				{
					const Cell& cell_2 = v1d.GetCell(k);
					auto val_2 = cell_2.GetValue();
					if( k != j && val_1 == val_2 )
					{
						std::cout << "Error, cell at " << cell_1._pos << " and at " << cell_2._pos << " have same value: " << (int)val_1 << '\n';
						return false;
					}
				}
			}
		}
	}
	return true;
}
//----------------------------------------------------------------------------
/// returns false if puzzle is inconsistent
bool
Grid::Check() const
{
	if( !Check( OR_ROW ) )
	{
		std::cout << "Error, invalid row\n";
		return false;
	}
	if( !Check( OR_COL ) )
	{
		std::cout << "Error, invalid col\n";
		return false;
	}
	if( !Check( OR_BLK ) )
	{
		std::cout << "Error, invalid block\n";
		return false;
	}
	return true;
}
//----------------------------------------------------------------------------
/// Erase all candidates from cells that have a value
void
Grid::InitCandidates()
{
	for( index_t i=0; i<9; i++ )  // for each row
	{
		View_1Dim_nc row = GetRow(i);
		for( index_t j=0; j<9; j++ ) // for each cell in the row
		{
			Cell& c = row.GetCell(j);
			if( c.GetValue() != 0 )
				c.RemoveAllCandidates();
		}
	}
}
//----------------------------------------------------------------------------
bool
Grid::Load( std::string fn )
{
	std::ifstream infile( fn );
	if( !infile.is_open() )
	{
		std::cout << "Error: unable to open file " << fn << '\n';
		return false;
	}
	int li = 0;
	std::string line;
	while( std::getline( infile, line ) )
	{
		if( !line.empty() )
			if( ( line[0] >= '0' &&  line[0] <= '9') || line[0]=='.' || line[0]=='_' || line[0]==' ' ) // lines starting with other characters are ignored
			{

				if( line.size() != 9  && li < 9 )
				{
					std::cout << "Error: line " << li<< " has a length of " << line.size() << ": -" << line << "-\n";;
					return false;
				}
				for( size_t col=0; col<line.size(); col++ )
				{
					if( line[col] == '.' || line[col] == '0' || line[col] == '_' || line[col] == ' ' )
						_data[li][col].SetValue( 0 );
					else
					{
						if( line[col] < '1' ||  line[col] > '9' )
						{
							std::cout << "Error: line " << li << " holds an invalid character: " << line[col] << " at col " << col << '\n';
							return false;
						}
						_data[li][col].SetValue( line[col] - '0' );
					}
				}
				li++;
			}
	}
	return true;
}
//----------------------------------------------------------------------------
View_1Dim_c
Grid::GetView( EN_ORIENTATION orient, index_t idx ) const
{
	switch( orient )
	{
		case OR_ROW: return GetRow(idx);   break;
		case OR_COL: return GetCol(idx);   break;
		case OR_BLK: return GetBlock(idx); break;
		default: assert(0);
	}
}
//----------------------------------------------------------------------------
View_1Dim_nc
Grid::GetView( EN_ORIENTATION orient, index_t idx )
{
	switch( orient )
	{
		case OR_ROW: return GetRow(idx);   break;
		case OR_COL: return GetCol(idx);   break;
		case OR_BLK: return GetBlock(idx); break;
		default: assert(0);
	}
}

//----------------------------------------------------------------------------
void
Grid::PrintAll( std::ostream& s, std::string txt )
{
	if( g_data.Verbose )
	{
		s << "Values: " << txt << "\n" << *this;
		PrintCandidates( std::cout, txt );
	}
	if( !Check() )
	{
		std::cout << "grid invalid !\n";
		assert(0);
	}
}
//----------------------------------------------------------------------------
/// Returns a set of cell positions that are on same row/col/block as \c src and share some common properties
/**
See:
- EN_GOCMODE
- Grid::GetOtherCells_cand()
- Grid::GetOtherCells_nbc()
*/
std::vector<pos_t>
Grid::GetOtherCells( const Cell& src, int arg, EN_ORIENTATION orient, EN_GOCMODE goc_mode ) const
{
	std::vector<pos_t> out;
	auto row = src._pos.first;
	auto col = src._pos.second;

	View_1Dim_c v1d;
	switch( orient )
	{
		case OR_ROW: v1d = GetView( OR_ROW, row ); break;
		case OR_COL: v1d = GetView( OR_COL, col ); break;
		case OR_BLK: v1d = GetView( OR_BLK, GetBlockIndex(row,col) ); break;
		default: assert(0);
	}
//std::cout << "GetOtherCells(" << GetString(orient) << "): 1st cell pos=" << v1d.GetCell(0)._pos << '\n';
	for( index_t i=0; i<9; i++ )  // for each cell of the view
	{
		const Cell& c = v1d.GetCell( i );
		if( c._pos != src._pos )
		{
			switch( goc_mode )
			{
				case GOCM_NB_CAND:
					if( c.NbCandidates() == arg )
						out.push_back( c._pos );
				break;
				case GOCM_CAND_VALUE:
					if( c.HasCandidate( arg ) )
						out.push_back( c._pos );
				break;
				default: assert(0);
			}
		}
	}
	return out;
}

//----------------------------------------------------------------------------
/// Returns a set of cell positions that are on same row/col/block as \c src and have candidate \c cand
std::vector<pos_t>
Grid::GetOtherCells_cand( const Cell& src, int cand, EN_ORIENTATION orient ) const
{
	return GetOtherCells( src, cand, orient, GOCM_CAND_VALUE );
}
//----------------------------------------------------------------------------
/// Returns a set of cell positions that are on same row/col/block as \c src and have \c nb candidates
std::vector<pos_t>
Grid::GetOtherCells_nbc( const Cell& src, int nbc, EN_ORIENTATION orient ) const
{
	return GetOtherCells( src, nbc, orient, GOCM_NB_CAND );
}
//----------------------------------------------------------------------------
int
Grid::NbUnknows() const
{
	int n = 0;
	for( index_t i=0; i<9; i++ )  // for each row
	{
		View_1Dim_c row = GetRow( i );
		for( index_t j=0; j<9; j++ ) // for each cell in the view
		{
			if( row.GetCell( j ).NbCandidates() != 0 )
				n++;
		}
	}
	return n;
}
//----------------------------------------------------------------------------
#if 0
int
Grid::NbUnknows2() const
{
	int n = 0;
	for( index_t i=0; i<9; i++ )  // for each row
	{
		View_1Dim_c row = GetRow( i );
		for( index_t j=0; j<9; j++ ) // for each cell in the view
		{
			if( row.GetCell( j ).GetValue() == 0 )
				n++;
		}
	}
	return n;
}
#endif
//----------------------------------------------------------------------------
/// Returns true if something changed
bool
Grid::ProcessAlgorithm( EN_ALGO algo )
{
	bool res = false;
	switch( algo )
	{
		case ALG_REMOVE_CAND:    res = Algo_RemoveCandidates( *this );     break;
		case ALG_SEARCH_PAIRS:   res = Algo_SearchNakedPairs( *this );     break;
		case ALG_SEARCH_TRIPLES: res = Algo_SearchNakedTriples( *this );   break;
		case ALG_SEARCH_SINGLE_CAND: res = Algo_SearchSingleCand( *this ); break;
		case ALG_SEARCH_MISSING_SINGLE: res = Algo_SeachForSingleMissing( *this ); break;
		case ALG_POINTING_PT:    res = Algo_PointingPairsTriples( *this ); break;
		case ALG_BOX_RED:        res = Algo_BoxReduction( *this ); break;
		case ALG_XY_WING:        res = Algo_XY_Wing( *this ); break;
#ifndef BUILD_WITHOUT_UDGCD
		case ALG_X_CYCLES:       res = X_Cycles( *this ); break;
#endif
		default: assert(0);
	}
//	SearchSingles();
//	PrintAll( std::cout, std::string( "after algo " + std::string( GetString(algo) ) ) );
//	assert( NbUnknows() == NbUnknows2() );
	return res;
}
//----------------------------------------------------------------------------
bool
Grid::Solve()
{
	int iter=0;
	int nu_before = NbUnknows();
	int nu_after = nu_before;

	bool stop = true;
	bool stop_1 = false;
	do
	{
//		std::cout << "-loop 1: starting iter " << iter << "\n";
		EN_ALGO algo = ALG_REMOVE_CAND;
		stop_1 = true;
		nu_before = nu_after;
		bool res = false;
		do
		{
			res = ProcessAlgorithm( algo );
			nu_after = NbUnknows();
//			std::cout << "  -loop 2: algo " << algo << "-" << GetString(algo) << ": res=" << res <<  ", nb unknowns left=" << nu_after << "\n";

			if( g_data.Verbose )
				PrintAll( std::cout, "iter " + std::to_string(iter) + ": after algo " + GetString(algo)  );

			if( !res )                                                       // if no changes happened, then switch to next algorithm
				algo = static_cast<EN_ALGO>( static_cast<int>(algo) +1 );
			else                                                             // else, we need to start over loop 1
				stop_1 = false;

		}
// we stop this loop IF
// - some changes occurred
// - we have processed the last algorithm
// - no more unknowns
		while( !res && algo != ALG_END && nu_after != 0 );

//		std::cout << "END of LOOP2, stop1=" << stop_1 << " nu_before=" << nu_before << " nu_after=" << nu_after << "\n";
		iter++;

		stop = false;
		if( ( nu_before == nu_after || nu_after == 0 ) && stop_1==true )
			stop = true;
//		std::cout << "END of LOOP1, stop1=" << stop_1 << " nu_before=" << nu_before << " nu_after=" << nu_after << "stop=" << stop << "\n";

	}
//	while( n_before != n_after &&  n_after != 0 );
	while( !stop );

//	std::cout << "nb_unknowns=" << nu_after << "\n";
	if( nu_after == 0 )
		return true;
	return false;
}


//----------------------------------------------------------------------------
Viewtable
Grid::BuildViewtable() const
{
	Viewtable vt;
	for( int i=0; i<9; i++ )  // for each row
	{
		View_1Dim_c row = GetRow(i);
		for( int j=0; j<9; j++ ) // for each cell in the row
		{
			Cell cell = row.GetCell(j);

			for( int k=0; k<9; k++ )
			{
				int r = i*3 + k/3;
				int c = j*3 + k%3;

				vt[r][c] = ' ';
				if( cell._cand[k+1] )
					vt[r][c] = k + '1';
			}
		}
	}
	return vt;
}
//----------------------------------------------------------------------------

