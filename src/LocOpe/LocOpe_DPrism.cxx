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

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepLib.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <BRepLib_MakeVertex.hxx>
#include <BRepLib_MakeWire.hxx>
#include <BRepTools.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <gp.hxx>
#include <gp_Ax3.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Vec.hxx>
#include <LocOpe.hxx>
#include <LocOpe_BuildShape.hxx>
#include <LocOpe_DPrism.hxx>
#include <Standard_ConstructionError.hxx>
#include <StdFail_NotDone.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

#ifdef OCCT_DEBUG
extern Standard_Boolean BRepFeat_GettraceFEAT();
#endif

#define NECHANT 7 // voir BRepFeat.cxx

//=================================================================================================

LocOpe_DPrism::LocOpe_DPrism(const TopoFace&  Spine,
                             const Standard_Real Height1,
                             const Standard_Real Height2,
                             const Standard_Real Angle)
    : mySpine(Spine)
{
  Standard_Integer i;

  myHeight        = Height1 + Height2;
  Standard_Real y = Height1 * sin(Angle);
  Standard_Real z = Height1 * cos(Angle);

  TopoVertex Vert2 = BRepLib_MakeVertex(Point3d(0, y, z));

  Standard_Real y1 = -Height2 * sin(Angle);
  Standard_Real z1 = -Height2 * cos(Angle);

  TopoVertex Vert1 = BRepLib_MakeVertex(Point3d(0, y1, z1));

  myProfile2 = BRepLib_MakeEdge(Vert1, Vert2);

  Standard_Real Umin, Umax, Vmin, Vmax;
  Umax = 0.;
  Umin = 0.;
  Vmin = 0.;
  Vmax = 0.;

  BRepTools1::UVBounds(Spine, Umin, Umax, Vmin, Vmax);
  Standard_Real Deltay = Max(Umax - Umin, Vmax - Vmin) + Abs(y);
  Deltay *= 2;

  TopoVertex Vert3 = BRepLib_MakeVertex(Point3d(0, y + Deltay, z));
  myProfile3          = BRepLib_MakeEdge(Vert2, Vert3);

  Umax = 0.;
  Umin = 0.;
  Vmin = 0.;
  Vmax = 0.;

  BRepTools1::UVBounds(Spine, Umin, Umax, Vmin, Vmax);
  Standard_Real Deltay1 = Max(Umax - Umin, Vmax - Vmin) + Abs(y1);
  Deltay1 *= 2;

  TopoVertex Vert4 = BRepLib_MakeVertex(Point3d(0, y1 + Deltay1, z1));
  myProfile1          = BRepLib_MakeEdge(Vert4, Vert1);

  myProfile = BRepLib_MakeWire(myProfile1, myProfile2, myProfile3);

  myDPrism.Perform(mySpine, myProfile, gp1::XOY());

  if (myDPrism.IsDone())
  {
    LocOpe_BuildShape    BS;
    ShapeBuilder         B;
    TopoCompound      C;
    TopoCompound      D;
    ShapeList lfaces, lcomplete;

    B.MakeCompound(C);
    TopTools_ListIteratorOfListOfShape it;
    ShapeExplorer                    ExpS(mySpine, TopAbs_EDGE);
    TopTools_MapOfShape                View;
    for (; ExpS.More(); ExpS.Next())
    {
      const TopoShape&         ES   = ExpS.Current();
      const ShapeList& lffs = myDPrism.GeneratedShapes(ES, myProfile1);
      for (it.Initialize(lffs); it.More(); it.Next())
      {
        if (View.Add(it.Value()))
          B.Add(C, it.Value());
      }
    }

    TopTools_IndexedDataMapOfShapeListOfShape theMapEF;
    TopExp1::MapShapesAndAncestors(C, TopAbs_EDGE, TopAbs_FACE, theMapEF);
    View.Clear();

    for (i = 1; i <= theMapEF.Extent(); i++)
    {
      if (theMapEF(i).Extent() == 1)
      {
        const TopoEdge& edg = TopoDS::Edge(theMapEF.FindKey(i));
        const TopoFace& fac = TopoDS::Face(theMapEF(i).First());
        if (View.Add(fac))
        {
          TopoShape aLocalShape = fac.EmptyCopied();
          TopoFace  newFace(TopoDS::Face(aLocalShape));
          //	  TopoFace newFace(TopoDS::Face(fac.EmptyCopied()));
          ShapeExplorer exp;
          for (exp.Init(fac.Oriented(TopAbs_FORWARD), TopAbs_WIRE); exp.More(); exp.Next())
          {
            //	    for (ShapeExplorer exp2(exp.Current(),TopAbs_EDGE);
            ShapeExplorer exp2(exp.Current(), TopAbs_EDGE);
            for (; exp2.More(); exp2.Next())
            {
              if (exp2.Current().IsSame(edg))
              {
                B.Add(newFace, exp.Current());
                lfaces.Append(newFace);
                lcomplete.Append(newFace);
                break;
              }
            }
            if (exp2.More())
            {
              break;
            }
          }
        }
      }
    }

    BS.Perform(lfaces);
    myFirstShape = BS.Shape();

    B.MakeCompound(D);

    ExpS.ReInit();
    View.Clear();

    for (; ExpS.More(); ExpS.Next())
    {
      const TopoShape&         ES   = ExpS.Current();
      const ShapeList& lfls = myDPrism.GeneratedShapes(ES, myProfile3);
      for (it.Initialize(lfls); it.More(); it.Next())
      {
        if (View.Add(it.Value()))
          B.Add(D, it.Value());
      }
    }

    lfaces.Clear();
    theMapEF.Clear();
    TopExp1::MapShapesAndAncestors(D, TopAbs_EDGE, TopAbs_FACE, theMapEF);
    View.Clear();

    for (i = 1; i <= theMapEF.Extent(); i++)
    {
      if (theMapEF(i).Extent() == 1)
      {
        const TopoEdge& edg = TopoDS::Edge(theMapEF.FindKey(i));
        const TopoFace& fac = TopoDS::Face(theMapEF(i).First());
        if (View.Add(fac))
        {
          TopoShape aLocalShape = fac.EmptyCopied();
          TopoFace  newFace(TopoDS::Face(aLocalShape));
          //	  TopoFace newFace(TopoDS::Face(fac.EmptyCopied()));
          ShapeExplorer exp;
          for (exp.Init(fac.Oriented(TopAbs_FORWARD), TopAbs_WIRE); exp.More(); exp.Next())
          {
            //	    for (ShapeExplorer exp2(exp.Current(),TopAbs_EDGE);
            ShapeExplorer exp2(exp.Current(), TopAbs_EDGE);
            for (; exp2.More(); exp2.Next())
            {
              if (exp2.Current().IsSame(edg))
              {
                B.Add(newFace, exp.Current());
                lfaces.Append(newFace);
                lcomplete.Append(newFace);
                break;
              }
            }
            if (exp2.More())
            {
              break;
            }
          }
        }
      }
    }
    BS.Perform(lfaces);
    myLastShape = BS.Shape();

    View.Clear();

    for (ExpS.ReInit(); ExpS.More(); ExpS.Next())
    {
      const TopoShape&         ES   = ExpS.Current();
      const ShapeList& lffs = myDPrism.GeneratedShapes(ES, myProfile2);

      for (it.Initialize(lffs); it.More(); it.Next())
      {
        if (it.Value().ShapeType() == TopAbs_EDGE)
        {
          break;
        }
      }
      if (it.More())
      {
        TopoShape RemovedEdge = it.Value();
        TopoFace  NewFace;
        TopoWire  NewWire;
        B.MakeWire(NewWire);
        TopAbs_Orientation Orref = TopAbs_FORWARD;
        ShapeExplorer    exp;
        for (it.Initialize(lffs); it.More(); it.Next())
        {
          if (it.Value().ShapeType() == TopAbs_FACE)
          {
            exp.Init(it.Value().Oriented(TopAbs_FORWARD), TopAbs_WIRE);
            const TopoShape theWire = exp.Current();
            if (NewFace.IsNull())
            {
              Handle(GeomSurface) S = BRepInspector::Surface(TopoDS::Face(it.Value()));
              if (S->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
              {
                S = Handle(Geom_RectangularTrimmedSurface)::DownCast(S)->BasisSurface();
              }
              if (S->DynamicType() != STANDARD_TYPE(GeomPlane))
              {
                break;
              }

              B.MakeFace(NewFace, S, BRepInspector::Tolerance(TopoDS::Face(it.Value())));
              NewFace.Orientation(TopAbs_FORWARD);
              Orref = theWire.Orientation();
              for (exp.Init(theWire.Oriented(TopAbs_FORWARD), TopAbs_EDGE); exp.More(); exp.Next())
              {
                if (!exp.Current().IsSame(RemovedEdge))
                {
                  B.Add(NewWire, exp.Current());
                }
              }
            }
            else
            {
              for (exp.Init(theWire.Oriented(TopAbs_FORWARD), TopAbs_EDGE); exp.More(); exp.Next())
              {
                if (!exp.Current().IsSame(RemovedEdge))
                {
                  if (theWire.Orientation() != Orref)
                  { // Les 2 faces planes ont des normales opposees
                    B.Add(NewWire, exp.Current());
                  }
                  else
                  {
                    B.Add(NewWire, exp.Current().Reversed());
                  }
                }
              }
            }
          }
        }
        if (!NewFace.IsNull())
        {
          B.Add(NewFace, NewWire.Oriented(Orref));
          lcomplete.Append(NewFace);
          ShapeList thelist;
          myMap.Bind(ES, thelist);
          myMap(ES).Append(NewFace);
        }
        else
        {
          for (it.Initialize(lffs); it.More(); it.Next())
          {
            if (View.Add(it.Value()) && it.Value().ShapeType() == TopAbs_FACE)
            {
              lcomplete.Append(it.Value());
            }
          }
        }
      }
      else
      {
        for (it.Initialize(lffs); it.More(); it.Next())
        {
          if (View.Add(it.Value()) && it.Value().ShapeType() == TopAbs_FACE)
          {
            lcomplete.Append(it.Value());
          }
        }
      }

      ShapeExplorer ExpS2;
      for (ExpS2.Init(ES, TopAbs_VERTEX); ExpS2.More(); ExpS2.Next())
      {
        const ShapeList& ls2 = myDPrism.GeneratedShapes(ExpS2.Current(), myProfile2);
        for (it.Initialize(ls2); it.More(); it.Next())
        {
          if (View.Add(it.Value()) && it.Value().ShapeType() == TopAbs_FACE)
          {
            lcomplete.Append(it.Value());
          }
        }
      }
    }

    BS.Perform(lcomplete);
    myRes = BS.Shape();
    BRepLib::UpdateTolerances(myRes);
  }
}

//=================================================================================================

LocOpe_DPrism::LocOpe_DPrism(const TopoFace&  Spine,
                             const Standard_Real Height,
                             const Standard_Real Angle)
    : mySpine(Spine)
{
  Standard_Integer i;
  myHeight        = Height;
  Standard_Real y = Height * sin(Angle);
  Standard_Real z = Height * cos(Angle);

  TopoVertex Vert1 = BRepLib_MakeVertex(Point3d(0, 0, 0));
  TopoVertex Vert2 = BRepLib_MakeVertex(Point3d(0, y, z));
  myProfile2          = BRepLib_MakeEdge(Vert1, Vert2);

  Standard_Real Umin, Umax, Vmin, Vmax;
  BRepTools1::UVBounds(Spine, Umin, Umax, Vmin, Vmax);
  Standard_Real Deltay = Max(Umax - Umin, Vmax - Vmin) + Abs(y);
  Deltay *= 2;

  TopoVertex Vert3 = BRepLib_MakeVertex(Point3d(0, y + Deltay, z));
  myProfile3          = BRepLib_MakeEdge(Vert2, Vert3);

  TopoVertex Vert4 = BRepLib_MakeVertex(Point3d(0, Deltay, 0));
  myProfile1          = BRepLib_MakeEdge(Vert4, Vert1);

  myProfile = BRepLib_MakeWire(myProfile1, myProfile2, myProfile3);
  myDPrism.Perform(mySpine, myProfile, gp1::XOY());

  if (myDPrism.IsDone())
  {
    LocOpe_BuildShape    BS;
    ShapeBuilder         B;
    TopoCompound      C;
    TopoCompound      D;
    ShapeList lfaces, lcomplete;

    B.MakeCompound(C);
    TopTools_ListIteratorOfListOfShape it;
    ShapeExplorer                    ExpS(mySpine, TopAbs_EDGE);
    TopTools_MapOfShape                View;
    for (; ExpS.More(); ExpS.Next())
    {
      const TopoShape&         ES   = ExpS.Current();
      const ShapeList& lffs = myDPrism.GeneratedShapes(ES, myProfile1);
      for (it.Initialize(lffs); it.More(); it.Next())
      {
        if (View.Add(it.Value()))
          B.Add(C, it.Value());
      }
    }

    TopTools_IndexedDataMapOfShapeListOfShape theMapEF;
    TopExp1::MapShapesAndAncestors(C, TopAbs_EDGE, TopAbs_FACE, theMapEF);
    View.Clear();

    for (i = 1; i <= theMapEF.Extent(); i++)
    {
      if (theMapEF(i).Extent() == 1)
      {
        const TopoEdge& edg = TopoDS::Edge(theMapEF.FindKey(i));
        const TopoFace& fac = TopoDS::Face(theMapEF(i).First());
        if (View.Add(fac))
        {
          TopoShape aLocalShape = fac.EmptyCopied();
          TopoFace  newFace(TopoDS::Face(aLocalShape));
          //	  TopoFace newFace(TopoDS::Face(fac.EmptyCopied()));
          ShapeExplorer exp;
          for (exp.Init(fac.Oriented(TopAbs_FORWARD), TopAbs_WIRE); exp.More(); exp.Next())
          {
            //	    for (ShapeExplorer exp2(exp.Current(),TopAbs_EDGE);
            ShapeExplorer exp2(exp.Current(), TopAbs_EDGE);
            for (; exp2.More(); exp2.Next())
            {
              if (exp2.Current().IsSame(edg))
              {
                B.Add(newFace, exp.Current());
                lfaces.Append(newFace);
                lcomplete.Append(newFace);
                break;
              }
            }
            if (exp2.More())
            {
              break;
            }
          }
        }
      }
    }

    BS.Perform(lfaces);
    myFirstShape = BS.Shape();

    B.MakeCompound(D);

    ExpS.ReInit();
    View.Clear();

    for (; ExpS.More(); ExpS.Next())
    {
      const TopoShape&         ES   = ExpS.Current();
      const ShapeList& lfls = myDPrism.GeneratedShapes(ES, myProfile3);
      for (it.Initialize(lfls); it.More(); it.Next())
      {
        if (View.Add(it.Value()))
          B.Add(D, it.Value());
      }
    }

    lfaces.Clear();
    theMapEF.Clear();
    TopExp1::MapShapesAndAncestors(D, TopAbs_EDGE, TopAbs_FACE, theMapEF);
    View.Clear();

    for (i = 1; i <= theMapEF.Extent(); i++)
    {
      if (theMapEF(i).Extent() == 1)
      {
        const TopoEdge& edg = TopoDS::Edge(theMapEF.FindKey(i));
        const TopoFace& fac = TopoDS::Face(theMapEF(i).First());
        if (View.Add(fac))
        {
          TopoShape aLocalShape = fac.EmptyCopied();
          TopoFace  newFace(TopoDS::Face(aLocalShape));
          //	  TopoFace newFace(TopoDS::Face(fac.EmptyCopied()));
          ShapeExplorer exp;
          for (exp.Init(fac.Oriented(TopAbs_FORWARD), TopAbs_WIRE); exp.More(); exp.Next())
          {
            //	    for (ShapeExplorer exp2(exp.Current(),TopAbs_EDGE);
            ShapeExplorer exp2(exp.Current(), TopAbs_EDGE);
            for (; exp2.More(); exp2.Next())
            {
              if (exp2.Current().IsSame(edg))
              {
                B.Add(newFace, exp.Current());
                lfaces.Append(newFace);
                lcomplete.Append(newFace);
                break;
              }
            }
            if (exp2.More())
            {
              break;
            }
          }
        }
      }
    }
    BS.Perform(lfaces);
    myLastShape = BS.Shape();

    View.Clear();
    for (ExpS.ReInit(); ExpS.More(); ExpS.Next())
    {
      const TopoShape&         ES = ExpS.Current();
      const ShapeList& ls = myDPrism.GeneratedShapes(ES, myProfile2);
      for (it.Initialize(ls); it.More(); it.Next())
      {
        if (View.Add(it.Value()))
        {
          lcomplete.Append(it.Value());
        }
      }
      ShapeExplorer ExpS2;
      for (ExpS2.Init(ES, TopAbs_VERTEX); ExpS2.More(); ExpS2.Next())
      {
        const ShapeList& ls2 = myDPrism.GeneratedShapes(ExpS2.Current(), myProfile2);
        for (it.Initialize(ls2); it.More(); it.Next())
        {
          if (View.Add(it.Value()) && it.Value().ShapeType() == TopAbs_FACE)
          {
            lcomplete.Append(it.Value());
          }
        }
      }
    }

    BS.Perform(lcomplete);
    myRes = BS.Shape();
    BRepLib::UpdateTolerances(myRes);
  }
}

//=================================================================================================

Standard_Boolean LocOpe_DPrism::IsDone() const
{
  return myDPrism.IsDone();
}

//=================================================================================================

const TopoShape& LocOpe_DPrism::Shape() const
{
  if (!myDPrism.IsDone())
  {
    throw StdFail_NotDone();
  }
  return myRes;
}

//=================================================================================================

const TopoShape& LocOpe_DPrism::Spine() const
{
  return mySpine;
}

//=================================================================================================

const TopoShape& LocOpe_DPrism::Profile() const
{
  return myProfile;
}

//=================================================================================================

const TopoShape& LocOpe_DPrism::FirstShape() const
{
  return myFirstShape;
}

//=================================================================================================

const TopoShape& LocOpe_DPrism::LastShape() const
{
  return myLastShape;
}

//=================================================================================================

const ShapeList& LocOpe_DPrism::Shapes(const TopoShape& S) const
{
  if (!myDPrism.IsDone())
  {
    throw StdFail_NotDone();
  }
  if (myMap.IsBound(S))
  {
    return myMap(S);
  }
  else
  {
    return myDPrism.GeneratedShapes(S, myProfile2);
  }
}

//=================================================================================================

void LocOpe_DPrism::Curves(TColGeom_SequenceOfCurve& Scurves) const
{
  // Retrieves dy and dz with myProfile2
  TopoVertex V1, V2;
  TopExp1::Vertices(myProfile2, V1, V2);
  Point3d        P1 = BRepInspector::Pnt(V1);
  Point3d        P2 = BRepInspector::Pnt(V2);
  Standard_Real dy = P2.Y() - P1.Y();
  Standard_Real dz = P2.Z() - P1.Z();
  Scurves.Clear();
  Handle(GeomSurface) S = BRepInspector::Surface(mySpine);
  if (S->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
  {
    S = Handle(Geom_RectangularTrimmedSurface)::DownCast(S)->BasisSurface();
  }

  Handle(GeomPlane) PP = Handle(GeomPlane)::DownCast(S);
  if (PP.IsNull())
  {
    throw Standard_ConstructionError();
  }

  gp_Pln P = PP->Pln();
  Dir3d Normale(P.Axis().Direction());
  if (!P.Direct())
  {
    Normale.Reverse();
  }

  TopTools_MapOfShape theMap;
  ShapeExplorer     exp(mySpine.Oriented(TopAbs_FORWARD), TopAbs_EDGE);
  TopLoc_Location     Loc;
  Handle(GeomCurve3d)  C;
  Standard_Real       f, l, prm;
  Standard_Integer    i;

  for (; exp.More(); exp.Next())
  {
    const TopoEdge& edg = TopoDS::Edge(exp.Current());
    if (!theMap.Add(edg))
    {
      continue;
    }
    if (!BRepInspector::Degenerated(edg))
    {
      C                = BRepInspector::Curve(edg, Loc, f, l);
      C                = Handle(GeomCurve3d)::DownCast(C->Transformed(Loc.Transformation()));
      Standard_Real u1 = -2 * Abs(myHeight);
      Standard_Real u2 = 2 * Abs(myHeight);

      for (i = 0; i <= NECHANT; i++)
      {
        prm = ((NECHANT - i) * f + i * l) / NECHANT;
        Point3d pt;
        Vector3d d1;
        C->D1(prm, pt, d1);
        if (exp.Current().Orientation() == TopAbs_REVERSED)
        {
          d1.Reverse();
        }
        d1.Normalize();
        Dir3d                    locy = Normale.Crossed(d1);
        Vector3d                    ldir = dy * locy.XYZ() + dz * Normale.XYZ();
        gp_Lin                    lin(pt, ldir);
        Handle(GeomLine)         Lin   = new GeomLine(lin);
        Handle(Geom_TrimmedCurve) trlin = new Geom_TrimmedCurve(Lin, u1, u2, Standard_True);
        Scurves.Append(trlin);
      }
    }
  }
}

//=================================================================================================

Handle(GeomCurve3d) LocOpe_DPrism::BarycCurve() const
{
  TopoVertex V1, V2;
  TopExp1::Vertices(myProfile2, V1, V2);
  Point3d        P1 = BRepInspector::Pnt(V1);
  Point3d        P2 = BRepInspector::Pnt(V2);
  Standard_Real dz = P2.Z() - P1.Z();

  Handle(GeomSurface) S = BRepInspector::Surface(mySpine);
  if (S->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
  {
    S = Handle(Geom_RectangularTrimmedSurface)::DownCast(S)->BasisSurface();
  }

  Handle(GeomPlane) PP = Handle(GeomPlane)::DownCast(S);
  if (PP.IsNull())
  {
    throw Standard_ConstructionError();
  }

  gp_Pln P = PP->Pln();
  Dir3d Normale(P.Axis().Direction());
  if (!P.Direct())
  {
    Normale.Reverse();
  }
  if (mySpine.Orientation() == TopAbs_REVERSED)
  {
#ifdef OCCT_DEBUG
    Standard_Boolean trc = BRepFeat_GettraceFEAT();
    if (trc)
    {
      std::cout << "LocOpe_DPrism::BarycCurve()" << std::endl;
      std::cout << " Reversed Spine orientation" << std::endl;
    }
#endif
    //    Normale.Reverse();  //cts20871
  }
  Vector3d Vec = dz * Normale.XYZ();

  Point3d               bar(0., 0., 0.);
  TColgp_SequenceOfPnt spt;
  if (!myFirstShape.IsNull())
  {
    LocOpe1::SampleEdges(myFirstShape, spt);
  }
  else
  {
    LocOpe1::SampleEdges(mySpine, spt);
  }
  for (Standard_Integer jj = 1; jj <= spt.Length(); jj++)
  {
    const Point3d& pvt = spt(jj);
    bar.ChangeCoord() += pvt.XYZ();
  }
  bar.ChangeCoord().Divide(spt.Length());
  Axis3d            newAx(bar, Vec);
  Handle(GeomLine) theLin = new GeomLine(newAx);
  return theLin;
}
