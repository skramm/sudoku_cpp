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
\file grid.h
\brief Holds some useful definitions

*/

#ifndef HG_GRID_H
#define HG_GRID_H

#include <array>
#include <sstream>
#include <vector>

#include <set>
#include <map>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <type_traits>


/// The values in the cells, in the range [1:9]
using value_t = uint8_t;

/// Index type
using index_t = uint8_t;

/// A position in the grid. "first" is row, "second" is column. 0-based
using pos_t = std::pair<index_t,index_t>;

/// A pair of (linked) positions
using pospair_t = std::pair<pos_t,pos_t>;

/// Each cell has such a map with 9 values, to hold candidates with true/false value. Values are in [1:9]
using cand_map_t=std::map<value_t,bool>;

struct Cell;
void LogStep( int level, const Cell& cell, std::string msg );


#ifdef NDEBUG
	#define ASSERT_1( a, b ) ;
#else
#define ASSERT_1( a, b ) \
	{ \
		if( (a) == false ) \
		{ \
			std::cerr << "ASSERT failure, value=" << (b) << '\n'; \
			std::exit(1); \
		} \
	}
#endif

//----------------------------------------------------------------------------
struct Pos
{
	private:
		pos_t pa;
	public:
/// Constructor 1a
	Pos( std::string spos )
	{
		pa.first  = spos[0] != 'J' ? spos[0] - 'A': 8;
		pa.second = spos[1] - '1';
	}
/// Constructor 1b
	Pos( const char* spos )
	{
		pa.first  = spos[0] != 'J' ? spos[0] - 'A': 8;
		pa.second = spos[1] - '1';
	}

/// Constructor 2
	Pos( index_t r, index_t c )
	{
		pa.first  = r;
		pa.second = c;
	}
/// Constructor 3
	Pos( pos_t p ) : pa(p)
	{}

	const index_t& row() const { return pa.first; }
	const index_t& col() const { return pa.second; }
	index_t& row() { return pa.first; }
	index_t& col() { return pa.second; }

	index_t getBlockIndex() const
	{
		return row()/3*3 + col()/3;
	}

	friend bool operator == ( const Pos& p1, const Pos& p2 )
	{
		return p1.pa == p2.pa;
	}

/// sorting operator, based on position
	friend bool operator < ( const Pos& p1, const Pos& p2 )
	{
		if( p1.row() < p2.row() )
			return true;
		if( p1.row() == p2.row() )
			return p1.col() < p2.col();
		return false;
	}

};
//----------------------------------------------------------------------------
/// Holds some global vars
struct GlobData
{
	int  LogSteps = 0;
#ifdef TESTMODE
	bool Verbose  = true;
#else
	bool Verbose  = false;
#endif
	int  NbSteps  = 0;
	bool doChecking = false;
};
extern GlobData g_data;

//----------------------------------------------------------------------------
/// Candidate Map
struct CandMap
{
	CandMap()
	{
		for( value_t i=1; i<10; i++ )
			cmap[i]=true;
	}
	bool Has( value_t cand )
	{
		return cmap[cand];
	}
	void Remove( value_t cand )
	{
		cmap[cand] = false;
	}

	const bool& operator [] ( const value_t& v ) const
	{
		return cmap.at(v);
	}

	bool& operator [] ( const value_t& v )
	{
		return cmap[v];
	}

	private:
		std::map<value_t,bool> cmap;
};
//----------------------------------------------------------------------------
/// Return type of SearchTriplesPattern(). Holds the naked triples values and positions in the row/col/block
struct NakedTriple
{
	bool found_NT = false;
	std::array<value_t,3> cand_values;
	std::array<index_t,3> cand_pos;
	void foundPattern() { found_NT = true; }

	friend std::ostream& operator << ( std::ostream& s, const NakedTriple& nt )
	{
//		s << "NakedTriple: found=" << std::boolalpha << nt.found_NT;
		if( nt.found_NT )
			s << '(' << (int)nt.cand_values[0] << '-' << (int)nt.cand_values[1] << '-' << (int)nt.cand_values[2] << ')';
		else
			s << "(--NONE!--)";
//		s << '\n';
		return s;
	}
};

//----------------------------------------------------------------------------
/// Orientation : column, row or block
enum EN_ORIENTATION { OR_COL=0, OR_ROW, OR_BLK, OR_INVALID };

inline
const char*
GetString( EN_ORIENTATION orient )
{
	switch( orient )
	{
		case OR_COL: return "COL"; break;
		case OR_ROW: return "ROW"; break;
		case OR_BLK: return "BLK"; break;
		default: assert(0);
	}
}

//----------------------------------------------------------------------------
/// Returns index inside block from (row,col) of cell
/** See opposite: getPosFromBlockIndex() */
inline
index_t
GetBlockIndex( index_t row, index_t col )
{
	return row/3*3 + col/3;
}

/// Returns index inside block from position (row,col) of cell
/** See opposite: getPosFromBlockIndex() */
inline
index_t
GetBlockIndex( pos_t p )
{
	return GetBlockIndex( p.first, p.second );
}

/// Input: a block index, in [0:9]
/// Output: the row of the block, in [0:2]
inline
index_t
getBlockRow( index_t blockIndex )
{
	assert( blockIndex<10 );
	return blockIndex/3;
}

/// Input: a block index, in [0:9]
/// Output: the row of the block, in [0:2]
inline
index_t
getBlockCol( index_t blockIndex )
{
	assert( blockIndex<10 );
	return blockIndex%3;
}

/// Returns position inside block from index
/** See opposite: GetBlockIndex() */
inline
pos_t
getPosFromBlockIndex(
	index_t blockId,  ///< Block index
	index_t idx       ///< Index of cell inside the block
)
{
	pos_t pos( blockId/3*3, blockId%3*3 );
	pos.first  += idx/3;
	pos.second += idx%3;
	return pos;
}
//----------------------------------------------------------------------------
inline
char
GetRowLetter( index_t i )
{
	char c = i+'A';
	if( i==8 )   // we replace 'I' by 'J' for readability
		c++;
	return c;
}
//----------------------------------------------------------------------------
/// Stream cell position as "A1", "B2", ...
inline
std::ostream& operator << ( std::ostream& s, const pos_t& p )
{
	s << GetRowLetter(p.first) << p.second+1;
	return s;
}

//----------------------------------------------------------------------------
/// Reason to remove a candidate, see \c Because
enum BecauseType
{
	B_noReason
	,B_ValuePresent
	,B_NakedTriples
	,B_PointingPairsTriples
	,B_NakedPair
};

/// Holds explanation of why we remove a candidate
struct Because
{
	Because() {}
	Because( BecauseType bt, EN_ORIENTATION orient )
		: _bt(bt), _orient( orient )
	{}

	Because( BecauseType bt, EN_ORIENTATION orient, NakedTriple tp )
		: _bt(bt), _orient( orient ), _nakedTriple(tp)
	{}

	Because( BecauseType bt, const std::vector<value_t>& np, EN_ORIENTATION orient )
		: _bt(bt), _np(np), _orient( orient )
	{}
	Because( BecauseType bt, index_t idx1, index_t idx2, EN_ORIENTATION orient )
		: _bt(bt), _idx1(idx1), _idx2(idx2), _orient( orient )
	{}
	std::string getString() const
	{
		std::stringstream oss;

		switch( _bt )
		{

			case B_ValuePresent:
			{
				oss << "is present in " << GetString( _orient )
					<< " at position ";
				pos_t pos;
				switch( _orient )
				{
					case OR_ROW: pos = pos_t( _idx1, _idx2 ); break;
					case OR_COL: pos = pos_t( _idx2, _idx1 ); break;
					case OR_BLK: pos = getPosFromBlockIndex( _idx1, _idx2 ); break;
					default: assert(0);
				}
				oss << pos;
			}
			break;

			case B_NakedTriples:
				oss << "Naked Triples in " << GetString( _orient ) << ": " << _nakedTriple;
			break;

			case B_PointingPairsTriples:
				oss << "Pointing Pairs or Triples in " << GetString( _orient );
			break;

			case B_NakedPair:
				oss << "Naked pair (" << (int)_np[0] << '-' << (int)_np[1] << ") in " << GetString( _orient );
			break;

			case B_noReason: break;
			default: assert(0);
		}
		return oss.str();
	}
	BecauseType    _bt = B_noReason;
	index_t        _idx1;
	index_t        _idx2;
	std::vector<value_t> _np;    ///< holds naked pair
	EN_ORIENTATION _orient = OR_INVALID;
	NakedTriple    _nakedTriple;
};


//----------------------------------------------------------------------------
/// Holds a cell, has either a value, either a set of candidates (in which case the value is 0)
struct Cell
{
	friend std::ostream& operator << ( std::ostream& s, const Cell& c )
	{
		s << "CELL: pos=" << c._pos << " val=" << (int)c._value;
		return s;
	}

	private:
	value_t   _value = 0;  ///< Cell value
	CandMap   _cand;       ///< Candidate map
	pos_t     _pos;        ///< Position of cell in grid

	public:
	value_t GetValue() const { return _value; }
	void SetValue( value_t v ) { _value = v; }

	void SetPos( index_t i, index_t j )
	{
		_pos.first  = i;
		_pos.second = j;
	}
	pos_t GetPos() const { return _pos; }

	void PrintCellCandidates( std::ostream& s )
	{
		bool found = false;
		for( value_t i=1; i<10; i++ )
			if( _cand[i] )
			{
				s << (int)i << ",";
				found = true;
			}
		if( !found )
			s << "(none)";
		s << '\n';
	}
	void RemoveAllCandidates()
	{
		for( value_t i=1; i<10; i++ )
			_cand[i] = false;
	}
	bool RemoveCellCandidates( std::vector<value_t> v_cand, Because bec=Because() )
	{
		bool b = false;
		for( auto v: v_cand )
		{
			bool b1 = RemoveCandidate( v, bec );
			if( b1 )
				b = true;
		}
		return b;
	}
/// Remove candidate \c val in the cell, returns true if the cell did hold that value as candidate, false if not
	bool RemoveCandidate( value_t val, Because bec=Because() )
	{
		if( _cand[val] )
		{
			if( bec._bt != B_noReason )
				LogStep( 2, *this, "remove candidate " + std::to_string(val) + " because " + bec.getString() );
			else
				LogStep( 2, *this, "remove candidate " + std::to_string(val) );
			_cand[val] = false;
			if( NbCandidates() == 1 )
			{
				_value = GetValueFromCandidate();
				_cand[_value] = false;
				LogStep( 1, *this, "assign value " + std::to_string(_value) );
			}
			return true;
		}
		return false;
	}

	bool RemoveAllCandidatesBut( value_t val )
	{
		bool removalDone(false);
		for( value_t i=1; i<10; i++ )
			if( i != val )
				if( RemoveCandidate( i ) )
					removalDone = true;
		return removalDone;
	}
	std::vector<value_t> GetCandidates() const
	{
		std::vector<value_t> v;
		for( value_t i=1; i<10; i++ )
			if( _cand[i] )
				v.push_back( i );
		return v;
	}
	uint8_t NbCandidates() const
	{
		uint8_t n = 0;
		for( value_t i=1; i<10; i++ )
			if( _cand[i] )
				n++;
		return n;
	}
	bool HasCandidate( value_t currentValue ) const
	{
		assert( currentValue>0 && currentValue<10 );
		return _cand[currentValue];
	}

	value_t GetValueFromCandidate()
	{
		assert( NbCandidates() == 1 );
		for( value_t i=1; i<10; i++ )
			if( _cand[i] )
			{
				_cand[i] = false;
				return i;
			}
		assert(0);
		return 0; // to avoid warning
	}
	bool IsInBlock( index_t bl ) const
	{
		return bl == GetBlockIndex( _pos );
	}
};
//----------------------------------------------------------------------------
/// holds pointers on one element (column, row or block) of the grid
template<typename T>
struct View_T
{
	std::array<T,9> _viewData;

	typename std::add_lvalue_reference< typename std::remove_pointer<T>::type >::type
	GetCell( int i )
	{
		assert( i<9 );
		return *_viewData[i];
	}
	const typename std::add_lvalue_reference< typename std::remove_pointer<T>::type >::type
	GetCell( int i ) const
	{
		assert( i<9 );
		return *_viewData[i];
	}
	bool RemoveCand( value_t v )
	{
		bool retval(false);
		for( auto& e: _viewData )
			if( e->RemoveCandidate( v ) )
				retval = true;
		return retval;
	}
};
//----------------------------------------------------------------------------

using View_1Dim_c  = View_T<const Cell*>;
using View_1Dim_nc = View_T<Cell*>;

using Viewtable = std::array<std::array<char,27>,27>;

inline
void
PrintView( std::ostream& s, View_1Dim_nc& v )
{
	s << "values: ";
	for( index_t i=0; i<9; i++ )
	{
		s << (int)v.GetCell(i).GetValue();
	}
	s << '\n';
	for( index_t i=0; i<9; i++ )
	{
		Cell& c = v.GetCell(i);
		s << i+1 << ": ";
		c.PrintCellCandidates( s );
	}
	s << '\n';
}

/// used in Grid::GetOtherCells()
enum EN_GOCMODE { GOCM_NB_CAND, GOCM_CAND_VALUE };


enum EN_ALGO
{
	ALG_REMOVE_CAND = 0,
	ALG_SEARCH_PAIRS,
	ALG_SEARCH_TRIPLES,
	ALG_SEARCH_SINGLE_CAND,
	ALG_SEARCH_MISSING_SINGLE,
	ALG_POINTING_PT,
	ALG_BOX_RED,
	ALG_XY_WING,
#ifndef BUILD_WITHOUT_UDGCD
	ALG_X_CYCLES,
#endif
	ALG_END
};

//----------------------------------------------------------------------------
template<typename T>
std::vector<T>
VectorRemoveDupes( std::vector<T>& vin )
{
	std::vector<T> vout;
	for( const auto& elem: vin )
	{
		if( std::find( std::begin(vout), std::end(vout), elem ) ==  std::end(vout) )
			vout.push_back( elem );
	}
/*	auto n = vin.size() - vout.size();
	if( n )
		std::cout << "VectorRemoveDupes(): REMOVED " << n << " DUPES\n";
*/
	return vout;
}
//----------------------------------------------------------------------------
template<typename T>
void
PrintVector( const std::vector<T>& v, std::string s )
{
	std::cout << s << ": " << v.size() << " elems\n";
	for( auto p: v )
			std::cout << p << ' ';
	std::cout << '\n';
}

template<>
inline
void
PrintVector<index_t>( const std::vector<index_t>& v, std::string s )
{
	std::cout << s << ": " << v.size() << " elems\n";
	for( auto p: v )
			std::cout << (int)p << ' ';
	std::cout << '\n';
}
/*
template<>
inline
void
PrintVector<std::pair<bool,value_t>>( const std::vector<std::pair<bool,value_t>& v, std::string s )
{
	std::cout << s << ": " << v.size() << " elems\n";
	for( auto p: v )
			std::cout << (int)p << ' ';
	std::cout << '\n';
}
*/
//----------------------------------------------------------------------------
/// Add element \c elem to vector \c v \b only if it is not already present
template<typename T>
void AddToVector( std::vector<T>& v, const T& elem )
{
	if( std::find( v.begin(), v.end(), elem ) == v.end() )
	{
//		std::cout << "  -adding elem " << elem << '\n';
		v.push_back( elem );
	}
}
//----------------------------------------------------------------------------
/// Adds to vector \c v the elements of \c v2, only if they are not already present
template<typename T>
void AddToVector( std::vector<T>& v, const std::vector<T>& v2 )
{
	for( const auto& elem: v2 )
		AddToVector( v, elem );
}
//----------------------------------------------------------------------------
class Grid
{
	friend std::ostream& operator << ( std::ostream&, const Grid& );

	public:
		Grid();
		bool loadFromFile( std::string fn=std::string() );
		bool saveToFile( std::string ) const;
		bool Check() const;
		bool Solve();
		void InitCandidates();
//		void SetVerbose(bool b ) { _verbose = b; }
		void PrintCandidates( std::ostream&, std::string=std::string() ) const;
		void PrintAll( std::ostream&, std::string ) const;

		bool buildFromString( std::string );
		Cell&       GetCellByPos( pos_t );
		const Cell& GetCellByPos( pos_t ) const;

		View_1Dim_c  GetView( EN_ORIENTATION, index_t ) const;
		View_1Dim_nc GetView( EN_ORIENTATION, index_t );

		std::vector<pos_t> GetOtherCells_nbc(  const Cell&, int nb, EN_ORIENTATION )   const;
		std::vector<pos_t> GetOtherCells_cand( const Cell&, int cand, EN_ORIENTATION ) const;

		const Cell& getCell(index_t idx) const;

	private:
		std::vector<pos_t> GetOtherCells( const Cell&, int, EN_ORIENTATION, EN_GOCMODE ) const;

	private:
		bool Check( EN_ORIENTATION ) const;

		int  NbUnknows() const;
		bool ProcessAlgorithm( EN_ALGO );

	private:
		std::array<std::array<Cell,9>,9> _data;

		Viewtable  BuildViewtable() const;

		View_1Dim_c   GetCol(index_t) const;
		View_1Dim_nc  GetCol(index_t);

		View_1Dim_c   GetRow(index_t) const;
		View_1Dim_nc  GetRow(index_t);

		View_1Dim_c   GetBlock( index_t ) const;
		View_1Dim_nc  GetBlock( index_t );
};
//----------------------------------------------------------------------------

inline
const Cell&
Grid::getCell( index_t idx ) const
{
	assert( idx<81 );
	auto row = idx/9;
	auto col = idx%9;
	return _data[row][col];
}
//----------------------------------------------------------------------------

inline
std::ostream& operator << ( std::ostream& s, View_1Dim_nc& v )
{
	for( int i=0; i<9; i++ )
		s << v.GetCell(i).GetValue() << " ";
	return s;
}

inline
Cell&
Grid::GetCellByPos( pos_t p )
{
	ASSERT_1( p.first  < 9, p.first );
	ASSERT_1( p.second < 9, p.second );
	return _data[p.first].at( p.second );
}

inline
const Cell&
Grid::GetCellByPos( pos_t p ) const
{
	ASSERT_1( p.first  < 9, p.first );
	ASSERT_1( p.second < 9, p.second );
	return _data[p.first].at( p.second );
}

inline
View_1Dim_c
Grid::GetCol( index_t c ) const
{
	assert( c<9 );
	View_1Dim_c col;
	for( index_t i=0; i<9; i++ )
		col._viewData[i] = &_data[i][c];
	return col;
}

inline
View_1Dim_nc
Grid::GetCol( index_t c )
{
	assert( c<9 );
	View_1Dim_nc col;
	for( index_t i=0; i<9; i++ )
		col._viewData[i] = &_data[i][c];
	return col;
}

/// return const mono-dimensional view of row \c r \in [0,8]
inline
View_1Dim_c
Grid::GetRow( index_t r ) const
{
	assert( r<9 );
	View_1Dim_c row;
	for( index_t i=0; i<9; i++ )
		row._viewData[i] = &_data[r][i];
	return row;
}

/// return non-const mono-dimensional view of row \c r \in [0,8]
inline
View_1Dim_nc
Grid::GetRow( index_t r )
{
	assert( r<9 );
	View_1Dim_nc row;
	for( index_t i=0; i<9; i++ )
		row._viewData[i] = &_data[r][i];
	return row;
}

/// return const mono-dimensional view of block \c idx \in [0,8]
inline
View_1Dim_c
Grid::GetBlock( index_t idx ) const
{
	View_1Dim_c g;
	assert( idx<9 );

	for( index_t i=0; i<3; i++ )
		for( index_t j=0; j<3; j++ )
		{
			g._viewData[i*3+j] = &_data[idx/3*3+i][idx%3*3+j];
//			std::cout << "idx=" << idx << " r=" << idx/3*3+i << " c=" << idx%3 * 3+j<< '\n';
		}
	return g;
}

/// return mono-dimensional view of block \c idx \in [0,8]
inline
View_1Dim_nc
Grid::GetBlock( index_t idx )
{
	View_1Dim_nc g;
	assert( idx<9 );

	for( index_t i=0; i<3; i++ )
		for( index_t j=0; j<3; j++ )
		{
			g._viewData[i*3+j] = &_data[idx/3*3+i][idx%3*3+j];
//			std::cout << "idx=" << idx << " r=" << idx/3*3+i << " c=" << idx%3 *3+j << '\n';
		}

	return g;
}

std::set<Pos> getCellsPos( EN_ORIENTATION, index_t );

//----------------------------------------------------------------------------

#endif // GRID_H
