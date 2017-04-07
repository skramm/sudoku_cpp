/**
This file is part of https://github.com/skramm/sudoku_cpp
Licence: GPLv3
*/

#define PRINT_ALGO_START \
	{ \
		if( g_data.Verbose ) \
			std::cout << "START: " << __FUNCTION__ << '\n'; \
	}

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

// some optionnal symbols ('til I write a makefile...)
//#define DEBUGMODE

#ifdef DEBUGMODE
	#define DEBUG if(1) std::cout
#else
	#define DEBUG if(0) std::cout
#endif


#include "grid.h"
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
/// Check in a 1d view if there is only one value missing. If so, then we can assign the missing value to that cell
bool
Grid::SeachForSingleMissing( EN_ORIENTATION orient )
{
	PRINT_ALGO_START;
	if( g_data.Verbose )
		std::cout << "view: " << GetString(orient) << '\n'; \

	for( index_t i=0; i<9; i++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_nc v1d = GetView( orient, i );

		std::vector<index_t>   v_zero;

		std::map<index_t,bool> m_zero; // a map with all
		for( index_t j=0; j<9; j++ )   // values from 1 to 9 initialized to 'true'
			m_zero[j+1] = true;

		for( index_t j=0; j<9; j++ ) // for each cell in the view
		{
			Cell& cell = v1d.GetCell(j);

			if( cell.GetValue() == 0 )
				v_zero.push_back( j ); // store position of cell
			else
				m_zero[cell.GetValue()] = false;
		}
		if( v_zero.size() == 1 ) // only one cell unknown
		{
			auto it = std::find_if(
				m_zero.begin(),
				m_zero.end(),
				[&]( const std::pair<index_t,bool>& p ){ return p.second==true;  } // lambda
			);
			assert( it != m_zero.end() );
			Cell& cell = v1d.GetCell( v_zero[0] );
			cell.RemoveAllCandidates();
			cell.SetValue( it->first );
			return true;
		}
	}
	return false;
}
//----------------------------------------------------------------------------
/// Remove candidates on rows/cols/blocks that have a value in another cell
bool
Grid::RemoveCandidates( EN_ORIENTATION orient )
{
	PRINT_ALGO_START;
	if( g_data.Verbose )
		std::cout << "view: " << GetString(orient) << '\n'; \

	bool res = false;
	for( index_t i=0; i<9; i++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_nc v1d = GetView( orient, i );

		for( index_t j=0; j<9; j++ ) // for each cell in the view
		{
			Cell& cell = v1d.GetCell(j);
			auto currentValue = cell.GetValue();
			if( currentValue != 0 )
			{
				for( index_t k=0; k<9; k++ ) // for each other cell in the view
					if( v1d.GetCell(k).HasCandidate( currentValue ) )
						res = v1d.GetCell(k).RemoveCandidate( currentValue );
			}
		}
	}
	return res;
}
//----------------------------------------------------------------------------
bool
Grid::SearchPairsTriple( EN_ORIENTATION orient, uint n )
{
	PRINT_ALGO_START;
	if( g_data.Verbose )
		std::cout << "view: " << GetString(orient) << " n=" << n << '\n'; \

	assert( n==2 || n==3 );

	bool res = false;

	for( index_t i=0; i<9; i++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX(orient);
		std::vector<index_t> v_pos(1);
		View_1Dim_nc v1d = GetView( orient, i );
		std::vector<value_t> v_cand_1;

		for( index_t j=0; j<8 && (v_pos.size()!=n); j++ ) // for each cell in the view (and stop if found 'n' matches)
		{
			Cell& cell_1 = v1d.GetCell(j);
			if( cell_1.NbCandidates() == n )
			{
				v_pos[0] = j;
				v_cand_1 = cell_1.GetCandidates();

				for( index_t k=j+1; k<9 && (v_pos.size()!=n); k++ ) // for each other cell in the view
				{
					Cell& cell_2 = v1d.GetCell(k);
					if( cell_2.NbCandidates() == n )
					{
						if( v_cand_1 == cell_2.GetCandidates() ) // if the candidates are the same, then we can remove these from the others cells of the view
						{
//							if( g_data.Verbose )
//								std::cout << "  -found match of pos " << (int)j+1 << " at pos " << (int)k+1 << '\n';
							v_pos.push_back( k );
						}
					}
				}
			}
		}
		assert( v_pos.size() <= n );
		if( v_pos.size() == n )
		{
			uchar Nb(0);
			for( index_t j=0; j<9; j++ ) // for each cell in the view
			{
				Cell& cell = v1d.GetCell(j);
				bool dontremove( false );
				for( uchar p=0; p<n; p++ )
					if( j == v_pos[p] )
						dontremove = true;
				if( !dontremove )
					if( cell.RemoveCandidates( v_cand_1 ) )
					{
						res = true;
						Nb++;
					}
			}
			if( g_data.Verbose && Nb )
				std::cout << " - found a " << (n==2 ? "pair" : "triple") << ", removed " << (int)Nb << " candidates from others cells in same view\n";
		}
	}
	return res;
}
//----------------------------------------------------------------------------
void
AddToCandidateCount( std::map<value_t,index_t>& m_count, const Cell& cell )
{
	auto v_cand = cell.GetCandidates();
	for( auto c: v_cand )
		m_count[c]++;
}

//----------------------------------------------------------------------------
/// Counts the number of times each candidate is present in a given row/block/col
std::map<value_t,index_t>
CoundCandidates( Grid& g, EN_ORIENTATION orient, index_t idx )
{
	View_1Dim_nc v1d = g.GetView( orient, idx );

	std::map<value_t,index_t> cand_count; // counts the number of times each candidate is present on the view
	for( value_t i=0; i<9; i++ )
		cand_count[i+1]=0;

	for( index_t j=0; j<9; j++ ) // for each cell in the view, count the candidates
		AddToCandidateCount( cand_count, v1d.GetCell(j) );

	return cand_count;
}
//----------------------------------------------------------------------------
/// Pointing pairs/triples. See http://www.sudokuwiki.org/Intersection_Removal
bool
PointingPairsTriples( Grid& g, EN_ORIENTATION orient )
{
	PRINT_ALGO_START;
	if( g_data.Verbose )
		std::cout << "view: " << GetString(orient) << '\n'; \

	assert( orient == OR_ROW || orient == OR_COL );

	bool ret_val(false);
	for( index_t i=0; i<9; i++ )  // for each row/col
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_nc v1d = g.GetView( orient, i );

		for( index_t b=0; b<3; b++ ) // for each of the 3 blocks dividing the view
		{
			DEBUG << " block=" << (int)b+1 << '\n';
			const Cell& first = v1d.GetCell(b*3); // get first cell of block
			auto bl_idx = GetBlockIndex( first._pos );
			auto cc1 = CoundCandidates( g, OR_BLK, bl_idx ); // number of candidates in that block
			std::map<value_t,index_t> cc2;
			for( index_t c=0; c<3; c++ )  // count candidates in the 3 cells of that block limit
				AddToCandidateCount( cc2, v1d.GetCell( b*3+c ) );

			for( value_t v=1; v<10; v++ )          // for each candidate value,
				if( cc2[v] > 1 && cc1[v] == cc2[v] )  // if we have 2 or 3 identical candidates, AND no other in the others cells of the same block
				{
					if( g_data.Verbose )
						std::cout << " - value: " << (int)v << " : found " << (cc2[v]==2 ? '2' : '3') << " alone in block " << (int)b+1 << '\n';
					for( index_t j=0; j<9; j++ )  // for each cell of the view
					{
						Cell& cell = v1d.GetCell(j);
						if( j/3 != b )
							if( cell.RemoveCandidate( v ) )
								ret_val = true;
					}
				}
		}
	}
	return ret_val;
}
//----------------------------------------------------------------------------
/// Search for cells in a row/col/block where a candidate appears in only one cell. If so, then it has to be the value in that cell
bool
Grid::SearchSingleCand( EN_ORIENTATION orient )
{
	PRINT_ALGO_START;
	if( g_data.Verbose )
		std::cout << "view: " << GetString(orient) << '\n'; \

	bool res = false;
	bool stop = false;
	for( index_t i=0; i<9 && stop==false; i++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_nc v1d = GetView( orient, i );

		auto cand_count = CoundCandidates( *this, orient, i );

		for( index_t val=1; val<10; val++ )  // analyse results
		{
			auto c = cand_count[val];
			if( c == 1 )
			{
				if( g_data.Verbose )
					std::cout << "found single: " << (int)val << '\n';
                for( index_t j=0; j<9; j++ ) // for each cell in the view
				{
					Cell& cell = v1d.GetCell(j);
					auto v_cand = cell.GetCandidates();
					auto it = std::find( v_cand.begin(), v_cand.end(), val );
					if( it != v_cand.end() ) // then, we found it
					{
						cell.RemoveAllCandidatesBut( val );
						res = true;
					}
				}
				stop = true;
			}
		}
	}
	return res;
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
/// Filter vector \c v_pos so that it holds only positions of cells holding only one of the candidates that are in \c v_cand.
/**
T is a container holding elements of type \c pos_t (example: \c std::vector<pos_t> )
*/
template<typename T>
void
FilterByCand( Grid& g, const std::vector<value_t>& v_cand, T& v_pos )
{
	T v_out;
//	std::vector<pos_t> v_out;
	assert( v_cand.size() == 2 );
	for( auto& pos : v_pos )
	{
		const Cell& c = g.GetCellByPos( pos );
		if(
			( c.HasCandidate( v_cand[0] ) && !c.HasCandidate( v_cand[1] ) )
			||
			( c.HasCandidate( v_cand[1] ) && !c.HasCandidate( v_cand[0] ) )
		)
			v_out.push_back( pos );
	}
	std::swap( v_out, v_pos );
}
//----------------------------------------------------------------------------
/// Helper function for FindSymmetricalMatches()
/** Finds indexes of correspondences betwen 2 vectors holding 2 elements.
Returned value holds \c first: index on first vector, \c second: index on second vector
*/
std::pair<index_t,index_t>
FindMatch( const std::vector<index_t>& v_key, const std::vector<index_t>& v_cand )
{
	assert( v_key.size()  == 2 );
	assert( v_cand.size() == 2 );

	std::pair<index_t,index_t> p_out(0,0)	;
	if( v_key[0] == v_cand[0] )
		return std::pair<index_t,index_t>(0,0);
	if( v_key[0] == v_cand[1] )
		return std::pair<index_t,index_t>(0,1);
	if( v_key[1] == v_cand[0] )
		return std::pair<index_t,index_t>(1,0);
	if( v_key[1] == v_cand[1] )
		return std::pair<index_t,index_t>(1,1);

	assert(0);
	return std::pair<index_t,index_t>(0,0); // just there to avoid a compilation warning
}
//----------------------------------------------------------------------------
/// Holds results of FindSymmetricalMatches()
struct symMatches
{
	pos_t pA;
	pos_t pB;
	value_t value;
	symMatches( pos_t _pA, pos_t _pB, value_t _v ) : pA(_pA),pB(_pB),value(_v) {}
	friend std::ostream& operator << ( std::ostream& s, const symMatches& sm )
	{
		s << "(cells:" << sm.pA << ',' << sm.pB << " val=" << (int)sm.value << ") ";
		return s;
	}

};
//----------------------------------------------------------------------------
/// Searches in the two vectors for a symmetric match (related to XY_Wing)
/**
see http://www.sudokuwiki.org/Y_Wing_Strategy

Say the two candidates are 1,4
and the set of cells holds these:
\beginverbatim
0: 4,6
2: 1,5
3: 1,2
4: 2,4
\endverbatim
then we see that we have a match with elems 3 and 4, and the common candidates (that will get removed in further steps) is 2
*/
std::vector<symMatches>
FindSymmetricalMatches(
	const Grid& grid,
	const std::vector<value_t>& v_key_cand,  ///< input: vector of 2 values (candidates of key cell)
	const std::vector<pos_t>&   v_cells      ///< input: vector of cell positions that are in the range and hold 2 candidates
)
{
	assert( v_key_cand.size() == 2 );

	std::vector<symMatches> v_out;
//	PrintVector( v_cells, "v_cells" ); std::cout << "Key cand: " << v_key_cand[0] << '-' << v_key_cand[1] << '\n';

	for( size_t i=0; i<v_cells.size()-1; i++ )
	{
		const auto& pA = v_cells[i];
		const Cell& cA = grid.GetCellByPos( pA );
		const auto& v_cand_A = cA.GetCandidates();
		auto match_idx_A = FindMatch( v_key_cand, v_cand_A );
		for( size_t j=i+1; j<v_cells.size(); j++ )
		{
			const auto& pB = v_cells[j];
//			std::cout <<"i="<<i << " j=" << j << " pos: pA=" << pA << " pB=" << pB << '\n';
			assert( pA != pB );
			const Cell& cB = grid.GetCellByPos( pB );
			const auto& v_cand_B = cB.GetCandidates();
			if( v_cand_A != v_cand_B )
			{
//				PrintVector( v_cand_A, "v_cand_A" ); PrintVector( v_cand_B, "v_cand_B" );

				auto match_idx_B = FindMatch( v_key_cand, v_cand_B );

//				std::cout << "match A: " << match_idx_A.first <<  '-' << match_idx_A.second << '\n';
//				std::cout << "match B: " << match_idx_B.first <<  '-' << match_idx_A.second << '\n';

				if( v_cand_A[!match_idx_A.second] == v_cand_B[!match_idx_B.second] )
				{
					std::pair<pos_t,pos_t> p_out;
					p_out.first  = pA;
					p_out.second = pB;
					value_t common_value = v_cand_A[!match_idx_A.second];
//					std::cout << "FOUND: pA=" << pA << " pB=" << pB <<  " common value=" << common_value << '\n';
					v_out.push_back( symMatches( pA,pB,common_value) );
				}
			}
		}
	}
//	std::cout << "symMatches size=" << v_out.size() << '\n';
	return v_out;
}
//----------------------------------------------------------------------------
/// Find common region covered by \c p1 and \c p2
/**
\todo probably a lot of optimizations over here...
*/
std::vector<pos_t>
FindCommonRegion( pos_t p1, pos_t p2 )
{
//	std::cout << "START: " << __FUNCTION__ << '\n';
//	std::cout << "p1=" << p1 << " p2=" << p2 << '\n';
	assert( p1 != p2 );

//	assert( p1.first != p2.first || p1.second != p2.second ); // must be on different rows OR different cols

	std::vector<pos_t> v_out;
	if( p1.first != p2.first && p1.second != p2.second ) // if they are NOT on same row or same col
	{
//		std::cout << "adding intersections\n";
		v_out.push_back( pos_t(p1.first,p2.second) ); // add the 2 intersection cells
		v_out.push_back( pos_t(p2.first,p1.second) );
	}

	if( p1.first/3 == p2.first/3 )          // if blocks are on the same row (0,1, or 2)
    {
//		std::cout <<" - same block row !\n";
		index_t col1 = p1.second/3*3;
		index_t col2 = p2.second/3*3;
		for( index_t i=0; i<3; i++ )
		{
			pos_t p_r1(p1.first, col2+i);
			if( p_r1 != p1 && p_r1 != p2 )
				AddToVector( v_out, p_r1 );

			pos_t p_r2(p2.first, col1+i);
			if( p_r2 != p1 && p_r2 != p2 )
				AddToVector( v_out, p_r2 );
		}
    }
    if( p1.second/3 == p2.second/3 )          // if blocks are on the same col (0,1, or 2)
    {
//		std::cout <<" - same block col !\n";
		index_t row1 = p1.first/3*3;
		index_t row2 = p2.first/3*3;
		for( index_t i=0; i<3; i++ )
		{
			pos_t p_c1(row2+i, p1.second);
			if( p_c1 != p1 && p_c1 != p2 )
				AddToVector( v_out, p_c1 );

			pos_t p_c2(row1+i, p2.second);
			if( p_c2 != p1 && p_c2 != p2 )
				AddToVector( v_out, p_c2 );
		}
    }
    if( g_data.Verbose )
		PrintVector( v_out, "Common region" );
	return v_out;
}
//----------------------------------------------------------------------------
/// Remove from all the cells in \c v_region the candidate \c cand, EXCEPT in (source) cell \c key
bool
RemoveCandidatesFromRegion( Grid& grid, const std::vector<pos_t>& v_region, value_t cand, const Cell& key )
{
	bool retval(false);
	for( auto& pos: v_region )
		if( pos != key._pos )
			if( grid.GetCellByPos(pos).RemoveCandidate( cand ) )
				retval = true;
	return retval;
}
//----------------------------------------------------------------------------
/// XY Wing algorithm
bool
XY_Wing( Grid& g )
{
	PRINT_ALGO_START;

	bool retval(false);
	for( index_t i=0; i<9; i++ )  // for each row
	{
		PRINT_MAIN_IDX(OR_ROW);
		View_1Dim_nc v1d = g.GetView( OR_ROW, i );
		for( index_t col=0; col<9; col++ )   // for each col
		{
			Cell& key = v1d.GetCell(col);
			auto v_cand = key.GetCandidates();
			if( v_cand.size() == 2 )
			{
				DEBUG << " col=" << (int)col+1 <<  " cell has " << v_cand.size() << " candidates\n";
				auto v_cells   = g.GetOtherCells_nbc( key, 2, OR_ROW );  // get cells on same row that have 2 candidates
				auto v_cells_b = g.GetOtherCells_nbc( key, 2, OR_COL );  // get cells on same col that have 2 candidates
				auto v_cells_c = g.GetOtherCells_nbc( key, 2, OR_BLK );  // get cells in same block that have 2 candidates
				AddToVector( v_cells, v_cells_b );
				AddToVector( v_cells, v_cells_c );

				if( v_cells.size() > 1 )
				{
					FilterByCand( g, v_cand, v_cells );                 // remove the ones that don't use any of the 2 candidates
					if( v_cells.size() > 1 )
					{
						std::vector<symMatches> v_matches = FindSymmetricalMatches( g, v_cand, v_cells );
						if( g_data.Verbose && v_matches.size() > 0 )
						{
							std::cout << "key cell: " << key._pos << '\n';
							PrintVector( v_matches, "Symmetric matches" );
						}
						for( const auto& p_match: v_matches )
						{
							auto v_region = FindCommonRegion( p_match.pA, p_match.pB );
							if( RemoveCandidatesFromRegion( g, v_region, p_match.value, key ) )
								retval = true;
							else
							{
								if(g_data.Verbose )
									std::cout << "No removals\n";
							}
						}
					}
				}
			}
		}
	}
	return retval;
}
//----------------------------------------------------------------------------
/// A strong link is when on same line/col/block, a candidate is only in two cells
struct StrongLink
{
	EN_ORIENTATION orient;
	pos_t p1, p2;
	StrongLink( EN_ORIENTATION o, pos_t pA, pos_t pB ) : orient(o), p1(pA), p2(pB)
	{}
	friend bool operator == ( const StrongLink& sl1, const StrongLink& sl2 )
	{
		if( sl1.p1 == sl2.p1 && sl1.p2 == sl2.p2 )
			return true;
		if( sl1.p1 == sl2.p2 && sl1.p2 == sl2.p1 )
			return true;
		return false;
	}
	friend std::ostream& operator << ( std::ostream& s, const StrongLink& sl )
	{
		s << '{' << sl.p1 << "-" << sl.p2 << '}';
		return s;
	}
};
//----------------------------------------------------------------------------
/// A strong link with value
struct StrongLink_V : public StrongLink
{
	value_t linkValue;

	StrongLink_V( value_t v, EN_ORIENTATION o, pos_t pA, pos_t pB ) : StrongLink(o, pA, pB), linkValue(v)
	{
	}
	friend bool operator == ( const StrongLink_V& sl1, const StrongLink_V& sl2 )
	{
		if( sl1.linkValue != sl2.linkValue )
			return false;
		return (StrongLink)sl1 == (StrongLink)sl2;
	}
	friend std::ostream& operator << ( std::ostream& s, const StrongLink_V& sl )
	{
		s << '{' << (int)sl.linkValue << ": " << sl.p1 << "-" << sl.p2 << '}';
		return s;
	}
};
//----------------------------------------------------------------------------
/// Holds for each value the set of associated strong links
struct MappedStrongLinks
{
	private:
		std::map<value_t,std::vector<StrongLink>> _map;
	public:

		MappedStrongLinks()
		{
			for( value_t i=1; i<10; i++ )
				_map[i] = std::vector<StrongLink>();
		}
		void AddLink( value_t val, StrongLink sl ) // value_t val, pos_t p1, pos_t p2 )
		{
			_map[val].push_back( sl );
		}

		size_t size() const
		{
			return _map.size();
		}

		std::vector<StrongLink> GetSLvect(value_t v)
		{
			return _map[v];
		}

	friend std::ostream& operator << ( std::ostream& s, const MappedStrongLinks& sl )
	{
		s <<  "MappedStrongLinks:\n";
		for( value_t i=1; i<10; i++ )
		{
#if 1
			s << (int)i << ": " << sl._map.at(i).size() << " links: ";
#else
			int nb = 0;
			if( sl._map.find(i) )
				nb = sl._map.at(i).size()
			s << (int)i << ": " << nb << " links: ";
			if( nb )
#endif
				for( const auto& v: sl._map.at(i) )
				{
					s << v << ","; //v.first << '-' << v.second << ", ";
				}
			s << '\n';
		}
		return s;
	}
};
//----------------------------------------------------------------------------
MappedStrongLinks
Convert2MappedSL( const std::vector<StrongLink_V>& vin )
{
	MappedStrongLinks msl;
	for( const auto& elem: vin )
		msl.AddLink( elem.linkValue, elem ); //.linkValue, elem.p1, elem.p2 );
	return msl;
}
//----------------------------------------------------------------------------
void
GetStrongLinks( EN_ORIENTATION orient, std::vector<StrongLink_V>& v_out, const Grid& g )
{
	for( index_t i=0; i<9; i++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_c v1d = g.GetView( orient, i );
		CandMap candMap;
		for( index_t col1=0; col1<8; col1++ )   // for each cell in the view
		{
			const Cell& cell1 = v1d.GetCell(col1);
			auto v_cand = cell1.GetCandidates();
			assert( v_cand.empty() || v_cand.size() > 1 );
			if( v_cand.size() > 1 )
			{
				for( auto cand: v_cand )
				{
					if( candMap.Has( cand ) )
					{
						uchar c = 0;
						index_t pos = 0;
						for( index_t col2=col1+1; col2<9; col2++ )   // for each OTHER cell in the view
						{
							const Cell& cell2 = v1d.GetCell(col2);
							if( cell2.HasCandidate( cand ) )
							{
								c++;
								pos = col2;
							}
						}
						if( c == 1 )                    // means we have ONLY 1 other cell with that candidate
							v_out.push_back( StrongLink_V( cand, orient, cell1.GetPos(), v1d.GetCell(pos).GetPos() ) );

						if( c > 1 )                    // if we find more than 1 other, then
							candMap.Remove( cand );    // don't consider this candidate any more
					}
				}
			}
		}
	}
}
//----------------------------------------------------------------------------
/// Finds all the strong links
/**
see:
- http://www.sudokuwiki.org/X_Cycles
- http://www.sudokuwiki.org/X_Cycles_Part_2
*/
MappedStrongLinks
FindStrongLinks( const Grid& g )
{
	std::vector<StrongLink_V> v_out;

	GetStrongLinks( OR_ROW, v_out, g );
	PrintVector( v_out, "ROW: Strong Links" );

	GetStrongLinks( OR_COL, v_out, g );
	PrintVector( v_out, "COL: Strong Links" );

	GetStrongLinks( OR_BLK, v_out, g );
	PrintVector( v_out, "BLK: Strong Links" );

	auto v_out2 = VectorRemoveDupes( v_out );
	PrintVector( v_out2, "Removed dupes" );

	return Convert2MappedSL( v_out2 );
}
//----------------------------------------------------------------------------
/// ? Needs to be defined
struct Cycle
{
};
//----------------------------------------------------------------------------
/// Helper function for X_Cycles()
/**
Iterates through the strong links for the given value and sees if a cycle to the initial position can be found
*/
Cycle
FindCycle( value_t v, const std::vector<StrongLink>& sl_vect )
{
	assert( sl_vect.size() > 1 );
	pos_t initial_pos = sl_vect[0].p1;
	pos_t current = sl_vect[0].p2;
	do
	{
//		if( FoundStrongLink( idx, sl_vect ) )

	}
	while(0);

}

//----------------------------------------------------------------------------
/// X Cycles algorithm (WIP)
/**
This enables removing some candidates

See http://www.sudokuwiki.org/X_Cycles

*/
bool
X_Cycles( Grid& g )
{
// 1 - first, get strong links
	auto msl = FindStrongLinks( g );
	std::cout << "msl:\n" << msl;

// 2 - then, check if they can form a cycle
	for( value_t v=1; v<10; v++ )
	{
		const auto& sl_vect = msl.GetSLvect(v);
		std::cout << "considering value " << v << ", vector has " << sl_vect.size() << " values\n";
		if( sl_vect.size() > 1 )
			Cycle cyc = FindCycle( v, sl_vect );
			for( const auto& sl: sl_vect )
			{

			}

	}

	return false;
}
//----------------------------------------------------------------------------
/// Pointing pairs/triples. See http://www.sudokuwiki.org/Intersection_Removal
bool
PointingPairsTriples( Grid& g )
{
	if( !PointingPairsTriples( g, OR_ROW ) )
		if( !PointingPairsTriples( g, OR_COL ) )
			return false;
	return true;
}
//----------------------------------------------------------------------------
bool
Grid::SearchPairs()
{
	if( !SearchPairsTriple( OR_ROW, 2 ) )
		if( !SearchPairsTriple( OR_COL, 2 ) )
			if( !SearchPairsTriple( OR_BLK, 2 ) )
				return false;
	return true;
}
//----------------------------------------------------------------------------
bool
Grid::SearchTriples()
{
	if( !SearchPairsTriple( OR_ROW, 3 ) )
		if( !SearchPairsTriple( OR_COL, 3 ) )
			if( !SearchPairsTriple( OR_BLK, 3 ) )
				return false;
	return true;
}
//----------------------------------------------------------------------------
/// Search for cells in a row/col/block where a candidates appears in only one cell
bool
Grid::SearchSingleCand()
{
	if( !SearchSingleCand( OR_ROW ) )
		if( !SearchSingleCand( OR_COL ) )
			if( !SearchSingleCand( OR_BLK ) )
				return false;
	return true;
}
//----------------------------------------------------------------------------
/// remove candidates on rows/cols/blocks that have a value in another cell
bool
Grid::RemoveCandidates()
{
	if( !RemoveCandidates( OR_ROW ) )
		if( !RemoveCandidates( OR_COL ) )
			if( !RemoveCandidates( OR_BLK ) )
				return false;
	return true;
}
//----------------------------------------------------------------------------
/// check in a view if there is only one value missing. If so, then we can assign a value to it
bool
Grid::SeachForSingleMissing()
{
	if( !SeachForSingleMissing( OR_ROW ) )
		if( !SeachForSingleMissing( OR_COL ) )
			if( !SeachForSingleMissing( OR_BLK ) )
				return false;
	return true;
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
		case ALG_REMOVE_CAND:    res=RemoveCandidates(); break;
		case ALG_SEARCH_PAIRS:   res=SearchPairs();      break;
		case ALG_SEARCH_TRIPLES: res=SearchTriples();      break;
		case ALG_SEARCH_SINGLE_CAND: res=SearchSingleCand(); break;
		case ALG_SEARCH_MISSING_SINGLE: res=SeachForSingleMissing(); break;
		case ALG_POINTING_PT:    res=PointingPairsTriples( *this ); break;
		case ALG_XY_WING:        res = XY_Wing( *this ); break;
		case ALG_X_CYCLES:       res = X_Cycles( *this ); break;
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

