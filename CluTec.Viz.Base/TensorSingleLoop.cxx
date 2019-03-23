////////////////////////////////////////////////////////////////////////////////////////////////////
// project:   CluTec.Viz.Base
// file:      TensorSingleLoop.cxx
//
// summary:   tensor single loop class
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

#include "TensorSingleLoop.h"

////////////////////////////////////////////////////////////////////////////////////
/// Single Loop Class Member Functions
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
/// Constructor

template<class CType>
CTensorSingleLoop<CType>::CTensorSingleLoop()
{
	m_pTIdx = 0;
}

////////////////////////////////////////////////////////////////////////////////////
/// Set Tensor Idx Instance

template<class CType>
void CTensorSingleLoop<CType>::Set(CTensorIdx<CType>& rT)
throw(CCluException)
{
	if (!rT.IsValid())
	{
		m_pTIdx = 0;
		m_mFreeIdx.Set(0);
		throw InvalidTensorIdxRef();
	}

	m_pTIdx = &rT;
	m_mFreeIdx.Set(0);

	const typename CTensorIdx<CType>::TFreeIdxMap& mapFreeIdx = rT.GetFreeIdxMap();
	typename CTensorIdx<CType>::TFreeIdxMap::const_iterator it_El, it_End;

	it_End = mapFreeIdx.end();

	// Store free indices in the order they appear in the map
	for (it_El = mapFreeIdx.begin();
	     it_El != it_End;
	     ++it_El)
	{
		m_mFreeIdx.Add(1);
		m_mFreeIdx.Last() = it_El->first;
	}
}

////////////////////////////////////////////////////////////////////////////////////
// Get number of steps at particular loop level.

template<class CType>
int CTensorSingleLoop<CType>::StepCount(int iLevel)
throw(CCluException)
{
	if ((iLevel < 0) || (iLevel >= int(m_mFreeIdx.Count())))
	{
		throw InvalidLoopLevel();
	}

	if (!m_pTIdx)
	{
		throw InvalidTensorIdxRef();
	}

	return m_pTIdx->GetStepCount(m_mFreeIdx[iLevel]);
}

////////////////////////////////////////////////////////////////////////////////////
// Get number of steps at particular loop level.

template<class CType>
int CTensorSingleLoop<CType>::TotalStepCount()
{
	if (!m_pTIdx)
	{
		throw InvalidTensorIdxRef();
	}

	int iIdx, iCnt = int(m_mFreeIdx.Count());
	int iTotalStepCount = 0;

	for (iIdx = 0; iIdx < iCnt; ++iIdx)
	{
		iTotalStepCount += m_pTIdx->GetStepCount(m_mFreeIdx[iIdx]);
	}

	return iTotalStepCount;
}

////////////////////////////////////////////////////////////////////////////////////
// Initialize Loops. Returns number of cascaded loops.

template<class CType>
int CTensorSingleLoop<CType>::Init()
throw(CCluException)
{
	if (!m_pTIdx)
	{
		throw InvalidTensorIdxRef();
	}

	m_pTIdx->InitAllLoops();

	return int(m_mFreeIdx.Count());
}

////////////////////////////////////////////////////////////////////////////////////
// Step loop. Returns loop level, in which step was made.
// Loop level starts at zero for most inner loop.
// If loop level is -1, all loops are finished.
// No exceptions generated by this function!

template<class CType>
int CTensorSingleLoop<CType>::Step()
{
	int iLevel = 0;

	if (m_mFreeIdx.Count() == 0)
	{
		return -1;
	}

	while (!m_pTIdx->StepLoop(m_mFreeIdx[iLevel]))
	{
		m_pTIdx->InitLoop(m_mFreeIdx[iLevel]);
		if (iLevel == m_mFreeIdx.Count() - 1)
		{
			return -1;	// All loops finished
		}
		++iLevel;
	}

	return iLevel;
}
