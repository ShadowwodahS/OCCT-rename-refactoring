// Created on: 1997-10-08
// Created by: Olga KOULECHOVA
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

#include <BRep_Builder.hxx>
#include <BRepAlgo.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBndLib.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepFeat.hxx>
#include <BRepFeat_Builder.hxx>
#include <BRepFeat_RibSlot.hxx>
#include <BRepIntCurveSurface_Inter.hxx>
#include <BRepLib.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <BRepLib_MakeFace.hxx>
#include <BRepLib_MakeVertex.hxx>
#include <BRepLib_MakeWire.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <CSLib.hxx>
#include <ElCLib.hxx>
#include <ElSLib.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Hyperbola.hxx>
#include <Geom_Line.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_Plane.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAPI.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomLib.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <LocOpe.hxx>
#include <LocOpe_CSIntersector.hxx>
#include <LocOpe_FindEdges.hxx>
#include <LocOpe_Gluer.hxx>
#include <LocOpe_PntFace.hxx>
#include <Precision.hxx>
#include <TColGeom_SequenceOfCurve.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

#ifdef OCCT_DEBUG
extern Standard_Boolean BRepFeat_GettraceFEAT();
extern Standard_Boolean BRepFeat_GettraceFEATRIB();
#endif

//=======================================================================
// function : LFPerform
// purpose  : topological reconstruction of ribs
//=======================================================================

void BRepFeat_RibSlot::LFPerform()
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEAT();
  if (trc)
    std::cout << "BRepFeat_RibSlot::LFPerform()" << std::endl;
#endif
  if (mySbase.IsNull() || myPbase.IsNull() || mySkface.IsNull() || myGShape.IsNull()
      || myLFMap.IsEmpty())
  {
#ifdef OCCT_DEBUG
    std::cout << "Topological reconstruction is impossible" << std::endl;
    if (trc)
      std::cout << " Fields not initialized" << std::endl;
#endif
    myStatusError = BRepFeat_NotInitialized;
    NotDone();
    return;
  }

  ShapeExplorer  exp, exp2;
  Standard_Integer theOpe = 2;

  if (!myGluedF.IsEmpty())
  {
    theOpe = 1;
  }

  // Hope that there is just a solid in the result
  if (!mySUntil.IsNull())
  {
    for (exp2.Init(mySUntil, TopAbs_FACE); exp2.More(); exp2.Next())
    {
      const TopoShape& funtil = exp2.Current();
      for (exp.Init(mySbase, TopAbs_FACE); exp.More(); exp.Next())
      {
        if (exp.Current().IsSame(funtil))
        {
          break;
        }
      }
      if (!exp.More())
      {
        break;
      }
    }
  }

  TopTools_ListIteratorOfListOfShape            it, it2;
  TopTools_DataMapIteratorOfDataMapOfShapeShape itm;
  // Standard_Integer sens = 0;

  LocOpe_Gluer theGlue;

  // case of gluing

  if (theOpe == 1)
  {
    Standard_Boolean Collage = Standard_True;

    LocOpe_FindEdges                   theFE;
    TopTools_DataMapOfShapeListOfShape locmap;
    theGlue.Init(mySbase, myGShape);
    for (itm.Initialize(myGluedF); itm.More(); itm.Next())
    {
      const TopoFace& glface = TopoDS::Face(itm.Key1());
      const TopoFace& fac    = TopoDS::Face(myGluedF(glface));
      for (exp.Init(myGShape, TopAbs_FACE); exp.More(); exp.Next())
      {
        if (exp.Current().IsSame(glface))
        {
          break;
        }
      }
      if (exp.More())
      {
        Collage = BRepFeat1::IsInside(glface, fac);
        if (!Collage)
        {
          theOpe = 2;
          break;
        }
        else
        {
          theGlue.Bind(glface, fac);
          theFE.Set(glface, fac);
          for (theFE.InitIterator(); theFE.More(); theFE.Next())
          {
            theGlue.Bind(theFE.EdgeFrom(), theFE.EdgeTo());
          }
        }
      }
    }

    LocOpe_Operation ope = theGlue.OpeType();
    if (ope == LocOpe_INVALID || (myFuse && ope != LocOpe_FUSE) || (!myFuse && ope != LocOpe_CUT)
        || (!Collage))
    {
      theOpe = 2;
#ifdef OCCT_DEBUG
      std::cout << "Passage to topological operations" << std::endl;
#endif
    }
  }

  // gluing is always applicable

  if (theOpe == 1)
  {
    theGlue.Perform();
    if (theGlue.IsDone())
    {
      UpdateDescendants(theGlue);
      myNewEdges = theGlue.Edges();
      myTgtEdges = theGlue.TgtEdges();
      //
      Done();
      myShape = theGlue.ResultingShape();
      BRepLib1::SameParameter(myShape, 1.e-7, Standard_True);
    }
    else
    {
      theOpe = 2;
#ifdef OCCT_DEBUG
      std::cout << "Passage to topologic operation" << std::endl;
#endif
    }
  }

  // case without gluing
  if (theOpe == 2)
  {
    BRepFeat_Builder                   theBuilder;
    ShapeList               partsoftool;
    BRepClass3d_SolidClassifier        oussa;
    Standard_Boolean                   bFlag;
    TopTools_ListIteratorOfListOfShape aIt;

    bFlag = (myPerfSelection == BRepFeat_NoSelection) ? 0 : 1;
    //
    theBuilder.Init(mySbase, myGShape);
    theBuilder.SetOperation(myFuse, bFlag);
    //
    theBuilder.Perform();
    if (bFlag)
    {
      theBuilder.PartsOfTool(partsoftool);
      aIt.Initialize(partsoftool);
      if (aIt.More() && myPerfSelection != BRepFeat_NoSelection)
      {
        Standard_Real toler = (BRepInspector::Tolerance(myPbase)) * 2;
        //
        for (; aIt.More(); aIt.Next())
        {
          oussa.Load(aIt.Value());
          oussa.Perform(myFirstPnt, toler);
          TopAbs_State sp1 = oussa.State();
          oussa.Perform(myLastPnt, toler);
          TopAbs_State sp2 = oussa.State();
          if (!(sp1 == TopAbs_OUT || sp2 == TopAbs_OUT))
          {
            const TopoShape& S = aIt.Value();
            theBuilder.KeepPart(S);
          }
        }
      }
      //
      theBuilder.PerformResult();
      myShape = theBuilder.Shape();
    }
    else
    {
      myShape = theBuilder.Shape();
    }
    Done();
  }
}

//=================================================================================================

Standard_Boolean BRepFeat_RibSlot::IsDeleted(const TopoShape& F)
{
  return (myMap(F).IsEmpty());
}

//=================================================================================================

const ShapeList& BRepFeat_RibSlot::Modified(const TopoShape& F)
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEAT();
  if (trc)
    std::cout << "BRepFeat_RibSlot::Modified" << std::endl;
#endif
  if (myMap.IsBound(F))
  {
    static ShapeList list;
    list.Clear();
    TopTools_ListIteratorOfListOfShape ite(myMap(F));
    for (; ite.More(); ite.Next())
    {
      const TopoShape& sh = ite.Value();
      if (!sh.IsSame(F))
        list.Append(sh);
    }
    return list;
  }
  return myGenerated; // empty list
}

//=================================================================================================

const ShapeList& BRepFeat_RibSlot::Generated(const TopoShape& S)
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEAT();
  if (trc)
    std::cout << "BRepFeat_RibSlot::Generated" << std::endl;
#endif
  if (S.ShapeType() != TopAbs_FACE)
  {
    myGenerated.Clear();
    if (myLFMap.IsEmpty() || !myLFMap.IsBound(S))
    {
      if (myMap.IsBound(S))
      { // check if filter on face or not
        static ShapeList list;
        list.Clear();
        TopTools_ListIteratorOfListOfShape ite(myMap(S));
        for (; ite.More(); ite.Next())
        {
          const TopoShape& sh = ite.Value();
          if (!sh.IsSame(S))
            list.Append(sh);
        }
        return list;
      }
      else
        return myGenerated;
    }
    else
    {
      myGenerated.Clear();
      TopTools_ListIteratorOfListOfShape it(myLFMap(S));
      static ShapeList        list;
      list.Clear();
      for (; it.More(); it.Next())
      {
        if (myMap.IsBound(it.Value()))
        {
          TopTools_ListIteratorOfListOfShape it1(myMap(it.Value()));
          for (; it1.More(); it1.Next())
          {
            const TopoShape& sh = it1.Value();
            if (!sh.IsSame(S))
              list.Append(sh);
          }
        }
      }
      return list;
    }
  }
  else
    return myGenerated;
}

//=================================================================================================

void BRepFeat_RibSlot::UpdateDescendants(const LocOpe_Gluer& G)
{
  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itdm;
  TopTools_ListIteratorOfListOfShape                  it, it2;
  TopTools_MapIteratorOfMapOfShape                    itm;

  for (itdm.Initialize(myMap); itdm.More(); itdm.Next())
  {
    const TopoShape& orig = itdm.Key1();
    TopTools_MapOfShape newdsc;
    for (it.Initialize(itdm.Value()); it.More(); it.Next())
    {
      const TopoFace& fdsc = TopoDS::Face(it.Value());
      for (it2.Initialize(G.DescendantFaces(fdsc)); it2.More(); it2.Next())
      {
        newdsc.Add(it2.Value());
      }
    }
    myMap.ChangeFind(orig).Clear();
    for (itm.Initialize(newdsc); itm.More(); itm.Next())
    {
      myMap.ChangeFind(orig).Append(itm.Key1());
    }
  }
}

//=================================================================================================

const ShapeList& BRepFeat_RibSlot::FirstShape() const
{
  if (!myFShape.IsNull())
  {
    return myMap(myFShape);
  }
  return myGenerated; // empty list
}

//=================================================================================================

const ShapeList& BRepFeat_RibSlot::LastShape() const
{
  if (!myLShape.IsNull())
  {
    return myMap(myLShape);
  }
  return myGenerated; // empty list
}

//=================================================================================================

const ShapeList& BRepFeat_RibSlot::FacesForDraft() const
{
  return myFacesForDraft;
}

//=================================================================================================

const ShapeList& BRepFeat_RibSlot::NewEdges() const
{
  return myNewEdges;
}

//=================================================================================================

const ShapeList& BRepFeat_RibSlot::TgtEdges() const
{
  return myTgtEdges;
}

//=================================================================================================

BRepFeat_StatusError BRepFeat_RibSlot::CurrentStatusError() const
{
  return myStatusError;
}

//=======================================================================
// function : CheckPoint
// purpose  : Proofing point material side (side of extrusion)
//=======================================================================

Point3d BRepFeat_RibSlot::CheckPoint(const TopoEdge& e,
                                    const Standard_Real, // bnd,
                                    const Handle(GeomPlane)& Pln)

{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEATRIB();
  if (trc)
    std::cout << "BRepFeat_RibSlot::CheckPoint" << std::endl;
#endif
  // Vector product : normal to plane X direction Wire
  // -> gives the material side
  // Proofing point somewhat inside the material side
  Standard_Real      f, l;
  Handle(GeomCurve3d) cc = BRepInspector::Curve(e, f, l);

  Vector3d        tgt;
  Point3d        pp;
  Standard_Real par = (f + l) / 2.;

  cc->D1(par, pp, tgt);

  if (e.Orientation() == TopAbs_REVERSED)
    tgt.Reverse();

  Vector3d D = -tgt.Crossed(Pln->Pln().Position1().Direction()) / 10.;
  pp.Translate(D);

  return pp;
}

//=======================================================================
// function : Normal
// purpose  : calculate the normal to a face in a point
//=======================================================================

Dir3d BRepFeat_RibSlot::Normal(const TopoFace& F, const Point3d& P)

{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEATRIB();
  if (trc)
    std::cout << "BRepFeat_RibSlot::Normal" << std::endl;
#endif
  Standard_Real U, V;
  Point3d        pt;

  BRepAdaptor_Surface AS(F, Standard_True);

  switch (AS.GetType())
  {

    case GeomAbs_Plane:
      ElSLib1::Parameters(AS.Plane1(), P, U, V);
      break;

    case GeomAbs_Cylinder:
      ElSLib1::Parameters(AS.Cylinder(), P, U, V);
      break;

    case GeomAbs_Cone:
      ElSLib1::Parameters(AS.Cone(), P, U, V);
      break;

    case GeomAbs_Torus:
      ElSLib1::Parameters(AS.Torus(), P, U, V);
      break;

    default: {
      return Dir3d(1., 0., 0.);
    }
  }

  Vector3d D1U, D1V;

  AS.D1(U, V, pt, D1U, D1V);
  Dir3d                 N;
  CSLib_DerivativeStatus St;
  CSLib1::Normal(D1U, D1V, Precision1::Confusion(), St, N);
  if (F.Orientation() == TopAbs_FORWARD)
    N.Reverse();
  return N;
}

//=======================================================================
// function : IntPar
// purpose  : calculate the parameter of a point on a curve
//=======================================================================

Standard_Real BRepFeat_RibSlot::IntPar(const Handle(GeomCurve3d)& C, const Point3d& P)

{
  if (C.IsNull())
    return 0.;

  GeomAdaptor_Curve AC(C);
  Standard_Real     U;

  switch (AC.GetType())
  {

    case GeomAbs_Line:
      U = ElCLib1::Parameter(AC.Line(), P);
      break;

    case GeomAbs_Circle:
      U = ElCLib1::Parameter(AC.Circle(), P);
      break;

    case GeomAbs_Ellipse:
      U = ElCLib1::Parameter(AC.Ellipse(), P);
      break;

    case GeomAbs_Hyperbola:
      U = ElCLib1::Parameter(AC.Hyperbola(), P);
      break;

    case GeomAbs_Parabola:
      U = ElCLib1::Parameter(AC.Parabola(), P);
      break;

    default:
      U = 0.;
  }

  return U;
}

//=======================================================================
// function : EdgeExtention
// purpose  : extension of a edge by tangence
//=======================================================================

void BRepFeat_RibSlot::EdgeExtention(TopoEdge&           e,
                                     const Standard_Real    bnd,
                                     const Standard_Boolean FirstLast)
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEAT();
  if (trc)
    std::cout << "BRepFeat_RibSlot::EdgeExtention" << std::endl;
#endif
  Standard_Real             f, l;
  Handle(GeomCurve3d)        cu = BRepInspector::Curve(e, f, l);
  Handle(Geom_BoundedCurve) C  = new Geom_TrimmedCurve(cu, f, l);

  TopoEdge E;

  if (cu->DynamicType() == STANDARD_TYPE(GeomLine)
      || cu->DynamicType() == STANDARD_TYPE(GeomCircle)
      || cu->DynamicType() == STANDARD_TYPE(Geom_Ellipse)
      || cu->DynamicType() == STANDARD_TYPE(Geom_Hyperbola)
      || cu->DynamicType() == STANDARD_TYPE(Geom_Parabola))
  {
    if (FirstLast)
    {
      BRepLib_MakeEdge Edg(cu, f - bnd / 10., l);
      E = TopoDS::Edge(Edg.Shape());
    }
    else
    {
      BRepLib_MakeEdge Edg(cu, f, l + bnd / 10.);
      E = TopoDS::Edge(Edg.Shape());
    }
  }
  else
  {
    Handle(GeomLine) ln;
    Point3d            Pt;
    Point3d            pnt;
    Vector3d            vct;
    if (FirstLast)
    {
      C->D1(f, pnt, vct);
      ln = new GeomLine(pnt, -vct);
      ln->D0(bnd / 1000., Pt);
      GeomLib1::ExtendCurveToPoint(C, Pt, GeomAbs_G1, Standard_False);
      BRepLib_MakeEdge Edg(C, Pt, BRepInspector::Pnt(TopExp1::LastVertex(e, Standard_True)));
      E = TopoDS::Edge(Edg.Shape());
    }
    else
    {
      C->D1(l, pnt, vct);
      ln = new GeomLine(pnt, vct);
      ln->D0(bnd / 1000., Pt);
      GeomLib1::ExtendCurveToPoint(C, Pt, GeomAbs_G1, Standard_True);
      BRepLib_MakeEdge Edg(C, BRepInspector::Pnt(TopExp1::FirstVertex(e, Standard_True)), Pt);
      E = TopoDS::Edge(Edg.Shape());
    }
  }
  e = E;
}

//=======================================================================
// function : ChoiceOfFaces
// purpose  : choose face of support in case of support on an edge
//=======================================================================

TopoFace BRepFeat_RibSlot::ChoiceOfFaces(ShapeList&     faces,
                                            const Handle(GeomCurve3d)& cc,
                                            const Standard_Real       par,
                                            const Standard_Real, // bnd,
                                            const Handle(GeomPlane)& Pln)

{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEATRIB();
  if (trc)
    std::cout << "BRepFeat_RibSlot::ChoiceOfFaces" << std::endl;
#endif
  TopoFace FFF;

  Point3d pp;
  Vector3d tgt;

  cc->D1(par, pp, tgt);

  Handle(GeomLine) l1 = new GeomLine(pp, tgt);

  TColGeom_SequenceOfCurve scur;
  Standard_Integer         Counter = 0;

  Axis3d Axe(pp, Pln->Position1().Direction());
  for (Standard_Integer i = 1; i <= 8; i++)
  {
    Handle(GeomCurve3d) L = Handle(GeomCurve3d)::DownCast(l1->Rotated(Axe, i * M_PI / 9.));
    scur.Append(L);
    Counter++;
  }

  TopTools_ListIteratorOfListOfShape it;
  it.Initialize(faces);
  Standard_Real Par = RealLast();
  for (; it.More(); it.Next())
  {
    const TopoFace&   f = TopoDS::Face(it.Value());
    LocOpe_CSIntersector ASI(f);
    ASI.Perform(scur);
    if (!ASI.IsDone())
      continue;
    for (Standard_Integer jj = 1; jj <= Counter; jj++)
    {
      if (ASI.NbPoints(jj) >= 1)
      {
        Standard_Real app = ASI.Point(jj, 1).Parameter();
        if (app >= 0 && app < Par)
        {
          Par = app;
          FFF = f;
        }
      }
    }
  }

  return FFF;
}

//=======================================================================
// function : HeightMax
// purpose  : Calculate the height of the prism following the parameters of a bounding box
//=======================================================================

Standard_Real BRepFeat_RibSlot::HeightMax(const TopoShape& theSbase,
                                          const TopoShape& theSUntil,
                                          Point3d&             p1,
                                          Point3d&             p2)
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEATRIB();
  if (trc)
    std::cout << "BRepFeat_RibSlot::HeightMax" << std::endl;
#endif
  Box2 Box1;
  BRepBndLib1::Add(theSbase, Box1);
  if (!theSUntil.IsNull())
  {
    BRepBndLib1::Add(theSUntil, Box1);
  }
  Standard_Real c[6], bnd;
  Box1.Get(c[0], c[2], c[4], c[1], c[3], c[5]);
  bnd = c[0];
  for (Standard_Integer i = 0; i < 6; i++)
  {
    if (c[i] > bnd)
      bnd = c[i];
  }
  p1.SetCoord(c[0] - 2. * bnd, c[1] - 2. * bnd, c[2] - 2. * bnd);
  p2.SetCoord(c[3] + 2. * bnd, c[4] + 2. * bnd, c[5] + 2. * bnd);
  return (bnd);
}

//=======================================================================
// function : ExtremeFaces
// purpose  : Calculate the base faces of the rib
//=======================================================================

Standard_Boolean BRepFeat_RibSlot::ExtremeFaces(const Standard_Boolean    RevolRib,
                                                const Standard_Real       bnd,
                                                const Handle(GeomPlane)& Pln,
                                                TopoEdge&              FirstEdge,
                                                TopoEdge&              LastEdge,
                                                TopoFace&              FirstFace,
                                                TopoFace&              LastFace,
                                                TopoVertex&            FirstVertex,
                                                TopoVertex&            LastVertex,
                                                Standard_Boolean&         OnFirstFace,
                                                Standard_Boolean&         OnLastFace,
                                                Standard_Boolean&         PtOnFirstEdge,
                                                Standard_Boolean&         PtOnLastEdge,
                                                TopoEdge&              OnFirstEdge,
                                                TopoEdge&              OnLastEdge)

{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEAT();
  if (trc)
    std::cout << "BRepFeat_RibSlot::ExtremeFaces" << std::endl;
#endif
  Standard_Boolean Data = Standard_True;
  FirstFace.Nullify();
  LastFace.Nullify();
  FirstEdge.Nullify();
  LastEdge.Nullify();
  PtOnFirstEdge = Standard_False;
  PtOnLastEdge  = Standard_False;
  OnFirstEdge.Nullify();
  OnLastEdge.Nullify();

  BRepIntCurveSurface_Inter inter;
  ShapeBuilder              B;
  ShapeExplorer           ex1;

  Standard_Boolean FirstOK = Standard_False, LastOK = Standard_False;

  Standard_Integer NumberOfEdges = 0;
  ShapeExplorer  exp(myWire, TopAbs_EDGE);

  for (; exp.More(); exp.Next())
  {
    NumberOfEdges++;
  }

  // ---the wire includes only one edge
  if (NumberOfEdges == 1)
  {
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << " One Edge" << std::endl;
#endif
    exp.ReInit();
    Standard_Real f, l; //, f1, l1, temp;
    Point3d        firstpoint, lastpoint;

    // Points limit the unique edge
    const TopoEdge& E  = TopoDS::Edge(exp.Current());
    Handle(GeomCurve3d) cc = BRepInspector::Curve(E, f, l);
    Point3d             p1 = BRepInspector::Pnt(TopExp1::FirstVertex(E, Standard_True));
    Point3d             p2 = BRepInspector::Pnt(TopExp1::LastVertex(E, Standard_True));

    Standard_Real FirstPar = f;
    Standard_Real LastPar  = l;

    // ---Find if 2 points limiting the unique edge of the wire
    //    are on an edge or a vertex of the base shape
    Standard_Boolean PtOnFirstVertex = Standard_False;
    Standard_Boolean PtOnLastVertex  = Standard_False;
    TopoVertex    OnFirstVertex, OnLastVertex;
    PtOnEdgeVertex(RevolRib,
                   mySbase,
                   p1,
                   FirstVertex,
                   LastVertex,
                   PtOnFirstEdge,
                   OnFirstEdge,
                   PtOnFirstVertex,
                   OnFirstVertex);
    PtOnEdgeVertex(RevolRib,
                   mySbase,
                   p2,
                   FirstVertex,
                   LastVertex,
                   PtOnLastEdge,
                   OnLastEdge,
                   PtOnLastVertex,
                   OnLastVertex);

    TopTools_MapOfShape Map;

    if (PtOnFirstEdge)
    {
      if (!PtOnFirstVertex)
      {
        // Find FirstFace : face of the base shape containing OnFirstEdge
        //                  meeting ChoiceOfFaces
        ShapeExplorer ex4, ex5;
        ex4.Init(mySbase, TopAbs_FACE);
        ShapeList faces;
        faces.Clear();
        Map.Clear();
        for (; ex4.More(); ex4.Next())
        {
          const TopoFace& fx = TopoDS::Face(ex4.Current());
          if (!Map.Add(fx))
            continue;
          ex5.Init(ex4.Current(), TopAbs_EDGE);
          for (; ex5.More(); ex5.Next())
          {
            const TopoEdge& ee = TopoDS::Edge(ex5.Current());
            if (ee.IsSame(OnFirstEdge))
            {
              faces.Append(fx);
            }
          }
        }
        if (!faces.IsEmpty())
        {
          TopoFace FFF = ChoiceOfFaces(faces, cc, FirstPar + bnd / 50., bnd / 50., Pln);
          if (!FFF.IsNull())
            FirstFace = FFF;
        }
      }
      else if (PtOnFirstVertex)
      {
        // Find FirstFace : face of the base shape containing OnFirstVertex
        //                  meeting ChoiceOfFaces
        ShapeExplorer ex4, ex5;
        ex4.Init(mySbase, TopAbs_FACE);
        ShapeList faces;
        faces.Clear();
        Map.Clear();
        for (; ex4.More(); ex4.Next())
        {
          const TopoFace& fx = TopoDS::Face(ex4.Current());
          if (!Map.Add(fx))
            continue;
          ex5.Init(ex4.Current(), TopAbs_VERTEX);
          for (; ex5.More(); ex5.Next())
          {
            const TopoVertex& vv = TopoDS::Vertex(ex5.Current());
            if (vv.IsSame(OnFirstVertex))
            {
              faces.Append(fx);
              break;
            }
          }
        }
        if (!faces.IsEmpty())
        {
          TopoFace FFF = ChoiceOfFaces(faces, cc, FirstPar + bnd / 50., bnd / 50., Pln);
          if (!FFF.IsNull())
            FirstFace = FFF;
        }
      }
      FirstEdge = E;
      BRepLib_MakeVertex v(p1);
      FirstVertex = v;
      OnFirstFace = Standard_True;
    }

    if (PtOnLastEdge)
    {
      if (!PtOnLastVertex)
      {
        // Find LastFace : face of the base shape containing OnLastEdge
        //                 meeting ChoiceOfFaces
        ShapeExplorer ex4, ex5;
        ex4.Init(mySbase, TopAbs_FACE);
        ShapeList faces;
        faces.Clear();
        Map.Clear();
        for (; ex4.More(); ex4.Next())
        {
          const TopoFace& fx = TopoDS::Face(ex4.Current());
          if (!Map.Add(fx))
            continue;
          ex5.Init(ex4.Current(), TopAbs_EDGE);
          for (; ex5.More(); ex5.Next())
          {
            const TopoEdge& ee = TopoDS::Edge(ex5.Current());
            if (ee.IsSame(OnLastEdge))
            {
              faces.Append(fx);
              break;
            }
          }
        }
        if (!faces.IsEmpty())
        {
          TopoFace FFF = ChoiceOfFaces(faces, cc, LastPar - bnd / 50., bnd / 50., Pln);
          if (!FFF.IsNull())
            LastFace = FFF;
        }
      }
      else if (PtOnLastEdge && PtOnLastVertex)
      {
        // Find LastFace : face of the base shape containing OnLastVertex
        //                 meeting ChoiceOfFaces
        ShapeExplorer ex4, ex5;
        ex4.Init(mySbase, TopAbs_FACE);
        ShapeList faces;
        faces.Clear();
        Map.Clear();
        for (; ex4.More(); ex4.Next())
        {
          const TopoFace& fx = TopoDS::Face(ex4.Current());
          if (!Map.Add(fx))
            continue;
          ex5.Init(ex4.Current(), TopAbs_VERTEX);
          for (; ex5.More(); ex5.Next())
          {
            const TopoVertex& vv = TopoDS::Vertex(ex5.Current());
            if (vv.IsSame(OnLastVertex))
            {
              faces.Append(fx);
              break;
            }
          }
        }
        if (!faces.IsEmpty())
        {
          TopoFace FFF = ChoiceOfFaces(faces, cc, LastPar - bnd / 50., bnd / 50., Pln);
          if (!FFF.IsNull())
            LastFace = FFF;
        }
      }
      LastEdge = E;
      BRepLib_MakeVertex v(p2);
      LastVertex = v;
      OnLastFace = Standard_True;
    }

    if (!FirstFace.IsNull() && !LastFace.IsNull())
    {
      return Standard_True;
    }

//--- FirstFace or LastFace was not found
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << " FirstFace or LastFace null" << std::endl;
#endif
    LocOpe_CSIntersector     ASI(mySbase);
    TColGeom_SequenceOfCurve scur;
    scur.Clear();
    scur.Append(cc);
    ASI.Perform(scur);
    Standard_Real lastpar, firstpar;
    if (ASI.IsDone() && ASI.NbPoints(1) >= 2)
    {
      lastpar                    = ASI.Point(1, ASI.NbPoints(1)).Parameter();
      Standard_Integer lastindex = ASI.NbPoints(1);
      if (lastpar > l)
      {
        for (Standard_Integer jj = ASI.NbPoints(1) - 1; jj >= 1; jj--)
        {
          Standard_Real par = ASI.Point(1, jj).Parameter();
          if (par <= l)
          {
            lastpar   = par;
            lastindex = jj;
            break;
          }
        }
      }
      Standard_Integer firstindex = lastindex - 1;
      firstpar                    = ASI.Point(1, firstindex).Parameter();

      if (FirstFace.IsNull())
      {
        FirstFace = ASI.Point(1, firstindex).Face();
        cc->D0(firstpar, firstpoint);
        BRepLib_MakeVertex v1(firstpoint);
        FirstVertex = TopoDS::Vertex(v1.Shape());
        FirstEdge   = E;
      }

      if (LastFace.IsNull())
      {
        LastFace = ASI.Point(1, lastindex).Face();
        cc->D0(lastpar, lastpoint);
        BRepLib_MakeVertex v2(lastpoint);
        LastVertex = TopoDS::Vertex(v2.Shape());
        LastEdge   = E;
      }
    }
    else
    {
#ifdef OCCT_DEBUG
      if (trc)
        std::cout << " Less than 2 intersection points" << std::endl;
#endif
      Data = Standard_False;
      return Data;
    }

    if (!OnFirstFace)
    {
      if (p1.Distance(firstpoint) <= Precision1::Confusion())
        OnFirstFace = Standard_True;
      else
        OnFirstFace = Standard_False;
    }

    if (!OnLastFace)
    {
      if (p2.Distance(lastpoint) <= Precision1::Confusion())
        OnLastFace = Standard_True;
      else
        OnLastFace = Standard_False;
    }

    if (FirstFace.IsNull() || LastFace.IsNull())
    {
#ifdef OCCT_DEBUG
      if (trc)
        std::cout << " First or Last Faces still null" << std::endl;
#endif
      Data = Standard_False;
    }
    else
    {
#ifdef OCCT_DEBUG
      if (trc)
        std::cout << " FirstFace and LastFace OK" << std::endl;
#endif
      Data = Standard_True;
    }

    return Data;
  }
  // ---The wire consists of several edges
  else
  {
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << " Multiple Edges" << std::endl;
#endif
    BRepTools_WireExplorer ex(myWire);
    for (; ex.More(); ex.Next())
    {
      const TopoEdge& E = TopoDS::Edge(ex.Current());
      Standard_Real      f, l;
      Handle(GeomCurve3d) Cur = BRepInspector::Curve(E, f, l);
      f                      = f - bnd / 10000;
      l                      = l + bnd / 10000;
      Handle(Geom_TrimmedCurve) curve;
      curve = new Geom_TrimmedCurve(Cur, f, l, Standard_True);
#ifdef OCCT_DEBUG
      Point3d P1 = BRepInspector::Pnt(TopExp1::FirstVertex(E, Standard_True));
      (void)P1;
#endif
      Point3d P2 = BRepInspector::Pnt(TopExp1::LastVertex(E, Standard_True));
      ex1.Init(mySbase, TopAbs_FACE);
      TopoVertex    theVertex;
      TopoEdge      theEdge;
      TopoFace      theFace;
      Standard_Boolean PtOnEdge   = Standard_False;
      Standard_Boolean PtOnVertex = Standard_False;
      TopoEdge      OnEdge;
      TopoVertex    OnVertex;
      Standard_Real    intpar;
      for (; ex1.More(); ex1.Next())
      {
        const TopoFace& aCurFace = TopoDS::Face(ex1.Current());
        GeomAdaptor_Curve  aGAC(curve);
        inter.Init(aCurFace, aGAC, BRepInspector::Tolerance(aCurFace));
        if (!inter.More())
          continue;
        for (; inter.More(); inter.Next())
        {
          Point3d thePoint = inter.Pnt();
          if (!FirstVertex.IsNull())
          {
            Point3d point = BRepInspector::Pnt(FirstVertex);
            if (point.Distance(thePoint) <= BRepInspector::Tolerance(aCurFace))
            {
              continue;
            }
          }
          intpar  = IntPar(curve, thePoint);
          theEdge = E;
          theFace = aCurFace;
          B.MakeVertex(theVertex, thePoint, Precision1::Confusion());
          if (!FirstOK)
          {
            if (thePoint.Distance(P2) <= Precision1::Confusion())
            {
              continue;
            }
          }

          // ---Find thepoint on an edge or a vertex of face f
          PtOnEdgeVertex(RevolRib,
                         aCurFace,
                         thePoint,
                         FirstVertex,
                         LastVertex,
                         PtOnEdge,
                         OnEdge,
                         PtOnVertex,
                         OnVertex);

          //          if(!theEdge.IsNull()) break;

          if (FirstEdge.IsNull() && !theEdge.IsNull() && !theFace.IsNull() && !theVertex.IsNull())
          {
            FirstEdge     = theEdge;
            FirstFace     = theFace;
            FirstVertex   = theVertex;
            PtOnFirstEdge = PtOnEdge;
            OnFirstEdge   = OnEdge;
            theEdge.Nullify();
            theFace.Nullify();
            theVertex.Nullify();
            if (PtOnEdge && !PtOnVertex)
            {
              ShapeList faces;
              faces.Clear();
              faces.Append(FirstFace);
              ShapeExplorer ex2;
              ex2.Init(mySbase, TopAbs_FACE);
              for (; ex2.More(); ex2.Next())
              {
                TopoFace     fx = TopoDS::Face(ex2.Current());
                ShapeExplorer ex3;
                ex3.Init(fx, TopAbs_EDGE);
                for (; ex3.More(); ex3.Next())
                {
                  const TopoEdge& e = TopoDS::Edge(ex3.Current());
                  if (e.IsSame(OnEdge) && !fx.IsSame(FirstFace))
                  {
                    faces.Append(fx);
                  }
                }
              }
              TopoFace FFF = ChoiceOfFaces(faces, curve, intpar + bnd / 10., bnd / 10., Pln);
              if (!FFF.IsNull())
                FirstFace = FFF;
            }
            else if (PtOnEdge && PtOnVertex)
            {
              ShapeList faces;
              faces.Clear();
              faces.Append(FirstFace);
              ShapeExplorer ex2;
              ex2.Init(mySbase, TopAbs_FACE);
              for (; ex2.More(); ex2.Next())
              {
                TopoFace     fx = TopoDS::Face(ex2.Current());
                ShapeExplorer ex3;
                ex3.Init(fx, TopAbs_VERTEX);
                for (; ex3.More(); ex3.Next())
                {
                  const TopoVertex& v = TopoDS::Vertex(ex3.Current());
                  if (v.IsSame(OnVertex) && !fx.IsSame(FirstFace))
                  {
                    faces.Append(fx);
                  }
                }
              }
              TopoFace FFF = ChoiceOfFaces(faces, curve, intpar + bnd / 10., bnd / 10., Pln);
              if (!FFF.IsNull())
                FirstFace = FFF;
            }
            if (!FirstEdge.IsNull() && !FirstFace.IsNull() && !FirstVertex.IsNull())
            {
              FirstOK = Standard_True;
            }
          }
          if (LastEdge.IsNull() && !theEdge.IsNull() && !theFace.IsNull() && !theVertex.IsNull()
              && !FirstEdge.IsNull())
          {
            LastEdge     = theEdge;
            LastFace     = theFace;
            LastVertex   = theVertex;
            PtOnLastEdge = PtOnEdge;
            OnLastEdge   = OnEdge;
            if (PtOnEdge && !PtOnVertex)
            {
              ShapeList faces;
              faces.Clear();
              faces.Append(LastFace);
              ShapeExplorer ex2;
              ex2.Init(mySbase, TopAbs_FACE);
              for (; ex2.More(); ex2.Next())
              {
                TopoFace     fx = TopoDS::Face(ex2.Current());
                ShapeExplorer ex3;
                ex3.Init(fx, TopAbs_EDGE);
                for (; ex3.More(); ex3.Next())
                {
                  const TopoEdge& e = TopoDS::Edge(ex3.Current());
                  if (e.IsSame(OnEdge) && !fx.IsSame(LastFace))
                  {
                    faces.Append(fx);
                  }
                }
              }
              TopoFace FFF = ChoiceOfFaces(faces, curve, intpar - bnd / 10., bnd / 10., Pln);
              if (!FFF.IsNull())
                LastFace = FFF;
            }
            else if (PtOnEdge && PtOnVertex)
            {
              ShapeList faces;
              faces.Clear();
              faces.Append(LastFace);
              ShapeExplorer ex2;
              ex2.Init(mySbase, TopAbs_FACE);
              for (; ex2.More(); ex2.Next())
              {
                TopoFace     fx = TopoDS::Face(ex2.Current());
                ShapeExplorer ex3;
                ex3.Init(fx, TopAbs_VERTEX);
                for (; ex3.More(); ex3.Next())
                {
                  const TopoVertex& v = TopoDS::Vertex(ex3.Current());
                  if (v.IsSame(OnVertex) && !fx.IsSame(LastFace))
                  {
                    faces.Append(fx);
                  }
                }
              }
              TopoFace FFF = ChoiceOfFaces(faces, curve, intpar - bnd / 10., bnd / 10., Pln);
              if (!FFF.IsNull())
                LastFace = FFF;
            }
            if (!LastEdge.IsNull() && !LastFace.IsNull() && !LastVertex.IsNull())
            {
              LastOK = Standard_True;
            }
            break;
          }
        }
      }
    }

    if (FirstOK && LastOK)
    {
      Data       = Standard_True;
      Point3d PP1 = BRepInspector::Pnt(TopExp1::FirstVertex(FirstEdge, Standard_True));
      Point3d PP2 = BRepInspector::Pnt(TopExp1::LastVertex(LastEdge, Standard_True));
      Point3d p1  = BRepInspector::Pnt(FirstVertex);
      Point3d p2  = BRepInspector::Pnt(LastVertex);
      if (p1.Distance(PP1) <= BRepInspector::Tolerance(FirstFace))
      {
        OnFirstFace = Standard_True;
      }
      if (p2.Distance(PP2) <= BRepInspector::Tolerance(LastFace))
      {
        OnLastFace = Standard_True;
      }
      return Standard_True;
    }
    else
    {
#ifdef OCCT_DEBUG
      if (trc)
        std::cout << " First or Last not OK" << std::endl;
#endif
      return Standard_False;
    }
  }
}

//=======================================================================
// function : PtOnEdgeVertex
// purpose  : Find if 2 limit points of the unique edge of a wire
//           are on an edge or a vertex of the base shape
//=======================================================================

void BRepFeat_RibSlot::PtOnEdgeVertex(const Standard_Boolean RevolRib,
                                      const TopoShape&    shape,
                                      const Point3d&          point,
                                      const TopoVertex&, // FirstVertex,
                                      const TopoVertex&, // LastVertex,
                                      Standard_Boolean& PtOnEdge,
                                      TopoEdge&      OnEdge,
                                      Standard_Boolean& PtOnVertex,
                                      TopoVertex&    OnVertex)

{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEATRIB();
  if (trc)
    std::cout << "BRepFeat_RibSlot::PtOnEdgeVertex" << std::endl;
#endif
  Standard_Boolean TestOK;
  //  PtOnEdge = Standard_False;
  //  OnEdge.Nullify();
  //  PtOnVertex = Standard_False;
  //  OnVertex.Nullify();

  ShapeExplorer EXP;
  EXP.Init(shape, TopAbs_EDGE);
  TopTools_MapOfShape Map;
  for (; EXP.More(); EXP.Next())
  {
    const TopoEdge& e = TopoDS::Edge(EXP.Current());
    if (!Map.Add(e))
      continue;
    if (!RevolRib)
    {
      if (BRepInspector::Degenerated(e))
        continue;
    }
    Standard_Real      fff, lll;
    Handle(GeomCurve3d) ccc = BRepInspector::Curve(e, fff, lll);
    if (!RevolRib)
    {
      ccc = new Geom_TrimmedCurve(ccc, fff, lll);
    }
    GeomAPI_ProjectPointOnCurve proj(point, ccc);
    TestOK = Standard_False;
    if (!RevolRib)
    {
      if (proj.NbPoints() == 1)
        TestOK = Standard_True;
    }
    else
    {
      if (proj.NbPoints() >= 1)
        TestOK = Standard_True;
    }
    if (TestOK && proj.Distance(1) <= BRepInspector::Tolerance(e))
    {
      PtOnEdge          = Standard_True;
      OnEdge            = e;
      TopoVertex ev1 = TopExp1::FirstVertex(e, Standard_True);
      TopoVertex ev2 = TopExp1::LastVertex(e, Standard_True);
      Point3d        ep1 = BRepInspector::Pnt(ev1);
      Point3d        ep2 = BRepInspector::Pnt(ev2);
      if (point.Distance(ep1) <= BRepInspector::Tolerance(ev1))
      {
        PtOnVertex = Standard_True;
        OnVertex   = ev1;
        break;
      }
      else if (point.Distance(ep2) <= BRepInspector::Tolerance(ev1))
      {
        PtOnVertex = Standard_True;
        OnVertex   = ev2;
        break;
      }
      break;
    }
  }
}

//=======================================================================
// function : SlidingProfile
// purpose  : construction of the profile face in case of sliding
//=======================================================================

Standard_Boolean BRepFeat_RibSlot::SlidingProfile(TopoFace&              Prof,
                                                  const Standard_Boolean    RevolRib,
                                                  const Standard_Real       myTol,
                                                  Standard_Integer&         Concavite,
                                                  const Handle(GeomPlane)& myPln,
                                                  const TopoFace&        BndFace,
                                                  const Point3d&             CheckPnt,
                                                  const TopoFace&        FirstFace,
                                                  const TopoFace&        LastFace,
                                                  const TopoVertex&, // FirstVertex,
                                                  const TopoVertex&, // LastVertex,
                                                  const TopoEdge& FirstEdge,
                                                  const TopoEdge& LastEdge)

{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEAT();
  if (trc)
    std::cout << "BRepFeat_RibSlot::SlidingProfile" << std::endl;
#endif
  Standard_Boolean ProfileOK = Standard_True;
  // --case of sliding : construction of the wire of the profile
  // --> 1 part bounding box + 1 part wire
  //   attention to the compatibility of orientations

  Dir3d           FN, LN;
  BRepLib_MakeWire WW;

  FN = Normal(FirstFace, myFirstPnt);
  LN = Normal(LastFace, myLastPnt);

  // Case of the groove (cut) <> rib (fuse)
  // -> we are in the material
  // -> make everything in 2d in the working plane : easier
  if (!myFuse)
  {
    FN = -FN;
    LN = -LN;
  }

  Handle(GeomLine) ln1, ln2;
  Point3d            Pt; //,p1, p2;

  ln2 = new GeomLine(myFirstPnt, FN);
  ln1 = new GeomLine(myLastPnt, LN);

  Handle(GeomCurve2d) ln2d1 = GeomAPI1::To2d(ln1, myPln->Pln());
  Handle(GeomCurve2d) ln2d2 = GeomAPI1::To2d(ln2, myPln->Pln());

  Geom2dAPI_InterCurveCurve inter(ln2d1, ln2d2, Precision1::Confusion());

  Standard_Boolean TestOK = Standard_True;
  if (RevolRib)
  {
    Dir3d d1, d2;
    d1 = ln1->Position1().Direction();
    d2 = ln2->Position1().Direction();
    if (d1.IsOpposite(d2, myTol))
    {
      Standard_Real par1 = ElCLib1::Parameter(ln1->Lin(), myFirstPnt);
      Standard_Real par2 = ElCLib1::Parameter(ln2->Lin(), myLastPnt);
      if (par1 >= myTol || par2 >= myTol)
      {
        Concavite = 2; // parallel and concave
        BRepLib_MakeEdge e1(myLastPnt, myFirstPnt);
        WW.Add(e1);
      }
    }
    if (d1.IsEqual(d2, myTol))
    {
      if (Concavite == 3)
        TestOK = Standard_False;
    }
  }

  if (TestOK)
  {
    if (inter.NbPoints() > 0)
    {
      gp_Pnt2d P = inter.Point(1);
      myPln->D0(P.X(), P.Y(), Pt);
      Standard_Real par = IntPar(ln1, Pt);
      if (par > 0)
        Concavite = 1; // concave
    }
  }

  // ---Construction of the profile face
  if (Concavite == 1)
  {
    // if concave : it is possible to extend first and last edges of the wire
    //              to the bounding box
    BRepLib_MakeEdge e1(myLastPnt, Pt);
    WW.Add(e1);
    BRepLib_MakeEdge e2(Pt, myFirstPnt);
    WW.Add(e2);
  }
  else if (Concavite == 3)
  {
    // BndEdge : edges of intersection with the bounding box
    TopoEdge BndEdge1, BndEdge2;
    // Points of intersection with the bounding box / Find Profile
    Point3d          BndPnt1, BndPnt2, LastPnt;
    ShapeExplorer expl;
    expl.Init(BndFace, TopAbs_WIRE);
    BRepTools_WireExplorer explo;
    TopoWire            BndWire = TopoDS::Wire(expl.Current());
    explo.Init(BndWire);
    for (; explo.More(); explo.Next())
    {
      const TopoEdge&        e = TopoDS::Edge(explo.Current());
      Standard_Real             first, last;
      Handle(GeomCurve3d)        c   = BRepInspector::Curve(e, first, last);
      Handle(GeomCurve2d)      c2d = GeomAPI1::To2d(c, myPln->Pln());
      Geom2dAPI_InterCurveCurve intcln1(ln2d1, c2d, Precision1::Confusion());
      if (intcln1.NbPoints() > 0)
      {
        gp_Pnt2d p2d = intcln1.Point(1);
        Point3d   p;
        myPln->D0(p2d.X(), p2d.Y(), p);
        Standard_Real parl = IntPar(ln1, p);
        Standard_Real parc = IntPar(c, p);
        if (parc >= first && parc <= last && parl >= 0)
        {
          BndEdge1 = e;
          BndPnt1  = p;
        }
      }

      Geom2dAPI_InterCurveCurve intcln2(ln2d2, c2d, Precision1::Confusion());
      if (intcln2.NbPoints() > 0)
      {
        gp_Pnt2d p2d = intcln2.Point(1);
        Point3d   p;
        myPln->D0(p2d.X(), p2d.Y(), p);
        Standard_Real parl = IntPar(ln2, p);
        Standard_Real parc = IntPar(c, p);
        if (parc >= first && parc <= last && parl >= 0)
        {
          BndEdge2 = e;
          BndPnt2  = p;
        }
      }
      if (!BndEdge1.IsNull() && !BndEdge2.IsNull())
        break;
    }

    if (BndEdge1.IsNull() || BndEdge2.IsNull())
    {
#ifdef OCCT_DEBUG
      if (trc)
        std::cout << " Null bounding edge" << std::endl;
#endif
      ProfileOK = Standard_False;
      return ProfileOK;
    }

    BRepLib_MakeEdge e1(myLastPnt, BndPnt1);
    WW.Add(e1);

    if (BndEdge1.IsSame(BndEdge2))
    {
      // Particular case : same edge -> simply determined path
      BRepLib_MakeEdge e2(BndPnt1, BndPnt2);
      WW.Add(e2);
      BRepLib_MakeEdge e3(BndPnt2, myFirstPnt);
      WW.Add(e3);
    }
    else
    {
      explo.Init(BndWire);
      for (; explo.More(); explo.Next())
      {
        const TopoEdge& e = TopoDS::Edge(explo.Current());
        if (e.IsSame(BndEdge1))
        {
          Point3d pp;
          pp = BRepInspector::Pnt(TopExp1::LastVertex(e, Standard_True));
          if (pp.Distance(BndPnt1) >= BRepInspector::Tolerance(e))
          {
            LastPnt = pp;
          }
          //            else {         //LinearForm
          //              Point3d ppp = BRepInspector::Pnt(TopExp1::FirstVertex(e,Standard_True));
          //              LastPnt = ppp;
          //            }
          BRepLib_MakeEdge e2(BndPnt1, LastPnt);
          WW.Add(e2);
          break;
        }
      }

      if (explo.More())
      {
        explo.Next();
        if (explo.Current().IsNull())
          explo.Init(BndWire);
      }
      else
        explo.Init(BndWire);

      // Check if this is BndEdge2
      // -> if yes : it is required to turn to join FirstPnt
      // -> if no : add edges
      Standard_Boolean Fin = Standard_False;
      while (!Fin)
      {
        const TopoEdge& e = TopoDS::Edge(explo.Current());
        if (!e.IsSame(BndEdge2))
        {
          Point3d pp;
          pp = BRepInspector::Pnt(TopExp1::LastVertex(e, Standard_True));
          BRepLib_MakeEdge ee(LastPnt, pp);
          WW.Add(ee);
          LastPnt = pp;
        }
        else
        {
          // the path is closed
          // -> since met BndEdge2, end of borders on BndFace
          Fin = Standard_True;
          BRepLib_MakeEdge ee(LastPnt, BndPnt2);
          WW.Add(ee);
          LastPnt = BndPnt2;
        }
        if (explo.More())
        {
          explo.Next();
          if (explo.Current().IsNull())
          {
            explo.Init(BndWire);
          }
        }
        else
          explo.Init(BndWire);
      }

      BRepLib_MakeEdge e3(BndPnt2, myFirstPnt);
      WW.Add(e3);
    }
  }

  // ---Construction of the profile

  // Explore the wire provided by the user
  // BRepTools_WireExplorer : correct order - without repetition <> TopExp1 : non ordered
  BRepTools_WireExplorer EX(myWire);

  Standard_Real      ff, ll;
  Handle(GeomCurve3d) FirstCurve = BRepInspector::Curve(FirstEdge, ff, ll);

  if (!FirstEdge.IsSame(LastEdge))
  {
    TopoVertex    FLVert = TopExp1::LastVertex(FirstEdge, Standard_True);
    Point3d           FLPnt  = BRepInspector::Pnt(FLVert);
    BRepLib_MakeEdge ef(FirstCurve, myFirstPnt, FLPnt);
    WW.Add(ef);
    for (; EX.More(); EX.Next())
    {
      const TopoEdge& E = EX.Current();
      if (E.IsSame(FirstEdge))
        break;
    }
    EX.Next();
    for (; EX.More(); EX.Next())
    {
      const TopoEdge& E = EX.Current();
      if (!E.IsSame(LastEdge))
      {
        WW.Add(E);
      }
      else
        break;
    }
    Handle(GeomCurve3d) LastCurve = BRepInspector::Curve(LastEdge, ff, ll);
    TopoVertex      LFVert    = TopExp1::FirstVertex(LastEdge, Standard_True);
    Point3d             LFPnt     = BRepInspector::Pnt(LFVert);
    BRepLib_MakeEdge   el(LastCurve, LFPnt, myLastPnt);
    WW.Add(el);
  }
  else
  {
    // only one edge : particular processing
    Standard_Real      fpar = IntPar(FirstCurve, myFirstPnt);
    Standard_Real      lpar = IntPar(FirstCurve, myLastPnt);
    Handle(GeomCurve3d) c;
    if (fpar > lpar)
      c = FirstCurve->Reversed();
    else
      c = FirstCurve;

    BRepLib_MakeEdge ef(c, myFirstPnt, myLastPnt);
    WW.Add(ef);
  }

  BRepLib_MakeFace f(myPln->Pln(), WW, Standard_True);
  TopoFace      fac = TopoDS::Face(f.Shape());

  if (!BRepAlgo1::IsValid(fac))
  {
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << " Invalid Face" << std::endl;
#endif
    ProfileOK = Standard_False;
    return ProfileOK;
  }

  if (Concavite != 3)
  {
    // if concave : face is OK
    Prof = fac;
  }
  else
  {
    // if not concave
    // CheckPnt : point slightly inside the material side
    // Bndface  : face/cut of the bounding box in the plane of the profile
    BRepTopAdaptor_FClass2d Cl(fac, BRepInspector::Tolerance(fac));
    Standard_Real           u, v;
    ElSLib1::Parameters(myPln->Pln(), CheckPnt, u, v);
    gp_Pnt2d checkpnt2d(u, v);
    if (Cl.Perform(checkpnt2d, Standard_True) == TopAbs_OUT)
    {
      // If face is not the correct part of BndFace take the complementary
      BooleanCut    c(BndFace, fac);
      ShapeExplorer    exp(c.Shape(), TopAbs_WIRE);
      const TopoWire& w = TopoDS::Wire(exp.Current());
      BRepLib_MakeFace   ffx(w);
      Prof = TopoDS::Face(ffx.Shape());
    }
    else
    {
      // If face is the correct part of BndFace  : face is OK
      Prof = fac;
    }
  }

  if (!BRepAlgo1::IsValid(Prof))
  {
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << " Invalid Face Profile" << std::endl;
#endif
    ProfileOK = Standard_False;
    return ProfileOK;
  }
  return ProfileOK;
}

//=======================================================================
// function : NoSlidingProfile
// purpose  : construction of the face profile in case of sliding
//=======================================================================

Standard_Boolean BRepFeat_RibSlot::NoSlidingProfile(TopoFace&              Prof,
                                                    const Standard_Boolean    RevolRib,
                                                    const Standard_Real       myTol,
                                                    Standard_Integer&         Concavite,
                                                    const Handle(GeomPlane)& myPln,
                                                    const Standard_Real       bnd,
                                                    const TopoFace&        BndFace,
                                                    const Point3d&             CheckPnt,
                                                    const TopoFace&,   // FirstFace,
                                                    const TopoFace&,   // LastFace,
                                                    const TopoVertex&, // FirstVertex,
                                                    const TopoVertex&, // LastVertex,
                                                    const TopoEdge&     FirstEdge,
                                                    const TopoEdge&     LastEdge,
                                                    const Standard_Boolean OnFirstFace,
                                                    const Standard_Boolean OnLastFace)

{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEAT();
  if (trc)
    std::cout << "BRepFeat_RibSlot::NoSlidingProfile" << std::endl;
#endif
  Standard_Boolean ProfileOK = Standard_True;

  Standard_Real l1, f1, f2, l2; //, p;
  TopoVertex theFV;
  theFV.Nullify();
  Point3d      theFirstpoint;
  TopoEdge theLastEdge;
  theLastEdge.Nullify();
  Point3d       firstpoint, lastpoint; //, pp1, pp2;
  Vector3d       firstvect, lastvect;
  TopoWire  w;
  ShapeBuilder BB;
  BB.MakeWire(w);
  // Point3d p1, p3;
  TopoEdge FalseFirstEdge, FalseLastEdge, FalseOnlyOne;

  Handle(GeomCurve3d) FirstCurve = BRepInspector::Curve(FirstEdge, f1, l1);
  Handle(GeomCurve3d) LastCurve  = BRepInspector::Curve(LastEdge, f2, l2);

  Handle(GeomLine) firstln, lastln;
  FirstCurve->D1(f1, firstpoint, firstvect);
  lastln = new GeomLine(firstpoint, -firstvect);
  LastCurve->D1(l2, lastpoint, lastvect);
  firstln = new GeomLine(lastpoint, lastvect);

  Point3d Pt;

  Handle(GeomCurve2d) ln2d1 = GeomAPI1::To2d(firstln, myPln->Pln());
  Handle(GeomCurve2d) ln2d2 = GeomAPI1::To2d(lastln, myPln->Pln());

  Geom2dAPI_InterCurveCurve inter(ln2d1, ln2d2, Precision1::Confusion());

  Standard_Boolean TestOK = Standard_True;
  if (RevolRib)
  {
    Dir3d d1, d2;
    d1 = firstln->Position1().Direction();
    d2 = lastln->Position1().Direction();
    if (d1.IsOpposite(d2, myTol))
    {
      Standard_Real par1 = ElCLib1::Parameter(firstln->Lin(), myFirstPnt);
      Standard_Real par2 = ElCLib1::Parameter(lastln->Lin(), myLastPnt);
      if (par1 >= myTol || par2 >= myTol)
        Concavite = 2; // parallel and concave
    }
    if (d1.IsEqual(d2, myTol))
    {
      if (Concavite == 3)
        TestOK = Standard_False;
    }
  }

  if (TestOK)
  {
    if (inter.NbPoints() > 0)
    {
      gp_Pnt2d P = inter.Point(1);
      myPln->D0(P.X(), P.Y(), Pt);
      Standard_Real par = IntPar(firstln, Pt);
      if (par > 0)
        Concavite = 1; // concave
    }
  }

  // ---Construction of the face profile
  if (Concavite == 3)
  {
    if (OnFirstFace)
    {
      Standard_Real f, l;
      FalseFirstEdge = FirstEdge;
      EdgeExtention(FalseFirstEdge, bnd, Standard_True);
      const TopoVertex& vv1 = TopExp1::FirstVertex(FalseFirstEdge, Standard_True);
      firstpoint               = BRepInspector::Pnt(vv1);
      Handle(GeomCurve3d) cc    = BRepInspector::Curve(FalseFirstEdge, f, l);
      cc->D1(f, firstpoint, firstvect);
      lastln = new GeomLine(firstpoint, -firstvect);
      if (FirstEdge.IsSame(LastEdge))
        FalseOnlyOne = FalseFirstEdge;
      ln2d2 = GeomAPI1::To2d(lastln, myPln->Pln());
    }
    if (OnLastFace)
    {
      Standard_Real f, l;
      if (!FirstEdge.IsSame(LastEdge))
      {
        FalseLastEdge = LastEdge;
      }
      else
      {
        if (FalseOnlyOne.IsNull())
          FalseOnlyOne = LastEdge;
        FalseLastEdge = FalseOnlyOne;
      }
      EdgeExtention(FalseLastEdge, bnd, Standard_False);
      if (FirstEdge.IsSame(LastEdge))
      {
        FalseOnlyOne = FalseLastEdge;
      }
      const TopoVertex& vv2 = TopExp1::LastVertex(FalseLastEdge, Standard_True);
      lastpoint                = BRepInspector::Pnt(vv2);
      Handle(GeomCurve3d) cc    = BRepInspector::Curve(FalseLastEdge, f, l);
      cc->D1(l, lastpoint, lastvect);
      lastpoint = BRepInspector::Pnt(vv2);
      firstln   = new GeomLine(lastpoint, lastvect);
      ln2d1     = GeomAPI1::To2d(firstln, myPln->Pln());
    }

    TopoEdge     BndEdge1, BndEdge2;
    Point3d          BndPnt1, BndPnt2, LastPnt;
    ShapeExplorer expl;
    expl.Init(BndFace, TopAbs_WIRE);
    BRepTools_WireExplorer explo;
    TopoWire            BndWire = TopoDS::Wire(expl.Current());
    explo.Init(BndWire);
    for (; explo.More(); explo.Next())
    {
      const TopoEdge&        e = TopoDS::Edge(explo.Current());
      Standard_Real             first, last;
      Handle(GeomCurve3d)        c   = BRepInspector::Curve(e, first, last);
      Handle(GeomCurve2d)      c2d = GeomAPI1::To2d(c, myPln->Pln());
      Geom2dAPI_InterCurveCurve intcln1(ln2d1, c2d, Precision1::Confusion());
      if (intcln1.NbPoints() > 0)
      {
        gp_Pnt2d p2d = intcln1.Point(1);
        Point3d   p;
        myPln->D0(p2d.X(), p2d.Y(), p);
        Standard_Real parl = IntPar(firstln, p);
        Standard_Real parc = IntPar(c, p);
        if (parc >= first && parc <= last && parl >= 0)
        {
          BndEdge1 = e;
          BndPnt1  = p;
        }
      }

      Geom2dAPI_InterCurveCurve intcln2(ln2d2, c2d, Precision1::Confusion());
      if (intcln2.NbPoints() > 0)
      {
        gp_Pnt2d p2d = intcln2.Point(1);
        Point3d   p;
        myPln->D0(p2d.X(), p2d.Y(), p);
        Standard_Real parl = IntPar(lastln, p);
        Standard_Real parc = IntPar(c, p);
        if (parc >= first && parc <= last && parl >= 0)
        {
          BndEdge2 = e;
          BndPnt2  = p;
        }
      }
      if (!BndEdge1.IsNull() && !BndEdge2.IsNull())
        break;
    }

    if (BndEdge1.IsNull() || BndEdge2.IsNull())
    {
#ifdef OCCT_DEBUG
      if (trc)
        std::cout << " Null bounding edge" << std::endl;
#endif
      ProfileOK = Standard_False;
      return ProfileOK;
    }

    TopoEdge ee1;
    if (theLastEdge.IsNull())
    {
      BRepLib_MakeEdge e1(lastpoint, BndPnt1);
      ee1 = TopoDS::Edge(e1.Shape());
    }
    else
    {
      const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
      BRepLib_MakeVertex   v2(BndPnt1);
      BRepLib_MakeEdge     e1(v1, v2);
      ee1 = TopoDS::Edge(e1.Shape());
    }
    BB.Add(w, ee1);
    theLastEdge = ee1;
    if (theFV.IsNull())
    {
      theFV         = TopExp1::FirstVertex(ee1, Standard_True);
      theFirstpoint = BRepInspector::Pnt(theFV);
    }

    if (BndEdge1.IsSame(BndEdge2))
    {
      TopoEdge ee2, ee3;
      if (theLastEdge.IsNull())
      {
        BRepLib_MakeEdge e2(BndPnt1, BndPnt2);
        ee2 = TopoDS::Edge(e2.Shape());
      }
      else
      {
        const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
        BRepLib_MakeVertex   v2(BndPnt2);
        BRepLib_MakeEdge     e2(v1, v2);
        ee2 = TopoDS::Edge(e2.Shape());
      }
      BB.Add(w, ee2);
      theLastEdge = ee2;
      if (theFV.IsNull())
      {
        theFV         = TopExp1::FirstVertex(ee2, Standard_True);
        theFirstpoint = BRepInspector::Pnt(theFV);
      }
      if (theLastEdge.IsNull())
      {
        BRepLib_MakeEdge e3(BndPnt2, firstpoint);
        ee3 = TopoDS::Edge(e3.Shape());
      }
      else
      {
        const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
        BRepLib_MakeVertex   v2(firstpoint);
        BRepLib_MakeEdge     e3(v1, v2);
        ee3 = TopoDS::Edge(e3.Shape());
      }
      BB.Add(w, ee3);
      theLastEdge = ee3;
      if (theFV.IsNull())
      {
        theFV         = TopExp1::FirstVertex(ee3, Standard_True);
        theFirstpoint = BRepInspector::Pnt(theFV);
      }
    }
    else
    {
      explo.Init(BndWire);
      for (; explo.More(); explo.Next())
      {
        const TopoEdge& e = TopoDS::Edge(explo.Current());
        if (e.IsSame(BndEdge1))
        {
          Point3d pp;
          pp = BRepInspector::Pnt(TopExp1::LastVertex(e, Standard_True));
          if (pp.Distance(BndPnt1) > BRepInspector::Tolerance(e))
          {
            LastPnt = pp;
          }
          TopoEdge eee;
          if (theLastEdge.IsNull())
          {
            BRepLib_MakeEdge e2(BndPnt1, LastPnt);
            eee = TopoDS::Edge(e2.Shape());
          }
          else
          {
            const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
            BRepLib_MakeVertex   v2(LastPnt);
            BRepLib_MakeEdge     e2(v1, v2);
            eee = TopoDS::Edge(e2.Shape());
          }
          BB.Add(w, eee);
          theLastEdge = eee;
          if (theFV.IsNull())
          {
            theFV         = TopExp1::FirstVertex(eee, Standard_True);
            theFirstpoint = BRepInspector::Pnt(theFV);
          }
          break;
        }
      }

      if (explo.More())
      {
        explo.Next();
        if (explo.Current().IsNull())
          explo.Init(BndWire);
      }
      else
        explo.Init(BndWire);
      Standard_Boolean Fin = Standard_False;
      while (!Fin)
      {
        const TopoEdge& e = TopoDS::Edge(explo.Current());
        if (!e.IsSame(BndEdge2))
        {
          Point3d pp;
          pp = BRepInspector::Pnt(TopExp1::LastVertex(e, Standard_True));
          TopoEdge eee1;
          if (theLastEdge.IsNull())
          {
            BRepLib_MakeEdge ee(LastPnt, pp);
            eee1 = TopoDS::Edge(ee.Shape());
          }
          else
          {
            const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
            BRepLib_MakeVertex   v2(pp);
            BRepLib_MakeEdge     ee(v1, v2);
            eee1 = TopoDS::Edge(ee.Shape());
          }
          BB.Add(w, eee1);
          theLastEdge = eee1;
          if (theFV.IsNull())
          {
            theFV         = TopExp1::FirstVertex(eee1, Standard_True);
            theFirstpoint = BRepInspector::Pnt(theFV);
          }
          LastPnt = pp;
        }
        else
        {
          Fin = Standard_True;
          TopoEdge eee2;
          if (theLastEdge.IsNull())
          {
            BRepLib_MakeEdge ee(LastPnt, BndPnt2);
            eee2 = TopoDS::Edge(ee.Shape());
          }
          else
          {
            const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
            BRepLib_MakeVertex   v2(BndPnt2);
            BRepLib_MakeEdge     ee(v1, v2);
            eee2 = TopoDS::Edge(ee.Shape());
          }
          BB.Add(w, eee2);
          theLastEdge = eee2;
          if (theFV.IsNull())
          {
            theFV         = TopExp1::FirstVertex(eee2, Standard_True);
            theFirstpoint = BRepInspector::Pnt(theFV);
          }
          LastPnt = BndPnt2;
        }
        if (explo.More())
        {
          explo.Next();
          if (explo.Current().IsNull())
          {
            explo.Init(BndWire);
          }
        }
        else
          explo.Init(BndWire);
      }

      TopoEdge eee3;
      if (theLastEdge.IsNull())
      {
        BRepLib_MakeEdge e3(BndPnt2, firstpoint);
        eee3 = TopoDS::Edge(e3.Shape());
      }
      else
      {
        const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
        BRepLib_MakeVertex   v2(firstpoint);
        BRepLib_MakeEdge     e3(v1, v2);
        eee3 = TopoDS::Edge(e3.Shape());
      }
      BB.Add(w, eee3);
      theLastEdge = eee3;
      if (theFV.IsNull())
      {
        theFV         = TopExp1::FirstVertex(eee3, Standard_True);
        theFirstpoint = BRepInspector::Pnt(theFV);
      }
    }
  }

  if (Concavite == 1)
  {
    TopoEdge eee4;
    if (theLastEdge.IsNull())
    {
      BRepLib_MakeEdge e(Pt, firstpoint);
      eee4 = TopoDS::Edge(e.Shape());
    }
    else
    {
      const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
      BRepLib_MakeVertex   v2(firstpoint);
      BRepLib_MakeEdge     e(v1, v2);
      eee4 = TopoDS::Edge(e.Shape());
    }
    BB.Add(w, eee4);
    if (theFV.IsNull())
    {
      theFV         = TopExp1::FirstVertex(eee4, Standard_True);
      theFirstpoint = BRepInspector::Pnt(theFV);
    }
    theLastEdge = eee4;
  }

  if (FirstEdge.IsSame(LastEdge))
  {
    if (!myLFMap.IsBound(FirstEdge))
    {
      ShapeList thelist;
      myLFMap.Bind(FirstEdge, thelist);
    }
    if (OnFirstFace || OnLastFace)
    {
      TopoEdge        theEdge;
      Standard_Real      f, l;
      Handle(GeomCurve3d) cc = BRepInspector::Curve(FalseOnlyOne, f, l);
      if (!theLastEdge.IsNull())
      {
        const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
        TopoVertex        v2;
        const Point3d&        pp = BRepInspector::Pnt(TopExp1::LastVertex(FalseOnlyOne, Standard_True));
        if (!theFV.IsNull() && theFirstpoint.Distance(pp) <= myTol)
        {
          v2 = theFV;
        }
        else
          v2 = TopExp1::LastVertex(FalseOnlyOne, Standard_True);
        BRepLib_MakeEdge e(cc, v1, v2);
        theEdge = TopoDS::Edge(e.Shape());
      }
      else
      {
        const TopoVertex& v1 = TopExp1::FirstVertex(FalseOnlyOne, Standard_True);
        TopoVertex        v2;
        const Point3d&        pp = BRepInspector::Pnt(TopExp1::LastVertex(FalseOnlyOne, Standard_True));
        if (!theFV.IsNull() && theFirstpoint.Distance(pp) <= myTol)
        {
          v2 = theFV;
        }
        else
          v2 = TopExp1::LastVertex(FalseOnlyOne, Standard_True);
        BRepLib_MakeEdge e(cc, v1, v2);
        theEdge = TopoDS::Edge(e.Shape());
      }
      myLFMap(FirstEdge).Append(theEdge);
      BB.Add(w, theEdge);
      if (theFV.IsNull())
        theFV = TopExp1::FirstVertex(theEdge, Standard_True);
      theLastEdge = theEdge;
    }
    else
    {
      Standard_Real      f, l;
      Handle(GeomCurve3d) cc = BRepInspector::Curve(FirstEdge, f, l);
      TopoEdge        theEdge;
      if (!theLastEdge.IsNull())
      {
        const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
        TopoVertex        v2;
        // Attention case Wire Reversed -> LastVertex without Standard_True
        const Point3d& pp = BRepInspector::Pnt(TopExp1::LastVertex(FirstEdge));
        if (!theFV.IsNull() && theFirstpoint.Distance(pp) <= myTol)
        {
          v2 = theFV;
        }
        else
          v2 = TopExp1::LastVertex(FirstEdge);
        BRepLib_MakeEdge e(cc, v1, v2);
        theEdge = TopoDS::Edge(e.Shape());
      }
      else
      {
        const TopoVertex& v1 = TopExp1::FirstVertex(FirstEdge, Standard_True);
        TopoVertex        v2;
        const Point3d&        pp = BRepInspector::Pnt(TopExp1::LastVertex(FirstEdge, Standard_True));
        if (!theFV.IsNull() && theFirstpoint.Distance(pp) <= myTol)
        {
          v2 = theFV;
        }
        else
          v2 = TopExp1::LastVertex(FirstEdge, Standard_True);
        BRepLib_MakeEdge e(cc, v1, v2);
        theEdge = TopoDS::Edge(e.Shape());
      }
      myLFMap(FirstEdge).Append(theEdge);
      BB.Add(w, theEdge);
      if (theFV.IsNull())
        theFV = TopExp1::FirstVertex(theEdge, Standard_True);
      theLastEdge = theEdge;
    }
  }
  else
  {
    if (!myLFMap.IsBound(FirstEdge))
    {
      ShapeList thelist1;
      myLFMap.Bind(FirstEdge, thelist1);
    }
    if (!OnFirstFace)
    {
      TopoEdge        theEdge;
      Standard_Real      f, l;
      Handle(GeomCurve3d) cc = BRepInspector::Curve(FirstEdge, f, l);
      if (!theLastEdge.IsNull())
      {
        const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
        const TopoVertex& v2 = TopExp1::LastVertex(FirstEdge, Standard_True);
        BRepLib_MakeEdge     e(cc, v1, v2);
        theEdge = TopoDS::Edge(e.Shape());
      }
      else
      {
        theEdge = FirstEdge;
      }
      myLFMap(FirstEdge).Append(theEdge);
      BB.Add(w, theEdge);
      if (theFV.IsNull())
        theFV = TopExp1::FirstVertex(theEdge, Standard_True);
      theLastEdge = theEdge;
    }
    else
    {
      TopoEdge        theEdge;
      Standard_Real      f, l;
      Handle(GeomCurve3d) cc = BRepInspector::Curve(FalseFirstEdge, f, l);
      if (!theLastEdge.IsNull())
      {
        const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
        const TopoVertex& v2 = TopExp1::LastVertex(FalseFirstEdge, Standard_True);
        BRepLib_MakeEdge     e(cc, v1, v2);
        theEdge = TopoDS::Edge(e.Shape());
      }
      else
      {
        theEdge = FalseFirstEdge;
      }
      myLFMap(FirstEdge).Append(theEdge);
      BB.Add(w, theEdge);
      if (theFV.IsNull())
        theFV = TopExp1::FirstVertex(theEdge, Standard_True);
      theLastEdge = theEdge;
    }

    BRepTools_WireExplorer ex(myWire);
    for (; ex.More(); ex.Next())
    {
      const TopoEdge& E = ex.Current();
      if (E.IsSame(FirstEdge))
        break;
    }

    ex.Next();

    for (; ex.More(); ex.Next())
    {
      const TopoEdge& E = ex.Current();
      if (!E.IsSame(LastEdge))
      {
        if (!myLFMap.IsBound(E))
        {
          ShapeList thelist2;
          myLFMap.Bind(E, thelist2);
        }
        TopoEdge        eee;
        Standard_Real      f, l;
        Handle(GeomCurve3d) cc = BRepInspector::Curve(E, f, l);
        if (!theLastEdge.IsNull())
        {
          const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
          const TopoVertex& v2 = TopExp1::LastVertex(E, Standard_True);
          BRepLib_MakeEdge     e(cc, v1, v2);
          eee = TopoDS::Edge(e.Shape());
        }
        else
        {
          eee = E;
        }
        myLFMap(E).Append(eee);
        BB.Add(w, eee);
        if (theFV.IsNull())
          theFV = TopExp1::FirstVertex(eee, Standard_True);
        theLastEdge = eee;
      }
      else
        break;
    }

    if (!OnLastFace)
    {
      if (!FirstEdge.IsSame(LastEdge))
      {
        const TopoEdge& edg = TopoDS::Edge(ex.Current());
        if (!myLFMap.IsBound(edg))
        {
          ShapeList thelist3;
          myLFMap.Bind(edg, thelist3);
        }
        TopoEdge        eee;
        Standard_Real      f, l;
        Handle(GeomCurve3d) cc = BRepInspector::Curve(edg, f, l);
        if (!theLastEdge.IsNull())
        {
          const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
          TopoVertex        v2;
          const Point3d&        pp = BRepInspector::Pnt(TopExp1::LastVertex(edg, Standard_True));
          if (!theFV.IsNull() && theFirstpoint.Distance(pp) <= myTol)
          {
            v2 = theFV;
          }
          else
            v2 = TopExp1::LastVertex(edg, Standard_True);
          BRepLib_MakeEdge e(cc, v1, v2);
          eee = TopoDS::Edge(e.Shape());
        }
        else
        {
          const TopoVertex& v1 = TopExp1::FirstVertex(edg, Standard_True);
          TopoVertex        v2;
          const Point3d&        pp = BRepInspector::Pnt(TopExp1::LastVertex(edg, Standard_True));
          if (!theFV.IsNull() && theFirstpoint.Distance(pp) <= myTol)
          {
            v2 = theFV;
          }
          else
            v2 = TopExp1::LastVertex(edg, Standard_True);
          BRepLib_MakeEdge e(cc, v1, v2);
          eee = TopoDS::Edge(e.Shape());
        }
        myLFMap(edg).Append(eee);
        BB.Add(w, eee);
        if (theFV.IsNull())
          theFV = TopExp1::FirstVertex(eee, Standard_True);
        theLastEdge = eee;
      }
      else
      {
        TopoEdge   eee;
        Standard_Real f, l;
        if (!myLFMap.IsBound(LastEdge))
        {
          ShapeList thelist4;
          myLFMap.Bind(LastEdge, thelist4);
        }
        Handle(GeomCurve3d) cc = BRepInspector::Curve(FalseOnlyOne, f, l);
        if (!theLastEdge.IsNull())
        {
          const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
          TopoVertex        v2;
          const Point3d&        pp = BRepInspector::Pnt(TopExp1::LastVertex(FalseOnlyOne, Standard_True));
          if (!theFV.IsNull() && theFirstpoint.Distance(pp) <= myTol)
          {
            v2 = theFV;
          }
          else
            v2 = TopExp1::LastVertex(FalseOnlyOne, Standard_True);
          BRepLib_MakeEdge e(cc, v1, v2);
          eee = TopoDS::Edge(e.Shape());
        }
        else
        {
          const TopoVertex& v1 = TopExp1::FirstVertex(FalseOnlyOne, Standard_True);
          TopoVertex        v2;
          const Point3d&        pp = BRepInspector::Pnt(TopExp1::LastVertex(FalseOnlyOne, Standard_True));
          if (!theFV.IsNull() && theFirstpoint.Distance(pp) <= myTol)
          {
            v2 = theFV;
          }
          else
            v2 = TopExp1::LastVertex(FalseOnlyOne, Standard_True);
          BRepLib_MakeEdge e(cc, v1, v2);
          eee = TopoDS::Edge(e.Shape());
        }
        myLFMap(LastEdge).Append(eee);
        BB.Add(w, eee);
        if (theFV.IsNull())
          theFV = TopExp1::FirstVertex(eee, Standard_True);
        theLastEdge = eee;
      }
    }
    else
    {
      TopoEdge   eee;
      Standard_Real f, l;
      if (!myLFMap.IsBound(LastEdge))
      {
        ShapeList thelist5;
        myLFMap.Bind(LastEdge, thelist5);
      }
      Handle(GeomCurve3d) cc = BRepInspector::Curve(FalseLastEdge, f, l);
      if (!theLastEdge.IsNull())
      {
        const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
        TopoVertex        v2;
        const Point3d&        pp = BRepInspector::Pnt(TopExp1::LastVertex(FalseLastEdge, Standard_True));
        if (!theFV.IsNull() && theFirstpoint.Distance(pp) <= myTol)
        {
          v2 = theFV;
        }
        else
          v2 = TopExp1::LastVertex(FalseLastEdge, Standard_True);
        BRepLib_MakeEdge e(cc, v1, v2);
        eee = TopoDS::Edge(e.Shape());
      }
      else
      {
        const TopoVertex& v1 = TopExp1::FirstVertex(FalseLastEdge, Standard_True);
        TopoVertex        v2;
        const Point3d&        pp = BRepInspector::Pnt(TopExp1::LastVertex(FalseLastEdge, Standard_True));
        if (!theFV.IsNull() && theFirstpoint.Distance(pp) <= myTol)
        {
          v2 = theFV;
        }
        else
          v2 = TopExp1::LastVertex(FalseLastEdge, Standard_True);
        BRepLib_MakeEdge e(cc, v1, v2);
        eee = TopoDS::Edge(e.Shape());
      }
      myLFMap(LastEdge).Append(eee);
      BB.Add(w, eee);
      if (theFV.IsNull())
        theFV = TopExp1::FirstVertex(eee, Standard_True);
      theLastEdge = eee;
    }
  }

  if (Concavite == 1)
  {
    TopoEdge eef;
    if (theLastEdge.IsNull())
    {
      BRepLib_MakeEdge ef(lastpoint, Pt);
      eef = TopoDS::Edge(ef.Shape());
    }
    else
    {
      const TopoVertex& v1 = TopExp1::LastVertex(theLastEdge, Standard_True);
      BRepLib_MakeVertex   vv(Pt);
      TopoVertex        v2 = TopoDS::Vertex(vv.Shape());
      if (!theFV.IsNull() && Pt.Distance(theFirstpoint) <= myTol)
        v2 = theFV;

      BRepLib_MakeEdge ef(v1, v2);
      eef = TopoDS::Edge(ef.Shape());
    }
    BB.Add(w, eef);
    if (theFV.IsNull())
      theFV = TopExp1::FirstVertex(eef, Standard_True);
    theLastEdge = eef;
  }

  if (Concavite == 2)
  {
    BRepLib_MakeEdge   ee(lastpoint, firstpoint);
    const TopoEdge& e = ee.Edge();
    BB.Add(w, e);
  }

  w.Closed(BRepInspector::IsClosed(w));
  BRepLib_MakeFace fa(myPln->Pln(), w, Standard_True);
  TopoFace      fac = TopoDS::Face(fa.Shape());

  if (!BRepAlgo1::IsValid(fac))
  {
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << " Invalid Face" << std::endl;
#endif
    ProfileOK = Standard_False;
    return ProfileOK;
  }

  //    if(!Concavite) {
  if (Concavite == 3)
  {
    BRepTopAdaptor_FClass2d Cl(fac, BRepInspector::Tolerance(fac));
    Standard_Real           u, v;
    ElSLib1::Parameters(myPln->Pln(), CheckPnt, u, v);
    gp_Pnt2d checkpnt2d(u, v);
    if (Cl.Perform(checkpnt2d, Standard_True) == TopAbs_OUT)
    {
      BooleanCut c(BndFace, fac);
      ShapeExplorer exp(c.Shape(), TopAbs_WIRE);
      UpdateDescendants(c, c.Shape(), Standard_False);
      const TopoWire& ww = TopoDS::Wire(exp.Current());
      BRepLib_MakeFace   ff(ww);
      Prof = TopoDS::Face(ff.Shape());
    }
    else
    {
      Prof = fac;
    }
  }
  else
  {
    Prof = fac;
  }

  if (!BRepAlgo1::IsValid(Prof))
  {
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << " Invalid Face Profile" << std::endl;
#endif
    ProfileOK = Standard_False;
    return ProfileOK;
  }
  return ProfileOK;
}

//=================================================================================================

void BRepFeat_RibSlot::UpdateDescendants(const BRepAlgoAPI_BooleanOperation& aBOP,
                                         const TopoShape&                 S,
                                         const Standard_Boolean              SkipFace)
{
  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itdm;
  TopTools_ListIteratorOfListOfShape                  it, it2;
  TopTools_MapIteratorOfMapOfShape                    itm;
  ShapeExplorer                                     exp;

  for (itdm.Initialize(myMap); itdm.More(); itdm.Next())
  {
    const TopoShape& orig = itdm.Key1();
    if (SkipFace && orig.ShapeType() == TopAbs_FACE)
    {
      continue;
    }
    TopTools_MapOfShape newdsc;

    // if (itdm.Value().IsEmpty()) {myMap.ChangeFind(orig).Append(orig);}

    for (it.Initialize(itdm.Value()); it.More(); it.Next())
    {
      const TopoShape& sh = it.Value();
      if (sh.ShapeType() != TopAbs_FACE)
        continue;
      const TopoFace& fdsc = TopoDS::Face(it.Value());
      for (exp.Init(S, TopAbs_FACE); exp.More(); exp.Next())
      {
        if (exp.Current().IsSame(fdsc))
        { // preserved
          newdsc.Add(fdsc);
          break;
        }
      }
      if (!exp.More())
      {
        BRepAlgoAPI_BooleanOperation* pBOP = (BRepAlgoAPI_BooleanOperation*)&aBOP;
        const ShapeList&   aLM  = pBOP->Modified(fdsc);
        it2.Initialize(aLM);
        for (; it2.More(); it2.Next())
        {
          const TopoShape& aS = it2.Value();
          newdsc.Add(aS);
        }
      }
    }
    myMap.ChangeFind(orig).Clear();
    for (itm.Initialize(newdsc); itm.More(); itm.Next())
    {
      // check the belonging to the shape...
      for (exp.Init(S, TopAbs_FACE); exp.More(); exp.Next())
      {
        if (exp.Current().IsSame(itm.Key1()))
        {
          //          const TopoShape& sh = itm.Key1();
          myMap.ChangeFind(orig).Append(itm.Key1());
          break;
        }
      }
    }
  }
}
