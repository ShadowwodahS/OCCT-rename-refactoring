// Created on: 1997-03-03
// Created by: Jean-Pierre COMBE
// Copyright (c) 1997-1999 Matra Datavision
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

#ifndef _PrsDim_IdenticRelation_HeaderFile
#define _PrsDim_IdenticRelation_HeaderFile

#include <PrsDim_Relation.hxx>
#include <gp_Pnt.hxx>
#include <PrsMgr_PresentationManager.hxx>
#include <SelectMgr_Selection.hxx>
#include <TColStd_ListOfTransient.hxx>

class TopoShape;
class GeomPlane;
class GeomLine;
class GeomCircle;
class Geom_Ellipse;
class TopoWire;
class TopoVertex;
class Dir3d;

DEFINE_STANDARD_HANDLE(PrsDim_IdenticRelation, PrsDim_Relation)

//! Constructs a constraint by a relation of identity
//! between two or more datums figuring in shape
//! Interactive Objects.
class PrsDim_IdenticRelation : public PrsDim_Relation
{
  DEFINE_STANDARD_RTTIEXT(PrsDim_IdenticRelation, PrsDim_Relation)
public:
  //! Initializes the relation of identity between the two
  //! entities, FirstShape and SecondShape. The plane
  //! aPlane is initialized in case a visual reference is
  //! needed to show identity.
  Standard_EXPORT PrsDim_IdenticRelation(const TopoShape&       FirstShape,
                                         const TopoShape&       SecondShape,
                                         const Handle(GeomPlane)& aPlane);

  Standard_Boolean HasUsers() const { return !myUsers.IsEmpty(); }

  const TColStd_ListOfTransient& Users() const { return myUsers; }

  void AddUser(const Handle(RefObject)& theUser) { myUsers.Append(theUser); }

  void ClearUsers() { myUsers.Clear(); }

  //! Returns true if the interactive object is movable.
  virtual Standard_Boolean IsMovable() const Standard_OVERRIDE { return Standard_True; }

private:
  Standard_EXPORT virtual void Compute(const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                                       const Handle(Prs3d_Presentation)&         thePrs,
                                       const Standard_Integer theMode) Standard_OVERRIDE;

  Standard_EXPORT virtual void ComputeSelection(const Handle(SelectionContainer)& theSel,
                                                const Standard_Integer theMode) Standard_OVERRIDE;

  Standard_EXPORT void ComputeOneEdgeOVertexPresentation(
    const Handle(Prs3d_Presentation)& aPresentation);

  Standard_EXPORT void ComputeTwoEdgesPresentation(const Handle(Prs3d_Presentation)& aPresentation);

  Standard_EXPORT void ComputeTwoLinesPresentation(const Handle(Prs3d_Presentation)& aPresentation,
                                                   const Handle(GeomLine)&          aLin,
                                                   Point3d&                           Pnt1On1,
                                                   Point3d&                           Pnt2On1,
                                                   Point3d&                           Pnt1On2,
                                                   Point3d&                           Pnt2On2,
                                                   const Standard_Boolean            isInf1,
                                                   const Standard_Boolean            isInf2);

  Standard_EXPORT void ComputeTwoCirclesPresentation(
    const Handle(Prs3d_Presentation)& aPresentation,
    const Handle(GeomCircle)&        aCircle,
    const Point3d&                     Pnt1On1,
    const Point3d&                     Pnt2On1,
    const Point3d&                     Pnt1On2,
    const Point3d&                     Pnt2On2);

  //! Computes the presentation of the identic constraint
  //! between 2 arcs in the case of automatic presentation
  Standard_EXPORT void ComputeAutoArcPresentation(const Handle(GeomCircle)& aCircle,
                                                  const Point3d&              firstp,
                                                  const Point3d&              lastp,
                                                  const Standard_Boolean isstatic = Standard_False);

  //! Computes the presentation of the identic constraint
  //! between 2 circles in the case of non automatic presentation
  Standard_EXPORT void ComputeNotAutoCircPresentation(const Handle(GeomCircle)& aCircle);

  //! Computes the presentation of the identic constraint
  //! between 2 arcs in the case of non automatic presentation
  Standard_EXPORT void ComputeNotAutoArcPresentation(const Handle(GeomCircle)& aCircle,
                                                     const Point3d&              pntfirst,
                                                     const Point3d&              pntlast);

  Standard_EXPORT void ComputeTwoEllipsesPresentation(const Handle(Prs3d_Presentation)& aPrs,
                                                      const Handle(Geom_Ellipse)&       anEll,
                                                      const Point3d&                     Pnt1On1,
                                                      const Point3d&                     Pnt2On1,
                                                      const Point3d&                     Pnt1On2,
                                                      const Point3d&                     Pnt2On2);

  //! Computes the presentation of the identic constraint
  //! between 2 arcs in the case of automatic presentation
  Standard_EXPORT void ComputeAutoArcPresentation(const Handle(Geom_Ellipse)& theEll,
                                                  const Point3d&               firstp,
                                                  const Point3d&               lastp,
                                                  const Standard_Boolean isstatic = Standard_False);

  //! Computes the presentation of the identic constraint
  //! between 2 ellipses in the case of non automatic presentation
  Standard_EXPORT void ComputeNotAutoElipsPresentation(const Handle(Geom_Ellipse)& theEll);

  //! Computes the presentation of the identic constraint
  //! between 2 arcs in the case of non automatic presentation
  Standard_EXPORT void ComputeNotAutoArcPresentation(const Handle(Geom_Ellipse)& theEll,
                                                     const Point3d&               pntfirst,
                                                     const Point3d&               pntlast);

  Standard_EXPORT void ComputeTwoVerticesPresentation(
    const Handle(Prs3d_Presentation)& aPresentation);

  Standard_EXPORT Standard_Real ComputeSegSize() const;

  Standard_EXPORT Standard_Boolean ComputeDirection(const TopoWire&   aWire,
                                                    const TopoVertex& aVertex,
                                                    Dir3d&              aDir) const;

  Standard_EXPORT Dir3d ComputeLineDirection(const Handle(GeomLine)& aLin,
                                              const Point3d&            anExtremity) const;

  Standard_EXPORT Dir3d ComputeCircleDirection(const Handle(GeomCircle)& aCirc,
                                                const TopoVertex&       ConnectedVertex) const;

private:
  TColStd_ListOfTransient myUsers;
  Standard_Boolean        isCircle;
  Point3d                  myFAttach;
  Point3d                  mySAttach;
  Point3d                  myCenter;
};

#endif // _PrsDim_IdenticRelation_HeaderFile
