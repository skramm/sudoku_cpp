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
\file
\brief Implementation for XY-chains algorithm

Some details about the algorithm:

Ref: https://www.sudokuwiki.org/XY_Chains

- The chain can have an odd or an even number of links
- The starting and ending cell can have the same pair of values or not
- The common value between start and end must be of a different color in these two cells
*/


#include "xy_chains.h"

#include "grid.h"
#include "header.h"
#include "algorithms.h"


#ifdef GENERATE_DOT_FILES
	#include <fstream>
	#include <boost/graph/graphviz.hpp>
	std::vector<Cell2>* g_ptr = 0;
#endif
#include <boost/graph/graph_utility.hpp> //  for boost::print_graph()


#ifdef GENERATE_DOT_FILES
//-------------------------------------------------------------------
/// A functor class used to printout the properties of the edges, related to Algo_XY_Chains()
template <class T1>
class EdgeWriter_B
{
	public:
		EdgeWriter_B(T1 v1) : _v1(v1)
		{}
		template <class Edge>
		void operator()( std::ostream &out, const Edge& e ) const
		{
			out << "[label=\"" << (int)_v1[e] << "\"]";
		}
	private:
		T1 _v1;
};
//-------------------------------------------------------------------
/// A functor class used to printout the properties of the nodes, related to Algo_XY_Chains()
template <class T1>
class NodeWriter_B
{
	public:
		NodeWriter_B(T1 v1) : _v1(v1)
		{}
		template <class Vertex>
		void operator()( std::ostream &out, const Vertex& v ) const
		{
			auto cell = g_ptr->at( _v1[v] );
			out << " [label=\"" << cell._pos << "\\n(" << (int)cell.get(0) << ',' << (int)cell.get(1) << ")\""
				<< " pos=\"" << (int)cell._pos.col() << ',' << -(int)cell._pos.row() << "!\"";
			if( _v1[v] == 0 )
				out << " penwidth=\"2.0\"";
			out << "]";
		}
	private:
		T1 _v1;
};

//-------------------------------------------------------------------
/// Helper function to printout nodes in the graph, used by boost::write_graphviz()
template <class T1>
inline
NodeWriter_B<T1>
make_node_writer_B( T1 v1 )
{
	return NodeWriter_B<T1>(v1);
}
//-------------------------------------------------------------------
/// Helper function to printout edges in the graph, used by boost::write_graphviz()
template <class T1>
inline
EdgeWriter_B<T1>
make_edge_writer_B( T1 v1 )
{
	return EdgeWriter_B<T1>(v1);
}
//-------------------------------------------------------------------
#endif // GENERATE_DOT_FILES

//-------------------------------------------------------------------
/// Returns true if cell are "linkable", that is they are:
/**
- either on same row
- either on same column
- either in same block
*/
bool
areLinkable( const Cell2& c1, const Cell2& c2 )
{
	if( c1._pos.row() == c2._pos.row() )
		return true;
	if( c1._pos.col() == c2._pos.col() )
		return true;
	if( c1._pos.getBlockIndex() == c2._pos.getBlockIndex() )
		return true;
	return false;
}
//-------------------------------------------------------------------
size_t
nbUnusedCells( const std::vector<Cell2>& v_cells )
{
	return std::count_if(
		std::begin( v_cells ),
		std::end( v_cells ),
		[]                         // lambda
		( const Cell2& c )
		{
			return c._graphIdx==-1;
		}
	);
}
//-------------------------------------------------------------------
size_t
findFirstUnused( const std::vector<Cell2>& v_cells )
{
	for( size_t i=0; i<v_cells.size(); i++ )
		if( v_cells[i]._graphIdx==-1 )
			return i;
	assert(0);	        // should never happen...
}
//-------------------------------------------------------------------
/// Check if cells share 0, 1, or 2 common values
/**
- if 1 common value, second member of pair will hold it
- if 2 common values, second member of pair will be irrelevant
*/
std::pair<uint8_t,value_t>
shareCommonValue( const Cell2& c1, const Cell2& c2 )
{
	assert( c1.get(0) != c1.get(1) );
	assert( c2.get(0) != c2.get(1) );

	std::pair<uint8_t,value_t>	res{0, 0};
	if( c1.get(0) == c2.get(0) || c1.get(0) == c2.get(1) )
	{
		res.first++;
		res.second = c1.get(0);
	}
	if( c1.get(1) == c2.get(0) || c1.get(1) == c2.get(1) )
	{
		res.first++;
		res.second = c1.get(1);
	}
	return res;
}

//-------------------------------------------------------------------
/// Add to set the cell position lying on row/col of p1/p2 and part of the block of p2/p1 (respectively)
void
addToPosSet(
	std::set<Pos>& posSet,   ///< output set, we add values here
	EN_ORIENTATION orient,
	Pos            p1,
	Pos            p2
)
{
	auto bi1=p1.getBlockIndex();
	auto bi2=p2.getBlockIndex();

	if( orient == OR_ROW )
	{
		posSet.insert( Pos( p1.row(), getBlockCol(bi2)*3+0 ) );
		posSet.insert( Pos( p1.row(), getBlockCol(bi2)*3+1 ) );
		posSet.insert( Pos( p1.row(), getBlockCol(bi2)*3+2 ) );

		posSet.insert( Pos( p2.row(), getBlockCol(bi1)*3+0 ) );
		posSet.insert( Pos( p2.row(), getBlockCol(bi1)*3+1 ) );
		posSet.insert( Pos( p2.row(), getBlockCol(bi1)*3+2 ) );
		return;
	}
	if( orient == OR_COL )
	{
		posSet.insert( Pos( getBlockRow(bi1)*3+0, p2.col() ) );
		posSet.insert( Pos( getBlockRow(bi1)*3+1, p2.col() ) );
		posSet.insert( Pos( getBlockRow(bi1)*3+2, p2.col() ) );

		posSet.insert( Pos( getBlockRow(bi2)*3+0, p1.col() ) );
		posSet.insert( Pos( getBlockRow(bi2)*3+1, p1.col() ) );
		posSet.insert( Pos( getBlockRow(bi2)*3+2, p1.col() ) );
		return;
	}
	assert(0);

}
//-------------------------------------------------------------------
/// Finds the intersection of the two final cells. Related to XY-Chains.
/// \todo NOT FINISHED, UNTESTED !!!
/**
Needed for XY-chains

This function will determine which row/col/block(s) have to be cleared of candidate value

- if the two cells are on same row/col, then all the row/col can be cleared of that candidate value


see doc/AS_XY_chain_example_1.png
The two cells are B3 and C5
*/
XYC_area
getArea( const Cell2& c1, const Cell2& c2 )
{
	auto scv = shareCommonValue( c1, c2 );
	assert( scv.first == 1 );

	XYC_area res( scv.second );

// step 1 - check if same row or same col (simplest case)
	bool isSameRowCol = false;
	if( c1._pos.row() == c2._pos.row() )      // same row
	{
		res._sPos = getCellsPos( OR_ROW, c1._pos.row()  );
		isSameRowCol = true;
	}
	if( c1._pos.col() == c2._pos.col() )  // same column
	{
		res._sPos = getCellsPos( OR_COL, c1._pos.col()  );
		isSameRowCol = true;
	}

	if( isSameRowCol )   // is same row or col, then erase the two cells of the output set
	{
		res._sPos.erase( std::find( std::begin(res._sPos), std::end(res._sPos), c1.pos() ) );
		res._sPos.erase( std::find( std::begin(res._sPos), std::end(res._sPos), c2.pos() ) );
		return res;
	}


// step 2 - if not, then add the two intersection cells
	res._sPos.insert( Pos( c1._pos.row(), c2._pos.col() ) );
	res._sPos.insert( Pos( c2._pos.row(), c1._pos.col() ) );

// step 3.1 - check if same block
	auto blockIndex1 = c1._pos.getBlockIndex();
	auto blockIndex2 = c2._pos.getBlockIndex();

	assert( blockIndex1 != blockIndex2 ); // SHOULD NOT HAPPEN !

	if( getBlockRow(blockIndex1) == getBlockRow(blockIndex2) )  // if blocks belong to same row,
	{                                                           // then, add the 3 cell intersecting row and block
		addToPosSet( res._sPos, OR_ROW, c1._pos, c2._pos );
	}
	else
	{
		if( getBlockCol(blockIndex1) == getBlockCol(blockIndex2) )
			addToPosSet( res._sPos, OR_COL, c1._pos, c2._pos );
	}

	return res;
}

//-------------------------------------------------------------------
/// Explore set of Cells having 2 candidates and add them to the graph, recursively
/**
Stop condition: when we can't add any more nodes
*/
void
buildGraphRecursive(
	Pgrvalset& 			graph_set,
	int                 graphIdx,
	vertex2_t           currVert,    ///< current vertex
	value_t             inVal,       ///< the value that lead us to that vertex, 0 if none (for starting)
	std::vector<Cell2>& v_cells      ///< the set of cells holding 2 candidates.
)
{
	static int iter;
	COUT( "iter " << ++iter << " currVert=" << currVert );
	graph2_t&          graph = graph_set.first;
	std::set<value_t>& vSet  = graph_set.second;

	auto curr_idx = graph[currVert].cell_idx;           // index on the cell we are considering
	auto& currCell = v_cells.at(curr_idx);     // we fetch the cell
	COUT( " curr_idx=" << (int)curr_idx
		<< " pos=" << currCell._pos
		<< " values=(" << (int)currCell._candidValues.first << ',' << (int)currCell._candidValues.second << ')'
		<< " nbUnusedCells=" << nbUnusedCells( v_cells )
		<< " nbVertices=" << boost::num_vertices(graph)
	);

	assert( nbUnusedCells( v_cells ) != 0 );

	for( index_t idx=0; idx<v_cells.size(); idx++ ) // iterate over set of cells
	{
		if( idx != curr_idx )          // if not the same cell !
		{
			auto& newCell = v_cells.at(idx);
			COUT ( (int)idx << ": considering cell " << newCell << ": " << newCell._candidValues );

			if( areLinkable( newCell, currCell ) )  // if cells are on same row/col/block
			{
				COUT( " -Linkable !" );
				auto scv = shareCommonValue( newCell, currCell );  // check if common value(s),
				if( (scv.first==1 && scv.second != inVal)         //  that is NOT the "entrance" value (in case of 1 common value)
					|| scv.first==2 )
				{
					auto linkVal = scv.second;                        // determine the link value
					if( scv.first==2 )
						linkVal = newCell.getOtherVal( inVal );

					if( newCell._graphIdx==-1 )                       // if not already used,
					{                                                 // its a new node in the graph
						auto newVert = boost::add_vertex( graph );
						graph[newVert].cell_idx = idx;
						COUT( "cell_idx =" << (int)idx );
						newCell._graphIdx = graphIdx;               // tag the cell as "used"
						newCell._vertex = newVert;

						vSet.insert( newCell._candidValues.first );
						vSet.insert( newCell._candidValues.second );

						auto edge = boost::add_edge( currVert, newVert, graph ).first;
						graph[edge].commonVal = linkVal;
						COUT( "Added edge " << currCell._pos << "--" << newCell._pos );

						if( nbUnusedCells( v_cells ) != 0 )
						{
							COUT( "nbUnused=" << nbUnusedCells( v_cells ) << ", RE-ENTRY !" )
							buildGraphRecursive( graph_set, graphIdx, newVert, linkVal, v_cells );
						}
					}
					else  // if already used, then add an edge (if edge not already present)
					{
						auto v = newCell._vertex;
						if( !boost::edge( v, currVert,graph ).second )  // if not there,
						{
							auto edge = boost::add_edge( currVert, v, graph ).first;  // then create that edge
							graph[edge].commonVal = scv.second;
						}
					}
				}
				else
					COUT( " -NO common value" );
			}
			else
				COUT( " -NOT linkable" );
		}
	}
}

//-------------------------------------------------------------------
void
printGraph( std::ostream& s, const graph2_t& graph )
{
	s << "nb ver=" << boost::num_vertices(graph) << '\n';
	for( auto itv=boost::vertices(graph).first; itv!=boost::vertices(graph).second; itv++ )
	{
		s << "idx=" << (int)graph[*itv].cell_idx << "\n";
	}
}
//-------------------------------------------------------------------
/// Builds the graphs from the set of cells holding 2 candidates. Not const, because each cell may get tagged as 'used' in graph.
/// Returns a vector of pairs (graph,cell values)
std::vector<Pgrvalset>
buildGraphs( std::vector<Cell2>& v_cells )
{
	std::vector<Pgrvalset> v_out;

	graph2_t graph;
	std::set<value_t> s_values;

	auto vert = boost::add_vertex( graph );  // add initial vertex
	graph[vert].cell_idx = 0;

	auto& cell = v_cells[0];
	cell._graphIdx = 0;                 // tag the initial cell as "used"
	cell._vertex = vert;
	s_values.insert( cell._candidValues.first );  // insert the values of initial cell
	s_values.insert( cell._candidValues.second );

	v_out.push_back( std::make_pair(graph,s_values) );       // add initial graph

	size_t idx_graph = 0;
	do
	{
		COUT( "start loop, idx_graph=" << idx_graph << " nbGraph=" << v_out.size() << " vert=" << vert << " nbVert=" << boost::num_vertices(v_out[idx_graph].first) );
		buildGraphRecursive( v_out[idx_graph], idx_graph, vert, 0, v_cells );
		idx_graph++;
		if( nbUnusedCells(v_cells) != 0 )        // then some cells where not connected, so we create a new graph
		{
			auto idx_c = findFirstUnused( v_cells );
			COUT( "** CREATE NEW GRAPH, NbGraph=" << idx_graph << " nbUnusedCells=" << nbUnusedCells(v_cells) << " firstUnused=" <<idx_c );

			graph2_t graph2;                      // new graph
			vert = boost::add_vertex( graph2 );   // with its initial vertex
			graph2[vert].cell_idx = idx_c;

			std::set<value_t> s_values2;          // new set of values
			s_values2.insert( v_cells[idx_c]._candidValues.first );  // insert the values of initial cell
			s_values2.insert( v_cells[idx_c]._candidValues.second );

			v_out.push_back( std::make_pair(graph2,s_values2) );

			v_cells[idx_c]._graphIdx = idx_graph;
			v_cells[idx_c]._vertex = vert;
		}
	}
	while( nbUnusedCells(v_cells) != 0 );

#ifdef GENERATE_DOT_FILES
	static int c;
	g_ptr = &v_cells;
	index_t i=0;
	for( auto& grset: v_out )
	{
		std::ofstream file( "out/xyc_" + std::to_string(c) + "_" + std::to_string(i++) + ".dot" );
		assert( file.is_open() );
		boost::write_graphviz(
			file,
			grset.first,
			make_node_writer_B( boost::get( &GraphNode_B::cell_idx, grset.first ) ),
			make_edge_writer_B( boost::get( &GraphEdge_B::commonVal, grset.first ) )
		);
	}
	c++;
#endif

	return v_out;
}

//-------------------------------------------------------------------
/// Searches set of cells and returns those that:
/**
- hold the same value \c val
- belong to graph \c idx
*/
std::vector<Cell2>
findCellsHolding( const std::vector<Cell2>& v_cells, index_t gidx, value_t val )
{
	std::vector<Cell2> out;
	for( const auto& cell: v_cells )
	{
		if( cell._graphIdx == gidx )
			if( cell._candidValues.first == val || cell._candidValues.second == val )
				out.push_back( cell );
	}
	return out;
}
//--------------------------------------------------------------------
bool
valueIsSameColor( const Cell2& c1, const Cell2& c2, value_t val )
{
	auto a = shareCommonValue( c1, c2 );
	assert( a.first == 1 );
	assert( a.second == val );
	if(
		c1._candidValues.first == c2._candidValues.first
		||
		c1._candidValues.second == c2._candidValues.second
	)
		return true;
	return false;
}
//--------------------------------------------------------------------
/// Return value when searching in the area defined by an XY chain
enum En_CandRemov
{
	CRem_None,           ///< no candidates were remove
	CRem_OutOfCellSet,   ///< some candidates where removed, but this did not impact the set of cells
	CRem_CellSetImpacted ///< one candidate was removed on a cell that was in the set of cells
};
//--------------------------------------------------------------------
/// Remove candidates in the considered area, and stops if it touches a cell in the given set
En_CandRemov
removeCandidatesFromCellSet(
	Grid&                     grid,     ///< grid
	const std::vector<Cell2>& v_cells,  ///< set of cells forming the graph
	const XYC_area&           area      ///< area in which we want to remove candidates
)
{
	bool removal = false;
	for( const auto& cellPos: area._sPos )
	{
		auto& src_cell = grid.GetCellByPos( cellPos );
		if( src_cell.RemoveCandidate( area._commonValue ) )       // if there is a candidate to be removed in that cell,
		{                                           // then check if that cell is included in graph
			auto pos_src = src_cell.pos();
			removal = true;
			if(
				std::find_if(
					std::begin(v_cells),
					std::end(v_cells),
					[pos_src]                          // lambda
					( const Cell2& cell )
					{
						return cell.pos() == pos_src;
					}
				) != std::end(v_cells)
			)
			return CRem_CellSetImpacted;  // stop immediately
		}
	}
	return removal ? CRem_OutOfCellSet : CRem_None;
}
//--------------------------------------------------------------------
/// Iterate on subset holding cells that are linked (they form a chain) and
/// proceed to candidate removal
En_CandRemov
iterateOnCells(
	Grid&                     grid,
	const std::vector<Cell2>& v_cells,
	value_t                   val
)
{
	bool stop = false;
	En_CandRemov retval;

	for( index_t i=0; i<v_cells.size()-1  && !stop; i++ )
	{
		auto c1 = v_cells[i];
		for( index_t j=i+1; j<v_cells.size() && !stop; j++ )
		{
			auto c2 = v_cells[j];
			COUT( "c1: " << c1._pos << " c2:" << c2._pos );
			if( c1._pos.getBlockIndex() != c2._pos.getBlockIndex() )   // if not in same block
			{
				if( !valueIsSameColor(c1,c2,val) )  // if the value is not on the same color in those two cells
				{
					auto area = getArea( c1, c2 );
					COUT( "found chain, value=" << area._commonValue );
					auto reslocal = removeCandidatesFromCellSet( grid, v_cells, area );
					if( reslocal == CRem_OutOfCellSet )
						retval = CRem_OutOfCellSet;
					if( reslocal == CRem_CellSetImpacted )
					{
						retval = CRem_OutOfCellSet;
						stop = true;
					}
				}
			}
		}
	}
	return retval;
}
//-------------------------------------------------------------------
bool
exploreGraph(
	Grid&                  grid,
	std::vector<Cell2>&    v_cells,
	std::vector<Pgrvalset> v_pgs
)
{
	bool candidateRemoval = false;
	En_CandRemov res;

 // iterate on graphs, stop when no more OR when the set of cells has been impacted by a candidate removal
	for( index_t idx=0; idx<v_pgs.size() && res!= CRem_CellSetImpacted ; idx++ )
	{
		auto pgs = v_pgs[idx];
		for( auto val: pgs.second )  // iterate on the values that are in the current set
		{
			COUT( "graph " << (int)idx << ", current value=" << val );
			auto chval = findCellsHolding( v_cells, idx, val );
			COUT( "nb of cells with value " << val << "=" << chval.size() );
			if( chval.size() > 1 )
				res = iterateOnCells( grid, chval, val );
			if( res != CRem_None )
				candidateRemoval = true;
			if( res == CRem_CellSetImpacted ) // means: we have remove a candidate that was in the set of cells
				break;
		}
	}

	return candidateRemoval;
}

//-------------------------------------------------------------------
bool
Algo_XY_Chains( Grid& g )
{
// step 1 - build a set of cells having two candidates
	std::vector<Cell2> v_cells;
	for( index_t i=0; i<81; i++ )
	{
		const auto& cell = g.getCell(i);
		if( cell.NbCandidates() == 2 )
			v_cells.push_back( Cell2( cell ) );
	}

// step 2 - build a set of graphs covering all these cells
	auto v_pgs = buildGraphs( v_cells );

// step 3 - explore these graphs to find chains and remove candidates
	return exploreGraph( g, v_cells, v_pgs );
}
//-------------------------------------------------------------------
