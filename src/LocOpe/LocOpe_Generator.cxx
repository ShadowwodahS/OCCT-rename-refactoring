// Created on: 1996-01-09
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

#include <LocOpe_Generator.hxx>

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAlgo_Loop.hxx>
#include <BRepTools.hxx>
#include <ElCLib.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomProjLib.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Pln.hxx>
#include <LocOpe_BuildShape.hxx>
#include <LocOpe_GeneratedShape.hxx>
#include <Precision.hxx>
#include <Standard_NullObject.hxx>
#include <StdFail_NotDone.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

static Standard_Boolean ToFuse(const TopoFace&, const TopoFace&);

static Standard_Boolean ToFuse(const TopoEdge&, const TopoEdge&);

static Standard_Boolean ToFuse(const TopoEdge&,
                               const TopoFace&,
                               const TopoVertex&,
                               const TopTools_MapOfShape&);

static Standard_Real NewParameter(const TopoEdge&,
                                  const TopoVertex&,
                                  const TopoEdge&,
                                  const TopoVertex&);

//=================================================================================================

void LocOpe_Generator::Perform(const Handle(LocOpe_GeneratedShape)& G)
{
  if (myShape.IsNull())
  {
    throw Standard_NullObject();
  }
  myDone = Standard_False;
  myRes.Nullify();
  //  myDescFaces.Clear();
  myModShapes.Clear();
  //  myFFromE.Clear();

  const ShapeList& ledges = G->GeneratingEdges();

  // On genere une liste des faces a gauche du wire. Equivalent du LeftOf.
  // Attention : il faudra bien propager pour ne pas oublier des faces
  // a l`interieur

  ShapeExplorer     exp, exp2, exp3;
  TopTools_MapOfShape theLeft; // Faces a gauche

  TopTools_MapOfShape GEdg, GVtx; // Edges et vertex generateurs

  TopTools_ListIteratorOfListOfShape itl, itl2;

  for (itl.Initialize(ledges); itl.More(); itl.Next())
  {
    const TopoEdge& edg = TopoDS::Edge(itl.Value());

    GEdg.Add(edg);
    for (exp2.Init(edg, TopAbs_VERTEX); exp2.More(); exp2.Next())
    {
      const TopoVertex& vtx = TopoDS::Vertex(exp2.Current());
      if (!GVtx.Contains(vtx))
      {
        GVtx.Add(vtx);
      }
    }
    for (exp2.Init(myShape, TopAbs_FACE); exp2.More(); exp2.Next())
    {
      const TopoFace& fac = TopoDS::Face(exp2.Current());
      for (exp3.Init(fac, TopAbs_EDGE); exp3.More(); exp3.Next())
      {
        if (exp3.Current().IsSame(edg) && exp3.Current().Orientation() == edg.Orientation())
        {
          theLeft.Add(fac);
          ShapeList emptylist;
          if (!myModShapes.IsBound(fac))
          {
            myModShapes.Bind(fac, emptylist);
          }
          break;
        }
      }
      if (exp3.More())
      {
        break;
      }
    }
  }

  TopTools_IndexedDataMapOfShapeListOfShape theEFMap;
  TopExp1::MapShapesAndAncestors(myShape, TopAbs_EDGE, TopAbs_FACE, theEFMap);

  TopTools_DataMapOfShapeListOfShape theEEMap;
  TopTools_DataMapOfShapeListOfShape theFFMap;
  TopTools_MapOfShape                toRemove;
  TopTools_MapIteratorOfMapOfShape   itm;

  // recherche des fusions de faces
  for (itm.Initialize(GEdg); itm.More(); itm.Next())
  {
    const TopoEdge& edg = TopoDS::Edge(itm.Key1());
    if (!theEFMap.Contains(edg))
    {
      continue;
    }
    for (itl2.Initialize(theEFMap.FindFromKey(edg)); itl2.More(); itl2.Next())
    {
      if (!theLeft.Contains(itl2.Value()))
      {
        break;
      }
    }
    if (!itl2.More())
    { // edge "interne" au shell, ou bord libre
    }
    else
    {
      const TopoFace& fac    = TopoDS::Face(itl2.Value());
      TopoFace        facbis = G->Generated(edg);
      if (ToFuse(fac, facbis))
      {
        // On recherche si une face a deja fusionne avec facbis
        Standard_Boolean facbisfound = Standard_False;
        for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itf(theFFMap); itf.More();
             itf.Next())
        {
          if (itf.Key1().IsSame(fac))
          {
            continue;
          }
          for (itl.Initialize(itf.Value()); itl.More(); itl.Next())
          {
            if (itl.Value().IsSame(facbis))
            {
              facbisfound = Standard_True;
              break;
            }
          }
          if (facbisfound)
          {
            theFFMap(itf.Key1()).Append(fac);
            toRemove.Add(fac);
            toRemove.Add(edg);
            break;
          }
        }

        if (!facbisfound)
        {
          if (!theFFMap.IsBound(fac))
          {
            ShapeList thelist;
            theFFMap.Bind(fac, thelist);
          }
          for (itl.Initialize(theFFMap(fac)); itl.More(); itl.Next())
          {
            if (itl.Value().IsSame(facbis))
            {
              break;
            }
          }
          if (!itl.More())
          {
            theFFMap(fac).Append(facbis);
          }
          toRemove.Add(edg);
          toRemove.Add(facbis);
        }
      }
      else
      { // face generee par edg : on la marque.
        //	myFFromE.Bind(edg,facbis);
      }
    }
  }

  // Il faut ici ajouter dans toRemove les edges de connexites entre faces
  // a fusionner avec une meme face de base

  //  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itf(theFFMap);
  for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itf(theFFMap); itf.More(); itf.Next())
  {
    for (itl.Initialize(itf.Value()); itl.More(); itl.Next())
    {
      for (exp.Init(itl.Value(), TopAbs_EDGE); exp.More(); exp.Next())
      {
        const TopoEdge& ed = TopoDS::Edge(exp.Current());
        if (toRemove.Contains(ed))
        {
          continue;
        }
        for (itl2.Initialize(itf.Value()); itl2.More(); itl2.Next())
        {
          if (!itl2.Value().IsSame(itl.Value()))
          {
            for (exp2.Init(itl2.Value(), TopAbs_EDGE); exp2.More(); exp2.Next())
            {
              if (ed.IsSame(exp2.Current()))
              {
                toRemove.Add(ed);
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
  }

  ShapeList         RebuildFace;
  TopTools_MapOfShape          mapTreated;
  TopTools_DataMapOfShapeShape DontFuse;
  TopAbs_Orientation           orient, orface;

  for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itf(theFFMap); itf.More(); itf.Next())
  {
    const TopoFace& fac = TopoDS::Face(itf.Key1());
    for (exp.Init(fac, TopAbs_EDGE); exp.More(); exp.Next())
    {
      const TopoEdge& edg = TopoDS::Edge(exp.Current());
      if (mapTreated.Contains(edg))
      {
        continue; // on saute l`edge
      }

      mapTreated.Add(edg);
      for (exp2.Init(edg, TopAbs_VERTEX); exp2.More(); exp2.Next())
      {
        const TopoVertex& vtx = TopoDS::Vertex(exp2.Current());
        if (GVtx.Contains(vtx))
        {
          TopoEdge edgbis = G->Generated(vtx);

          if ((edgbis.IsNull() || BRepInspector::Degenerated(edgbis)) ||
              //	      toRemove.Contains(edgbis) ||
              !ToFuse(edg, edgbis))
          {
            continue;
          }
          // a voir
          if (BRepTools1::IsReallyClosed(edg, fac))
          {
            if (!theEEMap.IsBound(edg))
            {
              ShapeList thelist1;
              theEEMap.Bind(edg, thelist1);
              theEEMap(edg).Append(edgbis);
              toRemove.Add(edgbis); // toujours vrai pour edge double
              Standard_Boolean FuseEdge = Standard_True;
              TopoVertex    Vf, Vl;
              TopExp1::Vertices(edg, Vf, Vl);
              Standard_Boolean ConnectLast = (Vl.IsSame(vtx));
              for (exp3.Init(fac.Oriented(TopAbs_FORWARD), TopAbs_EDGE); exp3.More(); exp3.Next())
              {
                const TopoEdge& eee = TopoDS::Edge(exp3.Current());
                orient                 = eee.Orientation();
                if (!eee.IsSame(edg))
                {
                  TopExp1::Vertices(eee, Vf, Vl);
                  if ((Vf.IsSame(vtx) || Vl.IsSame(vtx)) && !toRemove.Contains(eee))
                  {
                    FuseEdge = Standard_False;
                    // On recherche celui qu`il ne faut pas fusionner

                    if ((Vf.IsSame(vtx) && orient == TopAbs_FORWARD)
                        || (Vl.IsSame(vtx) && orient == TopAbs_REVERSED))
                    {
                      if (ConnectLast)
                      {
                        DontFuse.Bind(edg, fac.Oriented(TopAbs_FORWARD));
                      }
                      else
                      {
                        DontFuse.Bind(edg, fac.Oriented(TopAbs_REVERSED));
                      }
                    }
                    else
                    {
                      if (ConnectLast)
                      {
                        DontFuse.Bind(edg, fac.Oriented(TopAbs_REVERSED));
                      }
                      else
                      {
                        DontFuse.Bind(edg, fac.Oriented(TopAbs_FORWARD));
                      }
                    }
                    break;
                  }
                }
              }
              if (FuseEdge)
              {
                toRemove.Add(vtx);
              }
            }
          }

          else
          {
            /*  A VOIR
                  if (!BRepInspector::IsClosed(edg,fac)) {
            */
            if (!theEEMap.IsBound(edg))
            {
              ShapeList thelist2;
              theEEMap.Bind(edg, thelist2);
            }
            theEEMap(edg).Append(edgbis);
            const ShapeList&        L = theEEMap(edg);
            TopTools_ListIteratorOfListOfShape Lit(L);
            Standard_Boolean                   OK = Standard_True;
            for (; Lit.More(); Lit.Next())
            {
              if (Lit.Value().IsSame(edgbis))
              {
                OK = Standard_False;
                break;
              }
            }
            if (OK)
              theEEMap(edg).Append(edgbis);

            itl.Initialize(theEFMap.FindFromKey(edg));
            Standard_Boolean FuseEdge = ToFuse(edg, fac, vtx, toRemove);
            if (!FuseEdge)
            {
              DontFuse.Bind(edg, fac);
            }
            else
            {
              for (; itl.More(); itl.Next())
              {
                if (!itl.Value().IsSame(fac))
                {
                  if (theFFMap.IsBound(itl.Value()))
                  {
                    FuseEdge = ToFuse(edg, TopoDS::Face(itl.Value()), vtx, toRemove);
                    // edge a fusionner
                    if (FuseEdge)
                    {
                      toRemove.Add(vtx);
                    }
                    else
                    {
                      if (toRemove.Contains(vtx))
                      {
                        toRemove.Remove(vtx);
                      }
                      DontFuse.Bind(edg, itl.Value());
                    }
                  }
                  else
                  { // on marque comme face a reconstruire
                    RebuildFace.Append(itl.Value());
                    if (toRemove.Contains(vtx))
                    {
                      toRemove.Remove(vtx);
                    }
                    DontFuse.Bind(edg, itl.Value());
                  }

                  break;
                }
              }
            }
          }
        }
      }
    }
  }

  for (itl.Initialize(RebuildFace); itl.More(); itl.Next())
  {
    ShapeList thelist3;
    theFFMap.Bind(itl.Value(), thelist3);
  }

  ShapeBuilder    B;
  TopoFace     newface;
  TopoWire     outw, newwire;
  TopoEdge     newedg;
  TopoVertex   newvtx;
  TopLoc_Location loc;
  Standard_Real   tol, prm, f, l, Uminc = 0., Umaxc = 0.;
  gp_Pnt2d        pf, pl;

  Handle(GeomSurface) S;
  Handle(GeomPlane)   P;
  Handle(GeomCurve3d)   C;

  // Fusion des edges
  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape ite(theEEMap);
  for (; ite.More(); ite.Next())
  {
    Standard_Boolean   KeepNewEdge = Standard_False;
    const TopoEdge& edg         = TopoDS::Edge(ite.Key1());
    BRepInspector::Range(edg, f, l);
    TopoShape aLocalEdge = edg.EmptyCopied();
    newedg                  = TopoDS::Edge(aLocalEdge);
    //    newedg = TopoDS::Edge(edg.EmptyCopied());
    newedg.Orientation(TopAbs_FORWARD);
    for (exp.Init(edg.Oriented(TopAbs_FORWARD), TopAbs_VERTEX); exp.More(); exp.Next())
    {
      const TopoVertex& vtx = TopoDS::Vertex(exp.Current());
      prm                      = BRepInspector::Parameter(vtx, edg);

      newvtx.Nullify();
      for (itl.Initialize(theEEMap(edg)); itl.More(); itl.Next())
      {
        const TopoEdge& edgbis = TopoDS::Edge(itl.Value());
        for (exp2.Init(edgbis, TopAbs_VERTEX); exp2.More(); exp2.Next())
        {
          if (exp2.Current().IsSame(vtx))
          {
            break;
          }
        }
        if (exp2.More())
        {
          for (exp2.ReInit(); exp2.More(); exp2.Next())
          {
            if (!exp2.Current().IsSame(vtx))
            {
              newvtx = TopoDS::Vertex(exp2.Current());
              prm    = NewParameter(edg, vtx, newedg, newvtx);
              break;
            }
          }
          break;
        }
      }

      if (toRemove.Contains(vtx) || (prm < l && prm > f))
      {
        B.Add(newedg, newvtx.Oriented(vtx.Orientation()));
        tol = BRepInspector::Tolerance(newvtx);
        B.UpdateVertex(newvtx, prm, newedg, tol);
        toRemove.Add(itl.Value()); // i-e edgbis
        KeepNewEdge = Standard_True;
      }
      else
      {
        B.Add(newedg, vtx.Oriented(vtx.Orientation()));
        tol = BRepInspector::Tolerance(vtx);
        B.UpdateVertex(vtx, prm, newedg, tol);
      }
    }
    if (KeepNewEdge)
    {
      ShapeList emptylist;
      if (!myModShapes.IsBound(edg))
      {
        myModShapes.Bind(edg, emptylist);
      }
      myModShapes(edg).Append(newedg);
      toRemove.Add(edg);
    }
  }

  TopTools_MapOfShape EdgAdded;

  // Fusion des faces, ou reconstruction
  for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itf(theFFMap); itf.More(); itf.Next())
  {
    const TopoFace&   fac     = TopoDS::Face(itf.Key1());
    Standard_Boolean     ModFace = Standard_False;
    ShapeList listofedg;

    EdgAdded.Clear();

    for (exp.Init(myShape, TopAbs_FACE); exp.More(); exp.Next())
    {
      if (exp.Current().IsSame(fac))
      {
        break;
      }
    }
    orface                             = exp.Current().Orientation();
    TopoShape aLocalFaceEmptyCopied = fac.EmptyCopied();
    newface                            = TopoDS::Face(aLocalFaceEmptyCopied);
    //    newface = TopoDS::Face(fac.EmptyCopied());
    newface.Orientation(TopAbs_FORWARD);
    S = BRepInspector::Surface(fac);
    if (S->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
    {
      S = Handle(Geom_RectangularTrimmedSurface)::DownCast(S)->BasisSurface();
    }
    P = Handle(GeomPlane)::DownCast(S);
    TopoWire wir;
    for (exp.Init(fac.Oriented(TopAbs_FORWARD), TopAbs_WIRE); exp.More(); exp.Next())
    {
      wir = TopoDS::Wire(exp.Current());
      for (exp2.Init(wir, TopAbs_EDGE); exp2.More(); exp2.Next())
      {
        const TopoEdge& edg = TopoDS::Edge(exp2.Current());
        if (toRemove.Contains(edg) || myModShapes.IsBound(edg))
        {
          break;
        }
      }
      if (!exp2.More())
      { // wire non modifie
        //	B.Add(newface,wir.Oriented(wir.Orientation()));
        for (exp2.Init(wir, TopAbs_EDGE); exp2.More(); exp2.Next())
        {
          listofedg.Append(exp2.Current());
        }
      }
      else
      {
        if (!ModFace)
        {

          // Petit truc crad pour les p-curves sur les cylindres...
          if (P.IsNull())
          {
            Standard_Real Vminc, Vmaxc;
            BRepTools1::UVBounds(fac, Uminc, Umaxc, Vminc, Vmaxc);
          }

          // premier passage : on met les wires non touches des faces
          // en vis a vis

          for (itl.Initialize(itf.Value()); itl.More(); itl.Next())
          {
            TopoFace facbis = TopoDS::Face(itl.Value());
            for (itl2.Initialize(G->OrientedFaces()); itl2.More(); itl2.Next())
            {
              if (itl2.Value().IsSame(facbis))
              {
                break;
              }
            }
            if (itl2.More())
            {
              orient = itl2.Value().Orientation();
              facbis.Orientation(orient);
            }
            else
            { // on fusionne avec une autre face du shape...
              for (exp2.Init(myShape, TopAbs_FACE); exp2.More(); exp2.Next())
              {
                if (exp2.Current().IsSame(facbis))
                {
                  facbis.Orientation(exp2.Current().Orientation());
                  break;
                }
              }
            }

            for (exp3.Init(facbis, TopAbs_WIRE); exp3.More(); exp3.Next())
            {
              for (exp2.Init(exp3.Current(), TopAbs_EDGE); exp2.More(); exp2.Next())
              {
                if (toRemove.Contains(exp2.Current()))
                {
                  break;
                }
              }
              if (!exp2.More())
              {
                TopoWire theNew;
                B.MakeWire(theNew); // FORWARD
                for (exp2.ReInit(); exp2.More(); exp2.Next())
                {
                  const TopoEdge& edg = TopoDS::Edge(exp2.Current());

                  orient = TopAbs1::Compose(orface, edg.Orientation());
                  //		  B.Add(theNew,edg.Oriented(or));
                  listofedg.Append(edg.Oriented(orient));
                  EdgAdded.Add(edg);
                  if (P.IsNull())
                  {
                    // on met les courbes 2d si on n`est pas sur un plan
                    // on ne devrait pas avoir de pb d`edge de couture.
                    tol = BRepInspector::Tolerance(edg);
                    C   = BRepInspector::Curve(edg, loc, f, l);
                    if (!loc.IsIdentity())
                    {
                      Handle(Geom_Geometry) GG = C->Transformed(loc.Transformation());
                      C                        = Handle(GeomCurve3d)::DownCast(GG);
                    }
                    if (C->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve))
                    {
                      C = Handle(Geom_TrimmedCurve)::DownCast(C)->BasisCurve();
                    }

                    Handle(GeomCurve2d) C2d = GeomProjLib1::Curve2d(C, f, l, S, tol);

                    // Tentative de recalage dans la facette
                    pf                            = C2d->Value(f);
                    pl                            = C2d->Value(l);
                    constexpr Standard_Real tttol = Precision1::Angular();
                    while (Min(pf.X(), pl.X()) >= Umaxc - tttol)
                    {
                      C2d->Translate(gp_Vec2d(-2. * M_PI, 0));
                      pf = C2d->Value(f);
                      pl = C2d->Value(l);
                    }

                    while (Max(pf.X(), pl.X()) <= Uminc + tttol)
                    {
                      C2d->Translate(gp_Vec2d(2. * M_PI, 0));
                      pf = C2d->Value(f);
                      pl = C2d->Value(l);
                    }

                    if (!BRepTools1::IsReallyClosed(edg, facbis))
                    {
                      B.UpdateEdge(edg, C2d, newface, tol);
                    }
                    //		else {
                    //		  std::cout << "Edge double bizarre... " << std::endl;
                    //		}
                  }
                }
                //		B.Add(newface,theNew);
              }
            }
          }
          ModFace = Standard_True;
        }

        // reconstruction du wire
        // B.MakeWire(newwire);

        Handle(GeomCurve2d) C2d, C2d1;

        //	for (exp2.Init(wir.Oriented(TopAbs_FORWARD),TopAbs_EDGE);
        for (exp2.Init(wir, TopAbs_EDGE); exp2.More(); exp2.Next())
        {
          const TopoEdge& edg = TopoDS::Edge(exp2.Current());
          orient                 = edg.Orientation();
          if (!toRemove.Contains(edg) && !theEEMap.IsBound(edg))
          {
            //	    B.Add(newwire,edg.Oriented(or));
            //            listofedg.Append(edg.Oriented(or));
            listofedg.Append(edg);
          }
          else if (myModShapes.IsBound(edg) || theEEMap.IsBound(edg))
          {
            if (myModShapes.IsBound(edg))
            {
              newedg = TopoDS::Edge(myModShapes(edg).First());
            }
            else
            {
              newedg = edg;
            }
            //	    B.Add(newwire,newedg.Oriented(or));
            listofedg.Append(newedg.Oriented(orient));
            C = BRepInspector::Curve(newedg, loc, f, l);
            if (!loc.IsIdentity())
            {
              Handle(Geom_Geometry) GG = C->Transformed(loc.Transformation());
              C                        = Handle(GeomCurve3d)::DownCast(GG);
            }
            if (C->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve))
            {
              C = Handle(Geom_TrimmedCurve)::DownCast(C)->BasisCurve();
            }
            if (P.IsNull())
            { // on met les courbes 2d si on n`est pas
              // sur un plan
              //	      TopAbs_Orientation oredonfafw = TopAbs1::Compose(or,orsav);
              TopAbs_Orientation oredonfafw = orient;
              tol                           = BRepInspector::Tolerance(newedg);
              TopoShape aLocalEdge       = edg.Oriented(oredonfafw);
              TopoShape aLocalFace       = fac.Oriented(TopAbs_FORWARD);
              C2d =
                BRepInspector::CurveOnSurface(TopoDS::Edge(aLocalEdge), TopoDS::Face(aLocalFace), f, l);
              //	      C2d = BRepInspector::CurveOnSurface
              //		(TopoDS::Edge(edg.Oriented(oredonfafw)),
              //		 TopoDS::Face(fac.Oriented(TopAbs_FORWARD)),
              //		 f,l);

              if (!BRepTools1::IsReallyClosed(edg, fac))
              {
                B.UpdateEdge(newedg, C2d, newface, tol);
              }
              else if (C2d1.IsNull())
              {
                C2d1 = C2d;
              }
              else
              {
                //		if (TopAbs1::Compose(orsav,or) == TopAbs_FORWARD) {
                if (orient == TopAbs_FORWARD)
                {
                  B.UpdateEdge(newedg, C2d, C2d1, newface, tol);
                }
                else
                {
                  B.UpdateEdge(newedg, C2d1, C2d, newface, tol);
                }
              }
            }
            Standard_Boolean AddPart = Standard_False;
            if (DontFuse.IsBound(edg))
            {
              if (!BRepTools1::IsReallyClosed(edg, fac))
              {
                if (DontFuse(edg).IsSame(fac))
                {
                  if (myModShapes.IsBound(edg))
                  {
                    AddPart = Standard_True;
                  }
                }
                else if (!toRemove.Contains(edg))
                {
                  AddPart = Standard_True;
                }
              }
              else
              {
                if (myModShapes.IsBound(edg))
                { // edg raccourci
                  //		  if (TopAbs1::Compose(orsav,or)==DontFuse(edg).Orientation()){
                  if (orient == DontFuse(edg).Orientation())
                  {
                    AddPart = Standard_True;
                  }
                }
                else
                {
                  //		  if (TopAbs1::Compose(orsav,or) ==
                  if (orient == TopAbs1::Reverse(DontFuse(edg).Orientation()))
                  {
                    AddPart = Standard_True;
                  }
                }
              }
            }
            if (AddPart)
            {
              itl2.Initialize(theEEMap(edg));
              Vector3d dir1, dir2;
              for (; itl2.More(); itl2.Next())
              {
                if (EdgAdded.Contains(itl2.Value()))
                {
                  continue;
                }
                const TopoEdge& edgbis = TopoDS::Edge(itl2.Value());
                TopoDS_Iterator    it1(newedg), it2;
                for (; it1.More(); it1.Next())
                {
                  for (it2.Initialize(edgbis); it2.More(); it2.Next())
                  {
                    if (it1.Value().IsSame(it2.Value()))
                    {
                      break;
                    }
                  }
                  if (it2.More())
                  {
                    break;
                  }
                }

                if (it1.More())
                {
                  Point3d        ptbid;
                  Standard_Real prmvt = BRepInspector::Parameter(TopoDS::Vertex(it1.Value()), newedg);
                  C->D1(prmvt, ptbid, dir1);

                  C = BRepInspector::Curve(edgbis, loc, f, l);
                  if (!loc.IsIdentity())
                  {
                    Handle(Geom_Geometry) GG = C->Transformed(loc.Transformation());
                    C                        = Handle(GeomCurve3d)::DownCast(GG);
                  }
                  if (C->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve))
                  {
                    C = Handle(Geom_TrimmedCurve)::DownCast(C)->BasisCurve();
                  }
                  prmvt = BRepInspector::Parameter(TopoDS::Vertex(it1.Value()), edgbis);
                  C->D1(prmvt, ptbid, dir2);
                }
                else
                {
                  dir1 = dir2 = Vector3d(1, 0, 0);
                }
                EdgAdded.Add(edgbis);
                if (dir1.Dot(dir2) < 0.)
                {
                  //		  B.Add(newwire,edgbis.Oriented(TopAbs1::Reverse(or)));
                  listofedg.Append(edgbis.Oriented(TopAbs1::Reverse(orient)));
                }
                else
                {
                  //		  B.Add(newwire,edgbis.Oriented(or));
                  listofedg.Append(edgbis.Oriented(orient));
                }

                if (P.IsNull())
                {
                  // on met les courbes 2d si on n`est pas sur un plan
                  // C est la courbe de edgbis, f et l s`y rapportent
                  Handle(GeomCurve2d) PTC = GeomProjLib1::Curve2d(C, f, l, S, tol);
                  if (S->IsUPeriodic())
                  {
                    Standard_Real Uref;
                    if (DontFuse.IsBound(edg))
                    {
                      TopAbs_Orientation oredge = DontFuse(edg).Orientation();
                      if (myModShapes.IsBound(edg))
                      { // edge raccourci...
                        TopoShape aLocalShape = edg.Oriented(oredge);
                        TopoShape aLocalFace  = fac.Oriented(TopAbs_FORWARD);
                        BRepInspector::UVPoints(TopoDS::Edge(aLocalShape),
                                            TopoDS::Face(aLocalFace),
                                            pf,
                                            pl);
                        //			BRepInspector::
                        //			UVPoints(TopoDS::Edge(edg.Oriented(oredge)),
                        //				 TopoDS::Face(fac.Oriented(TopAbs_FORWARD)),
                        //				 pf,pl);
                      }
                      else
                      {
                        TopoShape aLocalShape = edg.Oriented(TopAbs1::Reverse(oredge));
                        TopoShape aLocalFace  = fac.Oriented(TopAbs_FORWARD);
                        BRepInspector::UVPoints(TopoDS::Edge(aLocalShape),
                                            TopoDS::Face(aLocalFace),
                                            pf,
                                            pl);
                        //			BRepInspector::
                        //			UVPoints(TopoDS::Edge(edg.Oriented(TopAbs1::Reverse(oredge))),
                        //				 TopoDS::Face(fac.Oriented(TopAbs_FORWARD)),
                        //				 pf,pl);
                      }
                      Uref = pf.X();
                    }
                    else
                    {
                      BRepInspector::UVPoints(edg, fac, pf, pl);
                      Uref = pf.X();
                    }

                    Standard_Real NewU = (PTC->Value(f)).X();

                    //		    if(abs(NewU - Uref) > Epsilon(S->UPeriod()))   {
                    if (fabs(NewU - Uref) > Epsilon(S->UPeriod()))
                    {
                      PTC->Translate(gp_Vec2d((Uref - NewU), 0.));
                    }
                  }

                  B.UpdateEdge(edgbis, PTC, newface, tol);
                }
              }
            }
          }
        }

        // Recuperation des edges sur les faces a fusionner.

        // ICICICICI

        //	orface = TopAbs1::Compose(orsav,orface);

        Standard_Boolean includeinw = Standard_False;
        for (itl.Initialize(itf.Value()); itl.More(); itl.Next())
        {
          TopoFace      facbis  = TopoDS::Face(itl.Value());
          Standard_Boolean genface = Standard_True;
          for (itl2.Initialize(G->OrientedFaces()); itl2.More(); itl2.Next())
          {
            if (itl2.Value().IsSame(facbis))
            {
              break;
            }
          }
          if (itl2.More())
          {
            orient = itl2.Value().Orientation();
            facbis.Orientation(orient);
          }
          else
          { // on fusionne avec une autre face du shape...
            genface = Standard_False;
            for (exp2.Init(myShape, TopAbs_FACE); exp2.More(); exp2.Next())
            {
              if (exp2.Current().IsSame(facbis))
              {
                facbis.Orientation(exp2.Current().Orientation());
                break;
              }
            }
          }

          for (exp3.Init(facbis, TopAbs_WIRE); exp3.More(); exp3.Next())
          {
            for (exp2.Init(exp3.Current(), TopAbs_EDGE); exp2.More(); exp2.Next())
            {
              if (toRemove.Contains(exp2.Current()))
              {
                break;
              }
            }
            if (genface)
            {
              includeinw = Standard_True;
            }
            else
            {
              Standard_Boolean isIncludedInW = Standard_False;
              if (exp2.More())
              {
                for (exp2.ReInit(); exp2.More(); exp2.Next())
                {
                  if (!toRemove.Contains(exp2.Current()))
                  {
                    continue;
                  }
                  TopoVertex VF, VL;
                  TopExp1::Vertices(TopoDS::Edge(exp2.Current()), VF, VL);
                  ShapeExplorer exp4;
                  for (exp4.Init(wir, TopAbs_VERTEX);
                       //		  for (ShapeExplorer exp4(wir,TopAbs_VERTEX);
                       exp4.More();
                       exp4.Next())
                  {
                    if (exp4.Current().IsSame(VF) || exp4.Current().IsSame(VL))
                    {
                      isIncludedInW = Standard_True;
                      break;
                    }
                  }
                  if (isIncludedInW)
                  {
                    break;
                  }
                }
              }
            }

            if (!includeinw)
            {
              continue;
            }

            for (exp2.ReInit(); exp2.More(); exp2.Next())
            {
              const TopoEdge& edg = TopoDS::Edge(exp2.Current());
              if (!toRemove.Contains(edg) && !EdgAdded.Contains(edg))
              {

                orient = TopAbs1::Compose(orface, edg.Orientation());
                //		B.Add(newwire,edg.Oriented(or));
                listofedg.Append(edg.Oriented(orient));
                EdgAdded.Add(edg);
                if (P.IsNull())
                {
                  // on met les courbes 2d si on n`est pas sur un plan
                  // on ne devrait pas avoir de pb d`edge de couture.
                  tol = BRepInspector::Tolerance(edg);
                  C   = BRepInspector::Curve(edg, loc, f, l);
                  if (!loc.IsIdentity())
                  {
                    Handle(Geom_Geometry) GG = C->Transformed(loc.Transformation());
                    C                        = Handle(GeomCurve3d)::DownCast(GG);
                  }
                  if (C->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve))
                  {
                    C = Handle(Geom_TrimmedCurve)::DownCast(C)->BasisCurve();
                  }

                  C2d = GeomProjLib1::Curve2d(C, f, l, S, tol);

                  // Tentative de recalage dans la facette
                  pf                            = C2d->Value(f);
                  pl                            = C2d->Value(l);
                  constexpr Standard_Real tttol = Precision1::Angular();
                  while (Min(pf.X(), pl.X()) >= Umaxc - tttol)
                  {
                    C2d->Translate(gp_Vec2d(-2. * M_PI, 0));
                    pf = C2d->Value(f);
                    pl = C2d->Value(l);
                  }

                  while (Max(pf.X(), pl.X()) <= Uminc + tttol)
                  {
                    C2d->Translate(gp_Vec2d(2. * M_PI, 0));
                    pf = C2d->Value(f);
                    pl = C2d->Value(l);
                  }

                  if (!BRepTools1::IsReallyClosed(edg, facbis))
                  {
                    B.UpdateEdge(edg, C2d, newface, tol);
                  }
                  //		else {
                  //		  std::cout << "Edge double bizarre... " << std::endl;
                  //		}
                }
              }
            }
          }
        }
      }
    }
    if (!listofedg.IsEmpty())
    {
      BRepAlgo_Loop L;
      L.Init(newface);
      L.AddConstEdges(listofedg);
      L.Perform();
      L.WiresToFaces();
      const ShapeList& listoffaces = L.NewFaces();
      toRemove.Add(fac);
      //      if (!HasWire) {
      //	newface.Nullify();
      //      }
      myModShapes.Bind(fac, listoffaces);
      for (itl.Initialize(itf.Value()); itl.More(); itl.Next())
      {
        myModShapes.Bind(itl.Value(), listoffaces);
      }
    }
  }

  /* JAG 16.09.96 : on utilise LocOpe_BuildShape
    TopoShell theShell;
    B.MakeShell(theShell);
    for (exp.Init(myShape,TopAbs_FACE); exp.More(); exp.Next()) {
      const TopoFace& fac = TopoDS::Face(exp.Current());
      if (!theLeft.Contains(fac)) {
        if (!toRemove.Contains(exp.Current())) {
      B.Add(theShell,fac);
      myModShapes.Bind(fac,fac);
        }
        else if (!myModShapes(fac).IsNull())  {
      B.Add(theShell, myModShapes(fac).Oriented(fac.Orientation()));
        }
      }
    }


    TopAbs_Orientation orsolid = myShape.Orientation();
    for (itl.Initialize(G->OrientedFaces()); itl.More(); itl.Next()) {
      const TopoFace& fac = TopoDS::Face(itl.Value());
      if (toRemove.Contains(fac)) {
        continue;
      }

      if (orsolid == TopAbs_FORWARD) {
        B.Add(theShell,fac);
      }
      else {
        B.Add(theShell,fac.Reversed());
      }
      myModShapes.Bind(fac,fac);

    }

    B.MakeSolid(TopoDS::Solid(myRes));
    B.Add(myRes,theShell);
    myRes.Orientation(orsolid);

  */

  // 06.11.96
  // Debug temporaire. Il faudra prevoir un syntaxe de BuildShape
  // qui impose une ori de certaines faces.

  TopoFace FaceRefOri;

  ShapeList lfres;
  for (exp.Init(myShape, TopAbs_FACE); exp.More(); exp.Next())
  {
    const TopoFace& fac = TopoDS::Face(exp.Current());
    if (!theLeft.Contains(fac))
    {
      if (!toRemove.Contains(exp.Current()))
      {
        lfres.Append(fac);
        if (!myModShapes.IsBound(fac))
        {
          ShapeList emptylist;
          myModShapes.Bind(fac, emptylist);
        }
        myModShapes(fac).Append(fac);
        if (FaceRefOri.IsNull())
        {
          FaceRefOri = fac;
        }
      }
      else if (myModShapes.IsBound(fac))
      {
        lfres.Append(myModShapes(fac).First().Oriented(fac.Orientation()));
      }
    }
  }

  TopAbs_Orientation orsolid = myShape.Orientation();
  for (itl.Initialize(G->OrientedFaces()); itl.More(); itl.Next())
  {
    const TopoFace& fac = TopoDS::Face(itl.Value());
    if (toRemove.Contains(fac))
    {
      continue;
    }

    if (orsolid == TopAbs_FORWARD)
    {
      lfres.Append(fac);
    }
    else
    {
      lfres.Append(fac.Reversed());
    }
    if (!myModShapes.IsBound(fac))
    {
      ShapeList emptylist;
      myModShapes.Bind(fac, emptylist);
    }
    myModShapes(fac).Append(fac);
  }

  LocOpe_BuildShape BS(lfres);
  myRes = BS.Shape();
  // Suite debug du 06.11.96
  if (myRes.ShapeType() == TopAbs_SOLID)
  {
    for (exp.Init(myRes, TopAbs_FACE); exp.More(); exp.Next())
    {
      if (exp.Current().IsSame(FaceRefOri))
      {
        break;
      }
    }
    if (exp.More() && exp.Current().Orientation() != FaceRefOri.Orientation())
    {
      // 	---C++: return const&	---C++: return const&	---C++: return const&Si un seul Shell ,
      // on change son orientation
      TopoSolid NewSol;
      B.MakeSolid(NewSol);
      exp.Init(myRes, TopAbs_SHELL);
      B.Add(NewSol, exp.Current().Reversed());
      myRes.Nullify();
      myRes = NewSol;
    }
  }

  // recodage des regularites qui existaient sur le shape colle

  myDone = Standard_True;
}

//=================================================================================================

const ShapeList& LocOpe_Generator::DescendantFace(const TopoFace& F)
{
  // ShapeList list;

  if (!myDone)
  {
    throw StdFail_NotDone();
  }
  return myModShapes(F);
}

//=================================================================================================

Standard_Boolean ToFuse(const TopoFace& F1, const TopoFace& F2)
{
  if (F1.IsNull() || F2.IsNull())
  {
    return Standard_False;
  }

  Handle(GeomSurface)    S1, S2;
  TopLoc_Location         loc1, loc2;
  Handle(TypeInfo)   typS1, typS2;
  constexpr Standard_Real tollin = Precision1::Confusion();
  constexpr Standard_Real tolang = Precision1::Angular();

  S1 = BRepInspector::Surface(F1, loc1);
  S2 = BRepInspector::Surface(F2, loc2);

  typS1 = S1->DynamicType();
  typS2 = S2->DynamicType();

  if (typS1 == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
  {
    S1    = Handle(Geom_RectangularTrimmedSurface)::DownCast(S1)->BasisSurface();
    typS1 = S1->DynamicType();
  }

  if (typS2 == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
  {
    S2    = Handle(Geom_RectangularTrimmedSurface)::DownCast(S2)->BasisSurface();
    typS2 = S2->DynamicType();
  }

  if (typS1 != typS2)
  {
    return Standard_False;
  }

  Standard_Boolean ValRet = Standard_False;
  if (typS1 == STANDARD_TYPE(GeomPlane))
  {

    gp_Pln pl1(Handle(GeomPlane)::DownCast(S1)->Pln());
    gp_Pln pl2(Handle(GeomPlane)::DownCast(S2)->Pln());

    pl1.Transform(loc1.Transformation());
    pl2.Transform(loc2.Transformation());

    if (pl1.Position1().IsCoplanar(pl2.Position1(), tollin, tolang))
    {
      ValRet = Standard_True;
    }
  }

  return ValRet;
}

//=================================================================================================

Standard_Boolean ToFuse(const TopoEdge& E1, const TopoEdge& E2)
{

  if (E1.IsNull() || E2.IsNull())
  {
    return Standard_False;
  }

  Handle(GeomCurve3d)      C1, C2;
  TopLoc_Location         loc1, loc2;
  Handle(TypeInfo)   typC1, typC2;
  constexpr Standard_Real tollin = Precision1::Confusion();
  constexpr Standard_Real tolang = Precision1::Angular();
  Standard_Real           f, l;

  C1 = BRepInspector::Curve(E1, loc1, f, l);
  if (!loc1.IsIdentity())
  {
    Handle(Geom_Geometry) CC1 = C1->Transformed(loc1.Transformation());
    C1                        = Handle(GeomCurve3d)::DownCast(CC1);
  }

  C2 = BRepInspector::Curve(E2, loc2, f, l);
  if (!loc2.IsIdentity())
  {
    Handle(Geom_Geometry) CC2 = C2->Transformed(loc2.Transformation());
    C2                        = Handle(GeomCurve3d)::DownCast(CC2);
  }

  typC1 = C1->DynamicType();
  typC2 = C2->DynamicType();

  if (typC1 == STANDARD_TYPE(Geom_TrimmedCurve))
  {
    C1    = Handle(Geom_TrimmedCurve)::DownCast(C1)->BasisCurve();
    typC1 = C1->DynamicType();
  }
  if (typC2 == STANDARD_TYPE(Geom_TrimmedCurve))
  {
    C2    = Handle(Geom_TrimmedCurve)::DownCast(C2)->BasisCurve();
    typC2 = C2->DynamicType();
  }

  if (typC1 != typC2)
  {
    return Standard_False;
  }

  Standard_Boolean ValRet = Standard_False;
  if (typC1 == STANDARD_TYPE(GeomLine))
  {
    gp_Lin li1(Handle(GeomLine)::DownCast(C1)->Lin());
    gp_Lin li2(Handle(GeomLine)::DownCast(C2)->Lin());

    if (li1.Position1().IsCoaxial(li2.Position1(), tolang, tollin))
    {
      ValRet = Standard_True;
    }
  }

  return ValRet;
}

//=================================================================================================

Standard_Boolean ToFuse(const TopoEdge&         E,
                        const TopoFace&         F,
                        const TopoVertex&       V,
                        const TopTools_MapOfShape& toRemove)
{
  TopoVertex   Vf, Vl;
  ShapeExplorer exp;
  for (exp.Init(F, TopAbs_EDGE); exp.More(); exp.Next())
  {
    //  for (ShapeExplorer exp(F,TopAbs_EDGE); exp.More(); exp.Next()) {
    const TopoEdge& eee = TopoDS::Edge(exp.Current());
    if (!eee.IsSame(E))
    {
      TopExp1::Vertices(eee, Vf, Vl);
      if ((Vf.IsSame(V) || Vl.IsSame(V)) && !toRemove.Contains(eee))
      {
        return Standard_False;
      }
    }
  }
  return Standard_True;
}

//=================================================================================================

Standard_Real NewParameter(const TopoEdge&   Edg,
                           const TopoVertex& Vtx,
                           const TopoEdge&   NewEdg,
                           const TopoVertex& NewVtx)
{

  Handle(GeomCurve3d)    C;
  TopLoc_Location       loc;
  Handle(TypeInfo) typC;
  Standard_Real         f, l;

  Point3d P = BRepInspector::Pnt(NewVtx);

  C = BRepInspector::Curve(Edg, loc, f, l);
  if (!loc.IsIdentity())
  {
    Handle(Geom_Geometry) GG = C->Transformed(loc.Transformation());
    C                        = Handle(GeomCurve3d)::DownCast(GG);
  }
  typC = C->DynamicType();
  if (typC == STANDARD_TYPE(Geom_TrimmedCurve))
  {
    C    = Handle(Geom_TrimmedCurve)::DownCast(C)->BasisCurve();
    typC = C->DynamicType();
  }

  if (typC == STANDARD_TYPE(GeomLine))
  {
    return ElCLib1::Parameter(Handle(GeomLine)::DownCast(C)->Lin(), P);
  }
  else if (typC == STANDARD_TYPE(GeomCircle))
  {
    Standard_Real prm = ElCLib1::Parameter(Handle(GeomCircle)::DownCast(C)->Circ(), P);
    // Vtx vient d`une exploration de Edg orientee FORWARD

    TopAbs_Orientation orient = TopAbs1::Reverse(Vtx.Orientation());
    if (orient == TopAbs_FORWARD || orient == TopAbs_REVERSED)
    {
      //      for (ShapeExplorer exp(NewEdg.Oriented(TopAbs_FORWARD),TopAbs_VERTEX);
      ShapeExplorer exp(NewEdg.Oriented(TopAbs_FORWARD), TopAbs_VERTEX);
      for (; exp.More(); exp.Next())
      {
        if (exp.Current().Orientation() == orient)
        {
          break;
        }
      }
      if (exp.More())
      {
        Standard_Real prmmax = BRepInspector::Parameter(TopoDS::Vertex(exp.Current()), NewEdg);
        if (Abs(prmmax - prm) <= Epsilon(2. * M_PI))
        {
          if (orient == TopAbs_REVERSED)
          {
            prm -= 2. * M_PI;
          }
          else
          {
            prm += 2. * M_PI;
          }
        }
      }
    }
    return prm;
  }
  return 0;
}
