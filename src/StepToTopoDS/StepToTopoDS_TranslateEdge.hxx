// Created on: 1994-12-16
// Created by: Frederic MAUPAS
// Copyright (c) 1994-1999 Matra Datavision
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

#ifndef _StepToTopoDS_TranslateEdge_HeaderFile
#define _StepToTopoDS_TranslateEdge_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <StepToTopoDS_TranslateEdgeError.hxx>
#include <TopoDS_Shape.hxx>
#include <StepToTopoDS_Root.hxx>
class StepShape_Edge;
class StepToTopoDS_Tool;
class NamingTool2;
class StepGeom_Curve;
class StepShape_EdgeCurve;
class StepShape_Vertex;
class TopoEdge;
class TopoVertex;
class GeomCurve2d;
class StepGeom_Pcurve;
class GeomSurface;

class StepToTopoDS_TranslateEdge : public Root2
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT StepToTopoDS_TranslateEdge();

  Standard_EXPORT StepToTopoDS_TranslateEdge(
    const Handle(StepShape_Edge)& E,
    StepToTopoDS_Tool&            T,
    NamingTool2&          NMTool,
    const ConversionFactors&       theLocalFactors = ConversionFactors());

  Standard_EXPORT void Init(const Handle(StepShape_Edge)& E,
                            StepToTopoDS_Tool&            T,
                            NamingTool2&          NMTool,
                            const ConversionFactors&       theLocalFactors = ConversionFactors());

  //! Warning! C3D is assumed to be a Curve 3D ...
  //! other cases to checked before calling this
  Standard_EXPORT void MakeFromCurve3D(
    const Handle(StepGeom_Curve)&      C3D,
    const Handle(StepShape_EdgeCurve)& EC,
    const Handle(StepShape_Vertex)&    Vend,
    const Standard_Real                preci,
    TopoEdge&                       E,
    TopoVertex&                     V1,
    TopoVertex&                     V2,
    StepToTopoDS_Tool&                 T,
    const ConversionFactors&            theLocalFactors = ConversionFactors());

  Standard_EXPORT Handle(GeomCurve2d) MakePCurve(
    const Handle(StepGeom_Pcurve)& PCU,
    const Handle(GeomSurface)&    ConvSurf,
    const ConversionFactors&        theLocalFactors = ConversionFactors()) const;

  Standard_EXPORT const TopoShape& Value() const;

  Standard_EXPORT StepToTopoDS_TranslateEdgeError Error() const;

protected:
private:
  StepToTopoDS_TranslateEdgeError myError;
  TopoShape                    myResult;
};

#endif // _StepToTopoDS_TranslateEdge_HeaderFile
