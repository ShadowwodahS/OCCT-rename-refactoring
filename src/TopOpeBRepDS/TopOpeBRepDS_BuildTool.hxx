// Created on: 1993-06-17
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

#ifndef _TopOpeBRepDS_BuildTool_HeaderFile
#define _TopOpeBRepDS_BuildTool_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <BRep_Builder.hxx>
#include <TopOpeBRepTool_CurveTool.hxx>
#include <Standard_Boolean.hxx>
#include <TopOpeBRepTool_OutCurveType.hxx>
#include <Standard_Real.hxx>
#include <Standard_Integer.hxx>
#include <TopAbs_Orientation.hxx>
class GeomTool1;
class TopoShape;
class Point1;
class TopOpeBRepDS_Curve;
class TopOpeBRepDS_DataStructure;
class GeomCurve3d;
class TopOpeBRepDS_Surface;
class TopoEdge;
class TopoVertex;
class TopoFace;
class TopOpeBRepDS_HDataStructure;
class GeomCurve2d;
class GeomSurface;

//! Provides  a  Tool  to  build  topologies. Used  to
//! instantiate the Builder algorithm.
class TopOpeBRepDS_BuildTool
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TopOpeBRepDS_BuildTool();

  Standard_EXPORT TopOpeBRepDS_BuildTool(const TopOpeBRepTool_OutCurveType OutCurveType);

  Standard_EXPORT TopOpeBRepDS_BuildTool(const GeomTool1& GT);

  Standard_EXPORT const GeomTool1& GetGeomTool() const;

  Standard_EXPORT GeomTool1& ChangeGeomTool();

  Standard_EXPORT void MakeVertex(TopoShape& V, const Point1& P) const;

  Standard_EXPORT void MakeEdge(TopoShape& E, const TopOpeBRepDS_Curve& C) const;

  Standard_EXPORT void MakeEdge(TopoShape&                     E,
                                const TopOpeBRepDS_Curve&         C,
                                const TopOpeBRepDS_DataStructure& DS) const;

  Standard_EXPORT void MakeEdge(TopoShape&             E,
                                const Handle(GeomCurve3d)& C,
                                const Standard_Real       Tol) const;

  Standard_EXPORT void MakeEdge(TopoShape& E) const;

  Standard_EXPORT void MakeWire(TopoShape& W) const;

  Standard_EXPORT void MakeFace(TopoShape& F, const TopOpeBRepDS_Surface& S) const;

  Standard_EXPORT void MakeShell(TopoShape& Sh) const;

  Standard_EXPORT void MakeSolid(TopoShape& S) const;

  //! Make an edge <Eou> with the curve of the edge <Ein>
  Standard_EXPORT void CopyEdge(const TopoShape& Ein, TopoShape& Eou) const;

  Standard_EXPORT void GetOrientedEdgeVertices(TopoEdge&   E,
                                               TopoVertex& Vmin,
                                               TopoVertex& Vmax,
                                               Standard_Real& Parmin,
                                               Standard_Real& Parmax) const;

  Standard_EXPORT void UpdateEdgeCurveTol(const TopoFace&        F1,
                                          const TopoFace&        F2,
                                          TopoEdge&              E,
                                          const Handle(GeomCurve3d)& C3Dnew,
                                          const Standard_Real       tol3d,
                                          const Standard_Real       tol2d1,
                                          const Standard_Real       tol2d2,
                                          Standard_Real&            newtol,
                                          Standard_Real&            newparmin,
                                          Standard_Real&            newparmax) const;

  Standard_EXPORT void ApproxCurves(const TopOpeBRepDS_Curve&                  C,
                                    TopoEdge&                               E,
                                    Standard_Integer&                          inewC,
                                    const Handle(TopOpeBRepDS_HDataStructure)& HDS) const;

  Standard_EXPORT void ComputePCurves(const TopOpeBRepDS_Curve& C,
                                      TopoEdge&              E,
                                      TopOpeBRepDS_Curve&       newC,
                                      const Standard_Boolean    CompPC1,
                                      const Standard_Boolean    CompPC2,
                                      const Standard_Boolean    CompC3D) const;

  Standard_EXPORT void PutPCurves(const TopOpeBRepDS_Curve& newC,
                                  TopoEdge&              E,
                                  const Standard_Boolean    CompPC1,
                                  const Standard_Boolean    CompPC2) const;

  Standard_EXPORT void RecomputeCurves(const TopOpeBRepDS_Curve&                  C,
                                       const TopoEdge&                         oldE,
                                       TopoEdge&                               E,
                                       Standard_Integer&                          inewC,
                                       const Handle(TopOpeBRepDS_HDataStructure)& HDS) const;

  //! Make a face <Fou> with the surface of the face <Fin>
  Standard_EXPORT void CopyFace(const TopoShape& Fin, TopoShape& Fou) const;

  Standard_EXPORT void AddEdgeVertex(const TopoShape& Ein,
                                     TopoShape&       Eou,
                                     const TopoShape& V) const;

  Standard_EXPORT void AddEdgeVertex(TopoShape& E, const TopoShape& V) const;

  Standard_EXPORT void AddWireEdge(TopoShape& W, const TopoShape& E) const;

  Standard_EXPORT void AddFaceWire(TopoShape& F, const TopoShape& W) const;

  Standard_EXPORT void AddShellFace(TopoShape& Sh, const TopoShape& F) const;

  Standard_EXPORT void AddSolidShell(TopoShape& S, const TopoShape& Sh) const;

  //! Sets the parameter <P>  for  the vertex <V> on the
  //! edge <E>.
  Standard_EXPORT void Parameter(const TopoShape& E,
                                 const TopoShape& V,
                                 const Standard_Real P) const;

  //! Sets the range of edge <E>.
  Standard_EXPORT void Range(const TopoShape& E,
                             const Standard_Real first,
                             const Standard_Real last) const;

  //! Sets the range of edge <Eou> from <Ein>
  //! only when <Ein> has a closed geometry.
  Standard_EXPORT void UpdateEdge(const TopoShape& Ein, TopoShape& Eou) const;

  //! Compute the parameter of  the vertex <V>, supported
  //! by   the edge <E>, on the curve  <C>.
  Standard_EXPORT void Parameter(const TopOpeBRepDS_Curve& C,
                                 TopoShape&             E,
                                 TopoShape&             V) const;

  //! Sets the  curve <C> for the edge  <E>
  Standard_EXPORT void Curve3D(TopoShape&             E,
                               const Handle(GeomCurve3d)& C,
                               const Standard_Real       Tol) const;

  //! Sets  the pcurve <C> for  the edge <E> on the face
  //! <F>.  If OverWrite is True the old pcurve if there
  //! is one  is overwritten, else the  two  pcurves are
  //! set.
  Standard_EXPORT void PCurve(TopoShape&               F,
                              TopoShape&               E,
                              const Handle(GeomCurve2d)& C) const;

  Standard_EXPORT void PCurve(TopoShape&               F,
                              TopoShape&               E,
                              const TopOpeBRepDS_Curve&   CDS,
                              const Handle(GeomCurve2d)& C) const;

  Standard_EXPORT void Orientation(TopoShape& S, const TopAbs_Orientation O) const;

  Standard_EXPORT TopAbs_Orientation Orientation(const TopoShape& S) const;

  Standard_EXPORT void Closed(TopoShape& S, const Standard_Boolean B) const;

  Standard_EXPORT Standard_Boolean Approximation() const;

  Standard_EXPORT void UpdateSurface(const TopoShape& F, const Handle(GeomSurface)& SU) const;

  Standard_EXPORT void UpdateSurface(const TopoShape& E,
                                     const TopoShape& oldF,
                                     const TopoShape& newF) const;

  Standard_EXPORT Standard_Boolean OverWrite() const;

  Standard_EXPORT void OverWrite(const Standard_Boolean O);

  Standard_EXPORT Standard_Boolean Translate() const;

  Standard_EXPORT void Translate(const Standard_Boolean T);

protected:
private:
  Standard_EXPORT void TranslateOnPeriodic(TopoShape&         F,
                                           TopoShape&         E,
                                           Handle(GeomCurve2d)& C) const;

  ShapeBuilder             myBuilder;
  TopOpeBRepTool_CurveTool myCurveTool;
  Standard_Boolean         myOverWrite;
  Standard_Boolean         myTranslate;
};

#endif // _TopOpeBRepDS_BuildTool_HeaderFile
