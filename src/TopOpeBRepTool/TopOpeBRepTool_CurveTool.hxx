// Created on: 1993-06-24
// Created by: Jean Yves LEBEY
// Copyright (c) 1993-1999 Matra Datavision
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

#ifndef _TopOpeBRepTool_CurveTool_HeaderFile
#define _TopOpeBRepTool_CurveTool_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopOpeBRepTool_GeomTool.hxx>
#include <TopOpeBRepTool_OutCurveType.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
class GeomCurve3d;
class GeomCurve2d;
class TopoShape;

class CurveTool6
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT CurveTool6();

  Standard_EXPORT CurveTool6(const TopOpeBRepTool_OutCurveType OCT);

  Standard_EXPORT CurveTool6(const GeomTool1& GT);

  Standard_EXPORT GeomTool1& ChangeGeomTool();

  Standard_EXPORT const GeomTool1& GetGeomTool() const;

  Standard_EXPORT void SetGeomTool(const GeomTool1& GT);

  //! Approximates curves.
  //! Returns False in the case of failure
  Standard_EXPORT Standard_Boolean MakeCurves(const Standard_Real         min,
                                              const Standard_Real         max,
                                              const Handle(GeomCurve3d)&   C3D,
                                              const Handle(GeomCurve2d)& PC1,
                                              const Handle(GeomCurve2d)& PC2,
                                              const TopoShape&         S1,
                                              const TopoShape&         S2,
                                              Handle(GeomCurve3d)&         C3DN,
                                              Handle(GeomCurve2d)&       PC1N,
                                              Handle(GeomCurve2d)&       PC2N,
                                              Standard_Real&              Tol3d,
                                              Standard_Real&              Tol2d) const;

  Standard_EXPORT static Handle(GeomCurve3d) MakeBSpline1fromPnt(const TColgp_Array1OfPnt& P);

  Standard_EXPORT static Handle(GeomCurve2d) MakeBSpline1fromPnt2d(const TColgp_Array1OfPnt2d& P);

  Standard_EXPORT static Standard_Boolean IsProjectable(const TopoShape&       S,
                                                        const Handle(GeomCurve3d)& C);

  Standard_EXPORT static Handle(GeomCurve2d) MakePCurveOnFace(const TopoShape&       S,
                                                               const Handle(GeomCurve3d)& C,
                                                               Standard_Real&      TolReached2d,
                                                               const Standard_Real first = 0.0,
                                                               const Standard_Real last  = 0.0);

protected:
  GeomTool1 myGeomTool;

private:
};

#endif // _TopOpeBRepTool_CurveTool_HeaderFile
