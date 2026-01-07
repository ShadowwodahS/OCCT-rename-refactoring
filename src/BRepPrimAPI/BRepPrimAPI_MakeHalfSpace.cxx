// Created on: 1995-03-08
// Created by: Bruno DUMORTIER
// Copyright (c) 1995-1999 Matra Datavision
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

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepLProp_SLProps.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <gp.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <StdFail_NotDone.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>

//=================================================================================================

static Dir3d getNormalOnFace(const TopoFace&  theFace,
                              const Standard_Real theU,
                              const Standard_Real theV)
{
  Standard_Real     aPrec = gp::Resolution();
  BRepLProp_SLProps aProps(BRepAdaptor_Surface(theFace), theU, theV, 2, aPrec);
  Dir3d            aNormal = aProps.Normal();
  if (theFace.Orientation() == TopAbs_REVERSED)
    aNormal.Reverse();
  return aNormal;
}

//=======================================================================
// function : getNormalFromEdge
// purpose  : Get average normal at the point with the given parameter on the edge
//=======================================================================

static Standard_Boolean getNormalFromEdge(const TopoShape& theShape,
                                          const TopoEdge&  theEdge,
                                          const Standard_Real thePar,
                                          Dir3d&             theNormal)
{
  gp_XYZ          aSum;
  ShapeExplorer ex(theShape, TopAbs_FACE);
  for (; ex.More(); ex.Next())
  {
    const TopoFace& aF = TopoDS::Face(ex.Current());
    ShapeExplorer    ex1(aF, TopAbs_EDGE);
    for (; ex1.More(); ex1.Next())
    {
      if (ex1.Current().IsSame(theEdge))
      {
        Standard_Real        f, l;
        Handle(GeomCurve2d) aC2d  = BRepInspector::CurveOnSurface(theEdge, aF, f, l);
        gp_Pnt2d             aP2d  = aC2d->Value(thePar);
        Dir3d               aNorm = getNormalOnFace(aF, aP2d.X(), aP2d.Y());
        aSum += aNorm.XYZ();
      }
    }
  }
  if (aSum.SquareModulus() > gp::Resolution())
  {
    theNormal = aSum;
    return Standard_True;
  }
  return Standard_False;
}

//=======================================================================
// function : getNormalFromVertex
// purpose  : Get average normal at the point of the vertex
//=======================================================================

static Standard_Boolean getNormalFromVertex(const TopoShape&  theShape,
                                            const TopoVertex& theVer,
                                            Dir3d&              theNormal)
{
  gp_XYZ          aSum;
  ShapeExplorer ex(theShape, TopAbs_FACE);
  for (; ex.More(); ex.Next())
  {
    const TopoFace& aF = TopoDS::Face(ex.Current());
    ShapeExplorer    ex1(aF, TopAbs_VERTEX);
    for (; ex1.More(); ex1.Next())
    {
      if (ex1.Current().IsSame(theVer))
      {
        gp_Pnt2d aP2d  = BRepInspector::Parameters(theVer, aF);
        Dir3d   aNorm = getNormalOnFace(aF, aP2d.X(), aP2d.Y());
        aSum += aNorm.XYZ();
      }
    }
  }
  if (aSum.SquareModulus() > gp::Resolution())
  {
    theNormal = aSum;
    return Standard_True;
  }
  return Standard_False;
}

//=======================================================================
// function : FindExtrema
// purpose  : This function is called to find the nearest normal projection
//           of a point <aPnt> on a shape <aShape>.
//           1) return true if extrema is found.
//           2) Set in:
//             - theMinPnt : The solution point
//             - theNormal : The normal direction to the shape at projection point
//=======================================================================
static Standard_Boolean FindExtrema(const Point3d&       thePnt,
                                    const TopoShape& theShape,
                                    Point3d&             theMinPnt,
                                    Dir3d&             theNormal)
{
  TopoVertex aRefVertex = BRepBuilderAPI_MakeVertex(thePnt);

  BRepExtrema_DistShapeShape ext(aRefVertex, theShape);

  if (!ext.IsDone() || ext.NbSolution() == 0)
    return Standard_False;

  // the point projection exist
  Standard_Integer nbext = ext.NbSolution();
  // try to find a projection on face
  for (Standard_Integer iext = 1; iext <= nbext; iext++)
  {
    if (ext.SupportTypeShape2(iext) == BRepExtrema_IsInFace)
    {
      TopoFace aF = TopoDS::Face(ext.SupportOnShape2(iext));
      theMinPnt      = ext.PointOnShape2(iext);
      Standard_Real aU, aV;
      ext.ParOnFaceS2(iext, aU, aV);
      theNormal = getNormalOnFace(aF, aU, aV);
      return Standard_True;
    }
  }

  // if not found then take any edge or vertex solution
  for (Standard_Integer iext = 1; iext <= nbext; iext++)
  {
    if (ext.SupportTypeShape2(iext) == BRepExtrema_IsOnEdge)
    {
      theMinPnt = ext.PointOnShape2(iext);
      Standard_Real aPar;
      ext.ParOnEdgeS2(iext, aPar);
      TopoEdge aE = TopoDS::Edge(ext.SupportOnShape2(iext));
      if (getNormalFromEdge(theShape, aE, aPar, theNormal))
        return Standard_True;
    }
    else if (ext.SupportTypeShape2(iext) == BRepExtrema_IsVertex)
    {
      theMinPnt        = ext.PointOnShape2(iext);
      TopoVertex aV = TopoDS::Vertex(ext.SupportOnShape2(iext));
      if (getNormalFromVertex(theShape, aV, theNormal))
        return Standard_True;
    }
  }
  return Standard_False;
}

//=================================================================================================

static Standard_Boolean isOutside(const Point3d& thePnt,
                                  const Point3d& thePonF,
                                  const Dir3d& theNormal)
{
  Dir3d        anOppRef(thePnt.XYZ() - thePonF.XYZ());
  Standard_Real aSca = theNormal * anOppRef;
  // outside if same directions
  return aSca > 0.;
}

//=================================================================================================

BRepPrimAPI_MakeHalfSpace::BRepPrimAPI_MakeHalfSpace(const TopoFace& theFace,
                                                     const Point3d&      theRefPnt)
{
  // Set the flag is <IsDone> to False.
  NotDone();

  TopoShell aShell;

  Point3d aMinPnt;
  Dir3d aNormal;
  if (FindExtrema(theRefPnt, theFace, aMinPnt, aNormal))
  {
    Standard_Boolean toReverse = isOutside(theRefPnt, aMinPnt, aNormal);

    // Construction of the open solid.
    ShapeBuilder().MakeShell(aShell);
    ShapeBuilder().Add(aShell, theFace);
    ShapeBuilder().MakeSolid(mySolid);
    if (toReverse)
    {
      aShell.Reverse();
    }
    ShapeBuilder().Add(mySolid, aShell);
    myShape = mySolid;
    Done();
  }
}

//=================================================================================================

BRepPrimAPI_MakeHalfSpace::BRepPrimAPI_MakeHalfSpace(const TopoShell& theShell,
                                                     const Point3d&       theRefPnt)
{
  // Set the flag is <IsDone> to False.
  NotDone();

  // Find the point of the skin closest to the reference point.
  Point3d aMinPnt;
  Dir3d aNormal;
  if (FindExtrema(theRefPnt, theShell, aMinPnt, aNormal))
  {
    Standard_Boolean toReverse = isOutside(theRefPnt, aMinPnt, aNormal);

    // Construction of the open solid.
    TopoShell aShell = theShell;
    ShapeBuilder().MakeSolid(mySolid);
    if (toReverse)
    {
      aShell.Reverse();
    }
    ShapeBuilder().Add(mySolid, aShell);
    myShape = mySolid;
    Done();
  }
}

//=================================================================================================

const TopoSolid& BRepPrimAPI_MakeHalfSpace::Solid() const
{
  StdFail_NotDone_Raise_if(!IsDone(), "BRepPrimAPI_MakeHalfSpace::Solid");
  return mySolid;
}

//=================================================================================================

BRepPrimAPI_MakeHalfSpace::operator TopoSolid() const
{
  return Solid();
}
