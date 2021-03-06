////////////////////////////////////////////////////////////////////////////////////////////////////
// project:   CluTec.Viz.Parse
// file:      OCLabel.cpp
//
// summary:   Implements the oc label class
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

// OCLabel.cpp: Implementierung der Klasse COCLabel.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ParseBase.h"
#include "CodeElementList.h"
#include "CodeLabel.h"
#include "OCLabel.h"
#include "OCIDSymDef.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

COCLabel::COCLabel()
{
}

COCLabel::~COCLabel()
{
}

//////////////////////////////////////////////////////////////////////
/// Parse

bool COCLabel::Parse(CParseBase& rParse, CCodeElementList& rCodeList)
{
	CStrMem csElement;

	if (!rParse.ReadSymSubSpaceElement(csElement, m_csSymbolSpace))
	{
		rParse.SetError(CPB_INVALIDSYNTAX);
		return false;
	}

	CCodeFunction* pFunc;

	// If label is a function then carry on with appropriate parsing.
	if ((pFunc = rParse.GetFunc(csElement)) != 0)
	{
		return ParseFunc(pFunc, rParse, rCodeList);
	}

	CCodeLabel* pCode;

	if (!(pCode = new CCodeLabel))
	{
		rParse.SetError(CPB_OUTOFMEMORY);
		return false;
	}

	if (!pCode->SetLabel(csElement))
	{
		rParse.SetError(CPB_UNKNOWN);
		delete pCode;
		return false;
	}

	int iLine, iPos;
	rParse.GetCurLinePos(iLine, iPos);

	// Add Code Element to end of Code Element List.
	if (!rCodeList.Add(*pCode, true, iLine, iPos))
	{
		rParse.SetError(CPB_OUTOFMEMORY);
		delete pCode;
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// If Label refers to a function then this function is called
// to parse the rest.

bool COCLabel::ParseFunc(CCodeElement* pFunc, CParseBase& rParse, CCodeElementList& rCode)
{
	if (!pFunc)
	{
		rParse.SetError(CPB_UNKNOWN);
		return false;
	}

	bool bRes = true;

	// Make Operator Level of function current.
	// function is treated as a unary operator that binds strongest.
	rParse.PushOpLevel(OPLEVEL_FUNCTION);

	CCodeElementList* pCodeList;

	if (!(pCodeList = new CCodeElementList))
	{
		rParse.SetError(CPB_OUTOFMEMORY);
		return false;
	}

	// First evaluate RHS of function, i.e. parameter list.
	// Sub element list is owned by list
	if (!rCode.Add(*pCodeList, true))
	{
		rParse.SetError(CPB_OUTOFMEMORY);
		delete pCodeList;
		return false;
	}

	int iLine, iPos;
	rParse.GetCurLinePos(iLine, iPos);

	// Then call function
	// Operator element not owned by list
	if (!rCode.Add(*pFunc, false, iLine, iPos))
	{
		rParse.SetError(CPB_OUTOFMEMORY);
		return false;
	}

	CObjectClass* pOC = rParse.GetNextObjectClass();
	if (!pOC)
	{
		rParse.SetError(CPB_MISSINGORB);
		return false;
	}

	if (pOC->m_csIDSymbolSpace != OC_IDSYM_CALL_STR)
	{
		rParse.SetError(CPB_MISSINGORB);
		return false;
	}
	rParse.AddToCurPos(2);

	if (!rParse.BasicParseLoop(pCodeList))
	{
		if (rParse.GetError() != CPB_WEAKEROP)
		{
			bRes = false;
		}
		else
		{
			rParse.ClearError();
		}
	}

	// Make operator level of one level above active
	rParse.PopOpLevel();

	return bRes;
}
