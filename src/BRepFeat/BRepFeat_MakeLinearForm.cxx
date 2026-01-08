// Created on: 1997-04-14
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
#include <BRep_Tool.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepBuilderAPI.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepExtrema_ExtCF.hxx>
#include <BRepExtrema_ExtPC.hxx>
#include <BRepExtrema_ExtPF.hxx>
#include <BRepFeat.hxx>
#include <BRepFeat_MakeLinearForm.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <BRepLib_MakeFace.hxx>
#include <BRepLib_MakeVertex.hxx>
#include <BRepLib_MakeWire.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepTools.hxx>
#include <BRepTools_Modifier.hxx>
#include <BRepTools_TrsfModification.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <Geom_Curve.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <IntRes2d_IntersectionPoint.hxx>
#include <LocOpe_Gluer.hxx>
#include <LocOpe_LinearForm.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>

#ifdef OCCT_DEBUG
extern Standard_Boolean BRepFeat_GettraceFEAT();
extern Standard_Boolean BRepFeat_GettraceFEATRIB();
#endif

static void MajMap(const TopoShape&, // base
                   const LocOpe_LinearForm&,
                   TopTools_DataMapOfShapeListOfShape&, // myMap
                   TopoShape&,                       // myFShape
                   TopoShape&);                      // myLShape

static void SetGluedFaces(const TopTools_DataMapOfShapeListOfShape& theSlmap,
                          LocOpe_LinearForm&,
                          TopTools_DataMapOfShapeShape&);

//=================================================================================================

void BRepFeat_MakeLinearForm::Init(const TopoShape&       Sbase,
                                   const TopoWire&        W,
                                   const Handle(GeomPlane)& Plane1,
                                   const Vector3d&             Direc,
                                   const Vector3d&             Direc1,
                                   const Standard_Integer    Mode,
                                   const Standard_Boolean    Modify)
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEAT();
  if (trc)
    std::cout << "BRepFeat_MakeLinearForm::Init" << std::endl;
#endif
  Standard_Boolean RevolRib = Standard_False;
  Done();
  myGenerated.Clear();

  // modify = 0 if there is no intention to make sliding
  //        = 1 if one tries to make sliding
  Standard_Boolean Sliding = Modify;
  myLFMap.Clear();

  myShape.Nullify();
  myMap.Clear();
  myFShape.Nullify();
  myLShape.Nullify();
  mySbase = Sbase;
  mySkface.Nullify();
  myPbase.Nullify();

  myGShape.Nullify();
  mySUntil.Nullify();
  myListOfEdges.Clear();
  mySlface.Clear();

  TopoShape aLocalShapeW = W.Oriented(TopAbs_FORWARD);
  myWire                    = TopoDS::Wire(aLocalShapeW);
  //  myWire = TopoDS::Wire(W.Oriented(TopAbs_FORWARD));
  myDir  = Direc;
  myDir1 = Direc1;
  myPln  = Plane1;

  if (Mode == 0)
    myFuse = Standard_False;
  else // if(Mode == 1)
    myFuse = Standard_True;
#ifdef OCCT_DEBUG
  if (trc)
  {
    if (myFuse)
      std::cout << " Fuse" << std::endl;
    if (!myFuse)
      std::cout << " Cut" << std::endl;
  }
#endif

  // ---Determine Tolerance : max tolerance on parameters
  myTol = Precision1::Confusion();

  ShapeExplorer exx;
  exx.Init(myWire, TopAbs_VERTEX);
  for (; exx.More(); exx.Next())
  {
    const Standard_Real& tol = BRepInspector::Tolerance(TopoDS::Vertex(exx.Current()));
    if (tol > myTol)
      myTol = tol;
  }

  exx.Init(Sbase, TopAbs_VERTEX);
  for (; exx.More(); exx.Next())
  {
    const Standard_Real& tol = BRepInspector::Tolerance(TopoDS::Vertex(exx.Current()));
    if (tol > myTol)
      myTol = tol;
  }

  // ---Control of directions
  //    the wire should be in the rib
  Vector3d nulldir(0, 0, 0);
  if (!myDir1.IsEqual(nulldir, myTol, myTol))
  {
    Standard_Real ang = myDir1.Angle(myDir);
    if (ang != M_PI)
    {
#ifdef OCCT_DEBUG
      if (trc)
        std::cout << " Directions must be opposite" << std::endl;
#endif
      myStatusError = BRepFeat_BadDirect;
      NotDone();
      return;
    }
  }
  else
  {

// Rib is centre in the middle of translation
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << " Rib is centre" << std::endl;
#endif
    const Vector3d& DirTranslation = (Direc + Direc1) * 0.5;
    Transform3d       T;
    T.SetTranslation(DirTranslation);
    BRepBuilderAPI_Transform trf(T);
    trf.Perform(myWire);
    myWire = TopoDS::Wire(trf.Shape());
    myDir  = Direc - DirTranslation;
    myDir1 = Direc1 - DirTranslation;
    myPln->Transform(T);
  }

  // ---Calculate bounding box
  ShapeBuilder BB;

  ShapeList theList;

  TopoShape U;
  U.Nullify();
  Point3d        FirstCorner, LastCorner;
  Standard_Real bnd = HeightMax(mySbase, U, FirstCorner, LastCorner);
  myBnd             = bnd;

  BoxMaker Bndbox(FirstCorner, LastCorner);
  TopoSolid        BndBox = Bndbox.Solid();

  // ---Construction of the face workplane (section bounding box)
  BRepLib_MakeFace PlaneF(myPln->Pln(), -6. * myBnd, 6. * myBnd, -6. * myBnd, 6. * myBnd);
  TopoFace      PlaneFace = TopoDS::Face(PlaneF.Shape());

  BooleanCommon PlaneS(BndBox, PlaneFace);
  ShapeExplorer    EXP;
  TopoShape       PlaneSect = PlaneS.Shape();
  EXP.Init(PlaneSect, TopAbs_WIRE);
  TopoWire      www = TopoDS::Wire(EXP.Current());
  BRepLib_MakeFace Bndface(myPln->Pln(), www, Standard_True);
  TopoFace      BndFace = TopoDS::Face(Bndface.Shape());

  // ---Find support faces of the rib
  TopoEdge   FirstEdge, LastEdge;
  TopoFace   FirstFace, LastFace;
  TopoVertex FirstVertex, LastVertex;

  Standard_Boolean OnFirstFace   = Standard_False;
  Standard_Boolean OnLastFace    = Standard_False;
  Standard_Boolean PtOnFirstEdge = Standard_False;
  Standard_Boolean PtOnLastEdge  = Standard_False;
  TopoEdge      OnFirstEdge, OnLastEdge;
  OnFirstEdge.Nullify();
  OnLastEdge.Nullify();

  Standard_Boolean Data = ExtremeFaces(RevolRib,
                                       myBnd,
                                       myPln,
                                       FirstEdge,
                                       LastEdge,
                                       FirstFace,
                                       LastFace,
                                       FirstVertex,
                                       LastVertex,
                                       OnFirstFace,
                                       OnLastFace,
                                       PtOnFirstEdge,
                                       PtOnLastEdge,
                                       OnFirstEdge,
                                       OnLastEdge);

  if (!Data)
  {
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << " No Extreme faces" << std::endl;
#endif
    myStatusError = BRepFeat_NoExtFace;
    NotDone();
    return;
  }

  // ---Proofing Point for the side of the wire to be filled - side material
  Point3d CheckPnt = CheckPoint(FirstEdge, bnd / 10., myPln);

  //  Standard_Real f, l;

  // ---Control sliding valuable
  // Many cases when the sliding is abandoned
  Standard_Integer Concavite = 3; // a priori the profile is not concave

  myFirstPnt = BRepInspector::Pnt(FirstVertex);
  myLastPnt  = BRepInspector::Pnt(LastVertex);

  // SliList : list of faces concerned by the rib
  ShapeList SliList;
  SliList.Append(FirstFace);

  if (Sliding)
  { // sliding
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << " Sliding" << std::endl;
#endif
    Sliding                = Standard_False;
    Handle(GeomSurface) s = BRepInspector::Surface(FirstFace);
    if (s->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
    {
      s = Handle(Geom_RectangularTrimmedSurface)::DownCast(s)->BasisSurface();
    }
    if (s->DynamicType() == STANDARD_TYPE(GeomPlane)
        || s->DynamicType() == STANDARD_TYPE(Geom_CylindricalSurface))
    {
      // if plane or cylinder : sliding is possible
      Sliding = Standard_True;
    }
  }

  // Control only start and end points
  // -> no control at the middle - improve
  // Controle between Surface and segment between 2 limit points
  // is too expensive - improve
  if (Sliding)
  {
    Point3d p1(myFirstPnt.X() + myDir.X(), myFirstPnt.Y() + myDir.Y(), myFirstPnt.Z() + myDir.Z());
    BRepLib_MakeEdge  ee1(myFirstPnt, p1);
    BRepExtrema_ExtCF ext1(ee1, FirstFace);
    if (ext1.NbExt() == 1
        && ext1.SquareDistance(1)
             <= BRepInspector::Tolerance(FirstFace) * BRepInspector::Tolerance(FirstFace))
    {
      Point3d p2(myLastPnt.X() + myDir.X(), myLastPnt.Y() + myDir.Y(), myLastPnt.Z() + myDir.Z());
      BRepLib_MakeEdge  ee2(myLastPnt, p2);
      BRepExtrema_ExtCF ext2(ee2, LastFace); // ExtCF : curves and surfaces
      if (ext2.NbExt() == 1
          && ext2.SquareDistance(1)
               <= BRepInspector::Tolerance(LastFace) * BRepInspector::Tolerance(LastFace))
      {
        Sliding = Standard_True;
      }
      else
      {
        Sliding = Standard_False;
      }
    }
    else
    {
      Sliding = Standard_False;
    }
  }

  if (!myDir1.IsEqual(nulldir, Precision1::Confusion(), Precision1::Confusion()))
  {
    if (Sliding)
    {
      Point3d            p1(myFirstPnt.X() + myDir1.X(),
                myFirstPnt.Y() + myDir1.Y(),
                myFirstPnt.Z() + myDir1.Z());
      BRepLib_MakeEdge  ee1(myFirstPnt, p1);
      BRepExtrema_ExtCF ext1(ee1, FirstFace);
      if (ext1.NbExt() == 1
          && ext1.SquareDistance(1)
               <= BRepInspector::Tolerance(FirstFace) * BRepInspector::Tolerance(FirstFace))
      {
        Point3d            p2(myLastPnt.X() + myDir1.X(),
                  myLastPnt.Y() + myDir1.Y(),
                  myLastPnt.Z() + myDir1.Z());
        BRepLib_MakeEdge  ee2(myLastPnt, p2);
        BRepExtrema_ExtCF ext2(ee2, LastFace);
        if (ext2.NbExt() == 1
            && ext2.SquareDistance(1)
                 <= BRepInspector::Tolerance(LastFace) * BRepInspector::Tolerance(LastFace))
        {
          Sliding = Standard_True;
        }
        else
        {
          Sliding = Standard_False;
        }
      }
      else
      {
        Sliding = Standard_False;
      }
    }
  }

  // Construct a great profile that goes till the bounding box
  // -> by tangency with the first and the last edge of the Wire
  // -> by normals to the support faces : statistically better
  // Intersect everything to find the final profile

  // ---case of sliding : construction of the profile face
  if (Sliding)
  {
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << " still Sliding" << std::endl;
#endif
    TopoFace      Prof;
    Standard_Boolean ProfileOK;
    ProfileOK = SlidingProfile(Prof,
                               RevolRib,
                               myTol,
                               Concavite,
                               myPln,
                               BndFace,
                               CheckPnt,
                               FirstFace,
                               LastFace,
                               FirstVertex,
                               LastVertex,
                               FirstEdge,
                               LastEdge);

    if (!ProfileOK)
    {
#ifdef OCCT_DEBUG
      if (trc)
      {
        std::cout << "Not computable" << std::endl;
        std::cout << "Face profile not computable" << std::endl;
      }
#endif
      myStatusError = BRepFeat_NoFaceProf;
      NotDone();
      return;
    }

    // ---Propagation on faces of the initial shape
    // to find the faces concerned by the rib
    Standard_Boolean falseside = Standard_True;
    Sliding                    = Propagate(SliList, Prof, myFirstPnt, myLastPnt, falseside);
    // Control if there is everything required to have the material at the proper side
    if (falseside == Standard_False)
    {
#ifdef OCCT_DEBUG
      std::cout << "Verify plane and wire orientation" << std::endl;
#endif
      myStatusError = BRepFeat_FalseSide;
      NotDone();
      return;
    }
  }

  // ---Generation of the base of the rib profile

  TopoWire w;
  BB.MakeWire(w);
  TopoEdge   thePreviousEdge;
  TopoVertex theFV;
  thePreviousEdge.Nullify();

  // calculate the number of edges to fill the map
  Standard_Integer counter = 1;

  // ---case of sliding
  if (Sliding && !myListOfEdges.IsEmpty())
  {
    BRepTools_WireExplorer EX1(myWire);
    for (; EX1.More(); EX1.Next())
    {
      const TopoEdge& E = EX1.Current();
      if (!myLFMap.IsBound(E))
      {
        ShapeList theTmpList;
        myLFMap.Bind(E, theTmpList);
      }
      if (E.IsSame(FirstEdge))
      {
        Standard_Real      f, l;
        Handle(GeomCurve3d) cc = BRepInspector::Curve(E, f, l);
        cc                    = new Geom_TrimmedCurve(cc, f, l);
        Point3d pt;
        if (!FirstEdge.IsSame(LastEdge))
        {
          pt = BRepInspector::Pnt(TopExp1::LastVertex(E, Standard_True));
        }
        else
        {
          pt                 = myLastPnt;
          Standard_Real fpar = IntPar(cc, myFirstPnt);
          Standard_Real lpar = IntPar(cc, pt);
          if (fpar > lpar)
          {
            cc = cc->Reversed();
          }
        }
        TopoEdge ee1;
        if (thePreviousEdge.IsNull())
        {
          BRepLib_MakeVertex v1(myFirstPnt);
          BRepLib_MakeVertex v2(pt);
          BRepLib_MakeEdge   e(cc, v1, v2);
          ee1 = TopoDS::Edge(e.Shape());
        }
        else
        {
          const TopoVertex& v1 = TopExp1::LastVertex(thePreviousEdge, Standard_True);
          BRepLib_MakeVertex   v2(pt);

          BRepLib_MakeEdge e(cc, v1, v2);
          ee1 = TopoDS::Edge(e.Shape());
        }
        TopoShape aLocalShape = ee1.Oriented(E.Orientation());
        ee1                      = TopoDS::Edge(aLocalShape);
        //	ee1 = TopoDS::Edge(ee1.Oriented(E.Orientation()));
        if (counter == 1)
          theFV = TopExp1::FirstVertex(ee1, Standard_True);
        myLFMap(E).Append(ee1);
        BB.Add(w, ee1);
        thePreviousEdge = ee1;
        counter++;
        EX1.Next();
        break;
      }
    }

    // Case of several edges
    if (!FirstEdge.IsSame(LastEdge))
    {
      for (; EX1.More(); EX1.Next())
      {
        const TopoEdge& E = EX1.Current();
        if (!myLFMap.IsBound(E))
        {
          ShapeList thelist1;
          myLFMap.Bind(E, thelist1);
        }
        theList.Append(E);
        Standard_Real f, l;
        if (!E.IsSame(LastEdge))
        {
          Handle(GeomCurve3d) ccc = BRepInspector::Curve(E, f, l);
          TopoVertex      v1, v2;
          if (!thePreviousEdge.IsNull())
          {
            v1 = TopExp1::LastVertex(thePreviousEdge, Standard_True);
            v2 = TopExp1::LastVertex(E, Standard_True);
          }
          else
          {
            //	    v1 = TopExp1::LastVertex(E,Standard_True);
            v1 = TopExp1::FirstVertex(E, Standard_True);
            v2 = TopExp1::LastVertex(E, Standard_True);
          }
          BRepLib_MakeEdge E1(ccc, v1, v2);
          TopoEdge      E11         = TopoDS::Edge(E1.Shape());
          TopoShape     aLocalShape = E11.Oriented(E.Orientation());
          E11                          = TopoDS::Edge(aLocalShape);
          //	  E11 = TopoDS::Edge(E11.Oriented(E.Orientation()));
          thePreviousEdge = E11;
          myLFMap(E).Append(E11);
          BB.Add(w, E11);
          if (counter == 1)
            theFV = TopExp1::FirstVertex(E11, Standard_True);
          counter++;
        }
        else
        {
          Handle(GeomCurve3d) cc = BRepInspector::Curve(E, f, l);
          Point3d             pf = BRepInspector::Pnt(TopExp1::FirstVertex(E, Standard_True));
          Point3d             pl = myLastPnt;
          TopoEdge        ee;
          if (thePreviousEdge.IsNull())
          {
            BRepLib_MakeEdge e(cc, pf, pl);
            ee = TopoDS::Edge(e.Shape());
          }
          else
          {
            const TopoVertex& v1 = TopExp1::LastVertex(thePreviousEdge, Standard_True);
            BRepLib_MakeVertex   v2(pl);
            BRepLib_MakeEdge     e(cc, v1, v2);
            ee = TopoDS::Edge(e.Shape());
          }
          TopoShape aLocalShape = ee.Oriented(E.Orientation());
          ee                       = TopoDS::Edge(aLocalShape);
          //	  ee = TopoDS::Edge(ee.Oriented(E.Orientation()));
          BB.Add(w, ee);
          myLFMap(E).Append(ee);
          if (counter == 1)
            theFV = TopExp1::FirstVertex(ee, Standard_True);
          thePreviousEdge = ee;
          counter++;
          break;
        }
      }
    }

    TopTools_ListIteratorOfListOfShape it(myListOfEdges);
    Standard_Boolean                   FirstOK = Standard_False;
    Standard_Boolean                   LastOK  = Standard_False;

    Point3d               theLastPnt = myLastPnt;
    Standard_Integer     sens       = 0;
    TopoEdge          theEdge, theLEdge, theFEdge;
    Standard_Integer     counter1 = counter;
    ShapeList NewListOfEdges;
    NewListOfEdges.Clear();
    while (!FirstOK)
    {
      const TopoEdge&        edg = TopoDS::Edge(it.Value());
      Point3d                    fp, lp;
      Standard_Real             f, l;
      Handle(GeomCurve3d)        ccc = BRepInspector::Curve(edg, f, l);
      Handle(Geom_TrimmedCurve) cc  = new Geom_TrimmedCurve(ccc, f, l);
      if (edg.Orientation() == TopAbs_REVERSED)
        cc->Reverse();

      fp                 = cc->Value(cc->FirstParameter());
      lp                 = cc->Value(cc->LastParameter());
      Standard_Real dist = fp.Distance(theLastPnt);
      if (dist <= myTol)
      {
        sens   = 1;
        LastOK = Standard_True;
      }
      else
      {
        dist = lp.Distance(theLastPnt);
        if (dist <= myTol)
        {
          sens   = 2;
          LastOK = Standard_True;
          cc->Reverse();
        }
      }
      Standard_Integer FirstFlag = 0;
      if (sens == 1 && lp.Distance(myFirstPnt) <= myTol)
      {
        FirstOK   = Standard_True;
        FirstFlag = 1;
      }
      else if (sens == 2 && fp.Distance(myFirstPnt) <= myTol)
      {
        FirstOK   = Standard_True;
        FirstFlag = 2;
      }

      if (LastOK)
      {
        TopoEdge   eeee;
        Standard_Real fpar = cc->FirstParameter();
        Standard_Real lpar = cc->LastParameter();
        if (!FirstOK)
        {
          if (thePreviousEdge.IsNull())
          {
            BRepLib_MakeEdge e(cc, fpar, lpar);
            eeee = TopoDS::Edge(e.Shape());
          }
          else
          {
            const TopoVertex& v1 = TopExp1::LastVertex(thePreviousEdge, Standard_True);
            BB.UpdateVertex(v1, dist);
            BRepLib_MakeVertex v2(cc->Value(lpar));
            TopoVertex      nv = v2.Vertex();
            BRepLib_MakeEdge   e(cc, v1, nv);
            eeee = TopoDS::Edge(e.Shape());
          }
        }
        else
        {
          if (thePreviousEdge.IsNull())
          {
            BRepLib_MakeVertex v1(cc->Value(fpar));
            BRepLib_MakeEdge   e(cc, v1, theFV);
            eeee = TopoDS::Edge(e.Shape());
          }
          else
          {
            const TopoVertex& v1 = TopExp1::LastVertex(thePreviousEdge, Standard_True);
            BRepLib_MakeEdge     e(cc, v1, theFV);
            eeee = TopoDS::Edge(e.Shape());
          }
        }

        thePreviousEdge = eeee;
        BB.Add(w, eeee);
        if (counter == 1)
          theFV = TopExp1::FirstVertex(eeee, Standard_True);
        counter1++;
        NewListOfEdges.Append(edg);
        theEdge = eeee;

        if (dist <= myTol)
          theFEdge = edg;
        theLastPnt = BRepInspector::Pnt(TopExp1::LastVertex(theEdge, Standard_True));
      }

      if (FirstFlag == 1)
      {
        theLEdge = edg;
      }
      else if (FirstFlag == 2)
      {
        theLEdge = theEdge;
      }

      if (LastOK)
      {
        myListOfEdges.Remove(it);
        it.Initialize(myListOfEdges);
        LastOK = Standard_False;
      }
      else if (it.More())
        it.Next();
      else
      {
        Sliding = Standard_False;
        break;
      }
      sens = 0;
    }

    TopTools_DataMapOfShapeListOfShape SlidMap;
    SlidMap.Clear();

    if (Sliding && counter1 > counter)
    {
      TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itm;
      ShapeExplorer                                     EX2(w, TopAbs_EDGE);
      Standard_Integer                                    ii = 0;
      for (; EX2.More(); EX2.Next())
      {
        const TopoEdge& E = TopoDS::Edge(EX2.Current());
        ii++;
        if (ii >= counter && ii <= counter1)
        {
          it.Initialize(NewListOfEdges);
          Standard_Integer jj = 0;
          for (; it.More(); it.Next())
          {
            const TopoEdge& e2 = TopoDS::Edge(it.Value());
            jj++;
            if (jj == (ii - counter + 1))
            {
              itm.Initialize(mySlface);
              for (; itm.More(); itm.Next())
              {
                const TopoFace&                 fac  = TopoDS::Face(itm.Key1());
                const ShapeList&        ledg = itm.Value();
                TopTools_ListIteratorOfListOfShape itedg(ledg);
                // Standard_Integer iiii = 0;
                for (; itedg.More(); itedg.Next())
                {
                  const TopoEdge& e1 = TopoDS::Edge(itedg.Value());
                  if (e1.IsSame(e2))
                  {
                    if (!SlidMap.IsBound(fac))
                    {
                      ShapeList thelist2;
                      SlidMap.Bind(fac, thelist2);
                    }
                    SlidMap(fac).Append(E);
                  }
                }
              }
            }
          }
        }
      }
    }

    mySlface.Clear();
    mySlface = SlidMap;
  }

  // ---Arguments of LocOpe_LinearForm : arguments of the prism sliding
  if (Sliding)
  {
    TopoFace F;
    BB.MakeFace(F, myPln, myTol);
    w.Closed(BRepInspector::IsClosed(w));
    BB.Add(F, w);
    //    BRepLib_MakeFace F(myPln->Pln(),w, Standard_True);
    mySkface = F;
    myPbase  = mySkface;
    mySUntil.Nullify();
  }

  // ---Case without sliding : construction of the profile face
  if (!Sliding)
  {
#ifdef OCCT_DEBUG
    if (trc)
    {
      if (Modify)
        std::cout << " Sliding failure" << std::endl;
      std::cout << " no Sliding" << std::endl;
    }
#endif
    TopoFace      Prof;
    Standard_Boolean ProfileOK;
    ProfileOK = NoSlidingProfile(Prof,
                                 RevolRib,
                                 myTol,
                                 Concavite,
                                 myPln,
                                 bnd,
                                 BndFace,
                                 CheckPnt,
                                 FirstFace,
                                 LastFace,
                                 FirstVertex,
                                 LastVertex,
                                 FirstEdge,
                                 LastEdge,
                                 OnFirstFace,
                                 OnLastFace);

    if (!ProfileOK)
    {
#ifdef OCCT_DEBUG
      if (trc)
      {
        std::cout << "Not computable" << std::endl;
        std::cout << " Face profile not computable" << std::endl;
      }
#endif
      myStatusError = BRepFeat_NoFaceProf;
      NotDone();
      return;
    }

    // ---Propagation on faces of the initial shape
    // to find the faces concerned by the rib
    Standard_Boolean falseside = Standard_True;
    Propagate(SliList, Prof, myFirstPnt, myLastPnt, falseside);
    // Control if there is everything required to have the material at the proper side
    if (falseside == Standard_False)
    {
#ifdef OCCT_DEBUG
      std::cout << "Verify plane and wire orientation" << std::endl;
#endif
      myStatusError = BRepFeat_FalseSide;
      NotDone();
      return;
    }

    mySlface.Clear();

    TopTools_ListIteratorOfListOfShape it;
    it.Initialize(SliList);

    TopoShape comp;

    BB.MakeShell(TopoDS::Shell(comp));

    for (; it.More(); it.Next())
    {
      BB.Add(comp, it.Value());
    }
    comp.Closed(BRepInspector::IsClosed(comp));

    mySUntil = comp;

    mySkface = Prof;
    myPbase  = Prof;
  }

  mySliding = Sliding;

  ShapeExplorer exp;
  for (exp.Init(mySbase, TopAbs_FACE); exp.More(); exp.Next())
  {
    ShapeList thelist3;
    myMap.Bind(exp.Current(), thelist3);
    myMap(exp.Current()).Append(exp.Current());
  }
}

//=======================================================================
// function : Add
// purpose  : add des element de collage
//=======================================================================

void BRepFeat_MakeLinearForm::Add(const TopoEdge& E, const TopoFace& F)
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEAT();
  if (trc)
    std::cout << "BRepFeat_MakeLinearForm::Add" << std::endl;
#endif
  if (mySlface.IsEmpty())
  {
    ShapeExplorer exp;
    for (exp.Init(mySbase, TopAbs_FACE); exp.More(); exp.Next())
    {
      if (exp.Current().IsSame(F))
      {
        break;
      }
    }
    if (!exp.More())
    {
      throw Standard_ConstructionError();
    }

    if (!mySlface.IsBound(F))
    {
      ShapeList thelist;
      mySlface.Bind(F, thelist);
    }
    TopTools_ListIteratorOfListOfShape itl(mySlface(F));
    for (; itl.More(); itl.Next())
    {
      if (itl.Value().IsSame(E))
      {
        break;
      }
    }
    if (!itl.More())
    {
      mySlface(F).Append(E);
    }
  }
}

//=======================================================================
// function : Perform
// purpose  : construction of rib from a profile and the initial shape
//=======================================================================

void BRepFeat_MakeLinearForm::Perform()
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEAT();
  if (trc)
    std::cout << "BRepFeat_MakeLinearForm::Perform()" << std::endl;
#endif
  if (mySbase.IsNull() || mySkface.IsNull() || myPbase.IsNull())
  {
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << " Fields not initialized" << std::endl;
#endif
    myStatusError = BRepFeat_NotInitialized;
    NotDone();
    return;
  }

  Vector3d nulldir(0, 0, 0);

  Standard_Real Length = myDir.Magnitude() + myDir1.Magnitude();

  myGluedF.Clear();

  if (!mySUntil.IsNull())
    myPerfSelection = BRepFeat_SelectionU;
  else
    myPerfSelection = BRepFeat_NoSelection;

  Dir3d dir(myDir);
  Vector3d V = Length * dir;

  LocOpe_LinearForm theForm;

  if (myDir1.IsEqual(nulldir, Precision1::Confusion(), Precision1::Confusion()))
    theForm.Perform(myPbase, V, myFirstPnt, myLastPnt);
  else
    theForm.Perform(myPbase, V, myDir1, myFirstPnt, myLastPnt);

  TopoShape VraiForm = theForm.Shape(); // primitive of the rib

  myFacesForDraft.Append(theForm.FirstShape());
  myFacesForDraft.Append(theForm.LastShape());
  MajMap(myPbase, theForm, myMap, myFShape, myLShape); // management of descendants

  ShapeExplorer exx(myPbase, TopAbs_EDGE);
  for (; exx.More(); exx.Next())
  {
    const TopoEdge& e = TopoDS::Edge(exx.Current());
    if (!myMap.IsBound(e))
    {
#ifdef OCCT_DEBUG
      if (trc)
        std::cout << " Sliding face not in Base shape" << std::endl;
#endif
      myStatusError = BRepFeat_IncSlidFace;
      NotDone();
      return;
    }
  }

  myGShape = VraiForm;
  SetGluedFaces(mySlface, theForm, myGluedF); // management of sliding faces

  if (!myGluedF.IsEmpty() && !mySUntil.IsNull())
  {
#ifdef OCCT_DEBUG
    if (trc)
    {
      std::cout << "The case is not computable" << std::endl;
      std::cout << " Glued faces not empty and Until shape not null" << std::endl;
    }
#endif
    myStatusError = BRepFeat_InvShape;
    NotDone();
    return;
  }

  LFPerform();

  /*

    ShapeExplorer expr(mySbase, TopAbs_FACE);
    char nom1[20], nom2[20];
    Standard_Integer ii = 0;
    for(; expr.More(); expr.Next()) {
      ii++;
      sprintf(nom1, "faceinitial_%d", ii);
      DBRep1::Set(nom1, expr.Current());
      Standard_Integer jj = 0;
      const ShapeList& list = Modified(expr.Current());
      TopTools_ListIteratorOfListOfShape ite(list);
      for(; ite.More(); ite.Next()) {
        jj++;
        sprintf(nom2, "facemodifie_%d_%d", ii, jj);
        DBRep1::Set(nom2, ite.Value());
      }
    }

    expr.Init(myWire, TopAbs_EDGE);
    ii=0;
    for(; expr.More(); expr.Next()) {
      ii++;
      sprintf(nom1, "edgeinitial_%d", ii);
      DBRep1::Set(nom1, expr.Current());
      Standard_Integer jj = 0;
      const ShapeList& genf = Generated(expr.Current());
      TopTools_ListIteratorOfListOfShape ite(genf);
      for(; ite.More(); ite.Next()) {
        jj++;
        sprintf(nom2, "egdegeneree_%d_%d", ii, jj);
        DBRep1::Set(nom2, ite.Value());
      }
    }
  */
}

//=======================================================================
// function : Propagate
// purpose  : propagation on faces of the initial shape, find
// faces concerned by the rib
//=======================================================================
Standard_Boolean BRepFeat_MakeLinearForm::Propagate(ShapeList& SliList,
                                                    const TopoFace&    fac,
                                                    const Point3d&         Firstpnt,
                                                    const Point3d&         Lastpnt,
                                                    Standard_Boolean&     falseside)
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = BRepFeat_GettraceFEATRIB();
  if (trc)
    std::cout << "BRepFeat_MakeLinearForm::Propagate" << std::endl;
#endif
  Point3d Firstpoint = Firstpnt;
  Point3d Lastpoint  = Lastpnt;

  Standard_Boolean result = Standard_True;
  TopoFace      CurrentFace, saveFace;
  CurrentFace = TopoDS::Face(SliList.First());
  saveFace    = CurrentFace;

  Standard_Boolean LastOK = Standard_False, FirstOK = Standard_False;
  Standard_Boolean v1OK = Standard_False, v2OK = Standard_False;
  TopoVertex    v1, v2, v3, v4, ve1, ve2;

  BRepAlgoAPI_Section sect(fac, CurrentFace, Standard_False);

  sect.Approximation(Standard_True);
  sect.Build();

  ShapeExplorer  Ex;
  TopoEdge      eb, ec;
  Point3d           p1, p2;
  Standard_Real    t1 = 0., t2 = 0.;
  Standard_Boolean c1f, c2f, c1l, c2l;

  for (Ex.Init(sect.Shape(), TopAbs_EDGE); Ex.More(); Ex.Next())
  {
    ec  = TopoDS::Edge(Ex.Current());
    v1  = TopExp1::FirstVertex(ec, Standard_True);
    v2  = TopExp1::LastVertex(ec, Standard_True);
    p1  = BRepInspector::Pnt(v1);
    p2  = BRepInspector::Pnt(v2);
    t1  = BRepInspector::Tolerance(v1);
    t2  = BRepInspector::Tolerance(v2);
    c1f = p1.Distance(Firstpoint) <= t1;
    c2f = p2.Distance(Firstpoint) <= t2;
    c1l = p1.Distance(Lastpoint) <= t1;
    c2l = p2.Distance(Lastpoint) <= t2;
    if (c1f || c2f || c1l || c2l)
    {
      eb = ec;
      if (c1f || c1l)
        v1OK = Standard_True;
      if (c2f || c2l)
        v2OK = Standard_True;
      if (c1f || c2f)
        FirstOK = Standard_True;
      if (c1l || c2l)
        LastOK = Standard_True;
      break;
    }
  }

  if (eb.IsNull())
  {
    falseside = Standard_False;
    return Standard_False;
  }
  ShapeList thelist;
  mySlface.Bind(CurrentFace, thelist);
  mySlface(CurrentFace).Append(eb);

  myListOfEdges.Clear();
  myListOfEdges.Append(eb);

  // two points are on the same face.
  if (LastOK && FirstOK)
  {
    return result;
  }

  TopTools_IndexedDataMapOfShapeListOfShape mapedges;
  TopExp1::MapShapesAndAncestors(mySbase, TopAbs_EDGE, TopAbs_FACE, mapedges);
  ShapeExplorer ex;
  TopoEdge     FirstEdge;
  ShapeBuilder    BB;

  TopoVertex Vprevious;
  Point3d        ptprev;
  Standard_Real dp;

  while (!(LastOK && FirstOK))
  {
    if (v1OK)
    {
      Vprevious = v2;
      ptprev    = p2;
    }
    else
    {
      Vprevious = v1;
      ptprev    = p1;
    }

    // find edge connected to v1 or v2:
    for (ex.Init(CurrentFace, TopAbs_EDGE); ex.More(); ex.Next())
    {
      const TopoEdge& rfe = TopoDS::Edge(ex.Current());

      BRepExtrema_ExtPC projF(Vprevious, rfe);

      if (projF.IsDone() && projF.NbExt() >= 1)
      {
        Standard_Real    dist2min = RealLast();
        Standard_Integer index    = 0;
        for (Standard_Integer sol = 1; sol <= projF.NbExt(); sol++)
        {
          if (projF.SquareDistance(sol) <= dist2min)
          {
            index    = sol;
            dist2min = projF.SquareDistance(sol);
          }
        }
        if (index != 0)
        {
          if (dist2min <= BRepInspector::Tolerance(rfe) * BRepInspector::Tolerance(rfe))
          {
            FirstEdge = rfe;
            // If the edge is not perpendicular to the plane of the rib
            // it is required to set Sliding(result) to false.
            if (result)
            {
              result = Standard_False;
              ve1    = TopExp1::FirstVertex(rfe, Standard_True);
              ve2    = TopExp1::LastVertex(rfe, Standard_True);
              BRepExtrema_ExtPF perp(ve1, fac);
              if (perp.IsDone())
              {
                Point3d pe1 = perp.Point(1);
                perp.Perform(ve2, fac);
                if (perp.IsDone())
                {
                  Point3d pe2 = perp.Point(1);
                  if (pe1.Distance(pe2) <= BRepInspector::Tolerance(rfe))
                    result = Standard_True;
                }
              }
            }
            break;
          }
        }
      }
    }

    const ShapeList&        L = mapedges.FindFromKey(FirstEdge);
    TopTools_ListIteratorOfListOfShape It(L);

    for (; It.More(); It.Next())
    {
      const TopoFace& FF = TopoDS::Face(It.Value());
      if (!FF.IsSame(CurrentFace))
      {
        CurrentFace = FF;
        break;
      }
    }

    BRepAlgoAPI_Section sectf(fac, CurrentFace, Standard_False);
    sectf.Approximation(Standard_True);
    sectf.Build();

    TopoEdge edg1;
    for (Ex.Init(sectf.Shape(), TopAbs_EDGE); Ex.More(); Ex.Next())
    {
      edg1 = TopoDS::Edge(Ex.Current());
      v1   = TopExp1::FirstVertex(edg1, Standard_True);
      v2   = TopExp1::LastVertex(edg1, Standard_True);
      t1   = BRepInspector::Tolerance(v1);
      t2   = BRepInspector::Tolerance(v2);
      p1   = BRepInspector::Pnt(v1);
      p2   = BRepInspector::Pnt(v2);
      v1OK = p1.Distance(ptprev) <= t1;
      v2OK = p2.Distance(ptprev) <= t2;
      if (v1OK || v2OK)
        break;
    }

    if (v1OK)
    {
      if (!FirstOK)
      {
        dp = p2.Distance(Firstpoint);
        if (dp <= 2 * t2)
        {
          FirstOK = Standard_True;
          BB.UpdateVertex(v2, dp);
        }
      }
      if (!LastOK)
      {
        dp = p2.Distance(Lastpoint);
        if (dp <= 2 * t2)
        {
          LastOK = Standard_True;
          BB.UpdateVertex(v2, dp);
        }
      }
    }
    else if (v2OK)
    {
      if (!FirstOK)
      {
        dp = p1.Distance(Firstpoint);
        if (dp <= 2 * t1)
        {
          FirstOK = Standard_True;
          BB.UpdateVertex(v1, dp);
        }
      }
      if (!LastOK)
      {
        dp = p1.Distance(Lastpoint);
        if (dp <= 2 * t1)
        {
          LastOK = Standard_True;
          BB.UpdateVertex(v1, dp);
        }
      }
    }
    else
    {
      // end by chaining the section
      return Standard_False;
    }
    ShapeList thelist1;
    mySlface.Bind(CurrentFace, thelist1);
    mySlface(CurrentFace).Append(edg1);
    myListOfEdges.Append(edg1);
  }

  return result;
}

//=======================================================================
// function : MajMap
// purpose  : management of descendants
//=======================================================================

static void MajMap(const TopoShape&                 theB,
                   const LocOpe_LinearForm&            theP,
                   TopTools_DataMapOfShapeListOfShape& theMap,    // myMap
                   TopoShape&                       theFShape, // myFShape
                   TopoShape&                       theLShape)                       // myLShape
{
  ShapeExplorer exp(theP.FirstShape(), TopAbs_WIRE);
  if (exp.More())
  {
    theFShape = exp.Current();
    ShapeList thelist;
    theMap.Bind(theFShape, thelist);
    for (exp.Init(theP.FirstShape(), TopAbs_FACE); exp.More(); exp.Next())
    {
      theMap(theFShape).Append(exp.Current());
    }
  }

  exp.Init(theP.LastShape(), TopAbs_WIRE);
  if (exp.More())
  {
    theLShape = exp.Current();
    ShapeList thelist1;
    theMap.Bind(theLShape, thelist1);
    for (exp.Init(theP.LastShape(), TopAbs_FACE); exp.More(); exp.Next())
    {
      theMap(theLShape).Append(exp.Current());
    }
  }

  for (exp.Init(theB, TopAbs_EDGE); exp.More(); exp.Next())
  {
    if (!theMap.IsBound(exp.Current()))
    {
      ShapeList thelist2;
      theMap.Bind(exp.Current(), thelist2);
      theMap(exp.Current()) = theP.Shapes(exp.Current());
    }
  }
}

//=======================================================================
// function : SetGluedFaces
// purpose  : management of faces of gluing
//=======================================================================

static void SetGluedFaces(const TopTools_DataMapOfShapeListOfShape& theSlmap,
                          LocOpe_LinearForm&                        thePrism,
                          TopTools_DataMapOfShapeShape&             theMap)
{
  // Slidings
  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itm(theSlmap);
  if (!theSlmap.IsEmpty())
  {
    for (; itm.More(); itm.Next())
    {
      const TopoFace&                 fac  = TopoDS::Face(itm.Key1());
      const ShapeList&        ledg = itm.Value();
      TopTools_ListIteratorOfListOfShape it;
      for (it.Initialize(ledg); it.More(); it.Next())
      {
        const ShapeList& gfac = thePrism.Shapes(it.Value());
        if (gfac.Extent() != 1)
        {
#ifdef OCCT_DEBUG
          std::cout << "Pb SetGluedFace" << std::endl;
#endif
        }
        theMap.Bind(gfac.First(), fac);
      }
    }
  }
}
