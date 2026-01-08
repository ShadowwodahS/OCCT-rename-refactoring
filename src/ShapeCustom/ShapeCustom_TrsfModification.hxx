// Created on: 1999-03-09
// Created by: Roman LYGIN
// Copyright (c) 1999 Matra Datavision
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

#ifndef _ShapeCustom_TrsfModification_HeaderFile
#define _ShapeCustom_TrsfModification_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <BRepTools_TrsfModification.hxx>
class Transform3d;
class TopoFace;
class GeomSurface;
class TopLoc_Location;
class TopoEdge;
class GeomCurve3d;
class TopoVertex;
class Point3d;
class GeomCurve2d;

class ShapeCustom_TrsfModification;
DEFINE_STANDARD_HANDLE(ShapeCustom_TrsfModification, BRepTools_TrsfModification)

//! Complements BRepTools_TrsfModification to provide reversible
//! scaling regarding tolerances.
//! Uses actual tolerances (attached to the shapes) not ones
//! returned by BRepInspector::Tolerance to work with tolerances
//! lower than Precision1::Confusion.
class ShapeCustom_TrsfModification : public BRepTools_TrsfModification
{

public:
  //! Empty constructor
  Standard_EXPORT ShapeCustom_TrsfModification(const Transform3d& T);

  //! Calls inherited method.
  //! Sets <Tol> as actual tolerance of <F> multiplied with scale
  //! factor.
  Standard_EXPORT Standard_Boolean NewSurface(const TopoFace&    F,
                                              Handle(GeomSurface)& S,
                                              TopLoc_Location&      L,
                                              Standard_Real&        Tol,
                                              Standard_Boolean&     RevWires,
                                              Standard_Boolean&     RevFace) Standard_OVERRIDE;

  //! Calls inherited method.
  //! Sets <Tol> as actual tolerance of <E> multiplied with scale
  //! factor.
  Standard_EXPORT Standard_Boolean NewCurve(const TopoEdge&  E,
                                            Handle(GeomCurve3d)& C,
                                            TopLoc_Location&    L,
                                            Standard_Real&      Tol) Standard_OVERRIDE;

  //! Calls inherited method.
  //! Sets <Tol> as actual tolerance of <V> multiplied with scale
  //! factor.
  Standard_EXPORT Standard_Boolean NewPoint(const TopoVertex& V,
                                            Point3d&              P,
                                            Standard_Real&       Tol) Standard_OVERRIDE;

  //! Calls inherited method.
  //! Sets <Tol> as actual tolerance of <E> multiplied with scale
  //! factor.
  Standard_EXPORT Standard_Boolean NewCurve2d(const TopoEdge&    E,
                                              const TopoFace&    F,
                                              const TopoEdge&    NewE,
                                              const TopoFace&    NewF,
                                              Handle(GeomCurve2d)& C,
                                              Standard_Real&        Tol) Standard_OVERRIDE;

  //! Calls inherited method.
  //! Sets <Tol> as actual tolerance of <V> multiplied with scale
  //! factor.
  Standard_EXPORT Standard_Boolean NewParameter(const TopoVertex& V,
                                                const TopoEdge&   E,
                                                Standard_Real&       P,
                                                Standard_Real&       Tol) Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(ShapeCustom_TrsfModification, BRepTools_TrsfModification)

protected:
private:
};

#endif // _ShapeCustom_TrsfModification_HeaderFile
