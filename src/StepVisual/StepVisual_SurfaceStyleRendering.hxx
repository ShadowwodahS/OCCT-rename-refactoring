// Created on : Thu May 14 15:13:19 2020
// Created by: Igor KHOZHANOV
// Generator:	Express1 (EXPRESS -> CASCADE/XSTEP Translator) V2.0
// Copyright (c) Open CASCADE 2020
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

#ifndef _StepVisual_SurfaceStyleRendering_HeaderFile_
#define _StepVisual_SurfaceStyleRendering_HeaderFile_

#include <Standard.hxx>
#include <Standard_Type.hxx>
#include <Standard_Transient.hxx>

#include <StepVisual_ShadingSurfaceMethod.hxx>
#include <StepVisual_Colour.hxx>

class StepVisual_SurfaceStyleRendering;
DEFINE_STANDARD_HANDLE(StepVisual_SurfaceStyleRendering, RefObject)

//! Representation of STEP entity SurfaceStyleRendering
class StepVisual_SurfaceStyleRendering : public RefObject
{
public:
  //! default constructor
  Standard_EXPORT StepVisual_SurfaceStyleRendering();

  //! Initialize all fields (own and inherited)
  Standard_EXPORT void Init(const StepVisual_ShadingSurfaceMethod theRenderingMethod,
                            const Handle(Colour)&      theSurfaceColour);

  //! Returns field RenderingMethod
  Standard_EXPORT StepVisual_ShadingSurfaceMethod RenderingMethod() const;
  //! Sets field RenderingMethod
  Standard_EXPORT void SetRenderingMethod(const StepVisual_ShadingSurfaceMethod theRenderingMethod);

  //! Returns field SurfaceColour
  Standard_EXPORT Handle(Colour) SurfaceColour() const;
  //! Sets field SurfaceColour
  Standard_EXPORT void SetSurfaceColour(const Handle(Colour)& theSurfaceColour);

  DEFINE_STANDARD_RTTIEXT(StepVisual_SurfaceStyleRendering, RefObject)

private:
  StepVisual_ShadingSurfaceMethod myRenderingMethod;
  Handle(Colour)       mySurfaceColour;
};
#endif // _StepVisual_SurfaceStyleRendering_HeaderFile_
