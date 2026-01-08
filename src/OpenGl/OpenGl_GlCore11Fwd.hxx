// Created on: 2014-03-17
// Created by: Kirill GAVRILOV
// Copyright (c) 2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#ifndef OpenGl_GlCore11Fwd_HeaderFile
#define OpenGl_GlCore11Fwd_HeaderFile

#include <OpenGl_GlFunctions.hxx>

//! OpenGL 1.1 core without deprecated Fixed Pipeline entry points.
//! Notice that all functions within this structure are actually exported by system GL library.
//! The main purpose for these hint - to control visibility of functions per GL version
//! (global functions should not be used directly to achieve this effect!).
struct OpenGl_GlCore11Fwd : protected GlFunctions
{

public: //! @name Miscellaneous
  using GlFunctions::glBlendFunc;
  using GlFunctions::glClear;
  using GlFunctions::glClearColor;
  using GlFunctions::glColorMask;
  using GlFunctions::glCullFace;
  using GlFunctions::glDisable;
  using GlFunctions::glEnable;
  using GlFunctions::glFinish;
  using GlFunctions::glFlush;
  using GlFunctions::glFrontFace;
  using GlFunctions::glGetBooleanv;
  using GlFunctions::glGetError;
  using GlFunctions::glGetFloatv;
  using GlFunctions::glGetIntegerv;
  using GlFunctions::glGetString;
  using GlFunctions::glHint;
  using GlFunctions::glIsEnabled;
  using GlFunctions::glLineWidth;
  using GlFunctions::glPolygonOffset;
  using GlFunctions::glScissor;

public: //! @name Depth Buffer
  using GlFunctions::glClearDepth;
  using GlFunctions::glClearDepthf;
  using GlFunctions::glDepthFunc;
  using GlFunctions::glDepthMask;
  using GlFunctions::glDepthRange;
  using GlFunctions::glDepthRangef;

public: //! @name Transformation
  using GlFunctions::glViewport;

public: //! @name Vertex Arrays
  using GlFunctions::glDrawArrays;
  using GlFunctions::glDrawElements;

public: //! @name Raster functions
  using GlFunctions::glPixelStorei;
  using GlFunctions::glReadPixels;

public: //! @name Stenciling
  using GlFunctions::glClearStencil;
  using GlFunctions::glStencilFunc;
  using GlFunctions::glStencilMask;
  using GlFunctions::glStencilOp;

public: //! @name Texture mapping
  using GlFunctions::glBindTexture;
  using GlFunctions::glCopyTexImage2D;
  using GlFunctions::glCopyTexSubImage2D;
  using GlFunctions::glDeleteTextures;
  using GlFunctions::glGenTextures;
  using GlFunctions::glGetTexParameterfv;
  using GlFunctions::glGetTexParameteriv;
  using GlFunctions::glIsTexture;
  using GlFunctions::glTexImage2D;
  using GlFunctions::glTexParameterf;
  using GlFunctions::glTexParameterfv;
  using GlFunctions::glTexParameteri;
  using GlFunctions::glTexParameteriv;
  using GlFunctions::glTexSubImage2D;

public: //! @name desktop extensions - not supported in OpenGL ES 2..0
  using GlFunctions::glAlphaFunc;
  using GlFunctions::glCopyTexImage1D;
  using GlFunctions::glCopyTexSubImage1D;
  using GlFunctions::glGetTexImage;
  using GlFunctions::glPointSize;
  using GlFunctions::glTexImage1D;
  using GlFunctions::glTexSubImage1D;

  // added to OpenGL ES 3.0
  using GlFunctions::glDrawBuffer;
  using GlFunctions::glReadBuffer;

  // added to OpenGL ES 3.1
  using GlFunctions::glGetTexLevelParameteriv;

  // added to OpenGL ES 3.2
  using GlFunctions::glGetPointerv;

  using GlFunctions::glLogicOp;
  using GlFunctions::glPolygonMode;
};

#endif // _OpenGl_GlCore11Fwd_Header
