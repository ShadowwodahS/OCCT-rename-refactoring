// Created on: 1996-09-04
// Created by: Olga PILLOT
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

#include <BRepLib_MakeVertex.hxx>
#include <BRepSweep_Revol.hxx>
#include <BRepTools_Modifier.hxx>
#include <BRepTools_TrsfModification.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <gp_Ax1.hxx>
#include <gp_Circ.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <LocOpe.hxx>
#include <LocOpe_BuildShape.hxx>
#include <LocOpe_Revol.hxx>
#include <Precision.hxx>
#include <StdFail_NotDone.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

static Standard_Boolean FindCircle(const Axis3d&, const Point3d&, gp_Circ&);

//=================================================================================================

LocOpe_Revol::LocOpe_Revol()
    : myAngle(0.0),
      myAngTra(0.0),
      myIsTrans(Standard_False),
      myDone(Standard_False)
{
}

//=================================================================================================

void LocOpe_Revol::Perform(const TopoShape& Base, const Axis3d& Axis, const Standard_Real Angle)
{
  myMap.Clear();
  myFirstShape.Nullify();
  myLastShape.Nullify();
  myBase.Nullify();
  myRes.Nullify();
  myBase    = Base;
  myAngle   = Angle;
  myAxis    = Axis;
  myAngTra  = 0.;
  myIsTrans = Standard_False;
  IntPerf();
}

//=================================================================================================

void LocOpe_Revol::Perform(const TopoShape& Base,
                           const Axis3d&       Axis,
                           const Standard_Real Angle,
                           const Standard_Real angledec)
{
  myMap.Clear();
  myFirstShape.Nullify();
  myLastShape.Nullify();
  myBase.Nullify();
  myRes.Nullify();
  myBase    = Base;
  myAngle   = Angle;
  myAxis    = Axis;
  myAngTra  = angledec;
  myIsTrans = Standard_True;
  IntPerf();
}

//=================================================================================================

void LocOpe_Revol::IntPerf()
{
  TopoShape       theBase = myBase;
  ShapeModifier Modif;
  if (myIsTrans)
  {
    Transform3d T;
    T.SetRotation(myAxis, myAngTra);
    Handle(BRepTools_TrsfModification) modbase = new BRepTools_TrsfModification(T);
    Modif.Init(theBase);
    Modif.Perform(modbase);
    theBase = Modif.ModifiedShape(theBase);
  }

  BRepSweep_Revol theRevol(theBase, myAxis, myAngle);

  myFirstShape = theRevol.FirstShape();
  myLastShape  = theRevol.LastShape();

  ShapeExplorer exp;
  if (theBase.ShapeType() == TopAbs_FACE)
  {
    for (exp.Init(theBase, TopAbs_EDGE); exp.More(); exp.Next())
    {
      const TopoEdge& edg = TopoDS::Edge(exp.Current());
      if (!myMap.IsBound(edg))
      {
        ShapeList thelist;
        myMap.Bind(edg, thelist);
        TopoShape desc = theRevol.Shape(edg);
        if (!desc.IsNull())
        {
          myMap(edg).Append(desc);
        }
      }
    }
    myRes = theRevol.Shape();
  }

  else
  {
    // Cas base != FACE
    TopTools_IndexedDataMapOfShapeListOfShape theEFMap;
    TopExp1::MapShapesAndAncestors(theBase, TopAbs_EDGE, TopAbs_FACE, theEFMap);
    ShapeList lfaces;
    Standard_Boolean     toremove = Standard_False;
    for (Standard_Integer i = 1; i <= theEFMap.Extent(); i++)
    {
      const TopoShape&  edg = theEFMap.FindKey(i);
      ShapeList thelist1;
      myMap.Bind(edg, thelist1);
      TopoShape desc = theRevol.Shape(edg);
      if (!desc.IsNull())
      {
        if (theEFMap(i).Extent() >= 2)
        {
          toremove = Standard_True;
        }
        else
        {
          myMap(edg).Append(desc);
          lfaces.Append(desc);
        }
      }
    }
    if (toremove)
    {
      // Rajouter les faces de FirstShape et LastShape
      for (exp.Init(myFirstShape, TopAbs_FACE); exp.More(); exp.Next())
      {
        lfaces.Append(exp.Current());
      }
      for (exp.Init(myLastShape, TopAbs_FACE); exp.More(); exp.Next())
      {
        lfaces.Append(exp.Current());
      }

      LocOpe_BuildShape BS(lfaces);
      myRes = BS.Shape();
    }
    else
    {
      for (exp.Init(theBase, TopAbs_EDGE); exp.More(); exp.Next())
      {
        const TopoEdge& edg = TopoDS::Edge(exp.Current());
        if (!myMap.IsBound(edg))
        {
          ShapeList thelist2;
          myMap.Bind(edg, thelist2);
          TopoShape desc = theRevol.Shape(edg);
          if (!desc.IsNull())
          {
            myMap(edg).Append(desc);
          }
        }
      }
      myRes = theRevol.Shape();
    }
  }

  if (myIsTrans)
  {
    // m-a-j des descendants
    ShapeExplorer anExp;
    for (anExp.Init(myBase, TopAbs_EDGE); anExp.More(); anExp.Next())
    {
      const TopoEdge& edg    = TopoDS::Edge(anExp.Current());
      const TopoEdge& edgbis = TopoDS::Edge(Modif.ModifiedShape(edg));
      if (!edgbis.IsSame(edg) && myMap.IsBound(edgbis))
      {
        myMap.Bind(edg, myMap(edgbis));
        myMap.UnBind(edgbis);
      }
    }
  }
  myDone = Standard_True;
}

//=================================================================================================

const TopoShape& LocOpe_Revol::Shape() const
{
  if (!myDone)
  {
    throw StdFail_NotDone();
  }
  return myRes;
}

//=================================================================================================

const TopoShape& LocOpe_Revol::FirstShape() const
{
  return myFirstShape;
}

//=================================================================================================

const TopoShape& LocOpe_Revol::LastShape() const
{
  return myLastShape;
}

//=================================================================================================

const ShapeList& LocOpe_Revol::Shapes(const TopoShape& S) const
{
  return myMap(S);
}

//=================================================================================================

void LocOpe_Revol::Curves(TColGeom_SequenceOfCurve& Scurves) const
{
  Scurves.Clear();
  TColgp_SequenceOfPnt spt;
  LocOpe1::SampleEdges(myFirstShape, spt);
  for (Standard_Integer jj = 1; jj <= spt.Length(); jj++)
  {
    const Point3d& pvt = spt(jj);
    gp_Circ       CAX;
    if (FindCircle(myAxis, pvt, CAX))
    {
      Frame3d              A2 = CAX.Position();
      Standard_Real       r  = CAX.Radius();
      Handle(GeomCircle) Ci = new GeomCircle(A2, r);
      Scurves.Append(Ci);
    }
  }
}

//=================================================================================================

Handle(GeomCurve3d) LocOpe_Revol::BarycCurve() const
{
  Point3d               bar(0., 0., 0.);
  TColgp_SequenceOfPnt spt;
  LocOpe1::SampleEdges(myFirstShape, spt);
  for (Standard_Integer jj = 1; jj <= spt.Length(); jj++)
  {
    const Point3d& pvt = spt(jj);
    bar.ChangeCoord() += pvt.XYZ();
  }
  bar.ChangeCoord().Divide(spt.Length());
  gp_Circ             CAX;
  Handle(GeomCircle) theCi;
  if (FindCircle(myAxis, bar, CAX))
  {
    Frame3d        A2 = CAX.Position();
    Standard_Real r  = CAX.Radius();
    theCi            = new GeomCircle(A2, r);
  }
  return theCi;
}

//=================================================================================================

static Standard_Boolean FindCircle(const Axis3d& Ax, const Point3d& Pt, gp_Circ& Ci)
{

  const Dir3d& Dax = Ax.Direction();
  Vector3d        OP(Ax.Location(), Pt);

  Standard_Real prm = OP.Dot(Dax);

  Point3d        prj(Ax.Location().XYZ().Added(prm * Dax.XYZ()));
  Vector3d        axx(prj, Pt);
  Standard_Real Radius = axx.Magnitude();
  if (Radius < Precision::Confusion())
  {
    return Standard_False;
  }
  Ci.SetRadius(Radius);
  Ci.SetPosition(Frame3d(prj, Dax, axx));
  return Standard_True;
}
