// Created on: 1995-03-29
// Created by: Frederic MAUPAS
// Copyright (c) 1995-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#ifndef _StepToTopoDS_TranslateEdgeLoop_HeaderFile
#define _StepToTopoDS_TranslateEdgeLoop_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <StepToTopoDS_TranslateEdgeLoopError.hxx>
#include <TopoDS_Shape.hxx>
#include <StepToTopoDS_Root.hxx>
class StepShape_FaceBound;
class TopoFace;
class GeomSurface;
class StepGeom_Surface;
class StepToTopoDS_Tool;
class NamingTool2;

class StepToTopoDS_TranslateEdgeLoop : public Root2
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT StepToTopoDS_TranslateEdgeLoop();

  Standard_EXPORT StepToTopoDS_TranslateEdgeLoop(
    const Handle(StepShape_FaceBound)& FB,
    const TopoFace&                 F,
    const Handle(GeomSurface)&        S,
    const Handle(StepGeom_Surface)&    SS,
    const Standard_Boolean             ss,
    StepToTopoDS_Tool&                 T,
    NamingTool2&               NMTool,
    const ConversionFactors&            theLocalFactors = ConversionFactors());

  Standard_EXPORT void Init(const Handle(StepShape_FaceBound)& FB,
                            const TopoFace&                 F,
                            const Handle(GeomSurface)&        S,
                            const Handle(StepGeom_Surface)&    SS,
                            const Standard_Boolean             ss,
                            StepToTopoDS_Tool&                 T,
                            NamingTool2&               NMTool,
                            const ConversionFactors& theLocalFactors = ConversionFactors());

  Standard_EXPORT const TopoShape& Value() const;

  Standard_EXPORT StepToTopoDS_TranslateEdgeLoopError Error() const;

protected:
private:
  StepToTopoDS_TranslateEdgeLoopError myError;
  TopoShape                        myResult;
};

#endif // _StepToTopoDS_TranslateEdgeLoop_HeaderFile
