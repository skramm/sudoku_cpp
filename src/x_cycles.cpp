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
\file x_cycles.cpp
\brief X cycles algorithm implementation

See:
- http://www.sudokuwiki.org/X_Cycles
- http://www.sudokuwiki.org/X_Cycles_Part_2

*/


#ifdef BUILD_WITHOUT_UDGCD
#warning Building solver without X cycles algorithm!
#else

#include <fstream>

#include "udgcd.hpp"

#include "grid.h"
#include "header.h"
#include "x_cycles.h"
#include "algorithms.h"


#ifdef GENERATE_DOT_FILES
	#include <boost/graph/graphviz.hpp>
#endif

//----------------------------------------------------------------------------
const char*
GetString( En_CycleType ct )
{
	switch( ct )
	{
		case CT_undefined:   return "UNDEFINED"; break;
		case CT_Continuous:  return "Continuous"; break;
		case CT_Discont_2SL: return "Discontinuous/SL"; break;
		case CT_Discont_2WL: return "Discontinuous/WL"; break;
		case CT_Invalid:     return "INVALID"; break;
		default: assert(0);
	}
}
//----------------------------------------------------------------------------
Cell&
GetCommonCell( const Link& l1, const Link& l2, Grid& g )
{
	if( l1.p1 == l2.p2 || l1.p1 == l2.p1 )
		return g.GetCellByPos( l1.p1 );
	if( l1.p2 == l2.p1 || l1.p2 == l2.p2 )
		return g.GetCellByPos( l1.p2 );
	assert(0); // should never be here...
}
//----------------------------------------------------------------------------
/// Searches from position \c pos for all the weak links based on value \c val. Result is \b added to \c v_wl
void
FindWeakLinks( const Grid& g, value_t val, pos_t current_pos, EN_ORIENTATION orient, std::vector<Link>& v_wl )
{
//std::cout << "FindWeakLinks: val=" << (int)val << '\n';
	index_t idx = 0;
	switch( orient )
	{
		case OR_ROW: idx = current_pos.first;            break;
		case OR_COL: idx = current_pos.second;           break;
		case OR_BLK: idx = GetBlockIndex( current_pos ); break;
		default: assert(0);
	}

	View_1Dim_c v1d = g.GetView( orient, idx );

	std::vector<index_t> v_temp;
	for( index_t i=0; i<9; i++ )
	{
		const Cell& c = v1d.GetCell( i );
//		std::cout << "i=" << (int)i << " pos=" << c.GetPos() << " c.HasCandidate( val )=" << c.HasCandidate( val ) << '\n';
		if( c.HasCandidate( val ) && c.GetPos() != current_pos )
		{
			if( orient == OR_ROW || orient == OR_COL )           // if ROW/COL, then make sure it is not in same block
			{
				if( GetBlockIndex( c.GetPos() ) !=  GetBlockIndex( current_pos ) )
					v_temp.push_back( i );
			}
			else
				v_temp.push_back( i );                              // if not, just add it
		}
	}
//	std::cout << "or=" << GetString(orient) << " v_temp size="<<v_temp.size() << '\n';

	if( v_temp.size() > 1 )               // to be a weak link, we must have more than 2 cells with that value as candidates
		for( const auto& i: v_temp )
		{
			const Cell& c = v1d.GetCell( i );
			v_wl.push_back( Link{ current_pos, c.GetPos(), LT_Weak, orient } );
		}

//	std::cout << "after WeakLink search from pos " << current_pos << " with value -" << (int)val << "- with orientation " << GetString( orient ) << '\n';
//	PrintVector( v_wl, "WeakLink positions" );
}
//----------------------------------------------------------------------------
std::vector<Link>
FindAllWeakLinks( const Grid& g, value_t val, pos_t current_pos )
{
	std::vector<Link> v_wl;
	FindWeakLinks( g, val, current_pos, OR_ROW, v_wl );
	FindWeakLinks( g, val, current_pos, OR_COL, v_wl );
	FindWeakLinks( g, val, current_pos, OR_BLK, v_wl );

//	PrintVector( v_wl, "FindAllWeakLinks BEFORE dupes removal" );
	auto v_wl2 = VectorRemoveDupes( v_wl );

//	PrintVector( v_wl2, "FindAllWeakLinks after dupes removal" );
	return v_wl2;
}
//----------------------------------------------------------------------------
/// Finds in the grid \c g all the strong links for value \c val and orientation \c orient
void
FindStrongLinks( value_t val, EN_ORIENTATION orient, const Grid& g, std::vector<Link>& v_link )
{
	for( index_t idx=0; idx<9; idx++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_c v1d = g.GetView( orient, idx );
		CandMap candMap;
		for( index_t col1=0; col1<8; col1++ )   // for each cell in the view
		{
			const Cell& cell1 = v1d.GetCell(col1);
			if( cell1.HasCandidate( val ) && candMap.Has( val ) )
			{
				uint8_t c = 0;
				index_t pos = 0;
				for( index_t col2=col1+1; col2<9; col2++ )   // for each OTHER cell in the view
				{
					const Cell& cell2 = v1d.GetCell(col2);
					if( cell2.HasCandidate( val ) )
					{
						c++;
						pos = col2;
					}
				}
				if( c == 1 )                    // means we have ONLY 1 other cell with that candidate
					v_link.push_back( Link{ cell1.GetPos(), v1d.GetCell(pos).GetPos(), LT_Strong, orient } );

				if( c > 1 )                   // if we find more than 1 other, then
					candMap.Remove( val );    // don't consider this candidate any more
			}
		}
	}
}
//----------------------------------------------------------------------------
/// Finds in the grid \c g all the strong links for value \c val
std::vector<Link>
FindStrongLinks( value_t val, const Grid& g )
{
	std::vector<Link> v_link;
	FindStrongLinks( val, OR_ROW, g, v_link );
	FindStrongLinks( val, OR_COL, g, v_link );
	FindStrongLinks( val, OR_BLK, g, v_link );
//	PrintVector( v_link, "strong links BEFORE REMOVE DUPES" );
	auto v_link2 =  VectorRemoveDupes( v_link );
//	PrintVector( v_link2, "strong links AFTER REMOVE DUPES" );
	return v_link2;
}
//----------------------------------------------------------------------------
/// Vertex datatype, with BGL. Holds a cell position
struct GraphNode
{
	pos_t pos;
};
/// Edge datatype
struct GraphEdge
{
	En_LinkType link_type;
	EN_ORIENTATION link_orient;
};
//-------------------------------------------------------------------
/// A functor class used to printout the properties of the edges
template <class T1,class T2>
class EdgeWriter_2
{
	public:
		EdgeWriter_2(T1 v1, T2 v2) : _v1(v1),_v2(v2) {}
		template <class Edge>
		void operator()( std::ostream &out, const Edge& e ) const
		{

			out << "[label=\"" << GetString(_v2[e]) << "\"";
			if( _v1[e]==LT_Strong )
				out << ",style=\"bold\"";
			out << "]";
		}
	private:
		T1 _v1;
		T2 _v2;
};
//-------------------------------------------------------------------
/// A functor class used to printout the properties of the nodes
template <class T1>
class NodeWriter
{
	public:
		NodeWriter(T1 v1) : _v1(v1) {}
		template <class Vertex>
		void operator()( std::ostream &out, const Vertex& v ) const
		{
			out << "[label=\"" << _v1[v] << "\"]";
		}
	private:
		T1 _v1;
};
//-------------------------------------------------------------------
/// Helper function to printout nodes in the graph, used by boost::write_graphviz()
template <class T1>
inline
NodeWriter<T1>
make_node_writer( T1 v1 )
{
	return NodeWriter<T1>(v1);
}
//-------------------------------------------------------------------
/// Helper function to printout edges in the graph, used by boost::write_graphviz()
template <class T1,class T2>
inline
EdgeWriter_2<T1,T2>
make_edge_writer( T1 v1, T2 v2 )
{
	return EdgeWriter_2<T1,T2>(v1,v2);
}
//-------------------------------------------------------------------
/// A graph datatype, with BGL
typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS,
	GraphNode,
	GraphEdge
	> graph_t;

typedef typename boost::graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef typename boost::graph_traits<graph_t>::edge_descriptor   edge_t;

//----------------------------------------------------------------------------
/// Find vertex (get iterator on the vertex that we are searching), given its position \c pos
int
FindVertex( pos_t pos, const graph_t& g )
{
    auto it_pair = boost::vertices(g);

    auto it_v = std::find_if(
		it_pair.first,           // begin
		it_pair.second,          // end
		[&g,pos]                             // lambda: what we are capturing from outside
		(vertex_t v)                         // lambda: argument (the dereferenced iterator)
		{ return (g[v].pos) == pos; }  // lambda: what we do here: return true if Id of current vertex is equal to what we are searching
	);

    if( it_v == it_pair.second )                    // if not found,
		return -1;
	return static_cast<int>(*it_v);
}
//----------------------------------------------------------------------------
void
PrintGraphCycle( const std::vector<vertex_t>& cy, const graph_t& graph )
{
	for( size_t i=0; i<cy.size(); i++ )
	{
		auto idx1 = cy[i];
		auto idx2 = ( i+1!=cy.size() ? cy[i+1] : cy[0] );
		auto pair_edge = boost::edge( idx1, idx2, graph );
		assert( pair_edge.second );

		std::cout << '(' << graph[idx1].pos << ")-"
			<< GetString( graph[pair_edge.first].link_orient )
			<< '/'
			<< ( graph[pair_edge.first].link_type==LT_Strong ? 'S' : 'W' )
			<< '-';
	}
	std::cout << '\n';
}
//----------------------------------------------------------------------------
void
PrintGraphCycles( std::vector<std::vector<vertex_t>> cycles, std::string msg, const graph_t& graph )
{
	std::cout << "Cycles: " << msg << " nb=" << cycles.size() << '\n';
	for( const auto& cy: cycles )
		PrintGraphCycle( cy, graph );
}
//----------------------------------------------------------------------------
/// Returns true if the cycle \c cy has no more than 2 consecutive weak links
bool
CycleIsOk( const std::vector<vertex_t>& cy, const graph_t& graph )
{
//	std::cout << "CycleIsOk:"; PrintCycle( cy, graph );
	size_t count_WL = 0;
	for( size_t i=0; i<cy.size(); i++ )
	{
//		std::cout <<"i=" << i << std::endl;
		auto idx1 = cy[i];
		auto idx2 = ( i+1!=cy.size() ? cy[i+1] : cy[0] );
		auto pair_edge = boost::edge( idx1, idx2, graph );
		assert( pair_edge.second );
		if( graph[pair_edge.first].link_type == LT_Strong )
			count_WL = 0;
		else
			count_WL++;

		if( count_WL == 3 )
		{
//			std::cout << "count_WL == 3\n";
			return false;       // no need to continue
		}
	}
//	std::cout << "loop end, count_WL=" << count_WL << std::endl;
	if( count_WL>0 )     // if the last one was Weak, we need to check the next ones
	{
		auto idx1a = cy[0];
		auto idx2a = cy[1];
		auto pair_edge_1 = boost::edge( idx1a, idx2a, graph );
		assert( pair_edge_1.second );

//		std::cout << "link between " << graph[idx1a].pos << " and " << graph[idx2a].pos << std::endl;
        if( count_WL == 2 ) // if already 2, then just check the first link
        {
			if( graph[pair_edge_1.first].link_type == LT_Weak )
				return false;
        }
        else               // count_WL == 1: we need to check the to next links (pos 1 and 2)
        {
			if( graph[pair_edge_1.first].link_type == LT_Weak )
			{
				auto idx1b = cy[1];
				auto idx2b = cy[2];
				auto pair_edge_2 = boost::edge( idx1b, idx2b, graph );
				assert( pair_edge_2.second );
				if( graph[pair_edge_2.first].link_type == LT_Weak )
					return false;
			}
        }
	}

	return true;
}
//----------------------------------------------------------------------------
/// Returns the vector without the cycles having more than 2 consecutive weak links
std::vector<std::vector<vertex_t>>
FilterCycles( const std::vector<std::vector<vertex_t>>& vv_in, const graph_t& graph )
{
	std::vector<std::vector<vertex_t>> vv_out;
	for( const auto& c: vv_in )
		if( CycleIsOk( c, graph ) )
			vv_out.push_back( c );
	return vv_out;
}
//----------------------------------------------------------------------------
/// Converts the cycle from a BGL representation into a \c Cycle representation
Cycle
Convert2Cycle( const std::vector<vertex_t>& in_cycle, const graph_t& graph )
{
	Cycle out_cycle;
	for( size_t i=0; i<in_cycle.size(); i++ )
	{
		auto idx1 = in_cycle[i];
		auto idx2 = ( i+1!=in_cycle.size() ? in_cycle[i+1] : in_cycle[0] );
		auto edge = boost::edge( idx1, idx2, graph ).first;

		out_cycle.AddElem(
			Link(
				graph[idx1].pos,
				graph[idx2].pos,
				graph[edge].link_type,
				graph[edge].link_orient
			)
		);
	}
	return out_cycle;
}
//----------------------------------------------------------------------------
/// Converts the cycles from the "graph" representation into a vector of \c Cycle
std::vector<Cycle>
Convert2Cycles( const std::vector<std::vector<vertex_t>>& v_cycle, const graph_t& graph )
{
	std::vector<Cycle> v_out;
	for( const auto& cy: v_cycle )
		v_out.push_back( Convert2Cycle( cy, graph ) );
	return v_out;
}
//----------------------------------------------------------------------------
/// Finds all the cycles in the grid for value \c val.
/// Needs as input the set of Strong Links that have been found
/**
This function relies on the udgcd library.

If GENERATE_DOT_FILES is defined, this function will generate dot files for the graphs found.
You can plot them with <tt>make dot</tt>. Strong links will be bold, and weak links not.

For each value, two graph dot files will be generated:
- \c ls_V_X.dot: holds strong-links only graph, for value V
- \c la_V_X.dot: holds graph completed with weak links
*/
std::vector<Cycle>
FindCycles(
	const Grid&              g,
	value_t                  val,
	const std::vector<Link>& v_StrongLinks
)
{
	COUT( " val=" << (int)val );
// 1 - add all the strong links to the graph
	graph_t graph;
	for( const auto& sl: v_StrongLinks )
	{
		auto v1 = FindVertex( sl.p1, graph );
		if( v1 == -1 )
		{
			v1 = boost::add_vertex( graph );
			graph[v1].pos = sl.p1;
		}
		auto v2 = FindVertex( sl.p2, graph );
		if( v2 == -1 )
		{
			v2 = boost::add_vertex( graph );
			graph[v2].pos = sl.p2;
		}

        auto e = boost::add_edge( v1, v2, graph ).first;
		graph[e].link_type   = LT_Strong;
		graph[e].link_orient = sl._lorient;
	}

#ifdef GENERATE_DOT_FILES
	static std::array<int,9> dot_counter;
	std::ofstream file( "out/ls_" + std::to_string(val) + '_' + std::to_string(dot_counter[val]) + ".dot" );
	assert( file.is_open() );
	boost::write_graphviz(
		file,
		graph,
		make_node_writer( boost::get( &GraphNode::pos, graph ) ),
		make_edge_writer( boost::get( &GraphEdge::link_type, graph ), boost::get( &GraphEdge::link_orient, graph ) )
	);
#endif

// 2 - for each of the vertices, search if there are some weak links and add them
	auto pair_node_it = boost::vertices( graph );
	for( ; pair_node_it.first != pair_node_it.second; pair_node_it.first++ )
	{
		vertex_t src = *pair_node_it.first;
		auto pos_src = graph[src].pos;
		auto v_WeakLinks = FindAllWeakLinks( g, val, pos_src );
//		std::cout << "Nb Weak links for pos " << pos_src << "=" << v_WeakLinks.size() << '\n';
//		PrintVector(v_WeakLinks, "weaklinks");
		for( const auto& wl: v_WeakLinks )
		{
			auto pos_wl = wl.p1;        // consider the other side of the link,
			if( pos_wl == pos_src )     // related to source position
				pos_wl = wl.p2;

			auto v = FindVertex( pos_wl, graph );
			if( v == -1 )
			{
				v = boost::add_vertex( graph );
				graph[v].pos = pos_wl;
			}
			if( !boost::edge( src, v, graph ).second )                   // add the edge only if not already present
			{
				auto e = boost::add_edge( src, v, graph ).first;
				graph[e].link_type   = LT_Weak;
				graph[e].link_orient = wl._lorient;
			}
		}
	}
#ifdef GENERATE_DOT_FILES
	std::ofstream file2( "out/la_" + std::to_string(val) + '_' + std::to_string(dot_counter[val]) + ".dot" );
	assert( file2.is_open() );
	boost::write_graphviz(
		file2,
		graph,
		make_node_writer( boost::get( &GraphNode::pos, graph ) ),
		make_edge_writer( boost::get( &GraphEdge::link_type, graph ), boost::get( &GraphEdge::link_orient, graph ) )
	);
	dot_counter[val]++;
#endif

	if( g_data.Verbose )
		std::cout << "FindCycles(): start udgcd::findCycles()" << std::endl;
	auto cycles = udgcd::findCycles<graph_t,vertex_t>( graph );
//	std::cout << "VAL=" << (int)val << " nb cycles=" << cycles.size() << '\n';
//	PrintCycles( cycles, "v1", graph );
	if( g_data.Verbose )
		std::cout << " => found " << cycles.size() << " cycles\n";

	auto cycles2 = FilterCycles( cycles, graph );
//	std::cout << "FindCycles(): after filtering: VAL=" << (int)val << " nb cycles2=" << cycles2.size() << '\n';

	if( g_data.Verbose )
		PrintGraphCycles( cycles2, "AFTER FILTERING", graph );

	return Convert2Cycles( cycles2, graph );
}
//----------------------------------------------------------------------------
/// Analyze the cycle and return as a pair its type and the index where the discontinuity occurs (if any, -1 if none)
/**
Rules:
- if even nb of links: is either continuous or invalid
- if odd nb of links: is either CT_Discont_2SL, CT_Discont_2WL or invalid

\todo needs a rewrite !
We need to handle the case where a discontinuity occurs at the end !

Test cases:
\verbatim
		idx
size  0 1 2 3 4 5
  6   W-S-W-W-W-S  => invalid (3 weak links in a row)
  6   W-S-W-S-W-S  => continuous
  6   S-W-S-W-S-W  => continuous
  7   W-S-W-S-W-S-S => discontinuous, 2 strong links
  7   S-W-S-W-S-W-S => discontinuous, 2 strong links
  7   S-S-W-S-W-S-W => discontinuous, 2 strong links

  6   W-W-W-S-W-S => invalid: 3 weak links
  6   W-W-S-W-S-W => invalid: 3 weak links
  6   W-S-W-S-W-W => invalid: 3 weak links
  6   S-W-S-W-W-W => invalid: 3 weak links

\endverbatim
*/
CycleType
GetCycleType( const Cycle& cy )
{
	size_t count_WL = 0;   ///< Nb of Weak Links
	size_t count_SL = 0;   ///< Nb of Strong Links
	bool   has2WL(false);  ///< Has 2 Weak Links
	bool   has2SL(false);  ///< Has 2 Strong Links
	int    idx = -1;

// first check if even and only strong links => then, is continuous
	if( cy.size()%2 == 0 )     // if even
	{
		if( std::find_if(
				std::begin( cy.data() ),
				std::end(   cy.data() ),
				[](const Link& l){return l._ltype==LT_Weak; }
			)
			==
			std::end(  cy.data() )
		)
			return CycleType( CT_Continuous );
	}

	for( size_t i=0; i<cy.size(); i++ )
	{
		const auto& bl = cy.GetElem(i);
		if( bl._ltype == LT_Weak )
		{
			count_WL++;
			count_SL = 0;
		}
		else
		{
			count_WL = 0;
			count_SL++;
		}

		if( count_WL == 2 )
		{
			if( has2WL ) // if had previously counted 2
			{
				return CycleType( CT_Invalid );
			}
			else
			{
				has2WL = true;
				idx = i-1;
			}
		}

		if( count_SL == 2 )
		{
			if( has2SL ) // if had previously counted 2
				return CycleType( CT_Invalid );
			else
			{
				has2SL = true;
				idx = i-1;
			}
		}

		if( count_WL == 3 )
		{
			return CycleType( CT_Invalid );
		}
	}

	En_CycleType cyctype = CT_undefined;
	{
		if( has2WL )                         // if has 2 weak links
		{
			if( has2SL || cy.size()%2 == 0 ) // AND also 2 strong links or is even
				cyctype = CT_Invalid;
			else
				cyctype = CT_Discont_2WL;
		}
		else
		{
			if( has2SL )
				cyctype = CT_Discont_2SL;
			else
				cyctype = CT_Continuous;
		}
	}
	return CycleType( cyctype, idx );
}
//----------------------------------------------------------------------------
/// Explore a cycle and do the corresponding action, that is either:
/**
- Nice Loops Rule 1
- Nice Loops Rule 2
- Nice Loops Rule 3

See http://www.sudokuwiki.org/X_Cycles for details

Returns true if some removals have been processed
*/
bool
ExploreCycle( Cycle& cy, Grid& g, value_t val )
{
	COUT( __FUNCTION__ << "(): " << cy );
	bool removalDone( false );
	auto gct = GetCycleType( cy );
	assert( gct._ctype != CT_undefined );
	COUT( "Cycle type=" << GetString( gct._ctype ) << " idx=" << gct._idx );

	assert( gct._ctype != CT_undefined );
	switch( gct._ctype )
	{
		case CT_Continuous:                          // then, do the "Nice Loops Rule 1"
			COUT( "Cycle continuous: " << cy << " Nice Loops Rule 1" );
			for( size_t i=0; i<cy.size(); i++ )
			{
				const auto& link = cy.GetElem( i );
				COUT( "link: " << link );

				View_1Dim_nc view;       // step 1 - get the corresponding view (row/col/block)
				switch( link._lorient )
				{
					case OR_ROW: view = g.GetView( link._lorient, link.p1.first ); break;
					case OR_COL: view = g.GetView( link._lorient, link.p1.second ); break;
					case OR_BLK: view = g.GetView( link._lorient, GetBlockIndex( link.p1 ) ); break;
					default: assert(0);
				}

				for( index_t i=0; i<9; i++ ) // step 2 - parse the view and remove from the cells the value
				{                            //          (except for the two cells part of the link)
					auto& cell = view.GetCell( i );
					if( cell.GetPos() != link.p1 && cell.GetPos() != link.p2 )
						if( cell.RemoveCandidate( val ) )
							removalDone = true;
				}
			}
		break;

		case CT_Discont_2SL: // Nice Loops Rule 2
		{
			COUT( "* Nice Loops Rule 2" );
			assert( gct._idx != 0 );
			const auto& link1 = cy.GetElem( gct._idx);
			const auto& link2 = cy.GetElem( gct._idx+1 );
			Cell& c = GetCommonCell( link1, link2, g );
			if( c.RemoveAllCandidatesBut( val ) )
				removalDone = true;
		}
		break;

		case CT_Discont_2WL: // Nice Loops Rule 3
		{
			const auto& link1 = cy.GetElem( gct._idx );
			const auto& link2 = cy.GetElem( gct._idx+1 );
			COUT( "* Nice Loops Rule 3: link1=" << link1 << " link2=" << link2 );

			Cell& c = GetCommonCell( link1, link2, g );
			COUT( "Common cell=" << c );
			if( c.RemoveCandidate( val ) )
				removalDone = true;
		}
		break;

		case CT_Invalid: // don't do anything
		break;

		default: assert(0);
	}
	return removalDone;
}
//----------------------------------------------------------------------------
/// X Cycles algorithm (WIP)
/**
This enables removing some candidates

See http://www.sudokuwiki.org/X_Cycles

\todo we only explore the first strong link! But other cycles could be found by iterating over the other ones

*/
bool
X_Cycles( Grid& g )
{
	PRINT_ALGO_START_2;
	for( value_t v=1; v<10; v++ )             // for each possible value, get strong links, then search cycles
	{
		COUT( "* base value: " << (int)v );
		auto v_sl = FindStrongLinks( v, g );
		if( g_data.Verbose )
		{
			std::cout << "\nX_Cycles: process value " << (int)v << '\n';
			PrintVector( v_sl, "Strong Links set" );
		}
//		const auto& sl_vect = msl.GetSLvect(v);
//		std::cout << "considering value " << (int)v << ", vector has " << sl_vect.size() << " values\n";
		if( v_sl.size() > 1 )
		{
			auto v_cyc = FindCycles( g, v, v_sl );
//			std::cout << "VALUE=" << (int)v << " cycles:\n";
			if( g_data.Verbose )
				PrintVector( v_cyc, "v_cyc" );
			for( auto& cy: v_cyc )       // not const, because cycles will get tagged
				if( ExploreCycle( cy, g, v ) )
					return true;
		}
	}

	return false;
}
//----------------------------------------------------------------------------
#endif
