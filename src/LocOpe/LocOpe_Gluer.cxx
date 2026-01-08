// Created on: 1996-01-30
// Created by: Jacques GOUSSARD
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
#include <BRepExtrema_ExtPF.hxx>
#include <Extrema_ExtPS.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Surface.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec.hxx>
#include <LocOpe.hxx>
#include <LocOpe_Generator.hxx>
#include <LocOpe_GluedShape.hxx>
#include <LocOpe_Gluer.hxx>
#include <LocOpe_Spliter.hxx>
#include <LocOpe_WiresOnShape.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <StdFail_NotDone.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

static TopAbs_Orientation GetOrientation(const TopoFace&, const TopoFace&);

static Standard_Boolean Contains(const ShapeList&, const TopoShape&);

//=================================================================================================

void LocOpe_Gluer::Init(const TopoShape& Sbase, const TopoShape& Snew)
{
  mySb = Sbase;
  mySn = Snew;
  myMapEF.Clear();
  myMapEE.Clear();
  myDescF.Clear();
  myDone = Standard_False;
  myOri  = TopAbs_INTERNAL;
  myOpe  = LocOpe_INVALID;
}

//=================================================================================================

void LocOpe_Gluer::Bind(const TopoFace& Fnew, const TopoFace& Fbase)
{
  ShapeExplorer exp(mySn, TopAbs_FACE);

  for (; exp.More(); exp.Next())
  {
    if (exp.Current().IsSame(Fnew))
    {
      break;
    }
  }
  if (!exp.More())
  {
    throw Standard_ConstructionError();
  }

  TopoShape aLocalFace = Fnew.Oriented(exp.Current().Orientation());
  TopoFace  Fnor       = TopoDS::Face(aLocalFace);
  //  TopoFace Fnor = TopoDS::Face(Fnew.Oriented(exp.Current().Orientation()));

  for (exp.Init(mySb, TopAbs_FACE); exp.More(); exp.Next())
  {
    if (exp.Current().IsSame(Fbase))
    {
      break;
    }
  }
  if (!exp.More())
  {
    throw Standard_ConstructionError();
  }

  aLocalFace       = Fbase.Oriented(exp.Current().Orientation());
  TopoFace Fbor = TopoDS::Face(aLocalFace);
  //  TopoFace Fbor = TopoDS::Face(Fbase.Oriented(exp.Current().Orientation()));
  TopAbs_Orientation Ori = GetOrientation(Fnor, Fbor);

  if (myOri == TopAbs_INTERNAL)
  {
    myOri = Ori;
    if (myOri == TopAbs_REVERSED)
    {
      mySn.Reverse();
      myOpe = LocOpe_CUT;
    }
    else
    {
      myOpe = LocOpe_FUSE;
    }
  }
  else if (Ori != TopAbs_FORWARD)
  {
    myOpe = LocOpe_INVALID;
  }

  for (exp.Init(Fnor, TopAbs_EDGE); exp.More(); exp.Next())
  {
    const TopoEdge& edg = TopoDS::Edge(exp.Current());
    //    if (!myMapEF.IsBound(edg)) {
    //      myMapEF.Bind(edg,Fbor);
    if (!myMapEF.Contains(edg))
    {
      myMapEF.Add(edg, Fbor);
    }
    //    else if (!myMapEF(edg).IsSame(Fbor)) {
    else if (!myMapEF.FindFromKey(edg).IsSame(Fbor))
    {
      //      myMapEF.UnBind(edg); // edg sur 2 face. a binder avec l`edge commun
      myMapEF.ChangeFromKey(edg).Nullify();
      // edg sur 2 face. a binder avec l`edge commun
    }
  }
  //  myMapEF.Bind(Fnor,Fbor);
  myMapEF.Add(Fnor, Fbor);
}

//=================================================================================================

void LocOpe_Gluer::Bind(const TopoEdge& Enew, const TopoEdge& Ebase)
{
  if (myMapEE.IsBound(Enew) && !myMapEE(Enew).IsSame(Ebase))
  {
    throw Standard_ConstructionError();
  }
  myMapEE.Bind(Enew, Ebase);
}

//=================================================================================================

void LocOpe_Gluer::Perform()
{
  Standard_Integer ind;
  if (myDone)
  {
    return;
  }
  if (mySb.IsNull() || mySn.IsNull() || myMapEF.IsEmpty() || myOpe == LocOpe_INVALID)
  {
    throw Standard_ConstructionError();
  }

  Handle(LocOpe_WiresOnShape) theWOnS = new LocOpe_WiresOnShape(mySb);
  Handle(LocOpe_GluedShape)   theGS   = new LocOpe_GluedShape(mySn);

  Standard_Integer lmap = myMapEF.Extent();

  for (ind = 1; ind <= lmap; ind++)
  {
    TopoShape S = myMapEF.FindKey(ind);
    if (S.ShapeType() == TopAbs_EDGE)
    {
      TopoShape S2 = myMapEF(ind);
      if (!S2.IsNull())
      {
        theWOnS->Bind(TopoDS::Edge(S), TopoDS::Face(S2));
      }
    }
    else
    { // TopAbs_FACE
      theGS->GlueOnFace(TopoDS::Face(S));
    }
  }

  TopTools_DataMapIteratorOfDataMapOfShapeShape itm(myMapEE);
  for (; itm.More(); itm.Next())
  {
    theWOnS->Bind(TopoDS::Edge(itm.Key1()), TopoDS::Edge(itm.Value()));
  }

  theWOnS->BindAll();

  if (!theWOnS->IsDone())
  {
    return;
  }

  LocOpe_Spliter theSplit(mySb);
  theSplit.Perform(theWOnS);
  if (!theSplit.IsDone())
  {
    return;
  }
  // Mise a jour des descendants
  //  for (ShapeExplorer exp(mySb,TopAbs_FACE); exp.More(); exp.Next()) {
  ShapeExplorer exp(mySb, TopAbs_FACE);
  for (; exp.More(); exp.Next())
  {
    myDescF.Bind(exp.Current(), theSplit.DescendantShapes(exp.Current()));
  }

  for (exp.Init(mySn, TopAbs_FACE); exp.More(); exp.Next())
  {
    ShapeList thelist;
    myDescF.Bind(exp.Current(), thelist);
    if (Contains(theGS->OrientedFaces(), exp.Current()))
    {
      myDescF(exp.Current()).Append(exp.Current());
    }
  }

  LocOpe_Generator theGen(theSplit.ResultingShape());
  theGen.Perform(theGS);

  myDone = theGen.IsDone();
  if (myDone)
  {
    myRes = theGen.ResultingShape();

    AddEdges();

    // Mise a jour des descendants
    TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itd;
    for (itd.Initialize(myDescF); itd.More(); itd.Next())
    {
      ShapeList               newDesc;
      TopTools_ListIteratorOfListOfShape itl;
      for (itl.Initialize(itd.Value()); itl.More(); itl.Next())
      {
        TopTools_ListIteratorOfListOfShape itl2(theGen.DescendantFace(TopoDS::Face(itl.Value())));
        for (; itl2.More(); itl2.Next())
        {
          const TopoFace& descface = TopoDS::Face(itl2.Value());
          if (!descface.IsNull())
          { // sinon la face a disparu
            newDesc.Append(descface);
          }
        }
      }
      myDescF(itd.Key1()) = newDesc;
    }
  }

  // recodage des regularites
  TopTools_IndexedDataMapOfShapeListOfShape theMapEF1, theMapEF2;
  TopExp1::MapShapesAndAncestors(mySn, TopAbs_EDGE, TopAbs_FACE, theMapEF1);
  TopExp1::MapShapesAndAncestors(myRes, TopAbs_EDGE, TopAbs_FACE, theMapEF2);

  for (ind = 1; ind <= theMapEF1.Extent(); ind++)
  {
    const TopoEdge&          edg = TopoDS::Edge(theMapEF1.FindKey(ind));
    const ShapeList& LL  = theMapEF1(ind);
    if (LL.Extent() == 2)
    {
      const TopoFace& fac1    = TopoDS::Face(LL.First());
      const TopoFace& fac2    = TopoDS::Face(LL.Last());
      GeomAbs_Shape      thecont = BRepInspector::Continuity(edg, fac1, fac2);
      if (thecont >= GeomAbs_G1)
      {
        // on essaie de recoder
        Standard_Integer ind2 = theMapEF2.FindIndex(edg);
        if (ind2 != 0)
        {
          const ShapeList& LL2 = theMapEF2(ind2);
          if (LL2.Extent() == 2)
          {
            const TopoFace& ff1 = TopoDS::Face(LL2.First());
            const TopoFace& ff2 = TopoDS::Face(LL2.Last());
            if ((ff1.IsSame(fac1) && ff2.IsSame(fac2)) || (ff1.IsSame(fac2) && ff2.IsSame(fac1)))
            {
            }
            else
            {
              ShapeBuilder B;
              B.Continuity(edg, ff1, ff2, thecont);
            }
          }
        }
      }
    }
  }
  // creation de la liste d`edge
  theWOnS->InitEdgeIterator();
  while (theWOnS->MoreEdge())
  {
    TopoEdge edg = theWOnS->Edge();
    for (ind = 1; ind <= theMapEF2.Extent(); ind++)
    {
      const TopoEdge& edg1 = TopoDS::Edge(theMapEF2.FindKey(ind));
      if (edg1.IsSame(edg))
      {
        myEdges.Append(edg);
        // recodage eventuel des regularites sur cet edge
        const ShapeList& L = theMapEF2(ind);
        if (L.Extent() == 2)
        {
          const TopoFace& fac1 = TopoDS::Face(L.First());
          const TopoFace& fac2 = TopoDS::Face(L.Last());
          if (LocOpe1::TgtFaces(edg, fac1, fac2))
          {
            myTgtEdges.Append(edg);
            GeomAbs_Shape thecont = BRepInspector::Continuity(edg, fac1, fac2);
            if (thecont < GeomAbs_G1)
            {
              ShapeBuilder B;
              B.Continuity(edg, fac1, fac2, GeomAbs_G1);
            }
          }
        }
      }
    }
    theWOnS->NextEdge();
  }

  // recodage eventuel des regularites sur cet edge
}

//=================================================================================================

const ShapeList& LocOpe_Gluer::DescendantFaces(const TopoFace& F) const
{
  if (!myDone)
  {
    throw StdFail_NotDone();
  }
  if (myDescF.IsBound(F))
    return myDescF(F);
  static ShapeList nullList;
  return nullList;
}

//=================================================================================================

static TopAbs_Orientation GetOrientation(const TopoFace& Fn, const TopoFace& Fb)
{

  Handle(GeomSurface) Sn, Sb;
  Sn = BRepInspector::Surface(Fn);
  Sb = BRepInspector::Surface(Fb);

  // Find a point on Sb
  ShapeExplorer exp;
  Standard_Real   f, l;
  gp_Pnt2d        ptvtx;
  Point3d          pvt;
  Vector3d          d1u, d1v, n1, n2;

  for (exp.Init(Fn, TopAbs_EDGE); exp.More(); exp.Next())
  {
    const TopoEdge&   edg = TopoDS::Edge(exp.Current());
    Handle(GeomCurve2d) C2d = BRepInspector::CurveOnSurface(edg, Fn, f, l);
    if (Precision::IsNegativeInfinite(f) && Precision::IsPositiveInfinite(l))
    {
      f = -100.;
      l = 100.;
    }
    else if (Precision::IsNegativeInfinite(f))
    {
      f = l - 200.;
    }
    else if (Precision::IsPositiveInfinite(l))
    {
      l = f + 200.;
    }
    Standard_Real deltau = (l - f) / 20.;
    for (Standard_Integer i = 1; i <= 21; i++)
    {
      C2d->D0(f + (i - 1) * deltau, ptvtx);
      Sn->D1(ptvtx.X(), ptvtx.Y(), pvt, d1u, d1v);
      n1 = d1u.Crossed(d1v);
      if (n1.Magnitude() > Precision::Confusion())
      {
        n1.Normalize();
        if (Fn.Orientation() == TopAbs_REVERSED)
        {
          n1.Reverse();
        }

        // Projection sur Sb
        GeomAdaptor_Surface GAS(Sb);
        Standard_Real       TolU = GAS.UResolution(Precision::Confusion());
        Standard_Real       TolV = GAS.VResolution(Precision::Confusion());
        Extrema_ExtPS       dist(pvt, GAS, TolU, TolV);
        if (dist.IsDone())
        {
          Standard_Real    dist2min = RealLast();
          Standard_Integer jmin     = 0;
          for (Standard_Integer j = 1; j <= dist.NbExt(); j++)
          {
            if (dist.SquareDistance(j) < dist2min)
            {
              jmin     = j;
              dist2min = dist.SquareDistance(j);
            }
          }
          if (jmin != 0)
          {
            Standard_Real uu, vv;
            dist.Point(jmin).Parameter(uu, vv);
            Sb->D1(uu, vv, pvt, d1u, d1v);
            n2 = d1u.Crossed(d1v);
            if (n2.Magnitude() > Precision::Confusion())
            {
              n2.Normalize();
              if (Fb.Orientation() == TopAbs_REVERSED)
              {
                n2.Reverse();
              }
              if (n1.Dot(n2) > 0.)
              {
                return TopAbs_REVERSED;
              }
              return TopAbs_FORWARD;
            }
          }
        }
      }
    }
  }
  return TopAbs_INTERNAL;
}

//=================================================================================================

static Standard_Boolean Contains(const ShapeList& L, const TopoShape& S)
{
  TopTools_ListIteratorOfListOfShape it;
  for (it.Initialize(L); it.More(); it.Next())
  {
    if (it.Value().IsSame(S))
    {
      return Standard_True;
    }
  }
  return Standard_False;
}

//=================================================================================================

void LocOpe_Gluer::AddEdges()
{
  ShapeExplorer exp, expsb;
  exp.Init(mySn, TopAbs_EDGE);

  TopLoc_Location Loc;
  //  Standard_Real l, f;
  TopTools_IndexedMapOfShape MapV, MapFPrism, MapE;
  ShapeExplorer            vexp;
  Standard_Integer           flag, i;

  TopExp1::MapShapes(mySn, TopAbs_FACE, MapFPrism);

  for (expsb.Init(myRes, TopAbs_FACE); expsb.More(); expsb.Next())
  {
    if (!MapFPrism.Contains(expsb.Current()))
    {
      MapV.Clear();
      TopExp1::MapShapes(expsb.Current(), TopAbs_VERTEX, MapV);
      TopExp1::MapShapes(expsb.Current(), TopAbs_EDGE, MapE);
      for (exp.Init(mySn, TopAbs_EDGE); exp.More(); exp.Next())
      {
        TopoEdge e = TopoDS::Edge(exp.Current());
        if (MapE.Contains(e))
          continue;
        flag = 0;
        vexp.Init(e, TopAbs_VERTEX);
        for (; vexp.More(); vexp.Next())
        {
          TopoVertex v = TopoDS::Vertex(vexp.Current());
          if (MapV.Contains(v))
          {
            flag = 1;
          }
        }
        if (flag == 1)
        {

          vexp.Init(e, TopAbs_VERTEX);
          BRepExtrema_ExtPF ext;
          ext.Initialize(TopoDS::Face(expsb.Current()));
          flag = 0;
          for (; vexp.More(); vexp.Next())
          {
            TopoVertex v = TopoDS::Vertex(vexp.Current());
            if (!MapV.Contains(v))
            {
              ext.Perform(v, TopoDS::Face(expsb.Current()));
              if (!ext.IsDone() || ext.NbExt() == 0)
              {
                flag = 0;
                break;
              }
              else
              {
                Standard_Real dist2min = ext.SquareDistance(1);
                for (i = 2; i <= ext.NbExt(); i++)
                {
                  dist2min = Min(dist2min, ext.SquareDistance(i));
                }
                if (dist2min >= BRepInspector::Tolerance(v) * BRepInspector::Tolerance(v))
                {
                  flag = 0;
                  break;
                }
                else
                  flag = 1;
              }
            }
            else
              flag = 1;
          }
          if (flag == 1)
          {
          }
        }
      }
    }
  }
}
