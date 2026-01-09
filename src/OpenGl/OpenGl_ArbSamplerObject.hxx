// Copyright (c) 2017 OPEN CASCADE SAS
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

#ifndef _OpenGl_ArbSamplerObject_Header
#define _OpenGl_ArbSamplerObject_Header

#include <OpenGl_GlFunctions.hxx>

//! Provide Sampler Object functionality (texture parameters stored independently from texture
//! itself). Available since OpenGL 3.3+ (GL_ARB_sampler_objects extension) and OpenGL ES 3.0+.
struct ArbSamplerObject : protected GlFunctions
{
  using GlFunctions::glBindSampler;
  using GlFunctions::glDeleteSamplers;
  using GlFunctions::glGenSamplers;
  using GlFunctions::glGetSamplerParameterfv;
  using GlFunctions::glGetSamplerParameteriv;
  using GlFunctions::glIsSampler;
  using GlFunctions::glSamplerParameterf;
  using GlFunctions::glSamplerParameterfv;
  using GlFunctions::glSamplerParameteri;
  using GlFunctions::glSamplerParameteriv;

#if !defined(GL_ES_VERSION_2_0)
  using GlFunctions::glGetSamplerParameterIiv;
  using GlFunctions::glGetSamplerParameterIuiv;
  using GlFunctions::glSamplerParameterIiv;
  using GlFunctions::glSamplerParameterIuiv;
#endif
};

#endif // _OpenGl_ArbSamplerObject_Header
