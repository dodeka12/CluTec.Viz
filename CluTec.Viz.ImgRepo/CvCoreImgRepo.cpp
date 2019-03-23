////////////////////////////////////////////////////////////////////////////////////////////////////
// project:   CluTec.Viz.ImgRepo.rtl
// file:      CvCoreImgRepo.cpp
//
// summary:   Implements the cv core image repo class
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

#include "stdafx.h"
#include "CvCoreImgRepo.h"


HANDLE __hMutexImgLock = 0;	// Lock on image access
void* __pvImgRep       = 0;	// Global image repository

CLUIR_API unsigned int sm_uLastUID = 0;	// Last used unique ID

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLUIR_API void InitImageRepository(void* pvRep)
{
	__pvImgRep      = pvRep;
	__hMutexImgLock = CreateMutex(NULL, FALSE, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLUIR_API void ReleaseImageRepository()
{
	delete __pvImgRep;
	__pvImgRep = nullptr;

	if (__hMutexImgLock != 0)
	{
		CloseHandle(__hMutexImgLock);
		__hMutexImgLock = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLUIR_API void* GetImageRepositoryPtr()
{
	return __pvImgRep;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLUIR_API bool LockImageAccess(int iWait)
{
	return WaitForSingleObject(__hMutexImgLock, iWait) != WAIT_TIMEOUT;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLUIR_API void UnlockImageAccess()
{
	ReleaseMutex(__hMutexImgLock);
}