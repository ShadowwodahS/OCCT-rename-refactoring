// Created on: 1994-08-25
// Created by: Jacques GOUSSARD
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

#ifndef _BRepTools_Modification_HeaderFile
#define _BRepTools_Modification_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <Standard_Transient.hxx>
#include <Standard_Real.hxx>
#include <GeomAbs_Shape.hxx>
class TopoFace;
class GeomSurface;
class TopLoc_Location;
class TopoEdge;
class GeomCurve3d;
class TopoVertex;
class Point3d;
class GeomCurve2d;
class MeshTriangulation;

class Poly_Polygon3D;
class Poly_PolygonOnTriangulation;

class ShapeModification;
DEFINE_STANDARD_HANDLE(ShapeModification, RefObject)

//! Defines geometric modifications to a shape, i.e.
//! changes to faces, edges and vertices.
class ShapeModification : public RefObject
{

public:
  //! Returns true if the face, F, has been modified.
  //! If the face has been modified:
  //! - S is the new geometry of the face,
  //! - L is its new location, and
  //! - Tol is the new tolerance.
  //! The flag, RevWires, is set to true when the
  //! modification reverses the normal of the surface, (i.e.
  //! the wires have to be reversed).
  //! The flag, RevFace, is set to true if the orientation of
  //! the modified face changes in the shells which contain it.
  //! If the face has not been modified this function returns
  //! false, and the values of S, L, Tol, RevWires and
  //! RevFace are not significant.
  Standard_EXPORT virtual Standard_Boolean NewSurface(const TopoFace&    F,
                                                      Handle(GeomSurface)& S,
                                                      TopLoc_Location&      L,
                                                      Standard_Real&        Tol,
                                                      Standard_Boolean&     RevWires,
                                                      Standard_Boolean&     RevFace) = 0;

  //! Returns true if the face has been modified according to changed triangulation.
  //! If the face has been modified:
  //! - T is a new triangulation on the face
  Standard_EXPORT virtual Standard_Boolean NewTriangulation(const TopoFace&          F,
                                                            Handle(MeshTriangulation)& T);

  //! Returns true if the edge, E, has been modified.
  //! If the edge has been modified:
  //! - C is the new geometry associated with the edge,
  //! - L is its new location, and
  //! - Tol is the new tolerance.
  //! If the edge has not been modified, this function
  //! returns false, and the values of C, L and Tol are not significant.
  Standard_EXPORT virtual Standard_Boolean NewCurve(const TopoEdge&  E,
                                                    Handle(GeomCurve3d)& C,
                                                    TopLoc_Location&    L,
                                                    Standard_Real&      Tol) = 0;

  //! Returns true if the edge has been modified according to changed polygon.
  //! If the edge has been modified:
  //! - P is a new polygon
  Standard_EXPORT virtual Standard_Boolean NewPolygon(const TopoEdge&      E,
                                                      Handle(Poly_Polygon3D)& P);

  //! Returns true if the edge has been modified according to changed polygon on triangulation.
  //! If the edge has been modified:
  //! - P is a new polygon on triangulation
  Standard_EXPORT virtual Standard_Boolean NewPolygonOnTriangulation(
    const TopoEdge&                   E,
    const TopoFace&                   F,
    Handle(Poly_PolygonOnTriangulation)& P);

  //! Returns true if the vertex V has been modified.
  //! If V has been modified:
  //! - P is the new geometry of the vertex, and
  //! - Tol is the new tolerance.
  //! If the vertex has not been modified this function
  //! returns false, and the values of P and Tol are not significant.
  Standard_EXPORT virtual Standard_Boolean NewPoint(const TopoVertex& V,
                                                    Point3d&              P,
                                                    Standard_Real&       Tol) = 0;

  //! Returns true if the edge, E, has a new curve on
  //! surface on the face, F.
  //! If a new curve exists:
  //! - C is the new geometry of the edge,
  //! - L is the new location, and
  //! - Tol is the new tolerance.
  //! NewE is the new edge created from E, and NewF is
  //! the new face created from F.
  //! If there is no new curve on the face, this function
  //! returns false, and the values of C, L and Tol are not significant.
  Standard_EXPORT virtual Standard_Boolean NewCurve2d(const TopoEdge&    E,
                                                      const TopoFace&    F,
                                                      const TopoEdge&    NewE,
                                                      const TopoFace&    NewF,
                                                      Handle(GeomCurve2d)& C,
                                                      Standard_Real&        Tol) = 0;

  //! Returns true if the vertex V has a new parameter on the edge E.
  //! If a new parameter exists:
  //! - P is the parameter, and
  //! - Tol is the new tolerance.
  //! If there is no new parameter this function returns
  //! false, and the values of P and Tol are not significant.
  Standard_EXPORT virtual Standard_Boolean NewParameter(const TopoVertex& V,
                                                        const TopoEdge&   E,
                                                        Standard_Real&       P,
                                                        Standard_Real&       Tol) = 0;

  //! Returns the  continuity of  <NewE> between <NewF1>
  //! and <NewF2>.
  //! <NewE> is the new  edge created from <E>.  <NewF1>
  //! (resp. <NewF2>) is the new  face created from <F1>
  //! (resp. <F2>).
  Standard_EXPORT virtual GeomAbs_Shape Continuity(const TopoEdge& E,
                                                   const TopoFace& F1,
                                                   const TopoFace& F2,
                                                   const TopoEdge& NewE,
                                                   const TopoFace& NewF1,
                                                   const TopoFace& NewF2) = 0;

  DEFINE_STANDARD_RTTIEXT(ShapeModification, RefObject)

protected:
private:
};

#endif // _BRepTools_Modification_HeaderFile
