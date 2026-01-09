// Created on: 1998-10-29
// Created by: Jean Yves LEBEY
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

#ifndef _TopOpeBRep_Point2d_HeaderFile
#define _TopOpeBRep_Point2d_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <IntRes2d_IntersectionPoint.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopOpeBRepDS_Transition.hxx>
#include <Standard_Integer.hxx>
#include <TopOpeBRep_P2Dstatus.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <TopOpeBRepDS_Config.hxx>
class TopOpeBRep_Hctxff2d;
class TopOpeBRep_Hctxee2d;

// resolve name collisions with X11 headers
#ifdef Status
  #undef Status
#endif

class TopOpeBRep_Point2d
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TopOpeBRep_Point2d();

  Standard_EXPORT void Dump(const Standard_Integer ie1 = 0, const Standard_Integer ie2 = 0) const;

  void SetPint(const IntersectionPoint3& P);

  Standard_Boolean HasPint() const;

  const IntersectionPoint3& Pint() const;

  void SetIsVertex(const Standard_Integer I, const Standard_Boolean B);

  Standard_Boolean IsVertex(const Standard_Integer I) const;

  void SetVertex(const Standard_Integer I, const TopoVertex& V);

  Standard_EXPORT const TopoVertex& Vertex(const Standard_Integer I) const;

  void SetTransition(const Standard_Integer I, const StateTransition& T);

  Standard_EXPORT const StateTransition& Transition(const Standard_Integer I) const;

  Standard_EXPORT StateTransition& ChangeTransition(const Standard_Integer I);

  void SetParameter(const Standard_Integer I, const Standard_Real P);

  Standard_Real Parameter(const Standard_Integer I) const;

  void SetIsPointOfSegment(const Standard_Boolean B);

  Standard_Boolean IsPointOfSegment() const;

  void SetSegmentAncestors(const Standard_Integer IP1, const Standard_Integer IP2);

  Standard_Boolean SegmentAncestors(Standard_Integer& IP1, Standard_Integer& IP2) const;

  void SetStatus(const TopOpeBRep_P2Dstatus S);

  TopOpeBRep_P2Dstatus Status() const;

  void SetIndex(const Standard_Integer X);

  Standard_Integer Index() const;

  void SetValue(const Point3d& P);

  const Point3d& Value() const;

  void SetValue2d(const gp_Pnt2d& P);

  const gp_Pnt2d& Value2d() const;

  void SetKeep(const Standard_Boolean B);

  Standard_Boolean Keep() const;

  void SetEdgesConfig(const TopOpeBRepDS_Config C);

  TopOpeBRepDS_Config EdgesConfig() const;

  void SetTolerance(const Standard_Real T);

  Standard_Real Tolerance() const;

  void SetHctxff2d(const Handle(TopOpeBRep_Hctxff2d)& ff2d);

  Handle(TopOpeBRep_Hctxff2d) Hctxff2d() const;

  void SetHctxee2d(const Handle(TopOpeBRep_Hctxee2d)& ee2d);

  Handle(TopOpeBRep_Hctxee2d) Hctxee2d() const;

  friend class TopOpeBRep_EdgesIntersector;

protected:
private:
  IntersectionPoint3  mypint;
  Standard_Boolean            myhaspint;
  Standard_Boolean            myisvertex1;
  TopoVertex               myvertex1;
  StateTransition     mytransition1;
  Standard_Real               myparameter1;
  Standard_Boolean            myisvertex2;
  TopoVertex               myvertex2;
  StateTransition     mytransition2;
  Standard_Real               myparameter2;
  Standard_Boolean            myispointofsegment;
  Standard_Integer            myips1;
  Standard_Integer            myips2;
  Standard_Boolean            myhasancestors;
  TopOpeBRep_P2Dstatus        mystatus;
  Standard_Integer            myindex;
  Point3d                      mypnt;
  gp_Pnt2d                    mypnt2d;
  Standard_Boolean            mykeep;
  TopOpeBRepDS_Config         myedgesconfig;
  Standard_Real               mytolerance;
  Handle(TopOpeBRep_Hctxff2d) myctxff2d;
  Handle(TopOpeBRep_Hctxee2d) myctxee2d;
};

#include <TopOpeBRep_Point2d.lxx>

#endif // _TopOpeBRep_Point2d_HeaderFile
