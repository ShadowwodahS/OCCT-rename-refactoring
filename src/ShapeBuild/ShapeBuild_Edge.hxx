// Created on: 1998-06-09
// Created by: data exchange team
// Copyright (c) 1998-1999 Matra Datavision
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

#ifndef _ShapeBuild_Edge_HeaderFile
#define _ShapeBuild_Edge_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

class TopoEdge;
class TopoVertex;
class TopoFace;
class GeomSurface;
class TopLoc_Location;
class GeomCurve2d;
class Transform2d;
class GeomCurve3d;

//! This class provides low-level operators for building an edge
//! 3d curve, copying edge with replaced vertices etc.
class ShapeBuild_Edge
{
public:
  DEFINE_STANDARD_ALLOC

  //! Copy edge and replace one or both its vertices to a given
  //! one(s). Vertex V1 replaces FORWARD vertex, and V2 - REVERSED,
  //! as they are found by TopoDS_Iterator.
  //! If V1 or V2 is NULL, the original vertex is taken
  Standard_EXPORT TopoEdge CopyReplaceVertices(const TopoEdge&   edge,
                                                  const TopoVertex& V1,
                                                  const TopoVertex& V2) const;

  //! Copies ranges for curve3d and all common pcurves from
  //! edge <fromedge> into edge <toedge>.
  Standard_EXPORT void CopyRanges(const TopoEdge&  toedge,
                                  const TopoEdge&  fromedge,
                                  const Standard_Real alpha = 0,
                                  const Standard_Real beta  = 1) const;

  //! Sets range on 3d curve only.
  Standard_EXPORT void SetRange3d(const TopoEdge&  edge,
                                  const Standard_Real first,
                                  const Standard_Real last) const;

  //! Makes a copy of pcurves from edge <fromedge> into edge
  //! <toedge>. Pcurves which are already present in <toedge>,
  //! are replaced by copies, other are copied. Ranges are also
  //! copied.
  Standard_EXPORT void CopyPCurves(const TopoEdge& toedge, const TopoEdge& fromedge) const;

  //! Make a copy of <edge> by call to CopyReplaceVertices()
  //! (i.e. construct new TEdge with the same pcurves and vertices).
  //! If <sharepcurves> is False, pcurves are also replaced by
  //! their copies with help of method CopyPCurves
  Standard_EXPORT TopoEdge Copy(const TopoEdge&     edge,
                                   const Standard_Boolean sharepcurves = Standard_True) const;

  //! Removes the PCurve(s) which could be recorded in an Edge for
  //! the given Face
  Standard_EXPORT void RemovePCurve(const TopoEdge& edge, const TopoFace& face) const;

  //! Removes the PCurve(s) which could be recorded in an Edge for
  //! the given Surface
  Standard_EXPORT void RemovePCurve(const TopoEdge&          edge,
                                    const Handle(GeomSurface)& surf) const;

  //! Removes the PCurve(s) which could be recorded in an Edge for
  //! the given Surface, with given Location
  Standard_EXPORT void RemovePCurve(const TopoEdge&          edge,
                                    const Handle(GeomSurface)& surf,
                                    const TopLoc_Location&      loc) const;

  //! Replace the PCurve in an Edge for the given Face
  //! In case if edge is seam, i.e. has 2 pcurves on that face,
  //! only pcurve corresponding to the orientation of the edge is
  //! replaced
  Standard_EXPORT void ReplacePCurve(const TopoEdge&          edge,
                                     const Handle(GeomCurve2d)& pcurve,
                                     const TopoFace&          face) const;

  //! Reassign edge pcurve lying on face <old> to another face <sub>.
  //! If edge has two pcurves on <old> face, only one of them will be
  //! reassigned, and other will left alone. Similarly, if edge already
  //! had a pcurve on face <sub>, it will have two pcurves on it.
  //! Returns True if succeeded, False if no pcurve lying on <old> found.
  Standard_EXPORT Standard_Boolean ReassignPCurve(const TopoEdge& edge,
                                                  const TopoFace& old,
                                                  const TopoFace& sub) const;

  //! Transforms the PCurve with given matrix and affinity U factor.
  Standard_EXPORT Handle(GeomCurve2d) TransformPCurve(const Handle(GeomCurve2d)& pcurve,
                                                       const Transform2d&            trans,
                                                       const Standard_Real         uFact,
                                                       Standard_Real&              aFirst,
                                                       Standard_Real&              aLast) const;

  //! Removes the Curve3D recorded in an Edge
  Standard_EXPORT void RemoveCurve3d(const TopoEdge& edge) const;

  //! Calls BRepTools1::BuildCurve3D
  Standard_EXPORT Standard_Boolean BuildCurve3d(const TopoEdge& edge) const;

  //! Makes edge with curve and location
  Standard_EXPORT void MakeEdge(TopoEdge&              edge,
                                const Handle(GeomCurve3d)& curve,
                                const TopLoc_Location&    L) const;

  //! Makes edge with curve, location and range [p1, p2]
  Standard_EXPORT void MakeEdge(TopoEdge&              edge,
                                const Handle(GeomCurve3d)& curve,
                                const TopLoc_Location&    L,
                                const Standard_Real       p1,
                                const Standard_Real       p2) const;

  //! Makes edge with pcurve and face
  Standard_EXPORT void MakeEdge(TopoEdge&                edge,
                                const Handle(GeomCurve2d)& pcurve,
                                const TopoFace&          face) const;

  //! Makes edge with pcurve, face and range [p1, p2]
  Standard_EXPORT void MakeEdge(TopoEdge&                edge,
                                const Handle(GeomCurve2d)& pcurve,
                                const TopoFace&          face,
                                const Standard_Real         p1,
                                const Standard_Real         p2) const;

  //! Makes edge with pcurve, surface and location
  Standard_EXPORT void MakeEdge(TopoEdge&                edge,
                                const Handle(GeomCurve2d)& pcurve,
                                const Handle(GeomSurface)& S,
                                const TopLoc_Location&      L) const;

  //! Makes edge with pcurve, surface, location and range [p1, p2]
  Standard_EXPORT void MakeEdge(TopoEdge&                edge,
                                const Handle(GeomCurve2d)& pcurve,
                                const Handle(GeomSurface)& S,
                                const TopLoc_Location&      L,
                                const Standard_Real         p1,
                                const Standard_Real         p2) const;

protected:
private:
};

#endif // _ShapeBuild_Edge_HeaderFile
