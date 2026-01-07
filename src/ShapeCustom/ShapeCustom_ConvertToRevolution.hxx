// Created on: 1999-05-08
// Created by: Andrey BETENEV
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

#ifndef _ShapeCustom_ConvertToRevolution_HeaderFile
#define _ShapeCustom_ConvertToRevolution_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <ShapeCustom_Modification.hxx>
#include <GeomAbs_Shape.hxx>
class TopoFace;
class GeomSurface;
class TopLoc_Location;
class TopoEdge;
class GeomCurve3d;
class TopoVertex;
class Point3d;
class GeomCurve2d;

class ShapeCustom_ConvertToRevolution;
DEFINE_STANDARD_HANDLE(ShapeCustom_ConvertToRevolution, ShapeCustom_Modification)

//! implements a modification for the BRepTools1
//! Modifier algorithm. Converts all elementary
//! surfaces into surfaces of revolution.
class ShapeCustom_ConvertToRevolution : public ShapeCustom_Modification
{

public:
  Standard_EXPORT ShapeCustom_ConvertToRevolution();

  //! Returns Standard_True if the face <F> has  been
  //! modified. In this case, <S> is the new geometric
  //! support of the face, <L> the new location,  <Tol>
  //! the new tolerance.  Otherwise, returns
  //! Standard_False, and <S>, <L>, <Tol> are  not
  //! significant.
  Standard_EXPORT Standard_Boolean NewSurface(const TopoFace&    F,
                                              Handle(GeomSurface)& S,
                                              TopLoc_Location&      L,
                                              Standard_Real&        Tol,
                                              Standard_Boolean&     RevWires,
                                              Standard_Boolean&     RevFace) Standard_OVERRIDE;

  //! Returns Standard_True  if  the edge  <E> has  been
  //! modified.  In this case,  <C> is the new geometric
  //! support of the  edge, <L> the  new location, <Tol>
  //! the         new    tolerance.   Otherwise, returns
  //! Standard_False,    and  <C>,  <L>,   <Tol> are not
  //! significant.
  Standard_EXPORT Standard_Boolean NewCurve(const TopoEdge&  E,
                                            Handle(GeomCurve3d)& C,
                                            TopLoc_Location&    L,
                                            Standard_Real&      Tol) Standard_OVERRIDE;

  //! Returns  Standard_True if the  vertex <V> has been
  //! modified.  In this  case, <P> is the new geometric
  //! support of the vertex,   <Tol> the new  tolerance.
  //! Otherwise, returns Standard_False, and <P>,  <Tol>
  //! are not significant.
  Standard_EXPORT Standard_Boolean NewPoint(const TopoVertex& V,
                                            Point3d&              P,
                                            Standard_Real&       Tol) Standard_OVERRIDE;

  //! Returns Standard_True if  the edge  <E> has a  new
  //! curve on surface on the face <F>.In this case, <C>
  //! is the new geometric support of  the edge, <L> the
  //! new location, <Tol> the new tolerance.
  //!
  //! Otherwise, returns  Standard_False, and <C>,  <L>,
  //! <Tol> are not significant.
  //!
  //! <NewE> is the new  edge created from  <E>.  <NewF>
  //! is the new face created from <F>. They may be useful.
  Standard_EXPORT Standard_Boolean NewCurve2d(const TopoEdge&    E,
                                              const TopoFace&    F,
                                              const TopoEdge&    NewE,
                                              const TopoFace&    NewF,
                                              Handle(GeomCurve2d)& C,
                                              Standard_Real&        Tol) Standard_OVERRIDE;

  //! Returns Standard_True if the Vertex  <V> has a new
  //! parameter on the  edge <E>. In  this case,  <P> is
  //! the parameter,    <Tol>  the     new    tolerance.
  //! Otherwise, returns Standard_False, and <P>,  <Tol>
  //! are not significant.
  Standard_EXPORT Standard_Boolean NewParameter(const TopoVertex& V,
                                                const TopoEdge&   E,
                                                Standard_Real&       P,
                                                Standard_Real&       Tol) Standard_OVERRIDE;

  //! Returns the  continuity of  <NewE> between <NewF1>
  //! and <NewF2>.
  //!
  //! <NewE> is the new  edge created from <E>.  <NewF1>
  //! (resp. <NewF2>) is the new  face created from <F1>
  //! (resp. <F2>).
  Standard_EXPORT GeomAbs_Shape Continuity(const TopoEdge& E,
                                           const TopoFace& F1,
                                           const TopoFace& F2,
                                           const TopoEdge& NewE,
                                           const TopoFace& NewF1,
                                           const TopoFace& NewF2) Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(ShapeCustom_ConvertToRevolution, ShapeCustom_Modification)

protected:
private:
};

#endif // _ShapeCustom_ConvertToRevolution_HeaderFile
