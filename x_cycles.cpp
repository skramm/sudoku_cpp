/**
\file x_cycles.cpp
\brief X cycles
*/

#include "grid.h"
#include "header.h"
#include <boost/graph/adjacency_list.hpp>

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
		char c = 'R';
		switch( sl.orient )
		{
//			case OR_ROW: c = 'R'; break;
			case OR_COL: c = 'C'; break;
			case OR_BLK: c = 'B'; break;
		}
		s << '{' << c << ": " << sl.p1 << "-" << sl.p2 << '}';
		return s;
	}
};
//----------------------------------------------------------------------------
/// A strong link with the  correspondant value that is linked together
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
		/// Constructor. Fills the map with empty vector
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
		msl.AddLink( elem.linkValue, elem );
	return msl;
}
//----------------------------------------------------------------------------
/// Searches the grid \c g by orientation \c orient and adds to \c v_out the strong links that are found
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
/// Related do X_Cycles()
enum En_CycleType
{
	CT_Cont,           ///< continuous cycle, pair nb of nodes, alternate Weak and Strong links
	CT_Discont_2SL,    ///< Discontinuous cycle, odd nb of nodes, 2 chained Strong links
	CT_Discont_2WL     ///< Discontinuous cycle, odd nb of nodes, 2 chained Weak links
};
//----------------------------------------------------------------------------
/// To avoid a meaningless boolean
enum En_LinkType
{
	LT_Strong, LT_Weak
};
//----------------------------------------------------------------------------
/// A cycle is associated with a value and a set of links. We store this as a vector of positions associated with a link type
struct Cycle
{
	value_t cycle_value;
	std::vector<std::pair<pos_t,En_LinkType>> v_links;
};
//----------------------------------------------------------------------------
/// Searches from pos \c pos for all the weak links based on value \c val. Result is added to \c v_wl
void
FindWeakLinks( const Grid& g, value_t val, pos_t current_pos, EN_ORIENTATION orient, std::vector<pos_t>& v_wl )
{
	auto idx = current_pos.first;
	switch( orient )
	{
		case OR_COL: idx = current_pos.second;            break;
		case OR_BLK: idx =  GetBlockIndex( current_pos ); break;
		default: assert(0);
	}

	View_1Dim_c v1d = g.GetView( orient, idx );

	for( index_t i=0; i<9; i++ )
	{
		const Cell& c = v1d.GetCell( i );
		if( c.HasCandidate( val ) && c.GetPos() != current_pos )
			v_wl.push_back( c.GetPos() );
	}
	std::cout << "after WeakLink search from pos " << current_pos << " with value -" << (int)val << "- with orientation " << GetString( orient ) << '\n';
	PrintVector( v_wl, "WeakLink positions" );
}
//----------------------------------------------------------------------------
std::vector<pos_t>
FindAllWeakLinks( const Grid& g, value_t val, pos_t current_pos, EN_ORIENTATION current_or )
{
	std::vector<pos_t> v_wl;
	if( current_or != OR_ROW )
		FindWeakLinks( g, val, current_pos, OR_ROW, v_wl );
	if( current_or != OR_COL )
		FindWeakLinks( g, val, current_pos, OR_COL, v_wl );
	if( current_or != OR_BLK )
		FindWeakLinks( g, val, current_pos, OR_BLK, v_wl );
	return VectorRemoveDupes( v_wl );
}

//----------------------------------------------------------------------------
struct Link
{
	pos_t pos_source;
	pos_t pos_target;
	En_LinkType link_type;
};
//----------------------------------------------------------------------------
std::vector<Link>
FindStrongLinks( value_t val, const Grid& g )
{
}

//----------------------------------------------------------------------------
std::vector<Link>
FindAllLinks( const Grid& g, value_t val, pos_t pos, EN_ORIENTATION current_or )
{
	std::vector<Link> v_out;
	auto v_wl = FindAllWeakLinks( g, val, pos, current_or );
	for( const auto& wl: v_wl )
		v_out.push_back( Link{ pos, wl, LT_Weak } );
	return v_out;
}
//----------------------------------------------------------------------------
/// Vertex datatype for the MULTIGRID algorithm, with BGL
struct GraphNode
{
	pos_t pos;
};

struct GraphEdge
{
	En_LinkType link_type;
};

//-------------------------------------------------------------------
/// A graph datatype for the MULTIGRID algorithm, with BGL
typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::bidirectionalS,
	GraphNode,
	GraphEdge
	> graph_t;

typedef typename boost::graph_traits<graph_t>::vertex_descriptor vertex_t;

//----------------------------------------------------------------------------
/// Called when we have found a cycle, the link \c wl holds as target the initial position
void
AddNewCycle( const Grid& g, const graph_t& graph, const Link& wl, std::vector<Cycle>& v_cycles )
{

}
//----------------------------------------------------------------------------
vertex_t
AddLink2Graph( graph_t& graph, vertex_t current_node, const Link& wl )
{
	auto new_vertex = boost::add_vertex( graph );
	graph[new_vertex].pos = wl.pos_target;
	auto edge = boost::add_edge( current_node, new_vertex, graph ).first;
	graph[edge].link_type = wl.link_type;
	return new_vertex;
}
//----------------------------------------------------------------------------
/// recursive function. Returns true if nodes have been added
void
FindNodes(
	const Grid& g,
	value_t val,
	pos_t initial_pos,   ///< initial pos, needed because if we find it, then we have a cycle
	vertex_t current_node,
	EN_ORIENTATION current_or,
	graph_t& graph,
	std::vector<Cycle>& v_cycles
)
{
	auto v_links = FindAllLinks( g, val, graph[current_node].pos, current_or );
	for( const auto& link: v_links )
		if( link.pos_target == initial_pos )
		{
			AddNewCycle( g, graph, link, v_cycles );
		}
		else
		{
			auto new_node = AddLink2Graph( graph, current_node, link );
            FindNodes( g, val, initial_pos, new_node, current_or, graph, v_cycles );
		}


//	PrintVector( v_wl, "List of weak link positions with dupes Removed" );
}
//----------------------------------------------------------------------------
/// Helper function for X_Cycles()
/**
Iterates through the strong links for the given value and sees if a cycle to the initial position can be found
*/
std::vector<Cycle>
FindCycles( const Grid& g, value_t val, const std::vector<StrongLink>& sl_vect )
{
	assert( sl_vect.size() > 1 );
	pos_t          initial_pos = sl_vect[0].p1;
	pos_t          current_pos = sl_vect[0].p2;
	EN_ORIENTATION current_or  = sl_vect[0].orient;

	std::vector<Cycle> v_cycles;
	std::cout << "start Cycle Search with SL: " << initial_pos << " - " << current_pos <<'\n';
	bool LastOneStrong = true;
	size_t idx = 1;
	graph_t graph;

	auto init_vertex = boost::add_vertex( graph );
	graph[init_vertex].pos = initial_pos;

	auto curr_vertex = boost::add_vertex( graph );
	graph[curr_vertex].pos = current_pos;

	auto ed = boost::add_edge( init_vertex, curr_vertex, graph ).first;
	graph[ed].link_type = LT_Strong;

	FindNodes( g, val, initial_pos, curr_vertex, current_or, graph, v_cycles );
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
// 1 - first, get strong links
	auto msl = FindStrongLinks( g );
	std::cout << "msl:\n" << msl;

// 2 - then, check if they can form a cycle
	for( value_t v=1; v<10; v++ )
	{
		auto v_sl = FindStrongLinks( v, g );
//		const auto& sl_vect = msl.GetSLvect(v);
//		std::cout << "considering value " << (int)v << ", vector has " << sl_vect.size() << " values\n";
		if( v_sl.size() > 1 )
			auto v_cyc = FindCycles( g, v, v_sl );
	}

	return false;
}
//----------------------------------------------------------------------------
