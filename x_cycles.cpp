/**
\file x_cycles.cpp
\brief X cycles

See:
- http://www.sudokuwiki.org/X_Cycles
- http://www.sudokuwiki.org/X_Cycles_Part_2

*/

#include "grid.h"
#include "header.h"
#include <boost/graph/adjacency_list.hpp>

//----------------------------------------------------------------------------
// Related do X_Cycles()
/*
enum En_CycleType
{
	CT_Cont,           ///< continuous cycle, pair nb of nodes, alternate Weak and Strong links
	CT_Discont_2SL,    ///< Discontinuous cycle, odd nb of nodes, 2 chained Strong links
	CT_Discont_2WL     ///< Discontinuous cycle, odd nb of nodes, 2 chained Weak links
};*/
//----------------------------------------------------------------------------
/// To avoid a meaningless boolean
enum En_LinkType
{
	LT_Strong, LT_Weak
};
//----------------------------------------------------------------------------
/// A link between two cells holding the same candidate. Can be "Weak" or "Strong"
struct Link
{
	pos_t p1, p2;
	En_LinkType type;
	EN_ORIENTATION orient;
//	Link( pos_t pa, pos_t pb ): p1(pa), p2(pb), type(LT_Weak),orient(OR_ROW)
//	{}
	friend bool operator == ( const Link& lA, const Link& lB )
	{
		if( lA.p1 == lB.p1 && lA.p2 == lB.p2 )
			return true;
		if( lA.p1 == lB.p2 && lA.p2 == lB.p1 )
			return true;
		return false;
	}

	friend std::ostream& operator << ( std::ostream& s, const Link& l )
	{
		s << '{' << (l.type==LT_Strong ? 'S' : 'W') << ',' << l.p1 << "-" << l.p2 << ',' << GetString( l.orient ) <<  '}';
		return s;
	}
};
//----------------------------------------------------------------------------
/// Searches from pos \c pos for all the weak links based on value \c val. Result is added to \c v_wl
void
FindWeakLinks( const Grid& g, value_t val, pos_t current_pos, EN_ORIENTATION orient, std::vector<Link>& v_wl )
{
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
		std::cout << "i=" << (int)i << " pos=" << c.GetPos() << '\n';
		if( c.HasCandidate( val ) && c.GetPos() != current_pos )
		{
			if( orient == OR_BLK )                                  // if BLK, then we must make sure it is not on same row/col
			{
				if( current_pos.first != c.GetPos().first && current_pos.second != c.GetPos().second )
					v_temp.push_back( i );
			}
			else
				v_temp.push_back( i );                              // if not, just add it
		}
	}
	std::cout << "or=" << GetString(orient) << " v_temp size="<<v_temp.size() << '\n';

	if( v_temp.size() > 2 )               // to be a weak link, we must have more than 2 cells with that value as candidates
		for( const auto& i: v_temp )
		{
			const Cell& c = v1d.GetCell( i );
			v_wl.push_back( Link{ current_pos, c.GetPos(), LT_Weak, orient } );
		}

//	std::cout << "after WeakLink search from pos " << current_pos << " with value -" << (int)val << "- with orientation " << GetString( orient ) << '\n';
	PrintVector( v_wl, "WeakLink positions" );
}
//----------------------------------------------------------------------------
std::vector<Link>
FindAllWeakLinks( const Grid& g, value_t val, pos_t current_pos, EN_ORIENTATION current_or )
{
	std::vector<Link> v_wl;
	if( current_or != OR_ROW )
		FindWeakLinks( g, val, current_pos, OR_ROW, v_wl );
	if( current_or != OR_COL )
		FindWeakLinks( g, val, current_pos, OR_COL, v_wl );
	if( current_or != OR_BLK )
		FindWeakLinks( g, val, current_pos, OR_BLK, v_wl );
	auto v_wl2 = VectorRemoveDupes( v_wl );

	PrintVector( v_wl2, "FindAllWeakLinks" );
	return v_wl2;
}
//----------------------------------------------------------------------------
/// Finds in the grid \c g all the strong links for value \c val and orientation \c orient
void
FindStrongLinks( value_t val, EN_ORIENTATION orient, const Grid& g, std::vector<Link>& v_link )
{
	for( index_t i=0; i<9; i++ )  // for each row/col/block
	{
		PRINT_MAIN_IDX(orient);
		View_1Dim_c v1d = g.GetView( orient, i );
		CandMap candMap;
		for( index_t col1=0; col1<8; col1++ )   // for each cell in the view
		{
			const Cell& cell1 = v1d.GetCell(col1);
			if( cell1.HasCandidate( val ) && candMap.Has( val ) )
			{
				uchar c = 0;
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
/// Vertex datatype, with BGL. Holds a cell positions
struct GraphNode
{
	pos_t pos;
};
/// Edge datatype
struct GraphEdge
{
	En_LinkType link_type;
};
//-------------------------------------------------------------------
/// A graph datatype, with BGL
typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::bidirectionalS,
	GraphNode,
	GraphEdge
	> graph_t;

typedef typename boost::graph_traits<graph_t>::vertex_descriptor vertex_t;

//----------------------------------------------------------------------------
/// A cycle is associated with a value and a set of links. We store this as a vector of positions associated with a link type
struct Cycle
{
//	value_t cycle_value;
	std::vector<std::pair<pos_t,En_LinkType>> v_links;

	friend std::ostream& operator << ( std::ostream& s, const Cycle& cy )
	{
		s << "Cycle: size=" << cy.v_links.size() <<'\n';
		for( const auto& p: cy.v_links )
			s << (p.second == LT_Strong ? 'S' : 'W') << '-' << p.first << ", ";
		s << '\n';
		return s;
	}

};
//----------------------------------------------------------------------------
// Used to store, when we find a cycle, in which position of the link is the original position
// enum En_FoundCycleLink { FCL_NoCycle, FCL_p1, FCL_p2 };

//----------------------------------------------------------------------------
/// Called when we have found a cycle, the link \c wl holds the initial position, \c fcl tells us which end
void
AddNewCycle( const Grid& g, vertex_t current_node, const graph_t& graph, const Link& link, std::vector<Cycle>& v_cycles )
{
//	assert( fcl != FCL_NoCycle );

	Cycle c;
//	c.cycle_value = val;
	std::cout << "NEW CYCLE: link:" << link << '\n';
	std::cout << "current node=" << graph[current_node].pos << '\n';

	c.v_links.push_back( std::make_pair( graph[current_node].pos, link.type ) );

	bool stop_loop = true;
	do
	{
		stop_loop = true;
		auto in_ed = boost::in_edges( current_node, graph );
		assert( in_ed.second - in_ed.first < 2 );                     // just to make sure we have 0 or 1 input edge

		if( in_ed.second - in_ed.first == 1 )                         // if one input edge only
		{                                                             // then we add the previous node
			auto edge = *in_ed.first;
			auto prev_node = boost::source( edge, graph );
			c.v_links.push_back( std::make_pair( graph[prev_node].pos, graph[edge].link_type )) ;
			current_node = prev_node;
			stop_loop = false;
		}
//		else // no input edges: means we made it to the top of the tree


	}
	while( !stop_loop );


	std::cout << "cycle added: " << c;

	v_cycles.push_back( c );
}
//----------------------------------------------------------------------------
vertex_t
AddLink2Graph( graph_t& graph, vertex_t current_node, const Link& wl )
{
	auto new_vertex = boost::add_vertex( graph );

	if( graph[current_node].pos == wl.p1 )
		graph[new_vertex].pos = wl.p2;
	else
		graph[new_vertex].pos = wl.p1;

	auto edge = boost::add_edge( current_node, new_vertex, graph ).first;
	graph[edge].link_type = wl.type;
	return new_vertex;
}
//----------------------------------------------------------------------------
/// Returns true if \c link can be found in \c graph
bool
LinkIsInGraph( const graph_t& graph, const Link& link )
{
	auto edges = boost::edges( graph );                  // get all the edges of the tree
	for( ;edges.first != edges.second; edges.first++ )   // iterate
	{
		auto e = *edges.first;
		auto n_s = boost::source( e, graph );
		auto n_t = boost::target( e, graph );
		auto pos_s = graph[n_s].pos;
		auto pos_t = graph[n_t].pos;
		if(
			( pos_s == link.p1 && pos_t == link.p2 )
			||
			( pos_s == link.p2 && pos_t == link.p1 )
		)
			return true;
	}
	return false;
}
//----------------------------------------------------------------------------
/// recursive function. Explores the \c current_node, coming from a link of orientation \c current_or.
/// Stops when an encountered link holds the \c initial_pos
void
FindNodes(
	const Grid&              g,
	value_t                  val,
	pos_t                    initial_pos,    ///< initial pos, needed because if we find it, then we have a cycle
	vertex_t                 current_node,
	EN_ORIENTATION           current_or,
	graph_t&                 graph,
	const std::vector<Link>& v_StrongLinks,
	std::vector<Cycle>&      v_cycles        ///< output cycles
)
{
	static int iter;
//	static int Nb_SL;
//	static int Nb_WL;
	auto current_pos = graph[current_node].pos;
	std::cout << "\n* FindNodes start: val=" << (int)val << " initial-pos=" << initial_pos << " current-pos=" << graph[current_node].pos << " iter " << iter++ << '\n';

	auto v_links = FindAllWeakLinks( g, val, graph[current_node].pos, current_or );
	std::cout << "we have " << v_links.size() << " weak links\n";

	// get previous position
	auto pair_edge_it = boost::in_edges( current_node, graph );
	assert( pair_edge_it.second - pair_edge_it.first == 1 );
	auto previous_node = boost::source( *pair_edge_it.first, graph );
	std::cout << "PREVIOUS="<< graph[previous_node].pos << '\n';

	for( const auto& sl: v_StrongLinks )
		if(
			( sl.p1 == graph[current_node].pos && sl.p2 != graph[previous_node].pos )
			||
			( sl.p2 == graph[current_node].pos && sl.p1 != graph[previous_node].pos )
		)
		{
			std::cout << "adding Strong link: " << sl << '\n';
			v_links.push_back( sl );
		}

	PrintVector( v_links, "before enumerating" );
	for( const auto& link: v_links )
	{
		std::cout << "FindNodes: considering link: " << link << '\n';

/*		En_FoundCycleLink fcl = FCL_NoCycle;
		if( link.p1 == initial_pos && link.p2 == current_pos )
			fcl = FCL_p1;
		if( link.p2 == initial_pos && link.p1 == current_pos )
			fcl = FCL_p2;*/

//		if( fcl != FCL_NoCycle )                                  // if we find the initial node
		if( link == Link{ initial_pos,current_pos } )             // if we find the initial node
		{                                                         // in the link, then this
			std::cout << " target=initial, found cycle !\n";
			AddNewCycle( g, current_node, graph, link, v_cycles );         // means that we have found a cycle !
		}
		else
		{
			std::cout << " target!=initial\n";
			if( !LinkIsInGraph( graph, link ) )
			{
/*				if( link.type == LT_Weak )
					Nb_WL++;
				else
					Nb_SL++;*/
				auto new_node = AddLink2Graph( graph, current_node, link );
				FindNodes( g, current_node, initial_pos, new_node, current_or, graph, v_StrongLinks, v_cycles );
			}
		}
	}

//	PrintVector( v_wl, "List of weak link positions with dupes Removed" );
}
//----------------------------------------------------------------------------
/// Searches all the graphs we have build to make sure that all the strong links have been considered.
/// Returns true if the link is found in one of the graphs.
bool
ExploreOtherGraphs( const std::vector<Link>& v_StrongLinks, size_t& idx, const std::vector<graph_t>& v_graph )
{
	bool Found = false;
	do
	{
		Found = false;
		idx++;
		std::cout << "ExploreOtherGraphs: idx= "<<idx << '\n';
		for( const graph_t g: v_graph )              // we check for all the previous graphs
			if( LinkIsInGraph( g, v_StrongLinks[idx] ) ) // and see if they hold the link
				Found = true;
		std::cout << "found=" << Found << '\n';
	}
	while( Found == false && idx+2 < v_StrongLinks.size() );
	return Found;
}
//----------------------------------------------------------------------------
/// Helper function for X_Cycles()
/**
Iterates through the strong links for the given value and sees if a cycle to the initial position can be found
*/
std::vector<Cycle>
FindCycles(
	const Grid&              g,
	value_t                  val,
	const std::vector<Link>& v_StrongLinks
)
{
	assert( v_StrongLinks.size() > 1 );
	assert( v_StrongLinks[0].type == LT_Strong );

	std::vector<Cycle> v_cycles;
	std::vector<graph_t> v_graph;
	size_t idx = 0;
	bool done = false;
	do
	{
		done = false;

		auto current_link = v_StrongLinks[idx];
		pos_t          initial_pos = current_link.p1;
		pos_t          current_pos = current_link.p2;
		EN_ORIENTATION current_or  = current_link.orient;

		std::cout << "VALUE " << (int)val << ": start Cycle Search with SL: " << initial_pos << " - " << current_pos <<'\n';

		graph_t graph;
		auto init_vertex = boost::add_vertex( graph );
		graph[init_vertex].pos = initial_pos;

		auto curr_vertex = boost::add_vertex( graph );
		graph[curr_vertex].pos = current_pos;

		auto ed = boost::add_edge( init_vertex, curr_vertex, graph ).first;
		graph[ed].link_type = LT_Strong;

		FindNodes( g, val, initial_pos, curr_vertex, current_or, graph, v_StrongLinks, v_cycles );
		PrintVector( v_cycles, "v_cycles" );

		std::cout << "adding graph to vector, size=" << v_graph.size()+1 << '\n';
		v_graph.push_back( graph );
		if( idx+1 >= v_StrongLinks.size() )
			done = true;
		else
			done = ExploreOtherGraphs( v_StrongLinks, idx, v_graph );
	}
	while( !done );

	return v_cycles;
}
//----------------------------------------------------------------------------
/// X Cycles algorithm (WIP)
/**
This enables removing some candidates

See http://www.sudokuwiki.org/X_Cycles

TODO: we only explore the first strong link! But other cycles could be found by iterating over the other ones

*/
bool
X_Cycles( Grid& g )
{
	for( value_t v=1; v<10; v++ )             // for each possible value, get strong links, then search cycles
	{
		std::cout << "\nX_Cycles: process value " << (int)v << '\n';
		auto v_sl = FindStrongLinks( v, g );
		PrintVector( v_sl, "Strong Links set" );
//		const auto& sl_vect = msl.GetSLvect(v);
//		std::cout << "considering value " << (int)v << ", vector has " << sl_vect.size() << " values\n";
		if( v_sl.size() > 1 )
		{
			auto v_cyc = FindCycles( g, v, v_sl );
			std::cout << "VALUE=" << (int)v << " cycles:\n";
			PrintVector( v_cyc, "v_cyc" );
		}
	}

	return false;
}
//----------------------------------------------------------------------------
