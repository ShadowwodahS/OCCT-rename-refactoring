// Created on: 2000-10-20
// Created by: Julia DOROVSKIKH
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#include <PrsDim_MidPointRelation.hxx>

#include <PrsDim.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <DsgPrs_MidPointPresentation.hxx>
#include <ElCLib.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Elips.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <Precision.hxx>
#include <Prs3d_Presentation.hxx>
#include <Select3D_SensitiveCurve.hxx>
#include <Select3D_SensitiveSegment.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <SelectMgr_Selection.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(PrsDim_MidPointRelation, PrsDim_Relation)

//=================================================================================================

PrsDim_MidPointRelation::PrsDim_MidPointRelation(const TopoShape&       aMidPointTool,
                                                 const TopoShape&       FirstShape,
                                                 const TopoShape&       SecondShape,
                                                 const Handle(GeomPlane)& aPlane)
    : PrsDim_Relation(),
      myTool(aMidPointTool)
{
  SetFirstShape(FirstShape);
  SetSecondShape(SecondShape);
  SetPlane(aPlane);
  myPosition = aPlane->Pln().Location();
}

//=================================================================================================

void PrsDim_MidPointRelation::Compute(const Handle(PrsMgr_PresentationManager)&,
                                      const Handle(Prs3d_Presentation)& aprs,
                                      const Standard_Integer)
{
  if (myTool.ShapeType() == TopAbs_VERTEX)
  {
    Point3d           pp;
    Standard_Boolean isonplane;
    if (PrsDim::ComputeGeometry(TopoDS::Vertex(myTool), pp, myPlane, isonplane))
    {
      if (!isonplane)
        ComputeProjVertexPresentation(aprs, TopoDS::Vertex(myTool), pp);
    }
    myMidPoint = pp;
  }
  else
    return;

  if (myAutomaticPosition)
    myPosition = myMidPoint;

  switch (myFShape.ShapeType())
  {
    case TopAbs_FACE: {
      ComputeFaceFromPnt(aprs, Standard_True);
    }
    break;
    case TopAbs_EDGE: {
      ComputeEdgeFromPnt(aprs, Standard_True);
    }
    break;
    case TopAbs_VERTEX: {
      ComputeVertexFromPnt(aprs, Standard_True);
    }
    break;
    default:
      break;
  }

  switch (mySShape.ShapeType())
  {
    case TopAbs_FACE: {
      ComputeFaceFromPnt(aprs, Standard_False);
    }
    break;
    case TopAbs_EDGE: {
      ComputeEdgeFromPnt(aprs, Standard_False);
    }
    break;
    case TopAbs_VERTEX: {
      ComputeVertexFromPnt(aprs, Standard_False);
    }
    break;
    default:
      break;
  }
}

//=================================================================================================

void PrsDim_MidPointRelation::ComputeSelection(const Handle(SelectionContainer)& aSel,
                                               const Standard_Integer)
{
  Handle(Select3D_SensitiveSegment) seg;
  Handle(SelectMgr_EntityOwner)     own = new SelectMgr_EntityOwner(this, 7);

  if (!myMidPoint.IsEqual(myFAttach, Precision::Confusion()))
  {
    // segment from mid point to the first geometry
    seg = new Select3D_SensitiveSegment(own, myFAttach, myMidPoint);
    aSel->Add(seg);
    // segment from mid point to the second geometry
    seg = new Select3D_SensitiveSegment(own, mySAttach, myMidPoint);
    aSel->Add(seg);
  }
  if (!myMidPoint.IsEqual(myPosition, Precision::Confusion()))
  {
    // segment from mid point to the text position
    seg = new Select3D_SensitiveSegment(own, myMidPoint, myPosition);
    aSel->Add(seg);
  }

  // center of the symmetry - circle around the MidPoint
  Frame3d ax = myPlane->Pln().Position().Ax2();
  ax.SetLocation(myMidPoint);
  Standard_Real                   rad = myFAttach.Distance(myMidPoint) / 20.0;
  gp_Circ                         aCircleM(ax, rad);
  Handle(GeomCurve3d)              thecir = new GeomCircle(aCircleM);
  Handle(Select3D_SensitiveCurve) scurv  = new Select3D_SensitiveCurve(own, thecir);
  aSel->Add(scurv);

  Handle(GeomCurve3d) curv;
  Point3d             firstp, lastp;
  Standard_Boolean   isInfinite, isOnPlane;
  Handle(GeomCurve3d) extCurv;

  // segment on first curve
  if (myFShape.ShapeType() == TopAbs_EDGE)
  {
    TopoEdge E = TopoDS::Edge(myFShape);
    if (!PrsDim::ComputeGeometry(E, curv, firstp, lastp, extCurv, isInfinite, isOnPlane, myPlane))
      return;
    if (curv->IsInstance(STANDARD_TYPE(GeomLine))) // case of line
    {
      // segment on line
      seg = new Select3D_SensitiveSegment(own, myFirstPnt1, myFirstPnt2);
      aSel->Add(seg);
    }
    else if (curv->IsInstance(STANDARD_TYPE(GeomCircle))) // case of circle
    {
      // segment on circle
      Handle(GeomCircle) thecirc = Handle(GeomCircle)::DownCast(curv);
      Standard_Real       udeb    = ElCLib1::Parameter(thecirc->Circ(), myFirstPnt1);
      Standard_Real       ufin    = ElCLib1::Parameter(thecirc->Circ(), myFirstPnt2);
      Handle(GeomCurve3d)  thecu   = new Geom_TrimmedCurve(thecirc, udeb, ufin);

      scurv = new Select3D_SensitiveCurve(own, thecu);
      aSel->Add(scurv);
    }
    else if (curv->IsInstance(STANDARD_TYPE(Geom_Ellipse))) // case of ellipse
    {
      // segment on ellipse
      Handle(Geom_Ellipse) theEll = Handle(Geom_Ellipse)::DownCast(curv);
      Standard_Real        udeb   = ElCLib1::Parameter(theEll->Elips(), myFirstPnt1);
      Standard_Real        ufin   = ElCLib1::Parameter(theEll->Elips(), myFirstPnt2);
      Handle(GeomCurve3d)   thecu  = new Geom_TrimmedCurve(theEll, udeb, ufin);

      scurv = new Select3D_SensitiveCurve(own, thecu);
      aSel->Add(scurv);
    }
  }

  // segment on second curve
  if (mySShape.ShapeType() == TopAbs_EDGE)
  {
    TopoEdge E = TopoDS::Edge(mySShape);
    if (!PrsDim::ComputeGeometry(E, curv, firstp, lastp, extCurv, isInfinite, isOnPlane, myPlane))
      return;
    if (curv->IsInstance(STANDARD_TYPE(GeomLine))) // case of line
    {
      // segment on line
      seg = new Select3D_SensitiveSegment(own, mySecondPnt1, mySecondPnt2);
      aSel->Add(seg);
    }
    else if (curv->IsInstance(STANDARD_TYPE(GeomCircle))) // case of circle
    {
      // segment on circle
      Handle(GeomCircle) thecirc = Handle(GeomCircle)::DownCast(curv);
      Standard_Real       udeb    = ElCLib1::Parameter(thecirc->Circ(), mySecondPnt1);
      Standard_Real       ufin    = ElCLib1::Parameter(thecirc->Circ(), mySecondPnt2);
      Handle(GeomCurve3d)  thecu   = new Geom_TrimmedCurve(thecirc, udeb, ufin);

      scurv = new Select3D_SensitiveCurve(own, thecu);
      aSel->Add(scurv);
    }
    else if (curv->IsInstance(STANDARD_TYPE(Geom_Ellipse))) // case of ellipse
    {
      // segment on ellipse
      Handle(Geom_Ellipse) theEll = Handle(Geom_Ellipse)::DownCast(curv);
      Standard_Real        udeb   = ElCLib1::Parameter(theEll->Elips(), mySecondPnt1);
      Standard_Real        ufin   = ElCLib1::Parameter(theEll->Elips(), mySecondPnt2);
      Handle(GeomCurve3d)   thecu  = new Geom_TrimmedCurve(theEll, udeb, ufin);

      scurv = new Select3D_SensitiveCurve(own, thecu);
      aSel->Add(scurv);
    }
  }
}

//=================================================================================================

void PrsDim_MidPointRelation::ComputeFaceFromPnt(const Handle(Prs3d_Presentation)&,
                                                 const Standard_Boolean /*first*/)
{
}

//=================================================================================================

void PrsDim_MidPointRelation::ComputeEdgeFromPnt(const Handle(Prs3d_Presentation)& aprs,
                                                 const Standard_Boolean            first)
{
  TopoEdge E;
  if (first)
    E = TopoDS::Edge(myFShape);
  else
    E = TopoDS::Edge(mySShape);

  Handle(GeomCurve3d) geom;
  Point3d             ptat1, ptat2;
  Handle(GeomCurve3d) extCurv;
  Standard_Boolean   isInfinite, isOnPlane;
  if (!PrsDim::ComputeGeometry(E, geom, ptat1, ptat2, extCurv, isInfinite, isOnPlane, myPlane))
    return;

  Frame3d ax = myPlane->Pln().Position().Ax2();

  if (geom->IsInstance(STANDARD_TYPE(GeomLine)))
  {
    if (!isInfinite)
      ComputePointsOnLine(ptat1, ptat2, first);
    else
    {
      const gp_Lin& line = Handle(GeomLine)::DownCast(geom)->Lin();
      ComputePointsOnLine(line, first);
    }
    if (first)
      DsgPrs_MidPointPresentation::Add(aprs,
                                       myDrawer,
                                       ax,
                                       myMidPoint,
                                       myPosition,
                                       myFAttach,
                                       myFirstPnt1,
                                       myFirstPnt2,
                                       first);
    else
      DsgPrs_MidPointPresentation::Add(aprs,
                                       myDrawer,
                                       ax,
                                       myMidPoint,
                                       myPosition,
                                       mySAttach,
                                       mySecondPnt1,
                                       mySecondPnt2,
                                       first);
  }
  else if (geom->IsInstance(STANDARD_TYPE(GeomCircle)))
  {
    Handle(GeomCircle) geom_cir(Handle(GeomCircle)::DownCast(geom));
    gp_Circ             circ(geom_cir->Circ());
    ComputePointsOnCirc(circ, ptat1, ptat2, first);
    if (first)
      DsgPrs_MidPointPresentation::Add(aprs,
                                       myDrawer,
                                       circ,
                                       myMidPoint,
                                       myPosition,
                                       myFAttach,
                                       myFirstPnt1,
                                       myFirstPnt2,
                                       first);
    else
      DsgPrs_MidPointPresentation::Add(aprs,
                                       myDrawer,
                                       circ,
                                       myMidPoint,
                                       myPosition,
                                       mySAttach,
                                       mySecondPnt1,
                                       mySecondPnt2,
                                       first);
  }
  else if (geom->IsInstance(STANDARD_TYPE(Geom_Ellipse)))
  {
    Handle(Geom_Ellipse) geom_ell(Handle(Geom_Ellipse)::DownCast(geom));
    gp_Elips             elips(geom_ell->Elips());
    ComputePointsOnElips(elips, ptat1, ptat2, first);
    if (first)
      DsgPrs_MidPointPresentation::Add(aprs,
                                       myDrawer,
                                       elips,
                                       myMidPoint,
                                       myPosition,
                                       myFAttach,
                                       myFirstPnt1,
                                       myFirstPnt2,
                                       first);
    else
      DsgPrs_MidPointPresentation::Add(aprs,
                                       myDrawer,
                                       elips,
                                       myMidPoint,
                                       myPosition,
                                       mySAttach,
                                       mySecondPnt1,
                                       mySecondPnt2,
                                       first);
  }
  else
    return;

  // projection on myPlane
  if (!isOnPlane)
    ComputeProjEdgePresentation(aprs, E, geom, ptat1, ptat2);
}

//=================================================================================================

void PrsDim_MidPointRelation::ComputeVertexFromPnt(const Handle(Prs3d_Presentation)& aprs,
                                                   const Standard_Boolean            first)
{
  Frame3d ax = myPlane->Pln().Position().Ax2();
  if (first)
  {
    Standard_Boolean isOnPlane;
    TopoVertex    V = TopoDS::Vertex(myFShape);
    PrsDim::ComputeGeometry(V, myFAttach, myPlane, isOnPlane);
    DsgPrs_MidPointPresentation::Add(aprs, myDrawer, ax, myMidPoint, myPosition, myFAttach, first);
    if (!isOnPlane)
      ComputeProjVertexPresentation(aprs, V, myFAttach);
  }
  else
  {
    Standard_Boolean isOnPlane;
    TopoVertex    V = TopoDS::Vertex(mySShape);
    PrsDim::ComputeGeometry(V, mySAttach, myPlane, isOnPlane);
    DsgPrs_MidPointPresentation::Add(aprs, myDrawer, ax, myMidPoint, myPosition, mySAttach, first);
    if (!isOnPlane)
      ComputeProjVertexPresentation(aprs, V, mySAttach);
  }
}

//=================================================================================================

void PrsDim_MidPointRelation::ComputePointsOnLine(const gp_Lin& aLin, const Standard_Boolean first)
{
  Standard_Real ppar     = ElCLib1::Parameter(aLin, myMidPoint);
  Point3d        anAttach = ElCLib1::Value(ppar, aLin);

  Standard_Real dist = anAttach.Distance(myMidPoint) / 10.0;
  if (dist < Precision::Confusion())
    dist = 10.0;

  Standard_Real fpar = ppar + dist;
  Standard_Real spar = ppar - dist;

  Point3d aPnt1 = ElCLib1::Value(fpar, aLin);
  Point3d aPnt2 = ElCLib1::Value(spar, aLin);

  if (first)
  {
    myFAttach   = anAttach;
    myFirstPnt1 = aPnt1;
    myFirstPnt2 = aPnt2;
  }
  else
  {
    mySAttach    = anAttach;
    mySecondPnt1 = aPnt1;
    mySecondPnt2 = aPnt2;
  }
}

//=================================================================================================

void PrsDim_MidPointRelation::ComputePointsOnLine(const Point3d&          pnt1,
                                                  const Point3d&          pnt2,
                                                  const Standard_Boolean first)
{
  Vector3d aVec(pnt1, pnt2);
  gp_Lin aLin(pnt1, Dir3d(aVec));

  Standard_Real fpar = ElCLib1::Parameter(aLin, pnt1);
  Standard_Real spar = ElCLib1::Parameter(aLin, pnt2);
  Standard_Real ppar = ElCLib1::Parameter(aLin, myMidPoint);

  Point3d        aProjPnt = ElCLib1::Value(ppar, aLin);
  Standard_Real dist     = myMidPoint.Distance(aProjPnt);
  Standard_Real ll       = pnt1.Distance(pnt2);
  Standard_Real segm     = Min(dist, ll) * 0.75;
  if (dist < Precision::Confusion())
    segm = ll * 0.75;

  Point3d anAttach, aPnt1, aPnt2;
  anAttach = aProjPnt;
  Vector3d aVecTr;
  if (ppar <= fpar)
  {
    aPnt2  = pnt1;
    aVecTr = Vector3d(pnt2, pnt1);
    aVecTr.Normalize();
    aPnt1 = aProjPnt.Translated(aVecTr * segm);
  }
  else if (ppar >= spar)
  {
    aPnt1  = pnt2;
    aVecTr = Vector3d(pnt1, pnt2);
    aVecTr.Normalize();
    aPnt2 = aProjPnt.Translated(aVecTr * segm);
  }
  else
  {
    Standard_Real dp1 = aProjPnt.Distance(pnt1);
    Standard_Real dp2 = aProjPnt.Distance(pnt2);

    segm   = Min(dist, dp1) * 0.75;
    aVecTr = Vector3d(aProjPnt, pnt1);
    aVecTr.Normalize();
    aPnt1 = aProjPnt.Translated(aVecTr * segm);

    segm   = Min(dist, dp2) * 0.75;
    aVecTr = Vector3d(aProjPnt, pnt2);
    aVecTr.Normalize();
    aPnt2 = aProjPnt.Translated(aVecTr * segm);
  }

  if (first)
  {
    myFAttach   = anAttach;
    myFirstPnt1 = aPnt1;
    myFirstPnt2 = aPnt2;
  }
  else
  {
    mySAttach    = anAttach;
    mySecondPnt1 = aPnt1;
    mySecondPnt2 = aPnt2;
  }
}

//=================================================================================================

void PrsDim_MidPointRelation::ComputePointsOnCirc(const gp_Circ&         aCirc,
                                                  const Point3d&          pnt1,
                                                  const Point3d&          pnt2,
                                                  const Standard_Boolean first)
{
  Point3d curpos = myMidPoint;

  // Case of confusion between the current position and the center
  // of the circle -> we move the current position
  constexpr Standard_Real confusion(Precision::Confusion());
  Point3d                  aCenter = aCirc.Location();
  if (aCenter.Distance(curpos) <= confusion)
  {
    Vector3d vprec(aCenter, pnt1);
    vprec.Normalize();
    curpos.Translate(vprec * 1e-5);
  }

  Standard_Real pcurpos = ElCLib1::Parameter(aCirc, curpos);

  Standard_Real rad = M_PI / 5.0;
  Standard_Real segm;

  Standard_Real pFPnt;
  Standard_Real pSPnt;

  if (pnt1.IsEqual(pnt2, confusion)) // full circle
  {
    pFPnt = pcurpos - rad;
    pSPnt = pcurpos + rad;
  }
  else
  {
    Standard_Real pFAttach = ElCLib1::Parameter(aCirc, pnt1);
    Standard_Real pSAttach = ElCLib1::Parameter(aCirc, pnt2);

    Standard_Real pSAttachM = pSAttach;
    Standard_Real deltap    = pSAttachM - pFAttach;
    if (deltap < 0)
    {
      deltap += 2 * M_PI;
      pSAttachM += 2 * M_PI;
    }
    pSAttachM -= pFAttach;

    Standard_Real pmiddleout = pSAttachM / 2.0 + M_PI;

    Standard_Real pcurpos1 = pcurpos;
    // define where curpos lays
    if (pcurpos1 < pFAttach)
    {
      pcurpos1 = pcurpos1 + 2 * M_PI - pFAttach;
      if (pcurpos1 > pSAttachM) // out
      {
        segm = Min(rad, deltap * 0.75);
        if (pcurpos1 > pmiddleout)
        {
          pcurpos = pFAttach;
          pFPnt   = pFAttach;
          pSPnt   = pFAttach + segm;
        }
        else
        {
          pcurpos = pSAttach;
          pFPnt   = pSAttach - segm;
          pSPnt   = pSAttach;
        }
      }
      else // on arc
      {
        Standard_Real dp1 = pcurpos1 - pFAttach;
        Standard_Real dp2 = pSAttachM - pcurpos1;

        segm  = Min(rad, dp1 * 0.75);
        pFPnt = pcurpos - segm;

        segm  = Min(rad, dp2 * 0.75);
        pSPnt = pcurpos + segm;
      }
    }
    else if (pcurpos1 > (pFAttach + deltap)) // out
    {
      pcurpos1 -= pFAttach;
      segm = Min(rad, deltap * 0.75);
      if (pcurpos1 > pmiddleout)
      {
        pcurpos = pFAttach;
        pFPnt   = pFAttach;
        pSPnt   = pFAttach + segm;
      }
      else
      {
        pcurpos = pSAttach;
        pFPnt   = pSAttach - segm;
        pSPnt   = pSAttach;
      }
    }
    else // on arc
    {
      Standard_Real dp1 = pcurpos1 - pFAttach;
      Standard_Real dp2 = pSAttach - pcurpos1;

      segm  = Min(rad, dp1 * 0.75);
      pFPnt = pcurpos - segm;

      segm  = Min(rad, dp2 * 0.75);
      pSPnt = pcurpos + segm;
    }
  }

  if (first)
  {
    myFAttach   = ElCLib1::Value(pcurpos, aCirc);
    myFirstPnt1 = ElCLib1::Value(pFPnt, aCirc);
    myFirstPnt2 = ElCLib1::Value(pSPnt, aCirc);
  }
  else
  {
    mySAttach    = ElCLib1::Value(pcurpos, aCirc);
    mySecondPnt1 = ElCLib1::Value(pFPnt, aCirc);
    mySecondPnt2 = ElCLib1::Value(pSPnt, aCirc);
  }
}

//=================================================================================================

void PrsDim_MidPointRelation::ComputePointsOnElips(const gp_Elips&        anEll,
                                                   const Point3d&          pnt1,
                                                   const Point3d&          pnt2,
                                                   const Standard_Boolean first)
{
  Point3d curpos = myMidPoint;

  // Case of confusion between the current position and the center
  // of the circle -> we move the current position
  constexpr Standard_Real confusion(Precision::Confusion());
  Point3d                  aCenter = anEll.Location();
  if (aCenter.Distance(curpos) <= confusion)
  {
    Vector3d vprec(aCenter, pnt1);
    vprec.Normalize();
    curpos.Translate(vprec * 1e-5);
  }

  Standard_Real pcurpos = ElCLib1::Parameter(anEll, curpos);

  Standard_Real rad = M_PI / 5.0;
  Standard_Real segm;

  Standard_Real pFPnt;
  Standard_Real pSPnt;

  if (pnt1.IsEqual(pnt2, confusion)) // full circle
  {
    pFPnt = pcurpos - rad;
    pSPnt = pcurpos + rad;
  }
  else
  {
    Standard_Real pFAttach = ElCLib1::Parameter(anEll, pnt1);
    Standard_Real pSAttach = ElCLib1::Parameter(anEll, pnt2);

    Standard_Real pSAttachM = pSAttach;
    Standard_Real deltap    = pSAttachM - pFAttach;
    if (deltap < 0)
    {
      deltap += 2 * M_PI;
      pSAttachM += 2 * M_PI;
    }
    pSAttachM -= pFAttach;

    Standard_Real pmiddleout = pSAttachM / 2.0 + M_PI;

    Standard_Real pcurpos1 = pcurpos;
    // define where curpos lays
    if (pcurpos1 < pFAttach)
    {
      pcurpos1 = pcurpos1 + 2 * M_PI - pFAttach;
      if (pcurpos1 > pSAttachM) // out
      {
        segm = Min(rad, deltap * 0.75);
        if (pcurpos1 > pmiddleout)
        {
          pcurpos = pFAttach;
          pFPnt   = pFAttach;
          pSPnt   = pFAttach + segm;
        }
        else
        {
          pcurpos = pSAttach;
          pFPnt   = pSAttach - segm;
          pSPnt   = pSAttach;
        }
      }
      else // on arc
      {
        Standard_Real dp1 = pcurpos1 - pFAttach;
        Standard_Real dp2 = pSAttachM - pcurpos1;

        segm  = Min(rad, dp1 * 0.75);
        pFPnt = pcurpos - segm;

        segm  = Min(rad, dp2 * 0.75);
        pSPnt = pcurpos + segm;
      }
    }
    else if (pcurpos1 > (pFAttach + deltap)) // out
    {
      pcurpos1 -= pFAttach;
      segm = Min(rad, deltap * 0.75);
      if (pcurpos1 > pmiddleout)
      {
        pcurpos = pFAttach;
        pFPnt   = pFAttach;
        pSPnt   = pFAttach + segm;
      }
      else
      {
        pcurpos = pSAttach;
        pFPnt   = pSAttach - segm;
        pSPnt   = pSAttach;
      }
    }
    else // on arc
    {
      Standard_Real dp1 = pcurpos1 - pFAttach;
      Standard_Real dp2 = pSAttach - pcurpos1;

      segm  = Min(rad, dp1 * 0.75);
      pFPnt = pcurpos - segm;

      segm  = Min(rad, dp2 * 0.75);
      pSPnt = pcurpos + segm;
    }
  }

  if (first)
  {
    myFAttach   = ElCLib1::Value(pcurpos, anEll);
    myFirstPnt1 = ElCLib1::Value(pFPnt, anEll);
    myFirstPnt2 = ElCLib1::Value(pSPnt, anEll);
  }
  else
  {
    mySAttach    = ElCLib1::Value(pcurpos, anEll);
    mySecondPnt1 = ElCLib1::Value(pFPnt, anEll);
    mySecondPnt2 = ElCLib1::Value(pSPnt, anEll);
  }
}
