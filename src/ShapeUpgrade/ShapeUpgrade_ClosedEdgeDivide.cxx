// Created on: 2000-05-25
// Created by: data exchange team
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

#include <BRep_Tool.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeUpgrade_ClosedEdgeDivide.hxx>
#include <ShapeUpgrade_SplitCurve2d.hxx>
#include <ShapeUpgrade_SplitCurve3d.hxx>
#include <Standard_Type.hxx>
#include <TColStd_HSequenceOfReal.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ShapeUpgrade_ClosedEdgeDivide, ShapeUpgrade_EdgeDivide)

//=================================================================================================

ShapeUpgrade_ClosedEdgeDivide::ShapeUpgrade_ClosedEdgeDivide()
    : ShapeUpgrade_EdgeDivide()
{
}

//=================================================================================================

Standard_Boolean ShapeUpgrade_ClosedEdgeDivide::Compute(const TopoEdge& anEdge)
{
  Clear();
  Edge1 sae;
  TopoVertex      V1 = sae.FirstVertex(anEdge);
  TopoVertex      V2 = sae.LastVertex(anEdge);
  if (V1.IsSame(V2) && !BRepInspector::Degenerated(anEdge))
  {
    const Standard_Integer nbPoints = 23;
    Point3d                 pntV     = BRepInspector::Pnt(V1);
    Standard_Real          TolV1    = LimitTolerance(BRepInspector::Tolerance(V1));
    TolV1                           = TolV1 * TolV1;
    Standard_Real      f, l;
    Handle(GeomCurve3d) curve3d = BRepInspector::Curve(anEdge, f, l);
    myHasCurve3d               = !curve3d.IsNull();
    Standard_Real        f2d = 0., l2d = 0.;
    Handle(GeomCurve2d) pcurve1;
    if (!myFace.IsNull())
    { // process free edges
      sae.PCurve(anEdge, myFace, pcurve1, f2d, l2d, Standard_False);
    }
    myHasCurve2d = !pcurve1.IsNull();

    if (myHasCurve3d)
    {
      Standard_Real maxPar = f, dMax = 0;
      Standard_Real step  = (l - f) / (nbPoints - 1);
      Standard_Real param = f + step;
      for (Standard_Integer i = 1; i < 23; i++, param += step)
      {
        Point3d        curPnt = curve3d->Value(param);
        Standard_Real dist   = pntV.SquareDistance(curPnt);
        if (dist > dMax)
        {
          maxPar = param;
          dMax   = dist;
        }
      }
      if (dMax > TolV1)
      {
        Handle(ShapeUpgrade_SplitCurve3d) theSplit3dTool = GetSplitCurve3dTool();
        theSplit3dTool->Init(curve3d, f, l);

        Handle(TColStd_HSequenceOfReal) values = new TColStd_HSequenceOfReal;
        values->Append(maxPar);
        theSplit3dTool->SetSplitValues(values);
        myKnots3d = theSplit3dTool->SplitValues();

        if (myHasCurve2d)
        {
          Handle(ShapeUpgrade_SplitCurve2d) theSplit2dTool = GetSplitCurve2dTool();
          theSplit2dTool->Init(pcurve1, f2d, l2d);
          myKnots2d = theSplit2dTool->SplitValues();
        }
        return Standard_True;
      }
      else
        return Standard_False;
    }

    if (myHasCurve2d)
    {
      Handle(GeomSurface) surf   = BRepInspector::Surface(myFace);
      Standard_Real        maxPar = f2d, dMax = 0;
      Standard_Real        step  = (l2d - f2d) / (nbPoints - 1);
      Standard_Real        param = f2d + step;
      for (Standard_Integer i = 1; i < 23; i++, param += step)
      {
        gp_Pnt2d      p2d    = pcurve1->Value(param);
        Point3d        curPnt = surf->Value(p2d.X(), p2d.Y());
        Standard_Real dist   = pntV.SquareDistance(curPnt);
        if (dist > dMax)
        {
          maxPar = param;
          dMax   = dist;
        }
      }
      if (dMax > TolV1)
      {

        Handle(ShapeUpgrade_SplitCurve2d) theSplit2dTool = GetSplitCurve2dTool();
        theSplit2dTool->Init(pcurve1, f2d, l2d);

        Handle(TColStd_HSequenceOfReal) values = new TColStd_HSequenceOfReal;
        values->Append(maxPar);
        theSplit2dTool->SetSplitValues(values);
        myKnots2d = theSplit2dTool->SplitValues();
        return Standard_True;
      }
      else
        return Standard_False;
    }

    return Standard_False;
  }
  else
    return Standard_False;
}
