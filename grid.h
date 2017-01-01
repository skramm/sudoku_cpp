#ifndef GRID_H
#define GRID_H

#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <type_traits>

typedef unsigned char uchar;
typedef uchar value_t;
typedef uchar index_t;
typedef std::pair<index_t,index_t> pos_t;
typedef std::map<value_t,bool> cand_map_t;

struct Cell;
void LogStep( const Cell& cell, std::string msg );

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
inline
void
InitCandMap( cand_map_t& cmap )
{
	for( value_t i=1; i<10; i++ )
		cmap[i]=true;
}

//----------------------------------------------------------------------------
/// Holds a cell, has either a value, either a set of candidates (in which case the value is 0)
struct Cell
{
	friend std::ostream& operator << ( std::ostream&, const pos_t& );

private:
	value_t    _value = 0; ///< value
	public:
	cand_map_t _cand;      ///< candidate map
	pos_t      _pos;       ///< position in grid [0-8]

	public:
	Cell()
	{
		InitCandMap( _cand );
	}
	value_t GetValue() const { return _value; }
	void SetValue( value_t v ) { _value = v; }

	void SetPos( index_t i, index_t j )
	{
		_pos.first  = i;
		_pos.second = j;
	}

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
		for( int i=1; i<10; i++ )
			_cand[i] = false;
	}
	bool RemoveCandidates( std::vector<value_t> v_cand )
	{
		bool b = false;
		for( auto v: v_cand )
		{
			bool b1 = RemoveCandidate( v );
			if( b1 )
				b = true;
		}
		return b;
	}
	bool RemoveCandidate( value_t val )
	{
		if( _cand[val] )
		{
			LogStep( *this, "remove candidate " + std::to_string(val) );
			_cand[val] = false;
			if( NbCandidates() == 1 )
			{
				_value = GetValueFromCandidate();
				_cand[_value] = false;
				LogStep( *this, "assign value " + std::to_string(_value) );
			}
			return true;
		}
		return false;
	}

	void RemoveAllCandidatesBut( value_t val )
	{
		for( int i=1; i<10; i++ )
			if( i != val )
				RemoveCandidate( i );
	}
	std::vector<value_t> GetCandidates() const
	{
		std::vector<value_t> v;
		for( int i=1; i<10; i++ )
			if( _cand.at(i) )
				v.push_back( i );
		return v;
	}
	uchar NbCandidates() const
	{
		int n = 0;
		for( int i=1; i<10; i++ )
			if( _cand.at(i) )
				n++;
		return n;
	}
	bool HasCandidate( value_t currentValue ) const
	{
		assert( currentValue>0 && currentValue<10 );
		return _cand.at(currentValue);
	}

	int GetValueFromCandidate()
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
};
//----------------------------------------------------------------------------
inline
char
GetRowLetter( index_t i )
{
	uchar c = i+'A';
	if( i==8 )   // so we replace 'I' by 'J' for readability
		c++;
	return c;
}
//----------------------------------------------------------------------------
inline
std::ostream& operator << ( std::ostream& s, const pos_t& p )
{
	s << GetRowLetter(p.first) << p.second+1;
	return s;
}

//----------------------------------------------------------------------------
inline
index_t
GetBlockIndex( index_t row, index_t col )
{
	return row/3*3 + col/3;
}
inline
index_t
GetBlockIndex( pos_t p )
{
	return GetBlockIndex( p.first, p.second );
}
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
};

typedef View_T<const Cell*> View_1Dim_c;
typedef View_T<Cell*>       View_1Dim_nc;

typedef std::array<std::array<char,27>,27> Viewtable;

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

enum EN_ORIENTATION { OR_COL=0, OR_ROW, OR_BLK };

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

enum EN_ALGO
{
	ALG_REMOVE_CAND = 0,
	ALG_SEARCH_PAIRS,
	ALG_SEARCH_TRIPLES,
	ALG_SEARCH_SINGLE_CAND,
	ALG_SEARCH_MISSING_SINGLE,
	ALG_XY_WING,

	ALG_END
};

inline
const char*
GetString( EN_ALGO orient )
{
	switch( orient )
	{
		case ALG_REMOVE_CAND: return "RemoveCand"; break;
		case ALG_SEARCH_PAIRS: return "SearchPairs"; break;
		case ALG_SEARCH_TRIPLES: return "SearchTriples"; break;
		case ALG_SEARCH_SINGLE_CAND: return "SearchSingleCand"; break;
		case ALG_SEARCH_MISSING_SINGLE: return "MissingSingle"; break;
		case ALG_XY_WING: return "XY_Wing"; break;
		default: assert(0);
	}
}

//----------------------------------------------------------------------------
template<typename T>
void
PrintVector( const std::vector<T>& v, std::string s )
{
	std::cout <<  s << ": ";
	for( auto p: v )
		std::cout << p << ' ';
	std::cout << '\n';
}
//----------------------------------------------------------------------------
/// Add element \c elem to vector \c v only if it is not already present
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
/// Adds to vector \c v the elements of v2 only if they are not already present
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
		bool Load( std::string fn=std::string() );
		bool Check() const;
		bool Solve();
		void InitCandidates();
//		void SetVerbose(bool b ) { _verbose = b; }
		void PrintCandidates( std::ostream&, std::string=std::string() ) const;
		void PrintAll( std::ostream&, std::string );

		Cell&       GetCellByPos( pos_t );
		const Cell& GetCellByPos( pos_t ) const;

	private:
		bool Check( EN_ORIENTATION ) const;

		bool RemoveCandidates( EN_ORIENTATION );
		bool RemoveCandidates();

		bool SearchPairs();
		bool SearchTriples();
//		bool SearchPairs( EN_ORIENTATION );
		bool SearchPairsTriple( EN_ORIENTATION, uint );

		bool SearchSingleCand();
		bool SearchSingleCand( EN_ORIENTATION );

		bool SeachForSingleMissing();
		bool SeachForSingleMissing( EN_ORIENTATION );

		bool XY_Wing();

		View_1Dim_c  GetView( EN_ORIENTATION, index_t ) const;
		View_1Dim_nc GetView( EN_ORIENTATION, index_t );

		int  NbUnknows() const;
		int  NbUnknows2() const;
		bool ProcessAlgorithm( EN_ALGO );

		std::vector<pos_t> GetOtherCells( const Cell& c, int nb, EN_ORIENTATION );

		void FilterByCand( const std::vector<value_t>& v_cand, std::vector<pos_t>& v_pos ) const;

//	public:
//		bool _verbose = false;

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

/// return mono-dimensional view of row \c r \in [0,8]
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
//----------------------------------------------------------------------------

#endif // GRID_H
