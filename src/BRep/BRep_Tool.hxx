// Created on: 1993-07-07
// Created by: Remi LEQUETTE
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

#ifndef _BRep_Tool_HeaderFile
#define _BRep_Tool_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <GeomAbs_Shape.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Pnt.hxx>
#include <Poly_ListOfTriangulation.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_Polygon2D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <TopAbs_ShapeEnum.hxx>

class TopoShape;
class TopoFace;
class TopLoc_Location;
class TopoEdge;
class TopoVertex;

//! Provides class methods  to  access to the geometry
//! of BRep shapes.
class BRepInspector
{
public:
  DEFINE_STANDARD_ALLOC

  //! If S is Shell, returns True if it has no free boundaries (edges).
  //! If S is Wire, returns True if it has no free ends (vertices).
  //! (Internal and External sub-shepes are ignored in these checks)
  //! If S is Edge, returns True if its vertices are the same.
  //! For other shape types returns S.Closed().
  Standard_EXPORT static Standard_Boolean IsClosed(const TopoShape& S);

  //! Returns the geometric surface of the face. Returns
  //! in <L> the location for the surface.
  Standard_EXPORT static const Handle(GeomSurface)& Surface(const TopoFace& F,
                                                             TopLoc_Location&   L);

  //! Returns the geometric  surface of the face. It can
  //! be a copy if there is a Location.
  Standard_EXPORT static Handle(GeomSurface) Surface(const TopoFace& F);

  //! Returns the triangulation of the face according to the mesh purpose.
  //! @param[in] theFace  the input face to find triangulation.
  //! @param[out] theLocation  the face location.
  //! @param[in] theMeshPurpose  a mesh purpose to find appropriate triangulation (NONE by default).
  //! @return an active triangulation in case of NONE purpose,
  //!         the first triangulation appropriate for the input purpose,
  //!         just the first triangulation if none matching other criteria and input purpose is
  //!         AnyFallback or null handle if there is no any suitable triangulation.
  Standard_EXPORT static const Handle(MeshTriangulation)& Triangulation(
    const TopoFace&     theFace,
    TopLoc_Location&       theLocation,
    const Poly_MeshPurpose theMeshPurpose = Poly_MeshPurpose_NONE);

  //! Returns all triangulations of the face.
  //! @param[in] theFace  the input face.
  //! @param[out] theLocation  the face location.
  //! @return list of all available face triangulations.
  Standard_EXPORT static const Poly_ListOfTriangulation& Triangulations(
    const TopoFace& theFace,
    TopLoc_Location&   theLocation);

  //! Returns the tolerance of the face.
  Standard_EXPORT static Standard_Real Tolerance(const TopoFace& F);

  //! Returns the  NaturalRestriction  flag of the  face.
  Standard_EXPORT static Standard_Boolean NaturalRestriction(const TopoFace& F);

  //! Returns True if <F> has a surface, false otherwise.
  Standard_EXPORT static Standard_Boolean IsGeometric(const TopoFace& F);

  //! Returns True if <E> is a 3d curve or a curve on
  //! surface.
  Standard_EXPORT static Standard_Boolean IsGeometric(const TopoEdge& E);

  //! Returns the 3D curve  of the edge.  May be  a Null
  //! handle. Returns in <L> the location for the curve.
  //! In <First> and <Last> the parameter range.
  Standard_EXPORT static const Handle(GeomCurve3d)& Curve(const TopoEdge& E,
                                                         TopLoc_Location&   L,
                                                         Standard_Real&     First,
                                                         Standard_Real&     Last);

  //! Returns the 3D curve  of the edge. May be a Null handle.
  //! In <First> and <Last> the parameter range.
  //! It can be a copy if there is a Location.
  Standard_EXPORT static Handle(GeomCurve3d) Curve(const TopoEdge& E,
                                                  Standard_Real&     First,
                                                  Standard_Real&     Last);

  //! Returns the 3D polygon of the edge. May be   a Null
  //! handle. Returns in <L> the location for the polygon.
  Standard_EXPORT static const Handle(Poly_Polygon3D)& Polygon3D(const TopoEdge& E,
                                                                 TopLoc_Location&   L);

  //! Returns the curve  associated to the  edge in  the
  //! parametric  space of  the  face.  Returns   a NULL
  //! handle  if this curve  does not exist.  Returns in
  //! <First> and <Last> the parameter range.
  //! If the surface is a plane the curve can be not stored but created a new
  //! each time. The flag pointed by <theIsStored> serves to indicate storage status.
  //! It is valued if the pointer is non-null.
  Standard_EXPORT static Handle(GeomCurve2d) CurveOnSurface(const TopoEdge& E,
                                                             const TopoFace& F,
                                                             Standard_Real&     First,
                                                             Standard_Real&     Last,
                                                             Standard_Boolean*  theIsStored = NULL);

  //! Returns the  curve associated to   the edge in the
  //! parametric  space of the   surface. Returns a NULL
  //! handle  if this curve does  not exist.  Returns in
  //! <First> and <Last> the parameter range.
  //! If the surface is a plane the curve can be not stored but created a new
  //! each time. The flag pointed by <theIsStored> serves to indicate storage status.
  //! It is valued if the pointer is non-null.
  Standard_EXPORT static Handle(GeomCurve2d) CurveOnSurface(const TopoEdge&          E,
                                                             const Handle(GeomSurface)& S,
                                                             const TopLoc_Location&      L,
                                                             Standard_Real&              First,
                                                             Standard_Real&              Last,
                                                             Standard_Boolean* theIsStored = NULL);

  //! For the planar surface builds the 2d curve for the edge
  //! by projection of the edge on plane.
  //! Returns a NULL handle if the surface is not planar or
  //! the projection failed.
  Standard_EXPORT static Handle(GeomCurve2d) CurveOnPlane(const TopoEdge&          E,
                                                           const Handle(GeomSurface)& S,
                                                           const TopLoc_Location&      L,
                                                           Standard_Real&              First,
                                                           Standard_Real&              Last);

  //! Returns in <C>, <S>, <L> a 2d curve, a surface and
  //! a location for the edge <E>. <C> and <S>  are null
  //! if the  edge has no curve on  surface.  Returns in
  //! <First> and <Last> the parameter range.
  Standard_EXPORT static void CurveOnSurface(const TopoEdge&    E,
                                             Handle(GeomCurve2d)& C,
                                             Handle(GeomSurface)& S,
                                             TopLoc_Location&      L,
                                             Standard_Real&        First,
                                             Standard_Real&        Last);

  //! Returns in <C>, <S>, <L> the 2d curve, the surface
  //! and the location for the edge <E> of rank <Index>.
  //! <C> and <S> are null if the index is out of range.
  //! Returns in <First> and <Last> the parameter range.
  Standard_EXPORT static void CurveOnSurface(const TopoEdge&     E,
                                             Handle(GeomCurve2d)&  C,
                                             Handle(GeomSurface)&  S,
                                             TopLoc_Location&       L,
                                             Standard_Real&         First,
                                             Standard_Real&         Last,
                                             const Standard_Integer Index);

  //! Returns the polygon associated to the  edge in  the
  //! parametric  space of  the  face.  Returns   a NULL
  //! handle  if this polygon  does not exist.
  Standard_EXPORT static Handle(Poly_Polygon2D) PolygonOnSurface(const TopoEdge& E,
                                                                 const TopoFace& F);

  //! Returns the polygon associated to the  edge in  the
  //! parametric  space of  the surface. Returns   a NULL
  //! handle  if this polygon  does not exist.
  Standard_EXPORT static Handle(Poly_Polygon2D) PolygonOnSurface(const TopoEdge&          E,
                                                                 const Handle(GeomSurface)& S,
                                                                 const TopLoc_Location&      L);

  //! Returns in <C>, <S>, <L> a 2d curve, a surface and
  //! a location for the edge <E>. <C> and <S>  are null
  //! if the  edge has no polygon on  surface.
  Standard_EXPORT static void PolygonOnSurface(const TopoEdge&      E,
                                               Handle(Poly_Polygon2D)& C,
                                               Handle(GeomSurface)&   S,
                                               TopLoc_Location&        L);

  //! Returns in <C>, <S>, <L> the 2d curve, the surface
  //! and the location for the edge <E> of rank <Index>.
  //! <C> and <S> are null if the index is out of range.
  Standard_EXPORT static void PolygonOnSurface(const TopoEdge&      E,
                                               Handle(Poly_Polygon2D)& C,
                                               Handle(GeomSurface)&   S,
                                               TopLoc_Location&        L,
                                               const Standard_Integer  Index);

  //! Returns the polygon associated to the  edge in  the
  //! parametric  space of  the  face.  Returns   a NULL
  //! handle  if this polygon  does not exist.
  Standard_EXPORT static const Handle(Poly_PolygonOnTriangulation)& PolygonOnTriangulation(
    const TopoEdge&                E,
    const Handle(MeshTriangulation)& T,
    const TopLoc_Location&            L);

  //! Returns in <P>, <T>, <L> a polygon on triangulation, a
  //! triangulation and a location for the edge <E>.
  //! <P>  and  <T>  are null  if  the  edge has no
  //! polygon on  triangulation.
  Standard_EXPORT static void PolygonOnTriangulation(const TopoEdge&                   E,
                                                     Handle(Poly_PolygonOnTriangulation)& P,
                                                     Handle(MeshTriangulation)&          T,
                                                     TopLoc_Location&                     L);

  //! Returns   in   <P>,  <T>,    <L> a     polygon  on
  //! triangulation,   a triangulation  and a  location for
  //! the edge <E> for the range index.  <C> and <S> are
  //! null if the edge has no polygon on triangulation.
  Standard_EXPORT static void PolygonOnTriangulation(const TopoEdge&                   E,
                                                     Handle(Poly_PolygonOnTriangulation)& P,
                                                     Handle(MeshTriangulation)&          T,
                                                     TopLoc_Location&                     L,
                                                     const Standard_Integer               Index);

  //! Returns  True  if  <E>  has  two  PCurves  in  the
  //! parametric space of <F>. i.e.  <F>  is on a closed
  //! surface and <E> is on the closing curve.
  Standard_EXPORT static Standard_Boolean IsClosed(const TopoEdge& E, const TopoFace& F);

  //! Returns  True  if  <E>  has  two  PCurves  in  the
  //! parametric space  of <S>.  i.e.   <S>  is a closed
  //! surface and <E> is on the closing curve.
  Standard_EXPORT static Standard_Boolean IsClosed(const TopoEdge&          E,
                                                   const Handle(GeomSurface)& S,
                                                   const TopLoc_Location&      L);

  //! Returns  True  if <E> has two arrays of indices in
  //! the triangulation <T>.
  Standard_EXPORT static Standard_Boolean IsClosed(const TopoEdge&                E,
                                                   const Handle(MeshTriangulation)& T,
                                                   const TopLoc_Location&            L);

  //! Returns the tolerance for <E>.
  Standard_EXPORT static Standard_Real Tolerance(const TopoEdge& E);

  //! Returns the SameParameter flag for the edge.
  Standard_EXPORT static Standard_Boolean SameParameter(const TopoEdge& E);

  //! Returns the SameRange flag for the edge.
  Standard_EXPORT static Standard_Boolean SameRange(const TopoEdge& E);

  //! Returns True  if the edge is degenerated.
  Standard_EXPORT static Standard_Boolean Degenerated(const TopoEdge& E);

  //! Gets the range of the 3d curve.
  Standard_EXPORT static void Range(const TopoEdge& E,
                                    Standard_Real&     First,
                                    Standard_Real&     Last);

  //! Gets the range  of the edge  on the pcurve on  the
  //! surface.
  Standard_EXPORT static void Range(const TopoEdge&          E,
                                    const Handle(GeomSurface)& S,
                                    const TopLoc_Location&      L,
                                    Standard_Real&              First,
                                    Standard_Real&              Last);

  //! Gets the range of the edge on the pcurve on the face.
  Standard_EXPORT static void Range(const TopoEdge& E,
                                    const TopoFace& F,
                                    Standard_Real&     First,
                                    Standard_Real&     Last);

  //! Gets the UV locations of the extremities of the edge.
  Standard_EXPORT static void UVPoints(const TopoEdge&          E,
                                       const Handle(GeomSurface)& S,
                                       const TopLoc_Location&      L,
                                       gp_Pnt2d&                   PFirst,
                                       gp_Pnt2d&                   PLast);

  //! Gets the UV locations of the extremities of the edge.
  Standard_EXPORT static void UVPoints(const TopoEdge& E,
                                       const TopoFace& F,
                                       gp_Pnt2d&          PFirst,
                                       gp_Pnt2d&          PLast);

  //! Sets the UV locations of the extremities of the edge.
  Standard_EXPORT static void SetUVPoints(const TopoEdge&          E,
                                          const Handle(GeomSurface)& S,
                                          const TopLoc_Location&      L,
                                          const gp_Pnt2d&             PFirst,
                                          const gp_Pnt2d&             PLast);

  //! Sets the UV locations of the extremities of the edge.
  Standard_EXPORT static void SetUVPoints(const TopoEdge& E,
                                          const TopoFace& F,
                                          const gp_Pnt2d&    PFirst,
                                          const gp_Pnt2d&    PLast);

  //! Returns True if the edge is on the surfaces of the
  //! two faces.
  Standard_EXPORT static Standard_Boolean HasContinuity(const TopoEdge& E,
                                                        const TopoFace& F1,
                                                        const TopoFace& F2);

  //! Returns the continuity.
  Standard_EXPORT static GeomAbs_Shape Continuity(const TopoEdge& E,
                                                  const TopoFace& F1,
                                                  const TopoFace& F2);

  //! Returns True if the edge is on the surfaces.
  Standard_EXPORT static Standard_Boolean HasContinuity(const TopoEdge&          E,
                                                        const Handle(GeomSurface)& S1,
                                                        const Handle(GeomSurface)& S2,
                                                        const TopLoc_Location&      L1,
                                                        const TopLoc_Location&      L2);

  //! Returns the continuity.
  Standard_EXPORT static GeomAbs_Shape Continuity(const TopoEdge&          E,
                                                  const Handle(GeomSurface)& S1,
                                                  const Handle(GeomSurface)& S2,
                                                  const TopLoc_Location&      L1,
                                                  const TopLoc_Location&      L2);

  //! Returns True if the edge has regularity on some
  //! two surfaces
  Standard_EXPORT static Standard_Boolean HasContinuity(const TopoEdge& E);

  //! Returns the max continuity of edge between some surfaces or GeomAbs_C0 if there no such
  //! surfaces.
  Standard_EXPORT static GeomAbs_Shape MaxContinuity(const TopoEdge& theEdge);

  //! Returns the 3d point.
  Standard_EXPORT static Point3d Pnt(const TopoVertex& V);

  //! Returns the tolerance.
  Standard_EXPORT static Standard_Real Tolerance(const TopoVertex& V);

  //! Finds the parameter of <theV> on <theE>.
  //! @param[in] theV  input vertex
  //! @param[in] theE  input edge
  //! @param[out] theParam   calculated parameter on the curve
  //! @return TRUE if done
  Standard_EXPORT static Standard_Boolean Parameter(const TopoVertex& theV,
                                                    const TopoEdge&   theE,
                                                    Standard_Real&       theParam);

  //! Returns the parameter of <V> on <E>.
  //! Throws Standard_NoSuchObject if no parameter on edge
  Standard_EXPORT static Standard_Real Parameter(const TopoVertex& V, const TopoEdge& E);

  //! Returns the  parameters  of   the  vertex   on the
  //! pcurve of the edge on the face.
  Standard_EXPORT static Standard_Real Parameter(const TopoVertex& V,
                                                 const TopoEdge&   E,
                                                 const TopoFace&   F);

  //! Returns the  parameters  of   the  vertex   on the
  //! pcurve of the edge on the surface.
  Standard_EXPORT static Standard_Real Parameter(const TopoVertex&        V,
                                                 const TopoEdge&          E,
                                                 const Handle(GeomSurface)& S,
                                                 const TopLoc_Location&      L);

  //! Returns the parameters of the vertex on the face.
  Standard_EXPORT static gp_Pnt2d Parameters(const TopoVertex& V, const TopoFace& F);

  //! Returns the maximum tolerance of input shape subshapes.
  //@param theShape    - Shape to search tolerance.
  //@param theSubShape - Search subshape, only Face, Edge or Vertex are supported.
  Standard_EXPORT static Standard_Real MaxTolerance(const TopoShape&    theShape,
                                                    const TopAbs_ShapeEnum theSubShape);
};

#endif // _BRep_Tool_HeaderFile
