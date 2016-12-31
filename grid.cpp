#include "grid.h"
#include <fstream>
//#include <utility>

#define PRINT_ALGO_START_0 \
	{ \
		if( _verbose ) \
			std::cout << "START: " << __FUNCTION__ << '\n'; \
	}

#define PRINT_ALGO_START \
	{ \
		if( _verbose ) \
			std::cout << "START: " << __FUNCTION__  << " with view: " << GetString(orient) << '\n'; \
	}
#define PRINT_MAIN_IDX \
	{ \
		if( _verbose ) \
			std::cout << " -main idx=" << i+1 << '\n'; \
	}

//----------------------------------------------------------------------------
void LogStep( const Cell& cell, std::string msg )
{
	static int step;
	std::cout << "*** step " << ++step << ": CELL " << cell._pos << ": " << msg << '\n';
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
	s << "nb unknowns (cand) " << g.NbUnknows() << "\n";
	s << "nb unknowns (values) " << g.NbUnknows2() << "\n";
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
	s << " -nb unknowns (values) " << NbUnknows2() << "\n";

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
/// Fills with a simple sample grid
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
//	if( _verbose )
//		std::cout << "Checking, orientation:" << GetString(orient) << '\n';

	for( index_t i=0; i<9; i++ )  // for each row/col/block
	{
		View_1Dim_c v1d = GetView( orient, i );

		for( index_t j=0; j<9; j++ ) // for each cell in the view
		{
			const Cell& cell_1 = v1d.GetCell(j);
			auto val_1 = cell_1.GetValue();
//			std::cout << "   j=" << j << " val=" << val_1 << "\n";
			if( val_1 != 0 )
			{
				for( index_t k=0; k<9; k++ ) // for each cell in the view
				{
					const Cell& cell_2 = v1d.GetCell(k);
					auto val_2 = cell_2.GetValue();
					if( k != j && val_1 == val_2 )
					{
						std::cout << "Error, cell at " << cell_1._pos << " and at " << cell_2._pos << " have same value: " << val_1 << '\n';
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
		std::cout << "Error, row invalid\n";
		return false;
	}
	if( !Check( OR_COL ) )
	{
		std::cout << "Error, col invalid\n";
		return false;
	}
	if( !Check( OR_BLK ) )
	{
		std::cout << "Error, block invalid\n";
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
//		std::cout << "line=" << line << "\n";
		if( !line.empty() )
			if( ( line[0] >= '0' &&  line[0] <= '9') || line[0]=='.' || line[0]=='_' ) // lines starting with other characters are ignored
			{

				if( line.size() != 9  && li < 9 )
				{
					std::cout << "Error: line " << li<< " has a length of " << line.size() << '\n';
					return false;
				}
				for( size_t col=0; col<line.size(); col++ )
				{
//					std::cout << "char=" << line[col] << "\n";
					if( line[col] == '.' || line[col] == '0' || line[col] == '_')
						_data[li][col].SetValue( 0 );
					else
					{
						if( line[col] < '1' ||  line[col] > '9' )
						{
							std::cout << "Error: line " << li << " holds an invalid character: " << line[col] << " at col " << col << '\n';
							return false;
						}
						_data[li][col].SetValue( line[col] - '0' );
//						std::cout << "li=" << li << " col=" << col << " val=" << line[col] - '0' << "\n";
					}
				}
				li++;
			}
	}
//	InitGrid();
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
/// check in a view if there is only one value missing. If so, then we can assign a value to it
bool
Grid::SeachForSingleMissing( EN_ORIENTATION orient )
{
	PRINT_ALGO_START;
	for( index_t i=0; i<9; i++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX;
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
/// remove candidates on rows/cols/blocks that have a value in another cell
bool
Grid::RemoveCandidates( EN_ORIENTATION orient )
{
	PRINT_ALGO_START;

	bool res = false;
	for( index_t i=0; i<9; i++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX;
		View_1Dim_nc v1d = GetView( orient, i );

		for( index_t j=0; j<9; j++ ) // for each cell in the view
		{
			Cell& cell = v1d.GetCell(j);
			auto currentValue = cell.GetValue();
			if( currentValue != 0 )
			{
//				std::cout << " -found val:"  << currentValue << " at " << cell._pos << "\n";
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
Grid::SearchPairs( EN_ORIENTATION orient )
{
	PRINT_ALGO_START;

	bool res = false;

	for( index_t i=0; i<9; i++ )  // for each row/col
	{
		PRINT_MAIN_IDX;
		View_1Dim_nc v1d = GetView( orient, i );

		bool found = false;
		std::vector<value_t> v_cand_1;
		int pos_1, pos_2;
		for( index_t j=0; j<8 && !found; j++ ) // for each cell in the view
		{
			pos_1 = j;
			Cell& cell_1 = v1d.GetCell(j);
			if( cell_1.NbCandidates() == 2 )
			{
				v_cand_1 = cell_1.GetCandidates();
//				PrintVector( v_cand_1, "Found one 2-cand" );
				for( index_t k=j+1; k<9; k++ ) // for each other cell in the view
				{
					pos_2 = k;
					Cell& cell_2 = v1d.GetCell(k);
					if( v_cand_1 == cell_2.GetCandidates() ) // if the candidates are the same, then we can remove these from the others cells of the view
					{
						found = true;
						break;
					}
				}
			}
		}
		if( found )
		{
//			if( _verbose )
//				PrintVector( v_cand_1, "Found pair" );
			for( index_t j=0; j<9; j++ ) // for each cell in the view
			{
				Cell& cell = v1d.GetCell(j);
				if( j != pos_1 && j != pos_2 )
					if( cell.RemoveCandidates( v_cand_1 ) )
						res = true;
			}
		}
	}
	return res;
}
//----------------------------------------------------------------------------
/// Search for cells in a row/col/block where a candidate appears in only one cell
bool
Grid::SearchSingleCand( EN_ORIENTATION orient )
{
	PRINT_ALGO_START;

	bool res = false;
	bool stop = false;
	for( index_t i=0; i<9 && stop==false; i++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX;
		View_1Dim_nc v1d = GetView( orient, i );

		std::map<value_t,index_t> cand_count; // counts the number of times each candidate is present on the view (and also holds the cell's positions)
		for( value_t i=0; i<9; i++ )
			cand_count[i+1]=0;

		for( index_t j=0; j<9; j++ ) // for each cell in the view, count the candidates
		{
			Cell& cell = v1d.GetCell(j);
			auto v_cand = cell.GetCandidates();
			for( auto c: v_cand )
				cand_count[c]++;
		}

		for( index_t val=1; val<10; val++ )  // analyse results
		{
			auto c = cand_count[val];
			if( c == 1 )
			{
				std::cout << "found single " << val << '\n';
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
	if( _verbose )
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
/// Returns a set of cell positions that are on same row/col/block as \c c and have \c nb candidates
std::vector<pos_t>
Grid::GetOtherCells( const Cell& src, int nb, EN_ORIENTATION orient )
{
	std::vector<pos_t> out;
	auto row = src._pos.first;
	auto col = src._pos.second;

	View_1Dim_nc v1d;
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
			if( c.NbCandidates() == nb )
				out.push_back( c._pos );
	}
	return out;
}
//----------------------------------------------------------------------------
/// filter vector \c v_pos so that it holds only positions of cells holding only one of the candidates that are in \c v_cand
void
Grid::FilterByCand( const std::vector<value_t>& v_cand, std::vector<pos_t>& v_pos ) const
{
//PRINT_ALGO_START_0;
	std::vector<pos_t> v_out;
	assert( v_cand.size() == 2 );
	for( auto& pos : v_pos )
	{
		const Cell& c = GetCellByPos( pos );
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
/// Finds indexes of correspondences betwen 2 vectors holding 2 elements.
/// Returns value holds \c first: index on first vector, \c second: index on second vector
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
struct symMatches
{
	pos_t pA;
	pos_t pB;
	value_t value;
	symMatches( pos_t _pA, pos_t _pB, value_t _v ) : pA(_pA),pB(_pB),value(_v) {}
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
then we see that we have a match with elems 3 and 4, and the common candidates (that will get removed in further steps) is 4
*/
std::vector<symMatches>
FindSymmetricalMatches(
	const Grid& grid,
	const std::vector<value_t>& v_key_cand,   ///< input: vector of 2 values (candidates of key cell)
	const std::vector<pos_t>&   v_cells      ///< input: vector of cell positions that are in the range and hold 2 candidates
)
{
	std::cout << "START: " << __FUNCTION__ << '\n';
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
				PrintVector( v_cand_A, "v_cand_A" ); PrintVector( v_cand_B, "v_cand_B" );

				auto match_idx_B = FindMatch( v_key_cand, v_cand_B );

				std::cout << "match A: " << match_idx_A.first <<  '-' << match_idx_A.second << '\n';
				std::cout << "match B: " << match_idx_B.first <<  '-' << match_idx_A.second << '\n';

				if( v_cand_A[!match_idx_A.second] == v_cand_B[!match_idx_B.second] )
				{
					std::pair<pos_t,pos_t> p_out;
					p_out.first  = pA;
					p_out.second = pB;
					value_t common_value = v_cand_A[!match_idx_A.second];
					std::cout << "FOUND: pA=" << pA << " pB=" << pB <<  " common value=" << common_value << '\n';
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
//    PrintVector( v_out, "region to clear" );
	return v_out;
}
//----------------------------------------------------------------------------
/// Remove from all the cells in \c v_region the candidate \c cand, EXCEPT in (source) cell \c key
void
RemoveCandidatesFromRegion( Grid& grid, const std::vector<pos_t>& v_region, value_t cand, const Cell& key )
{
	for( auto& pos: v_region )
		if( pos != key._pos )
			grid.GetCellByPos(pos).RemoveCandidate( cand );
}
//----------------------------------------------------------------------------
/// check in a view if there is only one value missing. If so, then we can assign a value to it
bool
Grid::XY_Wing()
{
	PRINT_ALGO_START_0;

	for( index_t row=0; row<9; row++ )  // for each row
	{
		View_1Dim_nc v1d = GetView( OR_ROW, row );
		for( index_t col=0; col<9; col++ )   // for each col
		{
			Cell& key = v1d.GetCell(col);
			auto v_cand = key.GetCandidates();
			if( v_cand.size() == 2 )
			{
				auto v_cells   = GetOtherCells( key, 2, OR_ROW );  // get cells on same row that have 2 candidates
				auto v_cells_b = GetOtherCells( key, 2, OR_COL );  // get cells on same col that have 2 candidates
				auto v_cells_c = GetOtherCells( key, 2, OR_BLK );  // get cells in same block that have 2 candidates
				v_cells.insert( v_cells.end(), v_cells_b.begin(), v_cells_b.end() );
				v_cells.insert( v_cells.end(), v_cells_c.begin(), v_cells_c.end() );

				if( v_cells.size() > 1 )
				{
					std::cout << "\n -KEY: " << GetRowLetter(row) << col+1 << '\n'; \
					FilterByCand( v_cand, v_cells );                 // remove the ones that don't use any of the 2 candidates
					PrintVector( v_cells, "v_cells: after" );

					if( v_cells.size() > 1 )
					{
						std::vector<symMatches> v_matches = FindSymmetricalMatches( *this, v_cand, v_cells );
						for( const auto& p_match: v_matches )
						{
							auto v_region = FindCommonRegion( p_match.pA, p_match.pB );
							RemoveCandidatesFromRegion( *this, v_region, p_match.value, key );
						}
					}
				}
			}
		}
	}
	return false;
}
//----------------------------------------------------------------------------
bool
Grid::SearchPairs()
{
	if( !SearchPairs( OR_ROW ) )
		if( !SearchPairs( OR_COL ) )
			if( !SearchPairs( OR_BLK ) )
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

//----------------------------------------------------------------------------
/// returns true if something changed
bool
Grid::ProcessAlgorithm( EN_ALGO algo )
{
	bool res = false;
	switch( algo )
	{
		case ALG_REMOVE_CAND:   res=RemoveCandidates(); break;
		case ALG_SEARCH_PAIRS:  res=SearchPairs();      break;
		case ALG_SEARCH_SINGLE_CAND: res=SearchSingleCand(); break;
		case ALG_SEARCH_MISSING_SINGLE: res=SeachForSingleMissing(); break;
		case ALG_XY_WING: res = XY_Wing(); break;
		default: assert(0);
	}
//	SearchSingles();
//	PrintAll( std::cout, std::string( "after algo " + std::string( GetString(algo) ) ) );
	assert( NbUnknows() == NbUnknows2() );
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

			if( _verbose )
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

	std::cout << "nb_unknowns=" << nu_after << "\n";
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

