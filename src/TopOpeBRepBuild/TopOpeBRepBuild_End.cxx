// Created on: 1993-06-14
// Created by: Jean Yves LEBEY
// Copyright (c) 1993-1999 Matra Datavision
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

#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Geom_Curve.hxx>
#include <gp_Pnt.hxx>
#include <Standard_ProgramError.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopOpeBRepBuild_define.hxx>
#include <TopOpeBRepBuild_GTopo.hxx>
#include <TopOpeBRepDS_DataMapIteratorOfDataMapOfShapeListOfShapeOn1State.hxx>
#include <TopOpeBRepDS_HDataStructure.hxx>
#include <TopOpeBRepTool_2d.hxx>
#include <TopOpeBRepTool_FuseEdges.hxx>
#include <TopOpeBRepTool_ShapeExplorer.hxx>

// #include <BRepAdaptor_Curve2d.hxx>
#ifdef OCCT_DEBUG
extern Standard_Boolean TopOpeBRepBuild_GetcontextNOFE();
#endif

//=================================================================================================

void TopOpeBRepBuild_Builder::End()
{
  const TopOpeBRepDS_DataStructure& BDS = myDataStructure->DS();
  {
    // recodage de la continuite (edge,(f1,f2)) != C0 perdue lors
    // du changement de surface-support d'une arete non decoupee
    // d'une face tangente.
    for (Standard_Integer i = 1; i <= 2; i++)
    {
      TopoShape S;
      TopAbs_State sta = TopAbs_UNKNOWN;
      if (i == 1)
      {
        S   = myShape1;
        sta = myState1;
      }
      else if (i == 2)
      {
        S   = myShape2;
        sta = myState2;
      }

      ShapeExplorer exs;
      for (exs.Init(S, TopAbs_SHELL); exs.More(); exs.Next())
      {
        //      for (ShapeExplorer exs(S,TopAbs_SHELL);exs.More();exs.Next()) {
        const TopoShape& SH       = exs.Current();
        Standard_Boolean    SHhassha = BDS.HasShape(SH);
        if (!SHhassha)
          continue;

        Standard_Boolean Fhassam = Standard_False;
        ShapeExplorer  exf;
        for (exf.Init(SH, TopAbs_FACE); exf.More(); exf.Next())
        {
          //	for (ShapeExplorer exf(SH,TopAbs_FACE);exf.More(); exf.Next()) {
          Fhassam = myDataStructure->HasSameDomain(exf.Current());
          if (Fhassam)
            break;
        }
        if (!Fhassam)
          continue;

        TopTools_IndexedDataMapOfShapeListOfShape M;
        TopExp1::MapShapesAndAncestors(S, TopAbs_EDGE, TopAbs_FACE, M);
        Standard_Integer nE = M.Extent();
        for (Standard_Integer iE = 1; iE <= nE; iE++)
        {
          const TopoEdge& E = TopoDS::Edge(M.FindKey(iE));
          if (IsSplit(E, sta))
            continue;
          const ShapeList& LF = M.FindFromIndex(iE);
          if (LF.Extent() < 2)
            continue;

          // NYI : > 2 faces connexes par E : iterer sur tous les couples
          TopTools_ListIteratorOfListOfShape itLF(LF);
          const TopoFace&                 F1 = TopoDS::Face(itLF.Value());
          itLF.Next();
          const TopoFace& F2 = TopoDS::Face(itLF.Value());
          GeomAbs_Shape      C  = BRepInspector::Continuity(E, F1, F2);
          if (C == GeomAbs_C0)
            continue;

          Standard_Boolean F1hassam = myDataStructure->HasSameDomain(F1);
          Standard_Boolean F2hassam = myDataStructure->HasSameDomain(F2);
          if (!F1hassam && !F2hassam)
            continue;

          Standard_Boolean F1issplit = IsSplit(F1, sta);
          Standard_Boolean F2issplit = IsSplit(F2, sta);
          F1issplit &= (Splits(F1, sta).Extent() != 0);
          F2issplit &= (Splits(F2, sta).Extent() != 0);
          if (!F1issplit && !F2issplit)
            continue;

          TopoFace FF1 = F1, FF2 = F2;
          for (Standard_Integer ii = 1; ii <= 2; ii++)
          {
            if ((ii == 1 && !F1issplit) || (ii == 2 && !F2issplit))
              continue;
            TopoFace F;
            if (ii == 1)
              F = F1;
            else
              F = F2;
            Standard_Boolean                   f = Standard_False;
            TopTools_ListIteratorOfListOfShape it;
            for (it.Initialize(Splits(F, sta)); it.More(); it.Next())
            {
              const TopoShape& SF = it.Value();
              if (SF.ShapeType() != TopAbs_FACE)
                continue;
              ShapeExplorer ex;
              for (ex.Init(SF, TopAbs_EDGE); ex.More(); ex.Next())
              {
                if (ex.Current().IsSame(E))
                {
                  if (ii == 1)
                    FF1 = TopoDS::Face(it.Value());
                  else
                    FF2 = TopoDS::Face(it.Value());
                  f = Standard_True;
                  break;
                }
              }
              if (f)
                break;
            }
          }
          ShapeBuilder B;
          B.Continuity(E, FF1, FF2, C);
        }
      }
    }
  }

  // M.A.J de la tolerance des vertex
  {
    // modified by NIZHNY-MKK  Fri Oct  6 16:13:33 2000.BEGIN
    TopTools_MapOfShape                aMapOfNewEdges, aMapOfNewVertices;
    TopTools_ListIteratorOfListOfShape anIt;
    Standard_Integer                   iteratorofnewshape = 0;
    for (iteratorofnewshape = 1; iteratorofnewshape <= myDataStructure->NbCurves();
         iteratorofnewshape++)
    {
      for (anIt.Initialize(NewEdges(iteratorofnewshape)); anIt.More(); anIt.Next())
      {
        aMapOfNewEdges.Add(anIt.Value());
      }
    }
    for (iteratorofnewshape = 1; iteratorofnewshape <= myDataStructure->NbPoints();
         iteratorofnewshape++)
    {
      aMapOfNewVertices.Add(NewVertex(iteratorofnewshape));
    }
    // modified by NIZHNY-MKK  Fri Oct  6 16:13:36 2000.END

    TopoCompound R;
    ShapeBuilder    B;
    B.MakeCompound(R);
    const ShapeList&        lmergesha1 = Merged(myShape1, myState1);
    TopTools_ListIteratorOfListOfShape it(lmergesha1);
    for (; it.More(); it.Next())
      B.Add(R, it.Value());
    const ShapeList& LOES = Section();
#ifdef OCCT_DEBUG
//    Standard_Integer nLOES = LOES.Extent();
#endif

    TopTools_IndexedDataMapOfShapeListOfShape idmoelof;
    TopExp1::MapShapesAndAncestors(R, TopAbs_EDGE, TopAbs_FACE, idmoelof);
    TopTools_IndexedDataMapOfShapeListOfShape idmovloe;
    TopExp1::MapShapesAndUniqueAncestors(R, TopAbs_VERTEX, TopAbs_EDGE, idmovloe);
    TopTools_IndexedDataMapOfShapeListOfShape idmovloes;
    for (TopTools_ListIteratorOfListOfShape I(LOES); I.More(); I.Next())
      TopExp1::MapShapesAndAncestors(I.Value(), TopAbs_VERTEX, TopAbs_EDGE, idmovloes);
    Standard_Integer iv, nv = idmovloe.Extent();
    for (iv = 1; iv <= nv; iv++)
    {
      Standard_Integer     nP1  = 0;
      const TopoVertex& V    = TopoDS::Vertex(idmovloe.FindKey(iv));
      Standard_Boolean     isbe = idmovloes.Contains(V);
      if (!isbe)
        continue;

      const ShapeList& loe = idmovloe.FindFromIndex(iv);

#ifdef OCCT_DEBUG
//      Standard_Integer nloe = loe.Extent();
#endif
      TopTools_ListIteratorOfListOfShape iloe;
      for (iloe.Initialize(loe); iloe.More(); iloe.Next())
      {
        const TopoEdge&          E    = TopoDS::Edge(iloe.Value());
        const ShapeList& lof  = idmoelof.FindFromKey(E);
        Standard_Integer            nlof = lof.Extent();
        nP1 += nlof + 1;
      }

      TColgp_Array1OfPnt TP(1, nP1);
      Standard_Integer   nP2 = 0;
      for (iloe.Initialize(loe); iloe.More(); iloe.Next())
      {
        const TopoEdge& E  = TopoDS::Edge(iloe.Value());
        Standard_Real      pv = BRepInspector::Parameter(V, E);
        Point3d             Pv;
        Standard_Real      f, l;
        Handle(GeomCurve3d) C3D = BRepInspector::Curve(E, f, l);
        if (!C3D.IsNull())
        {
          Pv        = C3D->Value(pv);
          TP(++nP2) = Pv;
        }
        const ShapeList& lof = idmoelof.FindFromKey(E);
#ifdef OCCT_DEBUG
//	Standard_Integer nlof = lof.Extent();
#endif
        for (TopTools_ListIteratorOfListOfShape ilof(lof); ilof.More(); ilof.Next())
        {
          const TopoFace&   F = TopoDS::Face(ilof.Value());
          Standard_Real        tolpc;
          Standard_Boolean     pcf = FC2D_HasCurveOnSurface(E, F);
          Handle(GeomCurve2d) C2D;
          if (!pcf)
          {
            C2D = FC2D_CurveOnSurface(E, F, f, l, tolpc);
            if (C2D.IsNull())
              throw Standard_ProgramError("TopOpeBRepBuild_Builder::End 1");
            Standard_Real tolE = BRepInspector::Tolerance(E);
            Standard_Real tol  = Max(tolE, tolpc);
            B.UpdateEdge(E, C2D, F, tol);
          }
          C2D                    = FC2D_CurveOnSurface(E, F, f, l, tolpc);
          gp_Pnt2d            P2 = C2D->Value(pv);
          BRepAdaptor_Surface BAS(F, Standard_False);
          Pv        = BAS.Value(P2.X(), P2.Y());
          TP(++nP2) = Pv;
          // modified by NIZHNY-MKK  Fri Sep 29 16:08:28 2000.BEGIN
          if (aMapOfNewEdges.Contains(E))
          {
            Standard_Real anEdgeTol = BRepInspector::Tolerance(E);
            Standard_Real aFaceTol  = BRepInspector::Tolerance(F);
            if (anEdgeTol < aFaceTol)
              B.UpdateEdge(E, aFaceTol);
          }
          // modified by NIZHNY-MKK  Fri Sep 29 16:08:32 2000.END
        }
        // modified by NIZHNY-MKK  Fri Sep 29 16:54:08 2000.BEGIN
        if (aMapOfNewVertices.Contains(V))
        {
          Standard_Real aVertexTol = BRepInspector::Tolerance(V);
          Standard_Real anEdgeTol  = BRepInspector::Tolerance(E);
          if (aVertexTol < anEdgeTol)
            B.UpdateVertex(V, anEdgeTol);
        }
        // modified by NIZHNY-MKK  Fri Sep 29 16:54:12 2000.END
      }

      Standard_Real newtol = BRepInspector::Tolerance(V);
      Box2       BOX;
      Point3d        Pv = BRepInspector::Pnt(V);
      BOX.Set(Pv);
      for (Standard_Integer i = 1; i <= nP2; i++)
      {
        const Point3d& Pi = TP(i);
        BOX.Update(Pi.X(), Pi.Y(), Pi.Z());
      }
      Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
      BOX.Get(aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);
      Point3d        P1(aXmin, aYmin, aZmin);
      Point3d        P2(aXmax, aYmax, aZmax);
      Standard_Real d = P1.Distance(P2);
      if (d > newtol)
      {
#ifdef OCCT_DEBUG
        std::cout << "\npoint P" << iv << " " << Pv.X() << " " << Pv.Y() << " " << Pv.Z()
                  << std::endl;
        std::cout << "TopOpeBRepBuild_Builder::End BOX newtol " << newtol << " -> " << d
                  << std::endl;
#endif
        newtol = d;
        B.UpdateVertex(V, newtol);
      }
    }
  }

  Standard_Boolean makeFE = Standard_True;
#ifdef OCCT_DEBUG
  makeFE = !TopOpeBRepBuild_GetcontextNOFE();
#endif

  if (makeFE)
  {
    //    TopAbs_State state = myState1;
    ShapeList& ls = ChangeMerged(myShape1, myState1);
    for (TopTools_ListIteratorOfListOfShape itls(ls); itls.More(); itls.Next())
    {
      TopoShape&            SFE = itls.ChangeValue();
      TopOpeBRepTool_FuseEdges FE(SFE);

      // avoid fusing old edges
      TopTools_IndexedMapOfShape mapOldEdges;
      TopExp1::MapShapes(myShape1, TopAbs_EDGE, mapOldEdges);
      TopExp1::MapShapes(myShape2, TopAbs_EDGE, mapOldEdges);
      FE.AvoidEdges(mapOldEdges);

      // Get List of edges that have been fused
      TopTools_DataMapOfIntegerListOfShape mle;
      FE.Edges(mle);

      Standard_Integer nle = mle.Extent();
      if (nle != 0)
      {
        FE.Perform();
        SFE = FE.Shape();

        TopTools_DataMapOfIntegerShape mre;
        TopTools_DataMapOfShapeShape   mlf;
        FE.ResultEdges(mre);
        FE.Faces(mlf);

        // edit the split to remove to edges to be fused and put them into the Merged
        //

        UpdateSplitAndMerged(mle, mre, mlf, TopAbs_IN);
        UpdateSplitAndMerged(mle, mre, mlf, TopAbs_OUT);
        UpdateSplitAndMerged(mle, mre, mlf, TopAbs_ON);
      }

    } // makeFE
  }
}

//=======================================================================
// function : UpdateSplitAndMerged
// purpose  :  edit the split to remove to edges to be fused and put them into the Merged
//=======================================================================

void TopOpeBRepBuild_Builder::UpdateSplitAndMerged(const TopTools_DataMapOfIntegerListOfShape& mle,
                                                   const TopTools_DataMapOfIntegerShape&       mre,
                                                   const TopTools_DataMapOfShapeShape&         mlf,
                                                   const TopAbs_State state)
{
  const TopOpeBRepDS_DataMapOfShapeListOfShapeOn1State&           MapSplit = MSplit(state);
  TopOpeBRepDS_DataMapIteratorOfDataMapOfShapeListOfShapeOn1State it;
  for (it.Initialize(MapSplit); it.More(); it.Next())
  {
    const TopoShape& e = it.Key();

    // For each edge of the MapSplit
    if (e.ShapeType() == TopAbs_EDGE)
    {

      // get the list of split edges.
      ShapeList& LstSplit = ChangeSplit(e, state);

      // for each edge of the list of split edges
      TopTools_ListIteratorOfListOfShape itSplitEdg;
      itSplitEdg.Initialize(LstSplit);
      while (itSplitEdg.More())
      {
        const TopoShape& edgecur = itSplitEdg.Value();

        // for each "packet" of edges to be fused
        TopTools_DataMapIteratorOfDataMapOfIntegerListOfShape itLstEdg;
        itLstEdg.Initialize(mle);
        Standard_Boolean Found = Standard_False;
        while (itLstEdg.More() && !Found)
        {
          const Standard_Integer&     iLst    = itLstEdg.Key();
          const ShapeList& LmapEdg = mle.Find(iLst);

          // look for each edge of the list if it is in the map Split
          TopTools_ListIteratorOfListOfShape itEdg;
          itEdg.Initialize(LmapEdg);
          while (itEdg.More() && !Found)
          {
            const TopoShape& edgefuse = itEdg.Value();
            if (edgecur.IsSame(edgefuse))
            {
              Found = Standard_True;

              LstSplit.Remove(itSplitEdg);

              // edit the list of merged
              TopAbs_State stateMerged;
              if (ShapeRank(e) == 1)
                stateMerged = myState1;
              else
                stateMerged = myState2;

              ShapeList LstMerged;
              LstMerged.Append(mre(iLst));
              ChangeMerged(e, stateMerged) = LstMerged;
            }
            itEdg.Next();
          }

          itLstEdg.Next();
        }

        if (!Found)
        {
          itSplitEdg.Next();
        }
      }
    }
    // For each face of the MapSplit
    else if (e.ShapeType() == TopAbs_FACE)
    {
      // get the list of split faces.
      ShapeList& LstSplit = ChangeSplit(e, state);

      // for each face of the list of split faces
      TopTools_ListIteratorOfListOfShape itSplitFac;
      itSplitFac.Initialize(LstSplit);
      while (itSplitFac.More())
      {
        const TopoShape& facecur = itSplitFac.Value();

        if (mlf.IsBound(facecur))
        {
          LstSplit.InsertBefore(mlf(facecur), itSplitFac);
          LstSplit.Remove(itSplitFac);
        }
        else
        {
          itSplitFac.Next();
        }
      }
    }
  }
}
