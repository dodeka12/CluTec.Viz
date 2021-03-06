////////////////////////////////////////////////////////////////////////////////////////////////////
// project:   CluTec.Viz.Parse
// file:      OCOperator.cpp
//
// summary:   Implements the oc operator class
//
//            Copyright (c) 2019 by Christian Perwass.
//
//            This file is part of the CluTecLib library.
//
//            The CluTecLib library is free software: you can redistribute it and / or modify
//            it under the terms of the GNU Lesser General Public License as published by
//            the Free Software Foundation, either version 3 of the License, or
//            (at your option) any later version.
//
//            The CluTecLib library is distributed in the hope that it will be useful,
//            but WITHOUT ANY WARRANTY; without even the implied warranty of
//            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//            GNU Lesser General Public License for more details.
//
//            You should have received a copy of the GNU Lesser General Public License
//            along with the CluTecLib library.
//            If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// OCOperator.cpp: Implementierung der Klasse COCOperator.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "OCIDSymDef.h"
#include "OCOperator.h"


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

COCOperator::COCOperator()
{
	m_iOpLevelBinary = -1;
	m_iOpLevelUnary = -1;
	m_pCodeOp = 0;
	m_bRightToLeft = true;
	m_bVoidOp = false;
}

COCOperator::~COCOperator()
{

}

//////////////////////////////////////////////////////////////////////
/// Parse Loop

bool COCOperator::Parse(CParseBase& rParse, CCodeElementList& rCode)
{
	if (!m_pCodeOp)
	{
		rParse.SetError(CPB_UNKNOWN);
		return false;
	}

	int iCurOpLevel = rParse.GetCurOpLevel();

	// Check if operator is first element in CodeElementList.
	// If yes, then operator is unary.
	// Operator is also unary if it can be unary and does not return a value.
	if (!rCode.Count() || (m_iOpLevelUnary >= 0 && m_bVoidOp))
	{
		if (m_iOpLevelUnary == -1)
		{
			// Operator cannot be used in unary context.
			rParse.SetError(CPB_NONUNARYOP);
			return false;
		}

		// Make Operator Level of this operator current.
		rParse.PushOpLevel(m_iOpLevelUnary);
	}
	else
	{
		// Check whether operator can be binary.
		if (m_iOpLevelBinary < 0)
		{
			// Operator cannot be used in binary context.
			rParse.SetError(CPB_NONBINARYOP);
			return false;
		}

		// if another operator is already current and this operator
		// binds weaker than the previous one, or if
		// this operator binds as strong but is evaluated from
		// left to right, then parse this
		// operator at a higher level.
		if (iCurOpLevel >= 0 && m_iOpLevelBinary > iCurOpLevel ||
			(m_iOpLevelBinary == iCurOpLevel && !m_bRightToLeft))
		{
			// Returning with false and setting error
			// to CPB_WEAKEROP results in a return to
			// the next higher operator level.
			rParse.SetError(CPB_WEAKEROP);
			return false; 
		}
		
		// Make Operator Level of this operator current.
		rParse.PushOpLevel(m_iOpLevelBinary);
	}

	bool bRes = true;
	CCodeElementList *pCodeList;

	if (!(pCodeList = new CCodeElementList))
	{
		rParse.SetError(CPB_OUTOFMEMORY);
		return false;
	}

	// First evaluate RHS of Operator
	// Sub element list is owned by list
	if (!rCode.Add(*pCodeList, true))
	{
		rParse.SetError(CPB_OUTOFMEMORY);
		delete pCodeList;
		return false;
	}

	int iLine, iPos;
	rParse.GetCurLinePos(iLine, iPos);

	// Then call operator
	// Operator element not owned by list
	if (!rCode.Add(*m_pCodeOp, false, iLine, iPos))
	{
		rParse.SetError(CPB_OUTOFMEMORY);
		delete pCodeList;
		return false;
	}

	if ((unsigned char) m_csIDSymbolSpace[0] == OC_IDSYM_SPECIAL_CHAR)
		rParse.AddToCurPos(2);
	else
		rParse.AddToCurPos(1);

	if (!rParse.BasicParseLoop(pCodeList))
	{
		if (rParse.GetError() != CPB_WEAKEROP)
			bRes = false;
		else
			rParse.ClearError();
	}

	// Make operator level of one level above active
	rParse.PopOpLevel();

	return bRes;
}
