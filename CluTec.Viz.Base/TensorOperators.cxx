////////////////////////////////////////////////////////////////////////////////////////////////////
// project:   CluTec.Viz.Base
// file:      TensorOperators.cxx
//
// summary:   tensor operators class
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

// Functions that deal with tensors.

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "TensorData.h"
#include "TensorIdx.h"
#include "TensorSingleLoop.h"
#include "TensorOperators.h"

////////////////////////////////////////////////////////////////////////////////////
/// Create a tensor from a tensor index

template<class CType>
bool MakeTensor(CTensorData<CType>& rT, CTensorIdx<CType>& rTIdx)
{
	CTensorSingleLoop<CType> Loop;

	Loop.Set(rTIdx);
	int iLoopCount = Loop.Init();

	if (iLoopCount > 0)
	{
		CType* pData;
		int iDim, iLoop, iPos;
		Mem<int> mDim;
		mDim.Set(iLoopCount);
		for (iLoop = iLoopCount - 1, iDim = 0; iLoop >= 0; iLoop--, iDim++)
		{
			mDim[iDim] = Loop.StepCount(iLoop);
		}

		rT.Reset(mDim);
		pData = rT.Data();
		iPos  = 0;

		do
		{
			pData[iPos++] = Loop.GetRef();
		}
		while (Loop.Step() >= 0);
	}
	else
	{
		rT = rTIdx.GetTensorData();
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Set components of tensor to scalar value

template<class CType>
bool SetTensorComps(CTensorIdx<CType>& rTIdx, CType dVal)
{
	CTensorSingleLoop<CType> Loop;

	Loop.Set(rTIdx);
	Loop.Init();

	do
	{
		Loop.GetRef() = dVal;
	}
	while (Loop.Step() >= 0);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Assign components of one tensor to components of another one.
// The LHS tensor may have more free indices than the RHS tensor.
// However, all free indices of the RHS tensor have to appear also
// in the LHS tensor.

template<class CType>
bool AssignTensorComps(CTensorIdx<CType>& rTTrg, CTensorIdx<CType>& rTSrc)
{
	CTensorDoubleLoop<CType> xLoop;
	CTensorData<CType> T;
	CTensorIdx<CType> TIdx;

	try
	{
		// Check whether free indices on RHS are contained in list
		// of free indices on LHS.
		const CTensorIdx<CType>::TFreeIdxMap& mapSrcFreeIdx = rTSrc.GetFreeIdxMap();
		const CTensorIdx<CType>::TFreeIdxMap& mapTrgFreeIdx = rTTrg.GetFreeIdxMap();
		CTensorIdx<CType>::TFreeIdxMap::const_iterator itElSrc, itElSrcEnd = mapSrcFreeIdx.end();

		// Loop over all RHS free indices
		for (itElSrc = mapSrcFreeIdx.cbegin(); itElSrc != itElSrcEnd; ++itElSrc)
		{
			// if a RHS free index is not contained in the LHS free index map
			// then this is an error.
			if (mapTrgFreeIdx.find(itElSrc->first) == mapTrgFreeIdx.end())
			{
				return false;
			}
		}

		CTensorData<CType>& rTDataTrg = rTTrg.GetTensorData();
		CTensorData<CType>& rTDataSrc = rTSrc.GetTensorData();

		// Check whether source and target use same tensor data
		if (&rTDataTrg == &rTDataSrc)
		{
			// if yes copy source and use it instead of given source
			T = rTDataSrc;

			// Create new tensor index
			TIdx.Set(T, rTSrc);

			xLoop.Set(rTTrg, TIdx);
		}
		else
		{
			xLoop.Set(rTTrg, rTSrc);
		}

		xLoop.Init();
		do
		{
			xLoop.GetLeftRef() = xLoop.GetRightRef();
		}
		while (xLoop.Step() >= 0);
	}
	catch (...)
	{
		return false;
	}

	//int iLoop, iTrgLoopCount, iSrcLoopCount;
	//CTensorSingleLoop<CType> LoopSrc, LoopTrg;
	//CTensorData<CType> T;
	//CTensorIdx<CType> TIdx;

	//try
	//{
	//	CTensorData<CType>& rTDataTrg = rTTrg.GetTensorData();
	//	CTensorData<CType>& rTDataSrc = rTSrc.GetTensorData();

	//	// Check whether source and target use same tensor data
	//	if (&rTDataTrg == &rTDataSrc)
	//	{
	//		// if yes copy source and use it instead of given source
	//		T = rTDataSrc;

	//		// Create new tensor index
	//		TIdx.Set(T, rTSrc);

	//		// Use it
	//		LoopSrc.Set(TIdx);
	//	}
	//	else
	//	{
	//		LoopSrc.Set(rTSrc);
	//	}

	//	LoopTrg.Set(rTTrg);

	//	iTrgLoopCount = LoopTrg.Init();
	//	iSrcLoopCount = LoopSrc.Init();

	//	if (iTrgLoopCount != iSrcLoopCount)
	//		return false;

	//	const Mem<int>& mFITrg = LoopTrg.GetFreeIdxList();
	//	const Mem<int>& mFISrc = LoopSrc.GetFreeIdxList();

	//	for (iLoop = 0; iLoop < iTrgLoopCount; iLoop++)
	//	{
	//		if (LoopTrg.StepCount(iLoop) != LoopSrc.StepCount(iLoop) ||
	//			mFITrg[iLoop] != mFISrc[iLoop])
	//			return false;
	//	}

	//	do
	//	{
	//		LoopTrg.GetRef() = LoopSrc.GetRef();
	//	}
	//	while (LoopTrg.Step() >= 0 && LoopSrc.Step() >= 0);
	//}
	//catch ( ... )
	//{
	//	return false;
	//}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Negate Tensor

template<class CType>
bool TensorIdxNegate(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft)
{
	try
	{
		if (!MakeTensor(rTResult, rTLeft))
		{
			return false;
		}

		int iLoop, iLoopCount;
		Mem<int> mIdx;
		const typename CTensorIdx<CType>::TFreeIdxMap& mapFreeIdx = rTLeft.GetFreeIdxMap();
		typename CTensorIdx<CType>::TFreeIdxMap::const_iterator it_El, it_End;

		mIdx.Set(iLoopCount = int(mapFreeIdx.size()));
		iLoop  = iLoopCount - 1;
		it_End = mapFreeIdx.end();
		for (it_El = mapFreeIdx.begin();
		     it_El != it_End;
		     ++it_El, --iLoop)
		{
			mIdx[iLoop] = it_El->first;
		}

		rTIdxResult.Set(rTResult, mIdx);

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			pData[iPos] = -pData[iPos];
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Negate Tensor

template<class CType>
bool TensorNegate(CTensorData<CType>& rTResult, CTensorData<CType>& rT)
{
	try
	{
		rTResult = rT;

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			pData[iPos] = -pData[iPos];
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Invert Tensor Components if possible. Zero components remain zero.

template<class CType>
bool TensorIdxInvertComps(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, const CType& dPrec)
{
	try
	{
		if (!MakeTensor(rTResult, rTLeft))
		{
			return false;
		}

		int iLoop, iLoopCount;
		Mem<int> mIdx;
		const typename CTensorIdx<CType>::TFreeIdxMap& mapFreeIdx = rTLeft.GetFreeIdxMap();
		typename CTensorIdx<CType>::TFreeIdxMap::const_iterator it_El, it_End;

		mIdx.Set(iLoopCount = int(mapFreeIdx.size()));
		iLoop  = iLoopCount - 1;
		it_End = mapFreeIdx.end();
		for (it_El = mapFreeIdx.begin();
		     it_El != it_End;
		     ++it_El, --iLoop)
		{
			mIdx[iLoop] = it_El->first;
		}

		rTIdxResult.Set(rTResult, mIdx);

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			if (!::IsZero(pData[iPos], dPrec))
			{
				pData[iPos] = CType(1) / pData[iPos];
			}
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Invert tensor components if possible. Zero components remain zero.

template<class CType>
bool TensorInvertComps(CTensorData<CType>& rTResult, CTensorData<CType>& rT, const CType& dPrec)
{
	try
	{
		rTResult = rT;

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			if (!::IsZero(pData[iPos], dPrec))
			{
				pData[iPos] = CType(1) / pData[iPos];
			}
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Product with contraction

template<class CType>
bool TensorProductContract(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, CTensorIdx<CType>& rTRight)
{
	CTensorContractLoop<CType> Loop;

	try
	{
		Loop.Set(rTResult, rTLeft, rTRight);
		rTIdxResult = Loop.GetResultTensorIdx();
		Loop.Init();

		do
		{
			Loop.GetResultRef() += Loop.GetLeftRef() * Loop.GetRightRef();
		}
		while (Loop.Step() >= 0);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Product without contraction

template<class CType>
bool TensorProductPoint(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, CTensorIdx<CType>& rTRight)
{
	CTensorPointLoop<CType> Loop;

	try
	{
		Loop.Set(rTResult, rTLeft, rTRight);
		rTIdxResult = Loop.GetResultTensorIdx();
		Loop.Init();

		do
		{
			Loop.GetResultRef() = Loop.GetLeftRef() * Loop.GetRightRef();
		}
		while (Loop.Step() >= 0);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Product with scalar

template<class CType>
bool TensorIdxProductScalar(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, const CType& dVal)
{
	try
	{
		if (!MakeTensor(rTResult, rTLeft))
		{
			return false;
		}

		int iLoop, iLoopCount;
		Mem<int> mIdx;
		const typename CTensorIdx<CType>::TFreeIdxMap& mapFreeIdx = rTLeft.GetFreeIdxMap();
		typename CTensorIdx<CType>::TFreeIdxMap::const_iterator it_El, it_End;

		mIdx.Set(iLoopCount = int(mapFreeIdx.size()));
		iLoop  = iLoopCount - 1;
		it_End = mapFreeIdx.end();
		for (it_El = mapFreeIdx.begin();
		     it_El != it_End;
		     ++it_El, --iLoop)
		{
			mIdx[iLoop] = it_El->first;
		}

		rTIdxResult.Set(rTResult, mIdx);

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			pData[iPos] *= dVal;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Product with scalar

template<class CType>
bool TensorProductScalar(CTensorData<CType>& rTResult, CTensorData<CType>& rT, const CType& dVal)
{
	try
	{
		rTResult = rT;

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			pData[iPos] *= dVal;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Addition without contraction

template<class CType>
bool TensorAddPoint(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, CTensorIdx<CType>& rTRight)
{
	CTensorPointLoop<CType> Loop;

	try
	{
		Loop.Set(rTResult, rTLeft, rTRight);
		rTIdxResult = Loop.GetResultTensorIdx();
		Loop.Init();

		do
		{
			Loop.GetResultRef() = Loop.GetLeftRef() + Loop.GetRightRef();
		}
		while (Loop.Step() >= 0);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Addition with scalar

template<class CType>
bool TensorIdxAddScalar(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, const CType& dVal)
{
	try
	{
		if (!MakeTensor(rTResult, rTLeft))
		{
			return false;
		}

		int iLoop, iLoopCount;
		Mem<int> mIdx;
		const typename CTensorIdx<CType>::TFreeIdxMap& mapFreeIdx = rTLeft.GetFreeIdxMap();
		typename CTensorIdx<CType>::TFreeIdxMap::const_iterator it_El, it_End;

		mIdx.Set(iLoopCount = int(mapFreeIdx.size()));
		iLoop  = iLoopCount - 1;
		it_End = mapFreeIdx.end();
		for (it_El = mapFreeIdx.begin();
		     it_El != it_End;
		     ++it_El, --iLoop)
		{
			mIdx[iLoop] = it_El->first;
		}

		rTIdxResult.Set(rTResult, mIdx);

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			pData[iPos] += dVal;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Addition with scalar

template<class CType>
bool TensorAddScalar(CTensorData<CType>& rTResult, CTensorData<CType>& rT, const CType& dVal)
{
	try
	{
		rTResult = rT;

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			pData[iPos] += dVal;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Subtraction without contraction

template<class CType>
bool TensorSubPoint(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, CTensorIdx<CType>& rTRight)
{
	CTensorPointLoop<CType> Loop;

	try
	{
		Loop.Set(rTResult, rTLeft, rTRight);
		rTIdxResult = Loop.GetResultTensorIdx();
		Loop.Init();

		do
		{
			Loop.GetResultRef() = Loop.GetLeftRef() - Loop.GetRightRef();
		}
		while (Loop.Step() >= 0);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Subtraction with scalar from right

template<class CType>
bool TensorIdxSubRightScalar(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, const CType& dVal)
{
	try
	{
		if (!MakeTensor(rTResult, rTLeft))
		{
			return false;
		}

		int iLoop, iLoopCount;
		Mem<int> mIdx;
		const typename CTensorIdx<CType>::TFreeIdxMap& mapFreeIdx = rTLeft.GetFreeIdxMap();
		typename CTensorIdx<CType>::TFreeIdxMap::const_iterator it_El, it_End;

		mIdx.Set(iLoopCount = int(mapFreeIdx.size()));
		iLoop  = iLoopCount - 1;
		it_End = mapFreeIdx.end();
		for (it_El = mapFreeIdx.begin();
		     it_El != it_End;
		     ++it_El, --iLoop)
		{
			mIdx[iLoop] = it_El->first;
		}

		rTIdxResult.Set(rTResult, mIdx);

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			pData[iPos] -= dVal;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Subtraction with scalar from right

template<class CType>
bool TensorSubRightScalar(CTensorData<CType>& rTResult, CTensorData<CType>& rT, const CType& dVal)
{
	try
	{
		rTResult = rT;

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			pData[iPos] -= dVal;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Subtraction from scalar on left

template<class CType>
bool TensorIdxSubLeftScalar(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, const CType& dVal)
{
	try
	{
		if (!MakeTensor(rTResult, rTLeft))
		{
			return false;
		}

		int iLoop, iLoopCount;
		Mem<int> mIdx;
		const typename CTensorIdx<CType>::TFreeIdxMap& mapFreeIdx = rTLeft.GetFreeIdxMap();
		typename CTensorIdx<CType>::TFreeIdxMap::const_iterator it_El, it_End;

		mIdx.Set(iLoopCount = int(mapFreeIdx.size()));
		iLoop  = iLoopCount - 1;
		it_End = mapFreeIdx.end();
		for (it_El = mapFreeIdx.begin();
		     it_El != it_End;
		     ++it_El, --iLoop)
		{
			mIdx[iLoop] = it_El->first;
		}

		rTIdxResult.Set(rTResult, mIdx);

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			pData[iPos] = dVal - pData[iPos];
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Subtraction from scalar on left

template<class CType>
bool TensorSubLeftScalar(CTensorData<CType>& rTResult, CTensorData<CType>& rT, const CType& dVal)
{
	try
	{
		rTResult = rT;

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			pData[iPos] = dVal - pData[iPos];
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/// Division
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// Div with contraction

template<class CType>
bool TensorDivContract(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, CTensorIdx<CType>& rTRight, const CType& dPrec)
{
	CTensorContractLoop<CType> Loop;

	try
	{
		Loop.Set(rTResult, rTLeft, rTRight);
		rTIdxResult = Loop.GetResultTensorIdx();
		Loop.Init();

		do
		{
			if (::IsZero(Loop.GetRightRef(), dPrec))
			{
				return false;
			}

			Loop.GetResultRef() += Loop.GetLeftRef() / Loop.GetRightRef();
		}
		while (Loop.Step() >= 0);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Div without contraction

template<class CType>
bool TensorDivPoint(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, CTensorIdx<CType>& rTRight, const CType& dPrec)
{
	CTensorPointLoop<CType> Loop;

	try
	{
		Loop.Set(rTResult, rTLeft, rTRight);
		rTIdxResult = Loop.GetResultTensorIdx();
		Loop.Init();

		do
		{
			if (::IsZero(Loop.GetRightRef(), dPrec))
			{
				return false;
			}

			Loop.GetResultRef() = Loop.GetLeftRef() / Loop.GetRightRef();
		}
		while (Loop.Step() >= 0);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Div with scalar

template<class CType>
bool TensorIdxDivRightScalar(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, const CType& dVal, const CType& dPrec)
{
	try
	{
		if (::IsZero(dVal, dPrec))
		{
			return false;
		}

		if (!MakeTensor(rTResult, rTLeft))
		{
			return false;
		}

		int iLoop, iLoopCount;
		Mem<int> mIdx;
		const typename CTensorIdx<CType>::TFreeIdxMap& mapFreeIdx = rTLeft.GetFreeIdxMap();
		typename CTensorIdx<CType>::TFreeIdxMap::const_iterator it_El, it_End;

		mIdx.Set(iLoopCount = int(mapFreeIdx.size()));
		iLoop  = iLoopCount - 1;
		it_End = mapFreeIdx.end();
		for (it_El = mapFreeIdx.begin();
		     it_El != it_End;
		     ++it_El, --iLoop)
		{
			mIdx[iLoop] = it_El->first;
		}

		rTIdxResult.Set(rTResult, mIdx);

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			pData[iPos] /= dVal;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Div with scalar

template<class CType>
bool TensorDivRightScalar(CTensorData<CType>& rTResult, CTensorData<CType>& rT, const CType& dVal, const CType& dPrec)
{
	try
	{
		if (::IsZero(dVal, dPrec))
		{
			return false;
		}

		rTResult = rT;

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			pData[iPos] /= dVal;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Div with scalar

template<class CType>
bool TensorIdxDivLeftScalar(CTensorData<CType>& rTResult, CTensorIdx<CType>& rTIdxResult, CTensorIdx<CType>& rTLeft, const CType& dVal, const CType& dPrec)
{
	try
	{
		if (!MakeTensor(rTResult, rTLeft))
		{
			return false;
		}

		int iLoop, iLoopCount;
		Mem<int> mIdx;
		const typename CTensorIdx<CType>::TFreeIdxMap& mapFreeIdx = rTLeft.GetFreeIdxMap();
		typename CTensorIdx<CType>::TFreeIdxMap::const_iterator it_El, it_End;

		mIdx.Set(iLoopCount = int(mapFreeIdx.size()));
		iLoop  = iLoopCount - 1;
		it_End = mapFreeIdx.end();
		for (it_El = mapFreeIdx.begin();
		     it_El != it_End;
		     ++it_El, --iLoop)
		{
			mIdx[iLoop] = it_El->first;
		}

		rTIdxResult.Set(rTResult, mIdx);

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			if (::IsZero(pData[iPos], dPrec))
			{
				return false;
			}

			pData[iPos] = dVal / pData[iPos];
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Div with scalar

template<class CType>
bool TensorDivLeftScalar(CTensorData<CType>& rTResult, CTensorData<CType>& rT, const CType& dVal, const CType& dPrec)
{
	try
	{
		rTResult = rT;

		int iPos, iSize = rTResult.Size();
		CType* pData = rTResult.Data();

		for (iPos = 0; iPos < iSize; iPos++)
		{
			if (::IsZero(pData[iPos], dPrec))
			{
				return false;
			}

			pData[iPos] = dVal / pData[iPos];
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}
