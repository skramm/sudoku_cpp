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

typedef typename boost::graph_traits<graph2_t>::vertex_descriptor vertex2_t;
typedef typename boost::graph_traits<graph2_t>::edge_descriptor   edge2_t;

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
//			std::cout << "NodeWriter_B:vertex=" << v << " idx=" << (int) _v1[v] << " g_ptr->size=" << g_ptr->size() << std::endl;
			auto cell = g_ptr->at( _v1[v] );
//			std::cout << "NodeWriter_B:cell=" << cell << std::endl;
			out << " [label=\"" << cell._pos << "\\n(" << (int)cell.get(0) << ',' << (int)cell.get(1) << ")\"]";
		}
	private:
		T1 _v1;
};

#if 0
template <class T1,class T2>
class NodeWriter_B2
{
	public:
		NodeWriter_B2(T1 v1, T2 v2) : _v1(v1), _v2(v2)
		{}
		template <class Vertex>
		void operator()( std::ostream &out, const Vertex& v ) const
		{
			auto cell = g_ptr->at( _v1[v] );
			out << " [label=\"" << cell._pos << "\\n(R=" << (int)_v2[v].first << ",G=" << (int)_v2[v].second << ")\"";
			if( cell._chainRole == CR_Start )
				out << ",penwidth=\"2.0\"";
			out << "]";
		}
	private:
		T1 _v1;
		T2 _v2;
};
#endif
//-------------------------------------------------------------------
/// Helper function to printout nodes in the graph, used by boost::write_graphviz()
template <class T1>
inline
NodeWriter_B<T1>
make_node_writer_B( T1 v1 )
{
	return NodeWriter_B<T1>(v1);
}
template <class T1,class T2>
inline
NodeWriter_B2<T1,T2>
make_node_writer_B2( T1 v1, T2 v2 )
{
	return NodeWriter_B2<T1,T2>(v1,v2);
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
	if( c1._pos.first == c2._pos.first )
		return true;
	if( c1._pos.second == c2._pos.second )
		return true;
	if( GetBlockIndex(c1._pos) == GetBlockIndex(c2._pos) )
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
			return !c._isUsed;
		}
	);
}
//-------------------------------------------------------------------
size_t
findFirstUnused( const std::vector<Cell2>& v_cells )
{
	for( size_t i=0; i<v_cells.size(); i++ )
		if( !v_cells[i]._isUsed )
			return i;
	assert(0);	        // should never happen...
}
//-------------------------------------------------------------------
#if 1
std::pair<bool,value_t>
shareCommonValue( const Cell2& c1, const Cell2& c2 )
{
	std::pair<bool,value_t>	res{true, 0};

	if( c1.get(0) == c2.get(0) || c1.get(0) == c2.get(1) )
		res.second = c1.get(0);
	else
	{
		if( c1.get(1) == c2.get(0) || c1.get(1) == c2.get(1) )
			res.second = c1.get(1);
		else
			res.first = false;
	}
	return res;
}
#else
bool
shareCommonValue( const Cell2& c1, const Cell2& c2 )
{
	if(
		c1.get(0) == c2.get(0) || c1.get(0) == c2.get(1) ||
		c1.get(1) == c2.get(0) || c1.get(1) == c2.get(1)
	)
		return true;
	return false;
}
#endif
//-------------------------------------------------------------------
/// Explore set of Cells having 2 candidates and add them to the graph, recursively
/**
Stop condition: when we can't add any more nodes
*/
void
buildGraphRecursive(
	graph2_t&           graph,
	vertex2_t           currVert,    ///< current vertex
	std::vector<Cell2>& v_cells      ///< the set of cells holding 2 candidates.
)
{
	static int iter;
	COUT( "iter " << ++iter << " currVert=" << currVert << " nbVert=" << boost::num_vertices(graph) );

	auto curr_idx = graph[currVert].cell_idx;           // index on the cell we are considering
	auto& currCell = v_cells.at(curr_idx);     // we fetch the cell
	COUT( " curr_idx=" << (int)curr_idx
		<< " pos=" << currCell._pos
		<< " nb_nodes=" << boost::num_vertices(graph)
		<< " nbUnusedCells=" << nbUnusedCells( v_cells )
		<< " nbVertices=" << boost::num_vertices(graph)
	);

	assert( nbUnusedCells( v_cells ) != 0 );

	for( index_t idx=0; idx<v_cells.size(); idx++ ) // iterate over set of cells
	{
		if( idx != curr_idx )          // if not the same cell !
		{
			auto& newCell = v_cells[idx];
			if( !newCell._isUsed )
			{
				COUT ( (int)idx << ": considering cell " << newCell );

				if( areLinkable( newCell, currCell ) )  // if cells are on same row/col/block
				{
					COUT( "Linkable !" );
					auto scv = shareCommonValue( newCell, currCell );  // if they share a common value,
					if( scv.first )  // if they share a common value,
					{                                               // THEN, its a new node in the graph !
						auto newVert = boost::add_vertex( graph );
						graph[newVert].cell_idx = idx;
						COUT( "cell_idx =" << (int)idx );
						newCell._isUsed = true;                  // tag the cell as "used"
						auto edge = boost::add_edge( currVert, newVert, graph ).first;
						graph[edge].commonVal = scv.second;
						COUT( "Added edge " << currCell._pos << "--" << newCell._pos );

						if( nbUnusedCells( v_cells ) != 0 )
						{
							COUT( "nbUnused=" << nbUnusedCells( v_cells ) << ", RE-ENTRY !" )
							buildGraphRecursive( graph, newVert, v_cells );
						}
					}
					else
						COUT( "NO common value" );
				}
				else
					COUT( "NOT linkable" );
			}
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
/// Builds the graphs from the set of cells holding 2 candidates. Not const, because each cell may get tagged as 'used' in graph
std::vector<graph2_t>
buildGraphs( std::vector<Cell2>& v_cells )
{
	std::vector<graph2_t> v_graphs;
	graph2_t graph;

	auto& cell = v_cells[0];
	cell._isUsed = true;                 // tag the cell as "used"

	auto vert = boost::add_vertex( graph );  // add initial vertex
	graph[vert].cell_idx = 0;
	v_graphs.push_back( graph );       // add initial graph

	size_t idx_graph = 0;
	do
	{
		COUT( "start loop, idx_graph=" << idx_graph << " nbGraph=" << v_graphs.size() << " vert=" << vert << " nbVert=" << boost::num_vertices(v_graphs[idx_graph]) );
		buildGraphRecursive( v_graphs[idx_graph], vert, v_cells );
//		printGraph( std::cout, v_graphs[idx_graph] );
		idx_graph++;
		COUT ( "AFTER: nbUnusedCells=" << nbUnusedCells(v_cells) );
		if( nbUnusedCells(v_cells) != 0 )        // then some cells where not connected, so we create a new graph
		{
			auto idx_c = findFirstUnused( v_cells );
			COUT( "NbGraph=" << idx_graph << " nbUnusedCells" << nbUnusedCells(v_cells) << " firstUnused=" <<idx_c );
			v_cells[idx_c]._isUsed = true;

			graph2_t graph2;
			vert = boost::add_vertex( graph2 );
			graph2[vert].cell_idx = idx_c;
			v_graphs.push_back( graph2 );

// checking
//			COUT( "NEW IDX=" << v_graphs[idx_graph][vert].cell_idx );
		}
	}
	while( nbUnusedCells(v_cells) != 0 );


//	COUT( "NbGraph=" << v_graphs.size() << " nbUnusedCells" << nbUnusedCells(v_cells) );

#ifdef GENERATE_DOT_FILES
	g_ptr = &v_cells;
	index_t i=0;
	for( auto& gr: v_graphs )
	{
		std::ofstream file( "out/xyc_" + std::to_string(i++) + ".dot" );
		assert( file.is_open() );
		boost::write_graphviz(
			file,
			gr,
			make_node_writer_B( boost::get( &GraphNode_B::cell_idx, gr ) ),
			make_edge_writer_B( boost::get( &GraphEdge_B::commonVal, graph ) )
		);
	}
#endif

	return v_graphs;
}


#if 0
//-------------------------------------------------------------------
std::vector<LinkXY>
buildSetOfLinks( const std::vector<Cell2>& v_cells )
{
	std::vector<LinkXY> v_links;
	for( index_t i=0; i<v_cells.size()-1; i++ )
	{
		const Cell2& c1 = v_cells[i];
		for( index_t j=0; j<v_cells.size(); j++ )
		{
			if( i !=j )
			{
				const Cell2& c2 = v_cells[j];
				if( areLinkable( c1, c2 ) )
				{
					auto scv = shareCommonValue(c1,c2);
					if( scv.first )
						AddToVector( v_links, LinkXY( c1,c2, scv.second ) );
				}
			}
		}
	}
	return v_links;
}
#endif
//-------------------------------------------------------------------
bool
Algo_XY_Chains( Grid& g )
{

// step 1 - build a set of cells having two candidates
	std::vector<Cell2> v_cells;
	for( index_t i=0; i<81; i++ )
	{
		auto cell = g.getCell(i);
		if( cell.NbCandidates() == 2 )
		{
			Cell2 c2( cell );
			v_cells.push_back( c2 );
		}
	}

// step 2 - build a set of graphs covering all these cells
	auto v_graph1 = buildGraphs( v_cells );

// step 3 - explore these graphs to find chains



#if 0
// for each of these, build a non-oriented graph of cells
	for( index_t idx=0; idx<v_cells.size(); idx++ )
	{
//		auto vc = cell.GetCandidates();
		auto gr0 = buildGraphFrom( idx, 0, v_cells );

		auto gr1 = buildGraphFrom( idx, 1, v_cells );
	}

// step 2 - iterate through these to see if we can build a chain

#endif

	return false;
}
//-------------------------------------------------------------------
