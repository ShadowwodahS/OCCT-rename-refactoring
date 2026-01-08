// Created on: 1996-10-02
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

#include <LocOpe_SplitDrafts.hxx>

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <BRepTools_Substitution.hxx>
#include <Extrema_ExtPC.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomFill_Pipe.hxx>
#include <GeomInt_IntSS.hxx>
#include <gp.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec2d.hxx>
#include <GProp_GProps.hxx>
#include <IntAna_QuadQuadGeo.hxx>
#include <IntCurveSurface_HInter.hxx>
#include <IntCurveSurface_IntersectionPoint.hxx>
#include <IntCurveSurface_IntersectionSegment.hxx>
#include <LocOpe_BuildShape.hxx>
#include <LocOpe_Spliter.hxx>
#include <LocOpe_SplitShape.hxx>
#include <LocOpe_WiresOnShape.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_NullObject.hxx>
#include <StdFail_NotDone.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

static Standard_Boolean NewPlane(const TopoFace&,
                                 const Dir3d&,
                                 const gp_Pln&,
                                 const Standard_Real,
                                 gp_Pln&,
                                 Axis3d&,
                                 const Standard_Boolean);

static void MakeFace(TopoFace&, ShapeList&);

static TopoEdge NewEdge(const TopoEdge&,
                           const TopoFace&,
                           const Handle(GeomSurface)&,
                           const TopoVertex&,
                           const TopoVertex&);

static Standard_Boolean Contains(const ShapeList&, const TopoShape&);

//=================================================================================================

void LocOpe_SplitDrafts::Init(const TopoShape& S)
{
  myShape = S;
  myResult.Nullify();
  myMap.Clear();
}

//=================================================================================================

void LocOpe_SplitDrafts::Perform(const TopoFace&  F,
                                 const TopoWire&  W,
                                 const Dir3d&       Extr,
                                 const gp_Pln&       NPl,
                                 const Standard_Real Angle)
{
  Perform(F, W, Extr, NPl, Angle, Extr, NPl, Angle, Standard_True, Standard_False);
}

//=================================================================================================

void LocOpe_SplitDrafts::Perform(const TopoFace&     F,
                                 const TopoWire&     W,
                                 const Dir3d&          Extrg,
                                 const gp_Pln&          NPlg,
                                 const Standard_Real    Angleg,
                                 const Dir3d&          Extrd,
                                 const gp_Pln&          NPld,
                                 const Standard_Real    Angled,
                                 const Standard_Boolean ModLeft,
                                 const Standard_Boolean ModRight)

{
  Standard_Integer j;

  myResult.Nullify();
  myMap.Clear();
  if (myShape.IsNull() || F.IsNull() || W.IsNull())
  {
    throw Standard_NullObject();
  }

  if (!ModLeft && !ModRight)
  {
    throw Standard_ConstructionError();
  }

  TopAbs_Orientation OriF = TopAbs_FORWARD;

  Standard_Boolean FinS = Standard_False;
  ShapeExplorer  exp, exp2;
  for (exp.Init(myShape, TopAbs_FACE); exp.More(); exp.Next())
  {
    const TopoShape&  fac = exp.Current();
    ShapeList thelist;
    myMap.Bind(fac, thelist);
    if (fac.IsSame(F))
    {
      OriF = fac.Orientation();
      FinS = Standard_True;
    }
  }

  if (!FinS)
  {
    std::cout << "LocOpe_SplitDrafts:!Fins throw Standard_ConstructionError()" << std::endl;
    throw Standard_ConstructionError();
  }

  gp_Pln       NewPlg, NewPld;
  Axis3d       NormalFg, NormalFd;
  TopoShape aLocalFace = F.Oriented(OriF);

  if (!NewPlane(TopoDS::Face(aLocalFace), Extrg, NPlg, Angleg, NewPlg, NormalFg, ModLeft)
      || !NewPlane(TopoDS::Face(aLocalFace), Extrd, NPld, Angled, NewPld, NormalFd, ModRight))
  {
    //  if (!NewPlane(TopoDS::Face(F.Oriented(OriF)),
    //		Extrg,NPlg,Angleg,NewPlg,NormalFg,ModLeft) ||
    //      !NewPlane(TopoDS::Face(F.Oriented(OriF)),
    //		Extrd,NPld,Angled,NewPld,NormalFd,ModRight)) {
    return;
  }

  TopTools_ListIteratorOfListOfShape itl;
  ShapeBuilder                       B;

  Handle(GeomSurface) NewSg       = new GeomPlane(NewPlg);
  Handle(GeomSurface) NewSd       = new GeomPlane(NewPld);
  Handle(GeomLine)    theLinePipe = new GeomLine(NormalFg); // ou NormalFd
  GeomInt_IntSS        i2s(NewSg, NewSd, Precision::Confusion());

  TopTools_MapOfShape         theMap;
  Handle(GeomAdaptor_Curve)   HAC = new GeomAdaptor_Curve;
  Handle(GeomAdaptor_Surface) HAS = new GeomAdaptor_Surface;
  IntCurveSurface_HInter      intcs;

  TopoWire theW = W;
  if (i2s.IsDone() && i2s.NbLines() > 0)
  {
    // on split le wire" << std::endl;

    GeomFill_Pipe thePipe;
    thePipe.GenerateParticularCase(Standard_True);
    thePipe.Init(theLinePipe, i2s.Line(1));
    thePipe.Perform(Standard_True);
    if (!thePipe.IsDone())
      throw Standard_ConstructionError("GeomFill_Pipe : Cannot make a surface");

    Handle(GeomSurface) Spl = thePipe.Surface();
    HAS->Load(Spl);

    LocOpe_SplitShape splw(W);

    for (exp.Init(W, TopAbs_EDGE); exp.More(); exp.Next())
    {
      const TopoEdge& edg = TopoDS::Edge(exp.Current());
      if (theMap.Add(edg))
      {
        TopLoc_Location    Loc;
        Standard_Real      f, l;
        Handle(GeomCurve3d) C = BRepInspector::Curve(edg, f, l);
        HAC->Load(C);
        intcs.Perform(HAC, HAS);
        if (!intcs.IsDone())
        {
          continue; // voir ce qu`on peut faire de mieux
        }

        if (intcs.NbSegments() >= 2)
        {
          continue; // Not yet implemented...and probably never"
        }

        if (intcs.NbSegments() == 1)
        {
          const IntersectionPoint1& P1 = intcs.Segment1(1).FirstPoint();
          const IntersectionPoint1& P2 = intcs.Segment1(1).SecondPoint();
          const Point3d&                            pf = P1.Pnt();
          const Point3d&                            pl = P2.Pnt();
          TopoVertex                            Vf, Vl;
          TopExp1::Vertices(edg, Vf, Vl);
          Point3d        Pf   = BRepInspector::Pnt(Vf);
          Point3d        Pl   = BRepInspector::Pnt(Vl);
          Standard_Real Tolf = BRepInspector::Tolerance(Vf);
          Standard_Real Toll = BRepInspector::Tolerance(Vl);
          Tolf *= Tolf;
          Toll *= Toll;

          Standard_Real dff = pf.SquareDistance(Pf);
          Standard_Real dfl = pf.SquareDistance(Pl);
          Standard_Real dlf = pl.SquareDistance(Pf);
          Standard_Real dll = pl.SquareDistance(Pl);

          if ((dff <= Tolf && dll <= Toll) || (dlf <= Tolf && dfl <= Toll))
          {
            continue;
          }
          else
          {
            // on segmente edg en pf et pl
            TopoVertex Vnewf, Vnewl;
            B.MakeVertex(Vnewf, pf, Precision::Confusion());
            B.MakeVertex(Vnewl, pl, Precision::Confusion());
            if (P1.W() >= f && P1.W() <= l && P2.W() >= f && P2.W() <= l)
            {
              splw.Add(Vnewf, P1.W(), edg);
              splw.Add(Vnewl, P2.W(), edg);
            }
            else
            {
              continue;
            }
          }
        }
        else if (intcs.NbPoints() != 0)
        {
          TopoVertex Vf, Vl;
          TopExp1::Vertices(edg, Vf, Vl);
          Point3d        Pf   = BRepInspector::Pnt(Vf);
          Point3d        Pl   = BRepInspector::Pnt(Vl);
          Standard_Real Tolf = BRepInspector::Tolerance(Vf);
          Standard_Real Toll = BRepInspector::Tolerance(Vl);
          Tolf *= Tolf;
          Toll *= Toll;

          for (Standard_Integer i = 1; i <= intcs.NbPoints(); i++)
          {
            const IntersectionPoint1& Pi  = intcs.Point(i);
            const Point3d&                            pi  = Pi.Pnt();
            Standard_Real                            dif = pi.SquareDistance(Pf);
            Standard_Real                            dil = pi.SquareDistance(Pl);
            if (dif <= Tolf)
            {
            }
            else if (dil <= Toll)
            {
            }
            else
            {
              if (Pi.W() >= f && Pi.W() <= l)
              {
                TopoVertex Vnew;
                B.MakeVertex(Vnew, pi, Precision::Confusion());
                splw.Add(Vnew, Pi.W(), edg);
              }
            }
          }
        }
      }
    }

    const ShapeList& lres = splw.DescendantShapes(W);
    if (lres.Extent() != 1)
    {
      return;
    }

    if (!W.IsSame(lres.First()))
    {
      theW.Nullify();
      theW = TopoDS::Wire(lres.First());
    }

    for (exp.ReInit(); exp.More(); exp.Next())
    {
      if (!myMap.IsBound(exp.Current()))
      {
        ShapeList thelist1;
        myMap.Bind(exp.Current(), thelist1);
        for (itl.Initialize(splw.DescendantShapes(exp.Current())); itl.More(); itl.Next())
        {
          myMap(exp.Current()).Append(itl.Value());
        }
        for (exp2.Init(exp.Current(), TopAbs_VERTEX); exp2.More(); exp2.Next())
        {
          if (!myMap.IsBound(exp2.Current()))
          {
            ShapeList thelist2;
            myMap.Bind(exp2.Current(), thelist2);
            myMap(exp2.Current()).Append(exp2.Current());
          }
        }
      }
    }
  }
  else
  {
    for (exp.Init(W, TopAbs_EDGE); exp.More(); exp.Next())
    {
      if (!myMap.IsBound(exp.Current()))
      {
        ShapeList thelist3;
        myMap.Bind(exp.Current(), thelist3);
        myMap(exp.Current()).Append(exp.Current());
        for (exp2.Init(exp.Current(), TopAbs_VERTEX); exp2.More(); exp2.Next())
        {
          if (!myMap.IsBound(exp2.Current()))
          {
            ShapeList thelist4;
            myMap.Bind(exp2.Current(), thelist4);
            myMap(exp2.Current()).Append(exp2.Current());
          }
        }
      }
    }
  }

  // On split la face par le wire

  Handle(LocOpe_WiresOnShape) WonS = new LocOpe_WiresOnShape(myShape);
  LocOpe_Spliter              Spls(myShape);
  WonS->Bind(theW, F);

  // JAG Le code suivant marchera apres integration de thick0
  //  LocOpe_FindEdges fined(W,F);
  //  for (fined.InitIterator(); fined.More(); fined.Next()) {
  //    WonS->Bind(fined.EdgeFrom(),fined.EdgeTo());
  //  }

  Spls.Perform(WonS);
  if (!Spls.IsDone())
  {
    return;
  }

  TopoShape                Res     = Spls.ResultingShape();
  const ShapeList& theLeft = Spls.DirectLeft();

  // Descendants
  for (exp.Init(myShape, TopAbs_FACE); exp.More(); exp.Next())
  {
    const TopoShape& fac = exp.Current();
    for (itl.Initialize(Spls.DescendantShapes(fac)); itl.More(); itl.Next())
    {
      myMap(fac).Append(itl.Value());
    }
  }

  TopTools_DataMapOfShapeShape MapW;
  for (exp.Init(theW, TopAbs_EDGE); exp.More(); exp.Next())
  {
    if (!MapW.IsBound(exp.Current()))
    {
      MapW.Bind(exp.Current(), TopoShape());
      for (exp2.Init(exp.Current(), TopAbs_VERTEX); exp2.More(); exp2.Next())
      {
        if (!MapW.IsBound(exp2.Current()))
        {
          MapW.Bind(exp2.Current(), TopoShape());
        }
      }
    }
  }

  TopTools_IndexedDataMapOfShapeListOfShape theMapEF;
  TopExp1::MapShapesAndAncestors(Res, TopAbs_EDGE, TopAbs_FACE, theMapEF);

  // On stocke les geometries potentiellement generees par les edges
  TopTools_IndexedDataMapOfShapeShape MapEV;        // genere
  TopTools_DataMapOfShapeListOfShape  MapSg, MapSd; // image a gauche et a droite

  Standard_Integer Nbedges, index;
  for (itl.Initialize(myMap(F)); itl.More(); itl.Next())
  {
    const TopoShape& fac = TopoDS::Face(itl.Value());
    for (exp.Init(fac, TopAbs_EDGE); exp.More(); exp.Next())
    {
      const TopoShape& edg = exp.Current();
      if (MapEV.FindIndex(edg) != 0)
      {
        continue;
      }
      if (MapW.IsBound(edg))
      { // edge du wire initial
        TopLoc_Location    Loc;
        Standard_Real      f, l;
        Handle(GeomCurve3d) C = BRepInspector::Curve(TopoDS::Edge(edg), Loc, f, l);
        if (C->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve))
        {
          C = Handle(Geom_TrimmedCurve)::DownCast(C)->BasisCurve();
        }
        C = Handle(GeomCurve3d)::DownCast(C->Transformed(Loc.Transformation()));

        GeomFill_Pipe thePipe;
        thePipe.GenerateParticularCase(Standard_True);
        thePipe.Init(theLinePipe, C);
        thePipe.Perform(Standard_True);
        if (!thePipe.IsDone())
          throw Standard_ConstructionError("GeomFill_Pipe : Cannot make a surface");

        Handle(GeomSurface) thePS = thePipe.Surface();
        if (thePS->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
        {
          thePS = Handle(Geom_RectangularTrimmedSurface)::DownCast(thePS)->BasisSurface();
        }

        TopoFace NewFace;
        B.MakeFace(NewFace, thePS, Precision::Confusion());
        MapEV.Add(edg, NewFace);
      }
      else
      { // on recupere la face.
        index = theMapEF.FindIndex(edg);
        if (theMapEF(index).Extent() != 2)
        {
          return; // NotDone
        }
        TopoFace theFace;
        if (theMapEF(index).First().IsSame(fac))
        {
          MapEV.Add(edg, theMapEF(index).Last());
        }
        else
        {
          MapEV.Add(edg, theMapEF(index).First());
        }
      }
    }
  }

  TopTools_DataMapOfShapeShape MapSonS;

  Nbedges = MapEV.Extent();
  for (index = 1; index <= Nbedges; index++)
  {
    for (exp.Init(MapEV.FindKey(index), TopAbs_VERTEX); exp.More(); exp.Next())
    {
      const TopoVertex& vtx = TopoDS::Vertex(exp.Current());
      if (MapEV.FindIndex(vtx) != 0)
      {
        continue;
      }

      // Localisation du vertex :
      //    - entre 2 edges d`origine : on recupere l`edge qui n`est
      //                                pas dans F
      //    - entre 2 edges du wire   : droite
      //    - mixte                   : intersection de surfaces
      for (j = 1; j <= Nbedges; j++)
      {
        if (j == index)
        {
          continue;
        }
        for (exp2.Init(MapEV.FindKey(j), TopAbs_VERTEX); exp2.More(); exp2.Next())
        {
          const TopoShape& vtx2 = exp2.Current();
          if (vtx2.IsSame(vtx))
          {
            break;
          }
        }
        if (exp2.More())
        {
          break;
        }
      }
      Standard_Integer    Choice = 0;
      const TopoShape& edg1   = MapEV.FindKey(index);
      TopoShape        edg2;
      if (j <= Nbedges)
      {
        edg2 = MapEV.FindKey(j);
      }
      else
      {
        edg2 = edg1;
      }
      if (MapW.IsBound(edg1))
      {
        if (j > Nbedges)
        {             // doit correspondre a edge ferme
          Choice = 2; // droite
        }
        else if (MapW.IsBound(MapEV.FindKey(j)))
        {
          Choice = 2; // droite
        }
        else
        {
          Choice = 3; // mixte
        }
      }
      else
      {
        if (j > Nbedges)
        {             // doit correspondre a edge ferme
          Choice = 1; // edge a retrouver
        }
        else if (!MapW.IsBound(MapEV.FindKey(j)))
        {
          Choice = 1; // edge a retrouver
        }
        else
        {
          Choice = 3; // mixte
        }
      }
      Handle(GeomCurve3d)   Newc;
      Handle(GeomCurve2d) newCs1, newCs2;
      Standard_Real        knownp = 0;
      TopoEdge          Ebind;
      switch (Choice)
      {
        case 1: {
          for (exp2.Init(Res, TopAbs_EDGE); exp2.More(); exp2.Next())
          {
            if (exp2.Current().IsSame(edg1) || exp2.Current().IsSame(edg2))
            {
              continue;
            }
            //	    for (ShapeExplorer exp3(exp2.Current().Oriented(TopAbs_FORWARD),
            ShapeExplorer exp3(exp2.Current().Oriented(TopAbs_FORWARD), TopAbs_VERTEX);
            for (; exp3.More(); exp3.Next())
            {
              if (exp3.Current().IsSame(vtx))
              {
                break;
              }
            }
            if (exp3.More())
            {
              break;
            }
          }
          if (exp2.More())
          {
            Standard_Real   f, l;
            TopLoc_Location Loc;
            Newc   = BRepInspector::Curve(TopoDS::Edge(exp2.Current()), Loc, f, l);
            Newc   = Handle(GeomCurve3d)::DownCast(Newc->Transformed(Loc.Transformation()));
            Ebind  = TopoDS::Edge(exp2.Current());
            knownp = BRepInspector::Parameter(vtx, Ebind);
          }
          else
          { // droite ??? il vaudrait mieux sortir
            return;

            //	    gp_Lin theLine(NormalFg);
            //	    theLine.Translate(NormalF.Location(),BRepInspector::Pnt(vtx));
            //	    Newc = new GeomLine(theLine);
            //	    knownp = 0.;
          }
        }
        break;
        case 2: {
          gp_Lin theLine(NormalFg);
          theLine.Translate(NormalFg.Location(), BRepInspector::Pnt(vtx));
          Newc   = new GeomLine(theLine);
          knownp = 0.;
        }
        break;
        case 3: {
          const TopoFace&   F1    = TopoDS::Face(MapEV.FindFromKey(edg1));
          const TopoFace&   F2    = TopoDS::Face(MapEV.FindFromKey(edg2));
          Handle(GeomSurface) S1    = BRepInspector::Surface(F1);
          Handle(GeomSurface) S2    = BRepInspector::Surface(F2);
          Standard_Boolean     AppS1 = Standard_False;
          Standard_Boolean     AppS2 = Standard_False;
          if (S1->DynamicType() != STANDARD_TYPE(GeomPlane))
          {
            AppS1 = Standard_True;
          }
          if (S2->DynamicType() != STANDARD_TYPE(GeomPlane))
          {
            AppS2 = Standard_True;
          }
          i2s.Perform(S1, S2, Precision::Confusion(), Standard_True, AppS1, AppS2);
          if (!i2s.IsDone() || i2s.NbLines() <= 0)
          {
            return;
          }

          Standard_Real     pmin = 0, Dist2, Dist2Min, Glob2Min = RealLast();
          GeomAdaptor_Curve TheCurve;

          Standard_Integer i, imin, k;
          Point3d           pv = BRepInspector::Pnt(vtx);
          imin                = 0;
          for (i = 1; i <= i2s.NbLines(); i++)
          {
            TheCurve.Load(i2s.Line(i));
            Extrema_ExtPC myExtPC(pv, TheCurve);

            if (myExtPC.IsDone())
            {
              Point3d        p1b, p2b;
              Standard_Real thepmin = TheCurve.FirstParameter();
              myExtPC.TrimmedSquareDistances(Dist2Min, Dist2, p1b, p2b);
              if (Dist2 < Dist2Min)
              {
                thepmin = TheCurve.LastParameter();
              }
              for (k = 1; k <= myExtPC.NbExt(); k++)
              {
                Dist2 = myExtPC.SquareDistance(k);
                if (Dist2 < Dist2Min)
                {
                  Dist2Min = Dist2;
                  thepmin  = myExtPC.Point(k).Parameter();
                }
              }

              if (Dist2Min < Glob2Min)
              {
                Glob2Min = Dist2Min;
                pmin     = thepmin;
                imin     = i;
              }
            }
          }
          if (imin == 0)
          {
            return;
          }

          Newc   = i2s.Line(imin);
          knownp = pmin;
          if (AppS1)
          {
            newCs1 = i2s.LineOnS1(imin);
          }
          if (AppS2)
          {
            newCs2 = i2s.LineOnS2(imin);
          }
        }
        break;
      }

      // Determination des vertex par intersection sur Plg ou/et Pld

      HAC->Load(Newc);
      Standard_Integer nbfois = 2;
      TopoVertex    vtx1, vtx2;
      Standard_Real    p1 = 0, p2 = 0;
      Standard_Boolean IsLeft = Standard_False;
      if (Choice == 1)
      {
        // edge retrouve : on ne fait qu`une seule intersection
        // il faut utiliser Plg ou Pld

        Standard_Integer indedgf = theMapEF.FindIndex(edg1);
        for (itl.Initialize(theMapEF(indedgf)); itl.More(); itl.Next())
        {
          if (Contains(myMap(F), itl.Value()))
          {
            if (Contains(theLeft, itl.Value()))
            {
              HAS->Load(NewSg);
              IsLeft = Standard_True;
            }
            else
            {
              HAS->Load(NewSd);
              IsLeft = Standard_False;
            }

            nbfois = 1;
            vtx2   = vtx;
            p2     = knownp;
            break;
          }
        }
        if (!itl.More())
        {
          std::cout << "LocOpe_SplitDrafts: betite probleme " << std::endl;
          return;
        }
      }
      else
      {
        HAS->Load(NewSg);
      }

      for (Standard_Integer it = 1; it <= nbfois; it++)
      {
        if (it == 2)
        {
          HAS->Load(NewSd);
        }

        intcs.Perform(HAC, HAS);
        if (!intcs.IsDone())
        {
          return; // voir ce qu`on peut faire de mieux
        }
        Standard_Integer imin  = 1;
        Standard_Real    delta = Abs(knownp - intcs.Point(1).W());
        for (Standard_Integer i = 2; i <= intcs.NbPoints(); i++)
        {
          Standard_Real newdelta = Abs(knownp - intcs.Point(i).W());
          if (newdelta < delta)
          {
            imin  = i;
            delta = newdelta;
          }
        }
        if (it == 1)
        {
          B.MakeVertex(vtx1, intcs.Point(imin).Pnt(), Precision::Confusion());
          p1     = intcs.Point(imin).W();
          knownp = p1;
        }
        else
        {
          B.MakeVertex(vtx2, intcs.Point(imin).Pnt(), Precision::Confusion());
          p2 = intcs.Point(imin).W();
        }
      }
      if (Abs(p1 - p2) > Precision::PConfusion())
      {
        TopoEdge NewEdge;
        B.MakeEdge(NewEdge, Newc, Precision::Confusion());
        if (p1 < p2)
        {
          B.Add(NewEdge, vtx1.Oriented(TopAbs_FORWARD));
          B.Add(NewEdge, vtx2.Oriented(TopAbs_REVERSED));
        }
        else
        {
          B.Add(NewEdge, vtx1.Oriented(TopAbs_REVERSED));
          B.Add(NewEdge, vtx2.Oriented(TopAbs_FORWARD));
        }
        B.UpdateVertex(vtx1, p1, NewEdge, Precision::Confusion());
        B.UpdateVertex(vtx2, p2, NewEdge, Precision::Confusion());
        if (!newCs1.IsNull())
        {
          B.UpdateEdge(NewEdge,
                       newCs1,
                       TopoDS::Face(MapEV.FindFromKey(edg1)),
                       Precision::Confusion());
        }

        if (!newCs2.IsNull())
        {
          B.UpdateEdge(NewEdge,
                       newCs2,
                       TopoDS::Face(MapEV.FindFromKey(edg2)),
                       Precision::Confusion());
        }

        MapEV.Add(vtx, NewEdge);

        if (Choice == 1)
        {
          TopoShape aLocalEdge = Ebind.EmptyCopied();
          TopoEdge  NE         = TopoDS::Edge(aLocalEdge);
          //	  TopoEdge NE = TopoDS::Edge(Ebind.EmptyCopied());
          for (exp2.Init(Ebind, TopAbs_VERTEX); exp2.More(); exp2.Next())
          {
            const TopoVertex& thevtx = TopoDS::Vertex(exp2.Current());
            if (thevtx.IsSame(vtx))
            {
              B.Add(NE, vtx1.Oriented(thevtx.Orientation()));
              B.UpdateVertex(vtx1, p1, NE, Precision::Confusion());
            }
            else
            {
              B.Add(NE, thevtx);
              Standard_Real theprm = BRepInspector::Parameter(thevtx, Ebind);
              B.UpdateVertex(thevtx, theprm, NE, BRepInspector::Tolerance(thevtx));
            }
          }
          MapSonS.Bind(Ebind, NE.Oriented(TopAbs_FORWARD));
          if (IsLeft)
          {
            ShapeList thelist5;
            MapSg.Bind(vtx, thelist5);
            MapSg(vtx).Append(vtx1);
          }
          else
          {
            ShapeList thelist6;
            MapSd.Bind(vtx, thelist6);
            MapSd(vtx).Append(vtx1);
          }
        }
        else
        {
          ShapeList thelist7, thelist8;
          MapSg.Bind(vtx, thelist7);
          MapSd.Bind(vtx, thelist8);
          MapSg(vtx).Append(vtx1);
          MapSd(vtx).Append(vtx2);
        }
      }
      else
      {
        MapEV.Add(vtx, vtx2); // on peut avoir vtx2 = vtx si choix == 1
        if (Choice == 1)
        {
          if (IsLeft)
          {
            ShapeList thelist9;
            MapSg.Bind(vtx, thelist9);
            MapSg(vtx).Append(vtx);
          }
          else
          {
            ShapeList thelist10;
            MapSd.Bind(vtx, thelist10);
            MapSd(vtx).Append(vtx);
          }
        }
        else
        {
          ShapeList thelist11, thelist12;
          MapSg.Bind(vtx, thelist11);
          MapSd.Bind(vtx, thelist12);
          MapSg(vtx).Append(vtx2);
          MapSd(vtx).Append(vtx2);
        }
      }
    }
  }

  theMap.Clear();
  for (exp.Init(theW, TopAbs_EDGE); exp.More(); exp.Next())
  {
    const TopoEdge& edg = TopoDS::Edge(exp.Current());
    if (!theMap.Add(edg))
    { // precaution sans doute inutile...
      continue;
    }
    Standard_Integer     indedg = MapEV.FindIndex(edg);
    TopoFace&         GenF   = TopoDS::Face(MapEV(indedg));
    ShapeList thelist13, thelist14;
    MapSg.Bind(edg, thelist13); // genere a gauche
    MapSd.Bind(edg, thelist14); // genere a droite
    TopoVertex Vf, Vl;
    TopoShape  aLocalEdge = edg.Oriented(TopAbs_FORWARD);
    TopExp1::Vertices(TopoDS::Edge(aLocalEdge), Vf, Vl);
    //    TopExp1::Vertices(TopoDS::Edge(edg.Oriented(TopAbs_FORWARD)),Vf,Vl);
    TopoShape Gvf = MapEV.FindFromKey(Vf);
    TopoShape Gvl = MapEV.FindFromKey(Vl);

    /* Le code suivant est OK. On essaie de l`ameliorer

        if (Gvf.ShapeType() == TopAbs_VERTEX &&
        Gvl.ShapeType() == TopAbs_VERTEX) {
          // en fait on doit pouvoir avoir 1 face a 2 cotes...
          if (Gvf.IsSame(Vf)) {
        MapW(edg) = edg;
        MapSg(edg).Append(edg.Oriented(TopAbs_FORWARD));
        MapSd(edg).Append(edg.Oriented(TopAbs_FORWARD));
          }
          else {
        TopoEdge NewEdg = NewEdge(edg,
                         GenF,NewSg,
                         TopoDS::Vertex(Gvf),
                         TopoDS::Vertex(Gvl));
        if (NewEdg.IsNull()) {
          return;
        }
        MapW(edg) = NewEdg;
        MapSg(edg).Append(NewEdg);
        MapSd(edg).Append(NewEdg);
          }
        }
        else if (Gvf.ShapeType() == TopAbs_VERTEX  ||
             Gvl.ShapeType() == TopAbs_VERTEX) {      // face triangulaire
          TopoVertex Vfd,Vld,Vfg,Vlg;
          if (Gvf.ShapeType() == TopAbs_VERTEX) {
        Vfg = TopoDS::Vertex(Gvf);
        Vfd = Vfg;
        Vlg = TopoDS::Vertex(MapSg(Vl).First());
        Vld = TopoDS::Vertex(MapSd(Vl).First());
          }
          else {
        Vlg = TopoDS::Vertex(Gvl);
        Vld = Vlg;
        Vfg = TopoDS::Vertex(MapSg(Vf).First());
        Vfd = TopoDS::Vertex(MapSd(Vf).First());
          }

          TopoEdge NewEdgg = NewEdge(edg,GenF,NewSg,Vfg,Vlg);
          if (NewEdgg.IsNull()) {
        return;
          }

          TopoEdge NewEdgd = NewEdge(edg,GenF,NewSd,Vfd,Vld);
          if (NewEdgg.IsNull()) {
        return;
          }
          MapSg(edg).Append(NewEdgg);
          MapSd(edg).Append(NewEdgd);

          ShapeList theedges;
          theedges.Append(NewEdgg);
          theedges.Append(NewEdgd);
          if (Gvf.ShapeType() == TopAbs_EDGE) {
        theedges.Append(Gvf);
          }
          else {//if (Gvl.ShapeType() == TopAbs_EDGE) {
        theedges.Append(Gvl);
          }
          MakeFace(GenF,theedges);
          MapW(edg) = GenF;
        }
        else {
          // une face a 4 cotes
          TopoVertex Vfd,Vld,Vfg,Vlg;

          Vfg = TopoDS::Vertex(MapSg(Vf).First());
          Vfd = TopoDS::Vertex(MapSd(Vf).First());
          Vlg = TopoDS::Vertex(MapSg(Vl).First());
          Vld = TopoDS::Vertex(MapSd(Vl).First());

          TopoVertex VVf1,VVl1,VVf2,VVl2;
          TopExp1::Vertices(TopoDS::Edge(Gvf.Oriented(TopAbs_FORWARD)),VVf1,VVl1);
          TopExp1::Vertices(TopoDS::Edge(Gvl.Oriented(TopAbs_FORWARD)),VVf2,VVl2);

          TopoEdge NewEdgg = NewEdge(edg,GenF,NewSg,Vfg,Vlg);
          if (NewEdgg.IsNull()) {
        return;
          }

          TopoEdge NewEdgd = NewEdge(edg,GenF,NewSd,Vfd,Vld);
          if (NewEdgd.IsNull()) {
        return;
          }

          if ((VVf1.IsSame(Vfg) && VVf2.IsSame(Vlg)) ||
          (VVf1.IsSame(Vfd) && VVf2.IsSame(Vld))) {
        // 4 cotes
        MapSg(edg).Append(NewEdgg);
        MapSd(edg).Append(NewEdgd);

        ShapeList theedges;
        theedges.Append(NewEdgg);
        theedges.Append(NewEdgd);
        theedges.Append(Gvf);
        theedges.Append(Gvl);

        MakeFace(GenF,theedges);
        MapW(edg) = GenF;
          }
          else {
    #ifdef OCCT_DEBUG
        std::cout << "Pb d'analyse" << std::endl;
    #endif
        return;
          }
        }
    */
    // nouveau code

    TopoVertex Vfd, Vld, Vfg, Vlg;
    if (Gvf.ShapeType() == TopAbs_VERTEX)
    {
      Vfg = TopoDS::Vertex(Gvf);
      Vfd = Vfg;
    }
    else
    {
      Vfg = TopoDS::Vertex(MapSg(Vf).First());
      Vfd = TopoDS::Vertex(MapSd(Vf).First());
    }
    if (Gvl.ShapeType() == TopAbs_VERTEX)
    {
      Vlg = TopoDS::Vertex(Gvl);
      Vld = Vlg;
    }
    else
    {
      Vlg = TopoDS::Vertex(MapSg(Vl).First());
      Vld = TopoDS::Vertex(MapSd(Vl).First());
    }

    TopoEdge NewEdgg = NewEdge(edg, GenF, NewSg, Vfg, Vlg);
    if (NewEdgg.IsNull())
    {
      return;
    }

    TopoEdge NewEdgd = NewEdge(edg, GenF, NewSd, Vfd, Vld);
    if (NewEdgg.IsNull())
    {
      return;
    }

    Standard_Boolean isedg = Standard_False;
    if (Gvf.ShapeType() == TopAbs_VERTEX && Gvl.ShapeType() == TopAbs_VERTEX)
    {
      // edg ou face a 2 cotes

      // Comparaison NewEdgg et NewEdgd
      Standard_Real      fg, lg, fd, ld;
      Handle(GeomCurve3d) Cg   = BRepInspector::Curve(NewEdgg, fg, lg);
      Handle(GeomCurve3d) Cd   = BRepInspector::Curve(NewEdgd, fd, ld);
      Standard_Real      prmg = (fg + lg) / 2.;
      Standard_Real      prmd = (fd + ld) / 2.;
      Point3d             pg   = Cg->Value(prmg);
      Point3d             pd   = Cd->Value(prmd);
      Standard_Real      Tol  = Max(BRepInspector::Tolerance(NewEdgg), BRepInspector::Tolerance(NewEdgg));
      if (pg.SquareDistance(pd) <= Tol * Tol)
      {
        isedg = Standard_True;
        // raffinement pour essayer de partager l`edge de depart...
        Standard_Boolean modified = Standard_True;
        if (Gvf.IsSame(Vf) && Gvl.IsSame(Vl))
        {
          // Comparaison avec l`edge de depart
          Cd   = BRepInspector::Curve(edg, fd, ld);
          prmd = (fd + ld) / 2.;
          pd   = Cd->Value(prmd);
          Tol  = Max(BRepInspector::Tolerance(NewEdgg), BRepInspector::Tolerance(edg));
          if (pg.SquareDistance(pd) <= Tol * Tol)
          {
            modified = Standard_False;
          }
        }

        if (!modified)
        {
          MapW(edg) = edg;
          MapSg(edg).Append(edg);
          MapSd(edg).Append(edg);
        }
        else
        {
          MapW(edg) = NewEdgg;
          MapSg(edg).Append(NewEdgg);
          MapSd(edg).Append(NewEdgg);
        }
      }
    }

    if (!isedg)
    {
      // face a 2 ou 3 ou 4 cotes
      MapSg(edg).Append(NewEdgg);
      MapSd(edg).Append(NewEdgd);

      ShapeList theedges;
      theedges.Append(NewEdgg);
      theedges.Append(NewEdgd);
      if (Gvf.ShapeType() == TopAbs_EDGE)
      {
        theedges.Append(Gvf);
      }
      if (Gvl.ShapeType() == TopAbs_EDGE)
      {
        theedges.Append(Gvl);
      }
      MakeFace(GenF, theedges);
      MapW(edg) = GenF;
    }
  }

  TopTools_MapOfShape  mapedgadded;
  ShapeList thefaces;

  for (itl.Initialize(myMap(F)); itl.More(); itl.Next())
  {
    const TopoFace& fac = TopoDS::Face(itl.Value());
    theMap.Clear();
    TopoFace      DrftFace; // elle est FORWARD
    Standard_Boolean IsLeft;
    if (Contains(theLeft, fac))
    {
      B.MakeFace(DrftFace, NewSg, BRepInspector::Tolerance(fac));
      IsLeft = Standard_True;
    }
    else
    {
      B.MakeFace(DrftFace, NewSd, BRepInspector::Tolerance(fac));
      IsLeft = Standard_False;
    }

    ShapeExplorer exp3;
    for (exp3.Init(fac.Oriented(TopAbs_FORWARD), TopAbs_WIRE); exp3.More(); exp3.Next())
    {
      const TopoShape& wir = exp3.Current();
      TopoWire         NewWireOnF;
      B.MakeWire(NewWireOnF);
      for (exp.Init(wir.Oriented(TopAbs_FORWARD), TopAbs_EDGE); exp.More(); exp.Next())
      {
        const TopoEdge& edg = TopoDS::Edge(exp.Current());
        if (!theMap.Add(edg))
        { // precaution sans doute inutile...
          continue;
        }
        if (MapW.IsBound(edg))
        { // edge du wire d`origine
          TopTools_ListIteratorOfListOfShape itld;
          TopAbs_Orientation                 ored = edg.Orientation();
          if (IsLeft)
          {
            itld.Initialize(MapSg(edg));
          }
          else
          {
            itld.Initialize(MapSd(edg));
          }
          for (; itld.More(); itld.Next())
          {
            if (itld.Value().Orientation() == TopAbs_REVERSED)
            {
              ored = TopAbs1::Reverse(ored);
            }
            TopoShape aLocalEdge = itld.Value().Oriented(ored);
            B.Add(NewWireOnF, TopoDS::Edge(aLocalEdge));
            //	    B.Add(NewWireOnF,TopoDS::Edge(itld.Value().Oriented(ored)));
          }
        }
        else
        {
          Handle(GeomSurface) NewS;
          if (IsLeft)
          {
            NewS = NewSg;
          }
          else
          {
            NewS = NewSd;
          }
          Standard_Integer   indedg = MapEV.FindIndex(edg);
          const TopoFace& GenF   = TopoDS::Face(MapEV(indedg));
          TopoVertex      Vf, Vl;
          TopoShape       aLocalEdge = edg.Oriented(TopAbs_FORWARD);
          TopExp1::Vertices(TopoDS::Edge(aLocalEdge), Vf, Vl);
          //	  TopExp1::Vertices(TopoDS::Edge(edg.Oriented(TopAbs_FORWARD)),Vf,Vl);
          TopoShape Gvf = MapEV.FindFromKey(Vf);
          TopoShape Gvl = MapEV.FindFromKey(Vl);
          if (Gvf.ShapeType() == TopAbs_VERTEX && Gvl.ShapeType() == TopAbs_VERTEX)
          {
            if (!Gvf.IsSame(Vf) || !Gvl.IsSame(Vl))
            {
              TopoEdge NewEdg =
                NewEdge(edg, GenF, NewS, TopoDS::Vertex(Gvf), TopoDS::Vertex(Gvl));
              if (NewEdg.IsNull())
              {
                return;
              }

              MapSonS.Bind(edg, NewEdg);

              if (NewEdg.Orientation() == TopAbs_REVERSED)
              {
                NewEdg.Orientation(TopAbs1::Reverse(edg.Orientation()));
              }
              else
              {
                NewEdg.Orientation(edg.Orientation());
              }
              B.Add(NewWireOnF, NewEdg);
            }
            else
            { // Frozen???
              B.Add(NewWireOnF, edg);
            }
          }
          else
          {
            TopoVertex Vff, Vll;
            if (Gvf.ShapeType() == TopAbs_VERTEX)
            {
              Vff = TopoDS::Vertex(Gvf);
            }
            else
            {
              if (IsLeft)
              {
                Vff = TopoDS::Vertex(MapSg(Vf).First());
              }
              else
              {
                Vff = TopoDS::Vertex(MapSd(Vf).First());
              }
            }
            if (Gvl.ShapeType() == TopAbs_VERTEX)
            {
              Vll = TopoDS::Vertex(Gvl);
            }
            else
            {
              if (IsLeft)
              {
                Vll = TopoDS::Vertex(MapSg(Vl).First());
              }
              else
              {
                Vll = TopoDS::Vertex(MapSd(Vl).First());
              }
            }

            TopoEdge NewEdg = NewEdge(edg, GenF, NewS, Vff, Vll);
            if (NewEdg.IsNull())
            {
              return;
            }

            if (!MapW.IsBound(Vf) && !MapW.IsBound(Vl))
            {
              MapSonS.Bind(edg, NewEdg);
            }
            //	    else if (MapW.IsBound(Vf) && MapW.IsBound(Vl)) {

            //	    }
            else
            {
              if (MapW.IsBound(Vf))
              {
                if (Gvf.ShapeType() != TopAbs_EDGE || mapedgadded.Contains(Gvf))
                {
                  MapSonS.Bind(edg, NewEdg);
                }
                else
                {
                  TopoWire NewWir;
                  B.MakeWire(NewWir);
                  B.Add(NewWir, NewEdg);

                  TopoVertex Vf2, Vl2;
                  TopExp1::Vertices(TopoDS::Edge(Gvf), Vf2, Vl2);

                  // TopAbs_Orientation ornw = NewEdg.Orientation();

                  // ici bug orientation : voir tspdrft6

                  //		  if ((ornw == TopAbs_FORWARD && Vl2.IsSame(Vff)) ||
                  //		      (ornw == TopAbs_REVERSED && Vl2.IsSame(Vll))) {
                  if (Vl2.IsSame(Vff))
                  {
                    B.Add(NewWir, Gvf.Oriented(TopAbs_FORWARD));
                  }
                  else
                  {
                    B.Add(NewWir, Gvf.Oriented(TopAbs_REVERSED));
                  }
                  mapedgadded.Add(Gvf);
                  MapSonS.Bind(edg, NewWir); // NewWire est FORWARD
                }
              }
              else
              {
                if (Gvl.ShapeType() != TopAbs_EDGE || mapedgadded.Contains(Gvl))
                {
                  MapSonS.Bind(edg, NewEdg);
                }
                else
                {
                  TopoWire NewWir;
                  B.MakeWire(NewWir);
                  B.Add(NewWir, NewEdg);

                  TopoVertex Vf2, Vl2;
                  TopExp1::Vertices(TopoDS::Edge(Gvl), Vf2, Vl2);

                  // TopAbs_Orientation ornw = NewEdg.Orientation();

                  // ici bug orientation : voir tspdrft6

                  //		  if ((ornw == TopAbs_FORWARD && Vl2.IsSame(Vff)) ||
                  //		      (ornw == TopAbs_REVERSED && Vl2.IsSame(Vll))) {
                  if (Vf2.IsSame(Vll))
                  {
                    B.Add(NewWir, Gvl.Oriented(TopAbs_FORWARD));
                  }
                  else
                  {
                    B.Add(NewWir, Gvl.Oriented(TopAbs_REVERSED));
                  }
                  mapedgadded.Add(Gvl);
                  MapSonS.Bind(edg, NewWir); // NewWire est FORWARD
                }
              }
            }
            if (NewEdg.Orientation() == TopAbs_REVERSED)
            {
              NewEdg.Orientation(TopAbs1::Reverse(edg.Orientation()));
            }
            else
            {
              NewEdg.Orientation(edg.Orientation());
            }
            B.Add(NewWireOnF, NewEdg);
          }
        }
      }
      B.Add(DrftFace, NewWireOnF.Oriented(wir.Orientation()));
    }
    thefaces.Append(DrftFace);
  }

  ShapeSubstitution                        theSubs;
  TopTools_DataMapIteratorOfDataMapOfShapeShape itdmss;
  for (itdmss.Initialize(MapSonS); itdmss.More(); itdmss.Next())
  {
    ShapeList lsubs;
    for (exp.Init(itdmss.Value(), TopAbs_EDGE); exp.More(); exp.Next())
    {
      lsubs.Append(exp.Current());
    }
    theSubs.Substitute(itdmss.Key1(), lsubs);
  }

  // on reconstruit les faces
  for (exp.Init(Res, TopAbs_FACE); exp.More(); exp.Next())
  {
    if (Contains(myMap(F), exp.Current()))
    {
      continue;
    }
    theSubs.Build(exp.Current());
  }

  // Stockage des descendants
  //  for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itdmsls(myMap);
  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itdmsls(myMap);
  for (; itdmsls.More(); itdmsls.Next())
  {
    if (itdmsls.Key1().ShapeType() == TopAbs_EDGE)
    {
      ShapeList thedesc;
      theMap.Clear();
      for (itl.Initialize(itdmsls.Value()); itl.More(); itl.Next())
      {
        if (theMap.Add(MapW(itl.Value())))
        {
          thedesc.Append(MapW(itl.Value()));
        }
      }
      myMap(itdmsls.Key1()) = thedesc;
    }
    else if (itdmsls.Key1().IsSame(F))
    {
      myMap(F).Clear();
      for (itl.Initialize(thefaces); itl.More(); itl.Next())
      {
        myMap(F).Append(itl.Value());
      }
    }
    else
    {
      ShapeList thedesc;
      theMap.Clear();
      for (itl.Initialize(itdmsls.Value()); itl.More(); itl.Next())
      {
        if (theSubs.IsCopied(itl.Value()))
        {
          if (theSubs.Copy(itl.Value()).Extent() != 1)
          {
#ifdef OCCT_DEBUG
            std::cout << "Invalid number of descendant" << std::endl;
#endif
            return;
          }
          else
          {
            if (theMap.Add(theSubs.Copy(itl.Value()).First()))
            {
              thedesc.Append(theSubs.Copy(itl.Value()).First());
            }
          }
        }
        else if (theMap.Add(itl.Value()))
        {
          thedesc.Append(itl.Value());
        }
      }
      myMap(itdmsls.Key1()) = thedesc;
    }
  }

  theMap.Clear();
  thefaces.Clear();
  for (itdmsls.Initialize(myMap); itdmsls.More(); itdmsls.Next())
  {
    for (itl.Initialize(itdmsls.Value()); itl.More(); itl.Next())
    {
      if (itl.Value().ShapeType() == TopAbs_FACE && theMap.Add(itl.Value()))
      {
        thefaces.Append(itl.Value());
      }
    }
  }
  LocOpe_BuildShape BS(thefaces);
  myResult = BS.Shape();
}

//=================================================================================================

const TopoShape& LocOpe_SplitDrafts::Shape() const
{
  if (myResult.IsNull())
  {
    throw StdFail_NotDone();
  }
  return myResult;
}

//=================================================================================================

const ShapeList& LocOpe_SplitDrafts::ShapesFromShape(const TopoShape& S) const
{
  if (myResult.IsNull())
  {
    throw StdFail_NotDone();
  }
  return myMap(S);
}

//=================================================================================================

static Standard_Boolean NewPlane(const TopoFace&     F,
                                 const Dir3d&          Extr,
                                 const gp_Pln&          Neutr,
                                 const Standard_Real    Ang,
                                 gp_Pln&                Newpl,
                                 Axis3d&                NormalF,
                                 const Standard_Boolean Modify)
{

  // Determination du nouveau plan incline
  Handle(GeomSurface) S = BRepInspector::Surface(F);
  if (S->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
  {
    S = Handle(Geom_RectangularTrimmedSurface)::DownCast(S)->BasisSurface();
  }
  Handle(GeomPlane) P = Handle(GeomPlane)::DownCast(S);
  if (P.IsNull())
  {
    return Standard_False;
  }

  gp_Pln Plorig = P->Pln();
  if (!Modify)
  {
    Newpl   = Plorig;
    NormalF = Newpl.Axis();
    if ((Newpl.Direct() && F.Orientation() == TopAbs_REVERSED)
        || (!Newpl.Direct() && F.Orientation() == TopAbs_FORWARD))
    {
      NormalF.Reverse();
    }
    return Standard_True;
  }

  Axis3d        Axe;
  Standard_Real Theta;

  QuadQuadGeoIntersection i2pl(Plorig, Neutr, Precision::Angular(), Precision::Confusion());

  if (i2pl.IsDone() && i2pl.TypeInter() == IntAna_Line)
  {
    gp_Lin LinInters = i2pl.Line(1);
    Dir3d nx        = LinInters.Direction();
    NormalF          = Plorig.Axis();
    Dir3d        ny = NormalF.Direction().Crossed(nx);
    Standard_Real a  = Extr.Dot(nx);
    if (Abs(a) <= 1 - Precision::Angular())
    {
      Standard_Real      b = Extr.Dot(ny);
      Standard_Real      c = Extr.Dot(NormalF.Direction());
      Standard_Boolean   direct(Plorig.Direct());
      TopAbs_Orientation Oris = F.Orientation();
      if ((direct && Oris == TopAbs_REVERSED) || (!direct && Oris == TopAbs_FORWARD))
      {
        b = -b;
        c = -c;
        NormalF.Reverse();
      }
      Standard_Real denom = Sqrt(1 - a * a);
      Standard_Real Sina  = Sin(Ang);
      if (denom > Abs(Sina))
      {
        Standard_Real phi    = ATan2(b / denom, c / denom);
        Standard_Real theta0 = ACos(Sina / denom);
        Theta                = theta0 - phi;
        if (Cos(Theta) < 0.)
        {
          Theta = -theta0 - phi;
        }
        Axe   = LinInters.Position1();
        Newpl = Plorig.Rotated(Axe, Theta);
        return Standard_True;
      }
    }
  }
  std::cout << "fin newplane return standard_false" << std::endl;
  return Standard_False;
}

//=================================================================================================

static void MakeFace(TopoFace& F, ShapeList& ledg)
{

  // ledg est une liste d'edge

  ShapeBuilder B;

  // Verification de l`existence des p-curves. Celles qui manquent
  // correspondent necessairement a des isos (et meme des iso u).

  Standard_Real f, l;
  //  for (TopTools_ListIteratorOfListOfShape itl(ledg);
  TopTools_ListIteratorOfListOfShape itl(ledg);
  for (; itl.More(); itl.Next())
  {
    const TopoEdge&   edg = TopoDS::Edge(itl.Value());
    Handle(GeomCurve2d) C2d = BRepInspector::CurveOnSurface(edg, F, f, l);
    if (C2d.IsNull())
    {
      BRepInspector::Range(edg, f, l);
      TopoVertex V1, V2;
      TopExp1::Vertices(edg, V1, V2);
      TopTools_ListIteratorOfListOfShape itl2;
      for (itl2.Initialize(ledg); itl2.More(); itl2.Next())
      {
        const TopoEdge& edg2 = TopoDS::Edge(itl2.Value());
        if (edg2.IsSame(edg))
        {
          continue;
        }
        TopoVertex Vp1, Vp2;
        TopExp1::Vertices(edg2, Vp1, Vp2);
        if (Vp1.IsSame(V1) || Vp2.IsSame(V1) || Vp1.IsSame(V2) || Vp2.IsSame(V2))
        {
          Standard_Real        f2, l2;
          Handle(GeomCurve2d) C22d = BRepInspector::CurveOnSurface(edg2, F, f2, l2);
          if (!C22d.IsNull())
          {
            gp_Pnt2d pt2d;
            if (Vp1.IsSame(V1))
            {
              pt2d = C22d->Value(f2);
              pt2d.SetY(pt2d.Y() - f);
            }
            else if (Vp2.IsSame(V1))
            {
              pt2d = C22d->Value(l2);
              pt2d.SetY(pt2d.Y() - f);
            }
            else if (Vp1.IsSame(V2))
            {
              pt2d = C22d->Value(f2);
              pt2d.SetY(pt2d.Y() - l);
            }
            else if (Vp2.IsSame(V2))
            {
              pt2d = C22d->Value(l2);
              pt2d.SetY(pt2d.Y() - l);
            }
            C2d = new Geom2d_Line(pt2d, gp1::DY2d());
            B.UpdateEdge(edg, C2d, F, BRepInspector::Tolerance(edg));
            break;
          }
        }
      }
      if (C2d.IsNull())
      {
        std::cout << "Ca merde violemment" << std::endl;
      }
    }
  }

  ShapeList lwires;
  Standard_Boolean     alldone = ledg.IsEmpty();
  while (!alldone)
  {
    TopoWire Wnew;
    B.MakeWire(Wnew);
    TopoShape       aLocalShapeCur = ledg.First();
    const TopoEdge& edg            = TopoDS::Edge(aLocalShapeCur);
    //    const TopoEdge& edg = TopoDS::Edge(ledg.First());
    TopoVertex VFirst, VLast;
    if (edg.Orientation() == TopAbs_FORWARD)
    {
      TopExp1::Vertices(edg, VFirst, VLast);
    }
    else
    {
      TopExp1::Vertices(edg, VLast, VFirst);
    }
    B.Add(Wnew, edg);
    ledg.RemoveFirst();
    // on suppose VFirst et VLast non nuls
    Standard_Boolean wdone = (ledg.IsEmpty() || VFirst.IsSame(VLast));
    while (!wdone)
    {
      TopoVertex VF, VL;

      TopAbs_Orientation oredg = TopAbs_FORWARD;

      for (itl.Initialize(ledg); itl.More(); itl.Next())
      {
        const TopoEdge& edg2        = TopoDS::Edge(itl.Value());
        TopoShape       aLocalShape = edg2.Oriented(TopAbs_FORWARD);
        TopExp1::Vertices(TopoDS::Edge(aLocalShape), VF, VL);
        //	TopExp1::Vertices(TopoDS::Edge(edg2.Oriented(TopAbs_FORWARD)),VF,VL);
        if (VF.IsSame(VLast))
        {
          VLast = VL;
          oredg = TopAbs_FORWARD;
          break;
        }
        else if (VL.IsSame(VFirst))
        {
          VFirst = VF;
          oredg  = TopAbs_FORWARD;
          break;
        }
        else if (VF.IsSame(VFirst))
        {
          VFirst = VL;
          oredg  = TopAbs_REVERSED;
          break;
        }
        else if (VL.IsSame(VLast))
        {
          VLast = VF;
          oredg = TopAbs_REVERSED;
          break;
        }
      }
      if (!itl.More())
      {
        wdone = Standard_True;
      }
      else
      {
        TopoShape aLocalShape = itl.Value().Oriented(oredg);
        B.Add(Wnew, TopoDS::Edge(aLocalShape));
        //	B.Add(Wnew,TopoDS::Edge(itl.Value().Oriented(oredg)));
        ledg.Remove(itl);
        wdone = (ledg.IsEmpty() || VFirst.IsSame(VLast));
      }
    }
    lwires.Append(Wnew);
    alldone = ledg.IsEmpty();
  }

  F.Orientation(TopAbs_FORWARD);
  for (itl.Initialize(lwires); itl.More(); itl.Next())
  {
    TopoShape aLocalShape = F.EmptyCopied();
    TopoFace  NewFace     = TopoDS::Face(aLocalShape);
    //    TopoFace NewFace = TopoDS::Face(F.EmptyCopied());
    B.Add(NewFace, itl.Value());
    GeometricProperties GP;
    BRepGProp1::SurfaceProperties(NewFace, GP);
    if (GP.Mass() < 0)
    {
      itl.ChangeValue().Reverse();
    }
  }
  if (lwires.Extent() == 1)
  {
    B.Add(F, lwires.First());
  }
  else
  {
    std::cout << "Not yet implemented : nbwire >= 2" << std::endl;
  }
}

//=================================================================================================

static Standard_Boolean Contains(const ShapeList& ll, const TopoShape& s)
{
  TopTools_ListIteratorOfListOfShape itl;
  for (itl.Initialize(ll); itl.More(); itl.Next())
  {
    if (itl.Value().IsSame(s))
    {
      return Standard_True;
    }
  }
  return Standard_False;
}

//=================================================================================================

static TopoEdge NewEdge(const TopoEdge&          edg,
                           const TopoFace&          F,
                           const Handle(GeomSurface)& NewS,
                           const TopoVertex&        V1,
                           const TopoVertex&        V2)
{
  TopoEdge          NewEdg;
  Handle(GeomSurface) S1    = BRepInspector::Surface(F);
  Standard_Boolean     AppS1 = Standard_False;
  if (S1->DynamicType() != STANDARD_TYPE(GeomPlane))
  {
    AppS1 = Standard_True;
  }

  GeomInt_IntSS i2s(S1, NewS, Precision::Confusion(), Standard_True, AppS1);
  if (!i2s.IsDone() || i2s.NbLines() <= 0)
  {
    return NewEdg;
  }

  ShapeBuilder B;
  //  Standard_Real pmin, Dist, DistMin;
  Standard_Real     Dist2, Dist2Min;
  Standard_Real     prmf = 0, prml = 0;
  GeomAdaptor_Curve TheCurve;

  Standard_Integer i, k;
  Point3d           pvf = BRepInspector::Pnt(V1);
  Point3d           pvl = BRepInspector::Pnt(V2);
  for (i = 1; i <= i2s.NbLines(); i++)
  {
    TheCurve.Load(i2s.Line(i));
    Extrema_ExtPC myExtPC(pvf, TheCurve);

    if (myExtPC.IsDone())
    {
      Point3d        p1b, p2b;
      Standard_Real thepmin = TheCurve.FirstParameter();
      myExtPC.TrimmedSquareDistances(Dist2Min, Dist2, p1b, p2b);
      if (Dist2 < Dist2Min && !TheCurve.IsPeriodic())
      {
        Dist2Min = Dist2;
        thepmin  = TheCurve.LastParameter();
      }
      for (k = 1; k <= myExtPC.NbExt(); k++)
      {
        Dist2 = myExtPC.SquareDistance(k);
        if (Dist2 < Dist2Min)
        {
          Dist2Min = Dist2;
          thepmin  = myExtPC.Point(k).Parameter();
        }
      }

      if (Dist2Min <= Precision::SquareConfusion())
      {
        prmf = thepmin;
        myExtPC.Perform(pvl);
        if (myExtPC.IsDone())
        {
          thepmin = TheCurve.LastParameter();
          myExtPC.TrimmedSquareDistances(Dist2, Dist2Min, p1b, p2b);
          if (Dist2 < Dist2Min && !TheCurve.IsClosed())
          {
            Dist2Min = Dist2;
            thepmin  = TheCurve.FirstParameter();
          }
          for (k = 1; k <= myExtPC.NbExt(); k++)
          {
            Dist2 = myExtPC.SquareDistance(k);
            if (Dist2 < Dist2Min)
            {
              Dist2Min = Dist2;
              thepmin  = myExtPC.Point(k).Parameter();
            }
          }

          if (Dist2Min <= Precision::SquareConfusion())
          {
            prml = thepmin;
            break;
          }
        }
      }
    }
  }

  if (i <= i2s.NbLines())
  {
    Standard_Boolean          rev  = Standard_False;
    TopoVertex             Vf   = V1;
    TopoVertex             Vl   = V2;
    const Handle(GeomCurve3d)& Cimg = i2s.Line(i);
    Handle(GeomCurve2d)      Cimg2d;
    if (AppS1)
    {
      Cimg2d = i2s.LineOnS1(i);
    }

    if (Cimg->IsPeriodic())
    {

      Standard_Real period = Cimg->Period();
      Standard_Real imf    = Cimg->FirstParameter();
      Standard_Real iml    = Cimg->LastParameter();

      Standard_Real f, l;
      BRepInspector::Range(edg, f, l);
      Standard_Real delt  = l - f;
      Standard_Real delt1 = Abs(prml - prmf);
      Standard_Real delt2 = Abs(period - delt1);

      if (delt1 == 0 || delt2 == 0)
      {
        //	prmf = 0;
        //	prml = period;
        prmf = imf;
        prml = iml;
      }
      else
      {
        if (Abs(delt1 - delt) > Abs(delt2 - delt))
        {
          // le bon ecart est delt2...
          if (prml > prmf)
          {
            if (prml < iml)
            {
              prmf += period;
            }
            else
            {
              prml -= period;
            }
          }
          else
          {
            if (prmf < iml)
            {
              prml += period;
            }
            else
            {
              prmf -= period;
            }
          }
        }
        else if (Abs(delt1 - delt) < Abs(delt2 - delt))
        {
          if (prmf >= iml && prml >= iml)
          {
            prmf -= period;
            prml -= period;
          }
          else if (prmf <= imf && prml <= imf)
          {
            prmf += period;
            prml += period;
          }
        }
        else
        { // egalite; on priveligie l'ordre f,l
          if (prmf > prml)
          {
            prmf -= period;
          }
          if (prmf >= iml && prml >= iml)
          {
            prmf -= period;
            prml -= period;
          }
          else if (prmf <= imf && prml <= imf)
          {
            prmf += period;
            prml += period;
          }
        }
      }
#ifdef OCCT_DEBUG
      Standard_Real ptol = Precision::PConfusion();
      if (prmf < imf - ptol || prmf > iml + ptol || prml < imf - ptol || prml > iml + ptol)
      {
        std::cout << "Ca ne va pas aller" << std::endl;
      }
#endif
    }

    if (S1->IsUPeriodic())
    {

      Standard_Real speriod = S1->UPeriod();
      //      Standard_Real f,l;
      gp_Pnt2d pf, pl;
      pf = Cimg2d->Value(prmf);
      pl = Cimg2d->Value(prml);

      Standard_Real Uf   = pf.X();
      Standard_Real Ul   = pl.X();
      Standard_Real ptra = 0.0;

      Standard_Real Ustart = Min(Uf, Ul);
      while (Ustart < -Precision::PConfusion())
      {
        Ustart += speriod;
        ptra += speriod;
      }
      while (Ustart > speriod - Precision::PConfusion())
      {
        Ustart -= speriod;
        ptra -= speriod;
      }
      if (ptra != 0)
      {
        Cimg2d->Translate(gp_Vec2d(ptra, 0.));
      }
    }
    if (prmf < prml)
    {
      Vf.Orientation(TopAbs_FORWARD);
      Vl.Orientation(TopAbs_REVERSED);
    }
    else
    {
      Vf.Orientation(TopAbs_REVERSED);
      Vl.Orientation(TopAbs_FORWARD);
      rev = Standard_True;
    }

    B.MakeEdge(NewEdg, Cimg, Precision::Confusion());

    B.Add(NewEdg, Vf);
    B.Add(NewEdg, Vl);
    B.UpdateVertex(Vf, prmf, NewEdg, Precision::Confusion());
    B.UpdateVertex(Vl, prml, NewEdg, Precision::Confusion());
    if (AppS1)
    {
      B.UpdateEdge(NewEdg, Cimg2d, F, Precision::Confusion());
    }

    if (rev)
    {
      NewEdg.Orientation(TopAbs_REVERSED);
    }
  }
  return NewEdg;
}
