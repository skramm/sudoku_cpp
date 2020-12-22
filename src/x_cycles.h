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
\brief specific header file for X_cycles algorithm
*/

#ifndef X_CYCLES_H_
#define X_CYCLES_H_

#include "circvec.h"

//----------------------------------------------------------------------------
/// Related to Cycle
enum En_CycleType
{
	CT_undefined,
	CT_Continuous,     ///< continuous cycle, pair nb of nodes, alternate Weak and Strong links
	CT_Discont_2SL,    ///< Discontinuous cycle, odd nb of nodes, 2 chained Strong links
	CT_Discont_2WL,    ///< Discontinuous cycle, odd nb of nodes, 2 chained Weak links
	CT_Invalid
};
//----------------------------------------------------------------------------
struct CycleType
{
	En_CycleType _ctype = CT_undefined;
	int _idx = -1;
	CycleType( En_CycleType ct ) : _ctype(ct)
	{}
	CycleType( En_CycleType ct, int idx ) : _ctype(ct), _idx(idx)
	{}
};
//----------------------------------------------------------------------------
/// To avoid a meaningless boolean
enum En_LinkType
{
	LT_Strong, LT_Weak
};
//----------------------------------------------------------------------------
/// A link between two cells, also holds the second cell position
struct Link
{
	pos_t p1, p2;
	En_LinkType    _ltype;
	EN_ORIENTATION _lorient = OR_ROW;

	friend bool operator == ( const Link& lA, const Link& lB )
	{
		if( lA.p1 == lB.p1 && lA.p2 == lB.p2 )
			return true;
		if( lA.p1 == lB.p2 && lA.p2 == lB.p1 )
			return true;
		return false;
	}
	Link( pos_t pA, pos_t pB, En_LinkType lt, EN_ORIENTATION o ): p1(pA), p2(pB), _ltype(lt), _lorient(o)
	{}
//#ifdef TESTMODE
	Link( En_LinkType lt ): _ltype(lt)
	{}
//#endif
	friend std::ostream& operator << ( std::ostream& s, const Link& l )
	{
		s << '{' << (l._ltype==LT_Strong ? 'S' : 'W') << ',' << l.p1 << "-" << l.p2 << ',' << GetString( l._lorient ) <<  '}';
		return s;
	}
};

//----------------------------------------------------------------------------
/// A cycle is associated with a value and a set of links.
/// We store this as a vector of positions associated with a link type
struct Cycle: public CircVec<Link>
{
	friend std::ostream& operator << ( std::ostream& s, const Cycle& cy )
	{
		s << "Cycle: size=" << cy.size() <<'\n';
		for( size_t i=0; i<cy.size(); i++ )
			s << cy.GetElem(i) << " - ";
		s << '\n';
		return s;
	}
};
//----------------------------------------------------------------------------

bool X_Cycles( Grid& g );
CycleType GetCycleType( const Cycle& cy );

#endif
