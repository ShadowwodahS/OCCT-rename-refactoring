// Created on: 1996-12-05
// Created by: Flore Lantheaume/Odile Olivier
// Copyright (c) 1996-1999 Matra Datavision
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

#ifndef _PrsDim_FixRelation_HeaderFile
#define _PrsDim_FixRelation_HeaderFile

#include <gp_Circ.hxx>
#include <TopoDS_Wire.hxx>
#include <PrsDim_Relation.hxx>

class GeomPlane;

DEFINE_STANDARD_HANDLE(PrsDim_FixRelation, PrsDim_Relation)

//! Constructs and manages a constraint by a fixed
//! relation between two or more interactive datums. This
//! constraint is represented by a wire from a shape -
//! point, vertex, or edge - in the first datum and a
//! corresponding shape in the second.
//! Warning: This relation is not bound with any kind of parametric
//! constraint : it represents the "status" of an parametric
//! object.
class PrsDim_FixRelation : public PrsDim_Relation
{
  DEFINE_STANDARD_RTTIEXT(PrsDim_FixRelation, PrsDim_Relation)
public:
  //! initializes the vertex aShape, the
  //! plane aPlane and the wire aWire, which connects
  //! the two vertices in a fixed relation.
  Standard_EXPORT PrsDim_FixRelation(const TopoShape&       aShape,
                                     const Handle(GeomPlane)& aPlane,
                                     const TopoWire&        aWire);

  //! initializes the vertex aShape, the
  //! plane aPlane and the wire aWire, the position
  //! aPosition, the arrow size anArrowSize and the
  //! wire aWire, which connects the two vertices in a fixed relation.
  Standard_EXPORT PrsDim_FixRelation(const TopoShape&       aShape,
                                     const Handle(GeomPlane)& aPlane,
                                     const TopoWire&        aWire,
                                     const Point3d&             aPosition,
                                     const Standard_Real       anArrowSize = 0.01);

  //! initializes the edge aShape and the plane aPlane.
  Standard_EXPORT PrsDim_FixRelation(const TopoShape& aShape, const Handle(GeomPlane)& aPlane);

  //! initializes the edge aShape, the
  //! plane aPlane, the position aPosition and the arrow
  //! size anArrowSize.
  Standard_EXPORT PrsDim_FixRelation(const TopoShape&       aShape,
                                     const Handle(GeomPlane)& aPlane,
                                     const Point3d&             aPosition,
                                     const Standard_Real       anArrowSize = 0.01);

  //! Returns the wire which connects vertices in a fixed relation.
  const TopoWire& Wire() { return myWire; }

  //! Constructs the wire aWire. This connects vertices
  //! which are in a fixed relation.
  void SetWire(const TopoWire& aWire) { myWire = aWire; }

  //! Returns true if the Interactive Objects in the relation
  //! are movable.
  virtual Standard_Boolean IsMovable() const Standard_OVERRIDE { return Standard_True; }

private:
  Standard_EXPORT virtual void Compute(const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                                       const Handle(Prs3d_Presentation)&         thePrs,
                                       const Standard_Integer theMode) Standard_OVERRIDE;

  Standard_EXPORT virtual void ComputeSelection(const Handle(SelectionContainer)& theSel,
                                                const Standard_Integer theMode) Standard_OVERRIDE;

  //! computes the presentation for <myFixShape> if it's a vertex.
  Standard_EXPORT void ComputeVertex(const TopoVertex& FixVertex, Point3d& curpos);

  Standard_EXPORT Point3d ComputePosition(const Handle(GeomCurve3d)& curv1,
                                         const Handle(GeomCurve3d)& curv2,
                                         const Point3d&             firstp1,
                                         const Point3d&             lastp1,
                                         const Point3d&             firstp2,
                                         const Point3d&             lastp2) const;

  Standard_EXPORT Point3d ComputePosition(const Handle(GeomCurve3d)& curv,
                                         const Point3d&             firstp,
                                         const Point3d&             lastp) const;

  //! computes the presentation for <myFixShape> if it's a
  //! edge.
  Standard_EXPORT void ComputeEdge(const TopoEdge& FixEdge, Point3d& curpos);

  Standard_EXPORT void ComputeLinePosition(const gp_Lin&  glin,
                                           Point3d&        pos,
                                           Standard_Real& pfirst,
                                           Standard_Real& plast);

  Standard_EXPORT void ComputeCirclePosition(const gp_Circ& gcirc,
                                             Point3d&        pos,
                                             Standard_Real& pfirst,
                                             Standard_Real& plast);

  Standard_EXPORT static Standard_Boolean ConnectedEdges(const TopoWire&   aWire,
                                                         const TopoVertex& aVertex,
                                                         TopoEdge&         Edge1,
                                                         TopoEdge&         Edge2);

private:
  TopoWire myWire;
  Point3d      myPntAttach;
};

#endif // _PrsDim_FixRelation_HeaderFile
