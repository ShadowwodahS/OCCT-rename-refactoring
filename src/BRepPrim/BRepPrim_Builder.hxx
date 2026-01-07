// Created on: 1992-03-12
// Created by: Philippe DAUTRY
// Copyright (c) 1992-1999 Matra Datavision
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

#ifndef _BRepPrim_Builder_HeaderFile
#define _BRepPrim_Builder_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <BRep_Builder.hxx>
class TopoShell;
class TopoFace;
class gp_Pln;
class TopoWire;
class TopoEdge;
class gp_Lin;
class gp_Circ;
class gp_Lin2d;
class gp_Circ2d;
class TopoVertex;
class Point3d;

//! implements the abstract Builder with the BRep Builder
class BRepPrim_Builder
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates an empty, useless  Builder. Necesseray for
  //! compilation.
  Standard_EXPORT BRepPrim_Builder();

  //! Creates from a Builder.
  Standard_EXPORT BRepPrim_Builder(const ShapeBuilder& B);

  const ShapeBuilder& Builder() const;

  //! Make a empty Shell.
  Standard_EXPORT void MakeShell(TopoShell& S) const;

  //! Returns in   <F> a  Face  built  with   the  plane
  //! equation <P>. Used by all primitives.
  Standard_EXPORT void MakeFace(TopoFace& F, const gp_Pln& P) const;

  //! Returns in <W> an empty Wire.
  Standard_EXPORT void MakeWire(TopoWire& W) const;

  //! Returns in <E> a degenerated edge.
  Standard_EXPORT void MakeDegeneratedEdge(TopoEdge& E) const;

  //! Returns   in <E>  an  Edge  built  with  the  line
  //! equation  <L>.
  Standard_EXPORT void MakeEdge(TopoEdge& E, const gp_Lin& L) const;

  //! Returns  in <E>   an  Edge  built  with the circle
  //! equation  <C>.
  Standard_EXPORT void MakeEdge(TopoEdge& E, const gp_Circ& C) const;

  //! Sets the line <L> to be the curve representing the
  //! edge <E> in the parametric space of the surface of
  //! <F>.
  Standard_EXPORT void SetPCurve(TopoEdge& E, const TopoFace& F, const gp_Lin2d& L) const;

  //! Sets the    lines  <L1,L2>  to   be     the curves
  //! representing the edge <E>  in the parametric space
  //! of the closed surface of <F>.
  Standard_EXPORT void SetPCurve(TopoEdge&       E,
                                 const TopoFace& F,
                                 const gp_Lin2d&    L1,
                                 const gp_Lin2d&    L2) const;

  //! Sets the  circle <C> to  be the curve representing
  //! the  edge <E>  in   the  parametric  space of  the
  //! surface of <F>.
  Standard_EXPORT void SetPCurve(TopoEdge& E, const TopoFace& F, const gp_Circ2d& C) const;

  //! Returns in <V> a Vertex built with the point <P>.
  Standard_EXPORT void MakeVertex(TopoVertex& V, const Point3d& P) const;

  //! Reverses the Face <F>.
  Standard_EXPORT void ReverseFace(TopoFace& F) const;

  //! Adds the Vertex <V> in the Edge <E>.  <P> is the
  //! parameter of the vertex on the  edge.  If direct
  //! is False the Vertex is reversed.
  Standard_EXPORT void AddEdgeVertex(TopoEdge&           E,
                                     const TopoVertex&   V,
                                     const Standard_Real    P,
                                     const Standard_Boolean direct) const;

  //! Adds  the Vertex <V>  in the Edge <E>.   <P1,P2>
  //! are the  parameters of the  vertex on the closed
  //! edge.
  Standard_EXPORT void AddEdgeVertex(TopoEdge&         E,
                                     const TopoVertex& V,
                                     const Standard_Real  P1,
                                     const Standard_Real  P2) const;

  //! <P1,P2> are the parameters of the  vertex on the
  //! edge.  The edge is a closed curve.
  Standard_EXPORT void SetParameters(TopoEdge&         E,
                                     const TopoVertex& V,
                                     const Standard_Real  P1,
                                     const Standard_Real  P2) const;

  //! Adds the Edge <E> in the  Wire <W>, if direct is
  //! False the Edge is reversed.
  Standard_EXPORT void AddWireEdge(TopoWire&           W,
                                   const TopoEdge&     E,
                                   const Standard_Boolean direct) const;

  //! Adds the Wire <W> in  the Face <F>.
  Standard_EXPORT void AddFaceWire(TopoFace& F, const TopoWire& W) const;

  //! Adds the Face <F>  in the Shell <Sh>.
  Standard_EXPORT void AddShellFace(TopoShell& Sh, const TopoFace& F) const;

  //! This is called once an edge is completed. It gives
  //! the opportunity to perform any post treatment.
  Standard_EXPORT void CompleteEdge(TopoEdge& E) const;

  //! This is called once a wire is  completed. It gives
  //! the opportunity to perform any post treatment.
  Standard_EXPORT void CompleteWire(TopoWire& W) const;

  //! This is called once a face is  completed. It gives
  //! the opportunity to perform any post treatment.
  Standard_EXPORT void CompleteFace(TopoFace& F) const;

  //! This is called once a shell is  completed. It gives
  //! the opportunity to perform any post treatment.
  Standard_EXPORT void CompleteShell(TopoShell& S) const;

protected:
private:
  ShapeBuilder myBuilder;
};

#include <BRepPrim_Builder.lxx>

#endif // _BRepPrim_Builder_HeaderFile
