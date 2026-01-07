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

#ifndef _BRepTools_TrsfModification_HeaderFile
#define _BRepTools_TrsfModification_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <gp_Trsf.hxx>
#include <BRepTools_Modification.hxx>
#include <GeomAbs_Shape.hxx>
class TopoFace;
class GeomSurface;
class TopLoc_Location;
class TopoEdge;
class GeomCurve3d;
class TopoVertex;
class Point3d;
class GeomCurve2d;

class BRepTools_TrsfModification;
DEFINE_STANDARD_HANDLE(BRepTools_TrsfModification, BRepTools_Modification)

//! Describes a modification that uses a Transform3d to
//! change the geometry of a shape. All functions return
//! true and transform the geometry of the shape.
class BRepTools_TrsfModification : public BRepTools_Modification
{

public:
  Standard_EXPORT BRepTools_TrsfModification(const Transform3d& T);

  //! Provides access to the Transform3d associated with this
  //! modification. The transformation can be changed.
  Standard_EXPORT Transform3d& Trsf();

  //! Sets a flag to indicate the need to copy mesh.
  Standard_EXPORT Standard_Boolean& IsCopyMesh();

  //! Returns true if the face F has been modified.
  //! If the face has been modified:
  //! - S is the new geometry of the face,
  //! - L is its new location, and
  //! - Tol is the new tolerance.
  //! RevWires is set to true when the modification
  //! reverses the normal of the surface (the wires have to be reversed).
  //! RevFace is set to true if the orientation of the
  //! modified face changes in the shells which contain it.
  //! For this class, RevFace returns true if the Transform3d
  //! associated with this modification is negative.
  Standard_EXPORT Standard_Boolean NewSurface(const TopoFace&    F,
                                              Handle(GeomSurface)& S,
                                              TopLoc_Location&      L,
                                              Standard_Real&        Tol,
                                              Standard_Boolean&     RevWires,
                                              Standard_Boolean&     RevFace) Standard_OVERRIDE;

  //! Returns true if the face has been modified according to changed triangulation.
  //! If the face has been modified:
  //! - T is a new triangulation on the face
  Standard_EXPORT Standard_Boolean
    NewTriangulation(const TopoFace& F, Handle(MeshTriangulation)& T) Standard_OVERRIDE;

  //! Returns true if the edge has been modified according to changed polygon.
  //! If the edge has been modified:
  //! - P is a new polygon
  Standard_EXPORT Standard_Boolean NewPolygon(const TopoEdge&      E,
                                              Handle(Poly_Polygon3D)& P) Standard_OVERRIDE;

  //! Returns true if the edge has been modified according to changed polygon on triangulation.
  //! If the edge has been modified:
  //! - P is a new polygon on triangulation
  Standard_EXPORT Standard_Boolean NewPolygonOnTriangulation(const TopoEdge&                   E,
                                                             const TopoFace&                   F,
                                                             Handle(Poly_PolygonOnTriangulation)& P)
    Standard_OVERRIDE;

  //! Always returns true indicating that the edge E is always modified.
  //! - C is the new geometric support of the edge,
  //! - L is the new location, and
  //! - Tol is the new tolerance.
  Standard_EXPORT Standard_Boolean NewCurve(const TopoEdge&  E,
                                            Handle(GeomCurve3d)& C,
                                            TopLoc_Location&    L,
                                            Standard_Real&      Tol) Standard_OVERRIDE;

  //! Returns true if the vertex V has been modified.
  //! If the vertex has been modified:
  //! - P is the new geometry of the vertex, and
  //! - Tol is the new tolerance.
  //! If the vertex has not been modified this function
  //! returns false, and the values of P and Tol are not significant.
  Standard_EXPORT Standard_Boolean NewPoint(const TopoVertex& V,
                                            Point3d&              P,
                                            Standard_Real&       Tol) Standard_OVERRIDE;

  //! Returns true if the edge E has a new curve on surface on the face F.
  //! If a new curve exists:
  //! - C is the new geometric support of the edge,
  //! - L is the new location, and
  //! - Tol the new tolerance.
  //! If no new curve exists, this function returns false, and
  //! the values of C, L and Tol are not significant.
  Standard_EXPORT Standard_Boolean NewCurve2d(const TopoEdge&    E,
                                              const TopoFace&    F,
                                              const TopoEdge&    NewE,
                                              const TopoFace&    NewF,
                                              Handle(GeomCurve2d)& C,
                                              Standard_Real&        Tol) Standard_OVERRIDE;

  //! Returns true if the Vertex V has a new parameter on the edge E.
  //! If a new parameter exists:
  //! - P is the parameter, and
  //! - Tol is the new tolerance.
  //! If no new parameter exists, this function returns false,
  //! and the values of P and Tol are not significant.
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

  DEFINE_STANDARD_RTTIEXT(BRepTools_TrsfModification, BRepTools_Modification)

protected:
private:
  Transform3d          myTrsf;
  Standard_Boolean myCopyMesh;
};

#endif // _BRepTools_TrsfModification_HeaderFile
