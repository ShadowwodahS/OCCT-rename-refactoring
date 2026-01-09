// Created on: 1995-01-27
// Created by: Jacques GOUSSARD
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

#ifndef _GeomInt_TheImpPrmSvSurfacesOfWLApprox_HeaderFile
#define _GeomInt_TheImpPrmSvSurfacesOfWLApprox_HeaderFile

#include <Adaptor3d_Surface.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec2d.hxx>
#include <gp_Vec.hxx>
#include <GeomInt_TheZerImpFuncOfTheImpPrmSvSurfacesOfWLApprox.hxx>
#include <ApproxInt_SvSurfaces.hxx>

class HSurfaceTool;
class Quadric1;
class QuadricTool;
class IntPatch_WLine;
class GeomInt_TheZerImpFuncOfTheImpPrmSvSurfacesOfWLApprox;

class GeomInt_TheImpPrmSvSurfacesOfWLApprox : public SurfaceSurfaces
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT GeomInt_TheImpPrmSvSurfacesOfWLApprox(const Handle(SurfaceAdaptor)& Surf1,
                                                        const Quadric1&           Surf2);

  Standard_EXPORT GeomInt_TheImpPrmSvSurfacesOfWLApprox(const Quadric1&           Surf1,
                                                        const Handle(SurfaceAdaptor)& Surf2);

  //! returns True if Tg,Tguv1 Tguv2 can be computed.
  Standard_EXPORT Standard_Boolean Compute(Standard_Real& u1,
                                           Standard_Real& v1,
                                           Standard_Real& u2,
                                           Standard_Real& v2,
                                           Point3d&        Pt,
                                           Vector3d&        Tg,
                                           gp_Vec2d&      Tguv1,
                                           gp_Vec2d&      Tguv2);

  Standard_EXPORT void Pnt(const Standard_Real u1,
                           const Standard_Real v1,
                           const Standard_Real u2,
                           const Standard_Real v2,
                           Point3d&             P);

  Standard_EXPORT Standard_Boolean SeekPoint(const Standard_Real u1,
                                             const Standard_Real v1,
                                             const Standard_Real u2,
                                             const Standard_Real v2,
                                             PointOn2Surfaces&    Point);

  Standard_EXPORT Standard_Boolean Tangency(const Standard_Real u1,
                                            const Standard_Real v1,
                                            const Standard_Real u2,
                                            const Standard_Real v2,
                                            Vector3d&             Tg);

  Standard_EXPORT Standard_Boolean TangencyOnSurf1(const Standard_Real u1,
                                                   const Standard_Real v1,
                                                   const Standard_Real u2,
                                                   const Standard_Real v2,
                                                   gp_Vec2d&           Tg);

  Standard_EXPORT Standard_Boolean TangencyOnSurf2(const Standard_Real u1,
                                                   const Standard_Real v1,
                                                   const Standard_Real u2,
                                                   const Standard_Real v2,
                                                   gp_Vec2d&           Tg);

  Standard_Boolean FillInitialVectorOfSolution(const Standard_Real u1,
                                               const Standard_Real v1,
                                               const Standard_Real u2,
                                               const Standard_Real v2,
                                               const Standard_Real binfu,
                                               const Standard_Real bsupu,
                                               const Standard_Real binfv,
                                               const Standard_Real bsupv,
                                               math_Vector&        X,
                                               Standard_Real&      TranslationU,
                                               Standard_Real&      TranslationV);

protected:
private:
  gp_Pnt2d                                             MyParOnS1;
  gp_Pnt2d                                             MyParOnS2;
  Point3d                                               MyPnt;
  gp_Vec2d                                             MyTguv1;
  gp_Vec2d                                             MyTguv2;
  Vector3d                                               MyTg;
  Standard_Boolean                                     MyIsTangent;
  Standard_Boolean                                     MyHasBeenComputed;
  gp_Pnt2d                                             MyParOnS1bis;
  gp_Pnt2d                                             MyParOnS2bis;
  Point3d                                               MyPntbis;
  gp_Vec2d                                             MyTguv1bis;
  gp_Vec2d                                             MyTguv2bis;
  Vector3d                                               MyTgbis;
  Standard_Boolean                                     MyIsTangentbis;
  Standard_Boolean                                     MyHasBeenComputedbis;
  Standard_Boolean                                     MyImplicitFirst;
  GeomInt_TheZerImpFuncOfTheImpPrmSvSurfacesOfWLApprox MyZerImpFunc;
};

#endif // _GeomInt_TheImpPrmSvSurfacesOfWLApprox_HeaderFile
