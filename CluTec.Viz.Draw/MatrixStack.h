////////////////////////////////////////////////////////////////////////////////////////////////////
// project:   CluTec.Viz.Draw
// file:      MatrixStack.h
//
// summary:   Declares the matrix stack class
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

#pragma once

#include <stack>
#include <vector>

#include "GL/gl.h"

#include "CluTec.Base/Exception.h"

#include "CluTec.Viz.ImgRepo/CvCoreImgRepo.h"

#include "CluTec.Viz.OpenGL.Extensions/Extensions.h"
#include "CluTec.Viz.OpenGL/Api.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// namespace: Clu
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Clu
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	Stack of matrices.
	/// </summary>
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class CMatrixStack
	{
	public:

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Default constructor.
		/// </summary>
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		CMatrixStack() = default;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Destructor.
		/// </summary>
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		~CMatrixStack() = default;

	public:

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Values that represent possible OpenGL matrix modes.
		/// </summary>
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		enum EMatrixMode
		{
			ModelView = GL_MODELVIEW,
			Projection = GL_PROJECTION,
			Texture = GL_TEXTURE,
		};

	private:

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Internal OpenGL matrix struct containing 16 floats.
		/// </summary>
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		struct SMatrix
		{
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/// <summary>
			/// 	Default constructor. Matrix is not initialized.
			/// </summary>
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			SMatrix()
			{
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/// <summary>
			/// 	Constructor that gets the given matrix from OpenGL.
			/// </summary>
			///
			/// <param name="uType"> The matrix type. </param>
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			SMatrix(unsigned uType)
			{
				CLU_OGL_CALL(glGetFloatv(uType, pfData));
			}

			/// <summary> Pointer to the memory. </summary>
			float pfData[16];
		};

	public:

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Pushes all matrix modes.
		/// </summary>
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void PushAll();

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Pops all matrix modes.
		/// </summary>
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void PopAll();

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Pushes the current matrix onto the stack for the given matrix mode. This activates the given matrix mode.
		/// </summary>
		///
		/// <param name="eMode"> The matrix mode to push. </param>
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void Push(EMatrixMode eMode);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Pops the top-of-stack and applies the matrix. This activates the given matrix mode.
		/// </summary>
		///
		/// <param name="eMode"> The matrix mode to pop. </param>
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void Pop(EMatrixMode eMode);

	public:

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Query if all stacks are empty.
		/// </summary>
		///
		/// <returns> True if all stacks are empty, false if not. </returns>
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		bool IsEmpty() const;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Query if the stack of the given matrix mode is empty
		/// </summary>
		///
		/// <param name="eMode"> The matrix mode. </param>
		///
		/// <returns> True if empty, false if not. </returns>
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		bool IsEmpty(EMatrixMode eMode) const;

	private:

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Removes and returns the top-of-stack object. Tests if the stack is empty.
		/// </summary>
		///
		/// <param name="xStack"> [in] The stack. </param>
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void _Pop(std::stack<SMatrix>& xStack);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Gets mode name.
		/// </summary>
		///
		/// <param name="eMode"> The mode. </param>
		///
		/// <returns> The mode name. </returns>
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		std::string _GetModeName(EMatrixMode eMode) const
		{
			switch (eMode)
			{
			case CMatrixStack::ModelView:
				return "ModelView";
			case CMatrixStack::Projection:
				return "Projection";
			case CMatrixStack::Texture:
				return "Texture";
			default:
				return "Unknown";
			}
		}

	private:

		/// <summary> The texture matrix stack for each texture unit. </summary>
		std::stack<SMatrix> m_xStackTextures[OGL_MAX_TEX_UNITS];
		/// <summary> The projection matrix stack. </summary>
		std::stack<SMatrix> m_xStackProjection;
		/// <summary> The model view stack. </summary>
		std::stack<SMatrix> m_xStackModelView;
	};
}
