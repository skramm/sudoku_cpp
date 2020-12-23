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



//#include <boost/graph/adjacency_list.hpp>


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
			out << "[";
			if( _v1[e]==false )              // if NOT Final Edge
				out << "style=\"bold\"";
			out << "]";
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
			out << " [label=\"" << cell._pos << "\\n(" << (int)cell.get(0) << ',' << (int)cell.get(1) << ")\"]";
		}
	private:
		T1 _v1;
};

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
/// Explore set of Cells having 2 candidates and add them to the graph, recursively
/**
Stop condition: when we can't add any more nodes
*/
void
buildGraphRecursive(
	graph2_t&           graph,       ///< graph
	vertex2_t           startVert,   ///< starting vertex (cell)
	value_t             startValue,  ///< starting cell value (has 2, so we have to know)
	vertex2_t           currVert,    ///< current vertex
	value_t             currValue,   ///< the value that brings us here
	std::vector<Cell2>& v_cells      ///< the set of cells holding 2 candidates.
)
{
	static int iter;
	auto curr_idx = graph[currVert].idx;           // index on the cell we are considering
	auto& curr_cell = v_cells.at(curr_idx);     // we fetch the cell
	COUT( "iter " << ++iter
		<< " curr_idx=" << (int)curr_idx
		<< " pos=" << curr_cell._pos
		<< " nb_nodes=" << boost::num_vertices(graph)
		<< " val=" << (int)currValue
	);
	for( index_t idx=0; idx<v_cells.size(); idx++ ) // iterate over set of cells
	{
		if( idx != curr_idx )          // if not the same cell !
		{
			auto& newCell = v_cells[idx];
			if( newCell._chainRole != CR_Start )  // if cell is not the start cell
			{
				COUT ( (int)idx  << ": considering cell at pos=" << newCell._pos
					<< " values=" << (int)newCell._candidValues.first << "-" << (int)newCell._candidValues.second );
				if( areLinkable( newCell, curr_cell ) )  // if cells are on same row/col/block
				{
					COUT( "Linkable !" );
					value_t vc1 = newCell.get(0);
					value_t vc2 = newCell.get(1);
					COUT( "vc1=" << (int)vc1 << " vc2=" << (int)vc2 );
					value_t currCompVal = curr_cell.getOther( currValue );

					if( currValue == vc1 || currValue == vc2 )          // if this considered cell holds the value of the current cell we are searching for,
						if( currCompVal != vc1 && currCompVal != vc2 )  // and it does NOT hold as other value the considered value
						{                                               // THEN, its a new node in the graph !
							auto newVert = boost::add_vertex( graph );

							newCell._chainRole = CR_Used;                  // tag the cell as "used"
							boost::add_edge( currVert, newVert, graph );
							COUT( "Added edge " << curr_cell._pos << "--" << newCell._pos << ", based on value '" << (int)currValue << "'" );

							auto newValue = vc1;
							auto newCompValue = vc2;
							if( currValue == vc1 )
								std::swap( newValue, newCompValue );
							graph[newVert] = GraphNode_B( idx, newValue, newCompValue );

							COUT( "newValue=" << (int)newValue << " newCompValue=" << (int)newCompValue );

							auto  start_idx = graph[startVert].idx;
							auto& start_cell = v_cells.at(start_idx);
							auto  startCompValue = start_cell.getOther( startValue );
							if(
//								areLinkable( newCell, start_cell )               // if cell is linkable with starting cell
//									&&                                        // AND
								startCompValue == newCompValue                //
							)
							{
								COUT( "adding final edge to " << start_cell._pos );
								newCell._chainRole = CR_End;
								auto finalEdge = boost::add_edge( startVert, newVert, graph ).first;
								graph[finalEdge].isFinalEdge = true;
							}
							else
							{
								COUT( "Rentry with value " << (int)newValue );
								buildGraphRecursive( graph, startVert, startValue, newVert, newValue, v_cells );
							}
						}
				}
				else
					COUT( "NOT linkable" );
			}
		}
	}
//	COUT( "END iter " << iter );
}

//-------------------------------------------------------------------
graph2_t
buildGraphFrom(
	index_t             idxCell,   ///< the index on the cell we are starting from, holds 2 candidates
	index_t             whichOne,  ///< 0 or 1
	std::vector<Cell2>& v_cells    ///< the set of cells holding 2 candidates. Not const, because each cell may get tagged as 'used' in graph
)
{
	graph2_t graph;
	auto vert = boost::add_vertex( graph );  // add initial vertex
	auto& cell = v_cells[idxCell];

	auto value = cell.get(whichOne);
	graph[vert] = GraphNode_B( idxCell, value, cell.getOther(value) );

	v_cells.at( idxCell )._chainRole = CR_Start;

	buildGraphRecursive( graph, vert, cell.get(whichOne), vert, cell.get(whichOne), v_cells );


#ifdef GENERATE_DOT_FILES
	g_ptr = &v_cells;
	std::ofstream file( "out/xyc_" + std::to_string(cell.get(whichOne)) + ".dot" );
	assert( file.is_open() );
	boost::write_graphviz(
		file,
		graph,
		make_node_writer_B2( boost::get( &GraphNode_B::idx, graph ), boost::get( &GraphNode_B::colorValues, graph ) ),
		make_edge_writer_B( boost::get( &GraphEdge_B::isFinalEdge, graph ) )
	);
#endif

	return graph;
}

//-------------------------------------------------------------------
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

// step 2 -build a set of links joining these cells
	auto setLinks = buildSetOfLinks( v_cells );
	PrintVector( setLinks, "set of links" );

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
