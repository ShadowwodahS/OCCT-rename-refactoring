// Created on: 2012-01-26
// Created by: Kirill GAVRILOV
// Copyright (c) 2012-2014 OPEN CASCADE SAS
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

#ifndef OpenGl_ArbFBO_HeaderFile
#define OpenGl_ArbFBO_HeaderFile

#include <OpenGl_GlFunctions.hxx>

//! FBO is available on OpenGL 2.0+ hardware
struct OpenGl_ArbFBO : protected GlFunctions
{

  using GlFunctions::glBindFramebuffer;
  using GlFunctions::glBindRenderbuffer;
  using GlFunctions::glCheckFramebufferStatus;
  using GlFunctions::glDeleteFramebuffers;
  using GlFunctions::glDeleteRenderbuffers;
  using GlFunctions::glFramebufferRenderbuffer;
  using GlFunctions::glFramebufferTexture2D;
  using GlFunctions::glGenerateMipmap;
  using GlFunctions::glGenFramebuffers;
  using GlFunctions::glGenRenderbuffers;
  using GlFunctions::glGetFramebufferAttachmentParameteriv;
  using GlFunctions::glGetRenderbufferParameteriv;
  using GlFunctions::glIsFramebuffer;
  using GlFunctions::glIsRenderbuffer;
  using GlFunctions::glRenderbufferStorage;

#if !defined(GL_ES_VERSION_2_0)
  using GlFunctions::glBlitFramebuffer;
  using GlFunctions::glFramebufferTexture1D;
  using GlFunctions::glFramebufferTexture3D;
  using GlFunctions::glFramebufferTextureLayer;
  using GlFunctions::glRenderbufferStorageMultisample;
#endif
};

//! FBO blit is available in OpenGL 3.0+.
//! Moved out from OpenGl_ArbFBO since it is unavailable in OpenGL ES 2.0.
struct OpenGl_ArbFBOBlit : protected GlFunctions
{

  using GlFunctions::glBlitFramebuffer;
};

#endif // _OpenGl_ArbFBO_H__
