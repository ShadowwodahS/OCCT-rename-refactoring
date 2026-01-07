// Created on: 1995-10-20
// Created by: Yves FRICAUD
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

#include <Adaptor3d_Surface.hxx>
#include <BOPTools_AlgoTools.hxx>
#include <BOPTools_AlgoTools3D.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepOffset_Analyse.hxx>
#include <BRepOffset_Interval.hxx>
#include <BRepOffset_Tool.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepTools.hxx>
#include <Geom_Curve.hxx>
#include <gp.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec.hxx>
#include <IntTools_Context.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_MapOfShape.hxx>
#include <ChFi3d.hxx>
#include <LocalAnalysis_SurfaceContinuity.hxx>

static void CorrectOrientationOfTangent(Vector3d&              TangVec,
                                        const TopoVertex& aVertex,
                                        const TopoEdge&   anEdge)
{
  TopoVertex Vlast = TopExp1::LastVertex(anEdge);
  if (aVertex.IsSame(Vlast))
    TangVec.Reverse();
}

static Standard_Boolean CheckMixedContinuity(const TopoEdge&  theEdge,
                                             const TopoFace&  theFace1,
                                             const TopoFace&  theFace2,
                                             const Standard_Real theAngTol);

//=================================================================================================

BRepOffset_Analyse::BRepOffset_Analyse()
    : myOffset(0.0),
      myDone(Standard_False)
{
}

//=================================================================================================

BRepOffset_Analyse::BRepOffset_Analyse(const TopoShape& S, const Standard_Real Angle)
    : myOffset(0.0),
      myDone(Standard_False)
{
  Perform(S, Angle);
}

//=================================================================================================

static void EdgeAnalyse(const TopoEdge&         E,
                        const TopoFace&         F1,
                        const TopoFace&         F2,
                        const Standard_Real        SinTol,
                        BRepOffset_ListOfInterval& LI)
{
  Standard_Real f, l;
  BRepInspector::Range(E, F1, f, l);
  BRepOffset_Interval I;
  I.First(f);
  I.Last(l);
  //
  BRepAdaptor_Surface aBAsurf1(F1, Standard_False);
  GeomAbs_SurfaceType aSurfType1 = aBAsurf1.GetType();

  BRepAdaptor_Surface aBAsurf2(F2, Standard_False);
  GeomAbs_SurfaceType aSurfType2 = aBAsurf2.GetType();

  Standard_Boolean isTwoPlanes = (aSurfType1 == GeomAbs_Plane && aSurfType2 == GeomAbs_Plane);

  ChFiDS_TypeOfConcavity ConnectType = ChFiDS_Other;

  if (isTwoPlanes) // then use only strong condition
  {
    if (BRepInspector::Continuity(E, F1, F2) > GeomAbs_C0)
      ConnectType = ChFiDS_Tangential;
    else
      ConnectType = ChFi3d1::DefineConnectType(E, F1, F2, SinTol, Standard_False);
  }
  else
  {
    Standard_Boolean isTwoSplines =
      (aSurfType1 == GeomAbs_BSplineSurface || aSurfType1 == GeomAbs_BezierSurface)
      && (aSurfType2 == GeomAbs_BSplineSurface || aSurfType2 == GeomAbs_BezierSurface);
    Standard_Boolean isMixedConcavity = Standard_False;
    if (isTwoSplines)
    {
      Standard_Real anAngTol = 0.1;
      isMixedConcavity       = CheckMixedContinuity(E, F1, F2, anAngTol);
    }

    if (!isMixedConcavity)
    {
      if (ChFi3d1::IsTangentFaces(E, F1, F2)) // weak condition
      {
        ConnectType = ChFiDS_Tangential;
      }
      else
      {
        ConnectType = ChFi3d1::DefineConnectType(E, F1, F2, SinTol, Standard_False);
      }
    }
    else
    {
      ConnectType = ChFiDS_Mixed;
    }
  }

  I.Type(ConnectType);
  LI.Append(I);
}

//=================================================================================================

Standard_Boolean CheckMixedContinuity(const TopoEdge&  theEdge,
                                      const TopoFace&  theFace1,
                                      const TopoFace&  theFace2,
                                      const Standard_Real theAngTol)
{
  Standard_Boolean aMixedCont = Standard_False;
  GeomAbs_Shape    aCurrOrder = BRepInspector::Continuity(theEdge, theFace1, theFace2);
  if (aCurrOrder > GeomAbs_C0)
  {
    // Method BRepInspector::Continuity(...) always returns minimal continuity between faces
    // so, if aCurrOrder > C0 it means that faces are tangent along whole edge.
    return aMixedCont;
  }
  // But we caqnnot trust result, if it is C0. because this value set by default.
  Standard_Real TolC0 = Max(0.001, 1.5 * BRepInspector::Tolerance(theEdge));

  Standard_Real aFirst;
  Standard_Real aLast;

  Handle(GeomCurve2d) aC2d1, aC2d2;

  if (!theFace1.IsSame(theFace2) && BRepInspector::IsClosed(theEdge, theFace1)
      && BRepInspector::IsClosed(theEdge, theFace2))
  {
    // Find the edge in the face 1: this edge will have correct orientation
    TopoEdge anEdgeInFace1;
    TopoFace aFace1 = theFace1;
    aFace1.Orientation(TopAbs_FORWARD);
    ShapeExplorer anExplo(aFace1, TopAbs_EDGE);
    for (; anExplo.More(); anExplo.Next())
    {
      const TopoEdge& anEdge = TopoDS::Edge(anExplo.Current());
      if (anEdge.IsSame(theEdge))
      {
        anEdgeInFace1 = anEdge;
        break;
      }
    }
    if (anEdgeInFace1.IsNull())
    {
      return aMixedCont;
    }

    aC2d1              = BRepInspector::CurveOnSurface(anEdgeInFace1, aFace1, aFirst, aLast);
    TopoFace aFace2 = theFace2;
    aFace2.Orientation(TopAbs_FORWARD);
    anEdgeInFace1.Reverse();
    aC2d2 = BRepInspector::CurveOnSurface(anEdgeInFace1, aFace2, aFirst, aLast);
  }
  else
  {
    // Obtaining of pcurves of edge on two faces.
    aC2d1 = BRepInspector::CurveOnSurface(theEdge, theFace1, aFirst, aLast);
    // For the case of seam edge
    TopoEdge EE = theEdge;
    if (theFace1.IsSame(theFace2))
    {
      EE.Reverse();
    }
    aC2d2 = BRepInspector::CurveOnSurface(EE, theFace2, aFirst, aLast);
  }

  if (aC2d1.IsNull() || aC2d2.IsNull())
  {
    return aMixedCont;
  }

  // Obtaining of two surfaces from adjacent faces.
  Handle(GeomSurface) aSurf1 = BRepInspector::Surface(theFace1);
  Handle(GeomSurface) aSurf2 = BRepInspector::Surface(theFace2);

  if (aSurf1.IsNull() || aSurf2.IsNull())
  {
    return aMixedCont;
  }

  Standard_Integer aNbSamples = 23;

  // Computation of the continuity.
  Standard_Real    aPar;
  Standard_Real    aDelta = (aLast - aFirst) / (aNbSamples - 1);
  Standard_Integer i, istart = 1;
  Standard_Boolean isG1 = Standard_False;

  for (i = 1, aPar = aFirst; i <= aNbSamples; i++, aPar += aDelta)
  {
    if (i == aNbSamples)
      aPar = aLast;

    LocalAnalysis_SurfaceContinuity aCont(aC2d1,
                                          aC2d2,
                                          aPar,
                                          aSurf1,
                                          aSurf2,
                                          GeomAbs_G1,
                                          0.001,
                                          TolC0,
                                          theAngTol,
                                          theAngTol,
                                          theAngTol);
    if (aCont.IsDone())
    {
      istart = i + 1;
      isG1   = aCont.IsG1();
      break;
    }
  }

  if (istart > aNbSamples / 2)
  {
    return aMixedCont;
  }

  for (i = istart, aPar = aFirst; i <= aNbSamples; i++, aPar += aDelta)
  {
    if (i == aNbSamples)
      aPar = aLast;

    LocalAnalysis_SurfaceContinuity aCont(aC2d1,
                                          aC2d2,
                                          aPar,
                                          aSurf1,
                                          aSurf2,
                                          GeomAbs_G1,
                                          0.001,
                                          TolC0,
                                          theAngTol,
                                          theAngTol,
                                          theAngTol);
    if (!aCont.IsDone())
    {
      continue;
    }

    if (aCont.IsG1() == isG1)
    {
      continue;
    }
    else
    {
      aMixedCont = Standard_True;
      break;
    }
  }

  return aMixedCont;
}

//=================================================================================================

static void BuildAncestors(const TopoShape& S, TopTools_IndexedDataMapOfShapeListOfShape& MA)
{
  MA.Clear();
  TopExp1::MapShapesAndUniqueAncestors(S, TopAbs_VERTEX, TopAbs_EDGE, MA);
  TopExp1::MapShapesAndUniqueAncestors(S, TopAbs_EDGE, TopAbs_FACE, MA);
}

//=================================================================================================

void BRepOffset_Analyse::Perform(const TopoShape&          S,
                                 const Standard_Real          Angle,
                                 const Message_ProgressRange& theRange)
{
  myShape = S;
  myNewFaces.Clear();
  myGenerated.Clear();
  myReplacement.Clear();
  myDescendants.Clear();

  myAngle              = Angle;
  Standard_Real SinTol = Abs(Sin(Angle));

  // Build ancestors.
  BuildAncestors(S, myAncestors);

  ShapeList  aLETang;
  ShapeExplorer       Exp(S.Oriented(TopAbs_FORWARD), TopAbs_EDGE);
  Message_ProgressScope aPSOuter(theRange, NULL, 2);
  Message_ProgressScope aPS(aPSOuter.Next(), "Performing edges analysis", 1, Standard_True);
  for (; Exp.More(); Exp.Next(), aPS.Next())
  {
    if (!aPS.More())
    {
      return;
    }
    const TopoEdge& E = TopoDS::Edge(Exp.Current());
    if (!myMapEdgeType.IsBound(E))
    {
      BRepOffset_ListOfInterval LI;
      myMapEdgeType.Bind(E, LI);

      const ShapeList& L = Ancestors(E);
      if (L.IsEmpty())
        continue;

      if (L.Extent() == 2)
      {
        const TopoFace& F1 = TopoDS::Face(L.First());
        const TopoFace& F2 = TopoDS::Face(L.Last());
        EdgeAnalyse(E, F1, F2, SinTol, myMapEdgeType(E));

        // For tangent faces add artificial perpendicular face
        // to close the gap between them (if they have different offset values)
        if (myMapEdgeType(E).Last().Type() == ChFiDS_Tangential)
          aLETang.Append(E);
      }
      else if (L.Extent() == 1)
      {
        Standard_Real      U1, U2;
        const TopoFace& F = TopoDS::Face(L.First());
        BRepInspector::Range(E, F, U1, U2);
        BRepOffset_Interval Inter(U1, U2, ChFiDS_Other);

        if (!BRepTools1::IsReallyClosed(E, F))
        {
          Inter.Type(ChFiDS_FreeBound);
        }
        myMapEdgeType(E).Append(Inter);
      }
      else
      {
#ifdef OCCT_DEBUG
        std::cout << "edge shared by more than two faces" << std::endl;
#endif
      }
    }
  }

  TreatTangentFaces(aLETang, aPSOuter.Next());
  if (!aPSOuter.More())
  {
    return;
  }
  myDone = Standard_True;
}

//=================================================================================================

void BRepOffset_Analyse::TreatTangentFaces(const ShapeList&  theLE,
                                           const Message_ProgressRange& theRange)
{
  if (theLE.IsEmpty() || myFaceOffsetMap.IsEmpty())
  {
    // Noting to do: either there are no tangent faces in the shape or
    //               the face offset map has not been provided
    return;
  }

  // Select the edges which connect faces with different offset values
  TopoCompound aCETangent;
  ShapeBuilder().MakeCompound(aCETangent);
  // Bind to each tangent edge a max offset value of its faces
  TopTools_DataMapOfShapeReal anEdgeOffsetMap;
  // Bind vertices of the tangent edges with connected edges
  // of the face with smaller offset value
  TopTools_DataMapOfShapeShape aDMVEMin;
  Message_ProgressScope        aPSOuter(theRange, NULL, 3);
  Message_ProgressScope        aPS1(aPSOuter.Next(),
                             "Binding vertices with connected edges",
                             theLE.Size());
  for (ShapeList::Iterator it(theLE); it.More(); it.Next(), aPS1.Next())
  {
    if (!aPS1.More())
    {
      return;
    }
    const TopoShape&         aE  = it.Value();
    const ShapeList& aLA = Ancestors(aE);

    const TopoShape &aF1 = aLA.First(), aF2 = aLA.Last();

    const Standard_Real* pOffsetVal1  = myFaceOffsetMap.Seek(aF1);
    const Standard_Real* pOffsetVal2  = myFaceOffsetMap.Seek(aF2);
    const Standard_Real  anOffsetVal1 = pOffsetVal1 ? Abs(*pOffsetVal1) : myOffset;
    const Standard_Real  anOffsetVal2 = pOffsetVal2 ? Abs(*pOffsetVal2) : myOffset;
    if (anOffsetVal1 != anOffsetVal2)
    {
      ShapeBuilder().Add(aCETangent, aE);
      anEdgeOffsetMap.Bind(aE, Max(anOffsetVal1, anOffsetVal2));

      const TopoShape& aFMin = anOffsetVal1 < anOffsetVal2 ? aF1 : aF2;
      for (TopoDS_Iterator itV(aE); itV.More(); itV.Next())
      {
        const TopoShape& aV = itV.Value();
        if (Ancestors(aV).Extent() == 3)
        {
          for (ShapeExplorer expE(aFMin, TopAbs_EDGE); expE.More(); expE.Next())
          {
            const TopoShape& aEMin = expE.Current();
            if (aEMin.IsSame(aE))
              continue;
            for (TopoDS_Iterator itV1(aEMin); itV1.More(); itV1.Next())
            {
              const TopoShape& aVx = itV1.Value();
              if (aV.IsSame(aVx))
                aDMVEMin.Bind(aV, aEMin);
            }
          }
        }
      }
    }
  }

  if (anEdgeOffsetMap.IsEmpty())
    return;

  // Create map of Face ancestors for the vertices on tangent edges
  TopTools_DataMapOfShapeListOfShape aDMVFAnc;

  Message_ProgressScope aPS2(aPSOuter.Next(), "Creating map of Face ancestors", theLE.Size());
  for (ShapeList::Iterator itE(theLE); itE.More(); itE.Next(), aPS2.Next())
  {
    if (!aPS2.More())
    {
      return;
    }
    const TopoShape& aE = itE.Value();
    if (!anEdgeOffsetMap.IsBound(aE))
      continue;

    TopTools_MapOfShape aMFence;
    {
      const ShapeList& aLEA = Ancestors(aE);
      for (ShapeList::Iterator itLEA(aLEA); itLEA.More(); itLEA.Next())
        aMFence.Add(itLEA.Value());
    }

    for (TopoDS_Iterator itV(aE); itV.More(); itV.Next())
    {
      const TopoShape&         aV   = itV.Value();
      ShapeList*       pLFA = aDMVFAnc.Bound(aV, ShapeList());
      const ShapeList& aLVA = Ancestors(aV);
      for (ShapeList::Iterator itLVA(aLVA); itLVA.More(); itLVA.Next())
      {
        const TopoEdge&               aEA        = TopoDS::Edge(itLVA.Value());
        const BRepOffset_ListOfInterval* pIntervals = myMapEdgeType.Seek(aEA);
        if (!pIntervals || pIntervals->IsEmpty())
          continue;
        if (pIntervals->First().Type() == ChFiDS_Tangential)
          continue;

        const ShapeList& aLEA = Ancestors(aEA);
        for (ShapeList::Iterator itLEA(aLEA); itLEA.More(); itLEA.Next())
        {
          const TopoShape& aFA = itLEA.Value();
          if (aMFence.Add(aFA))
            pLFA->Append(aFA);
        }
      }
    }
  }

  Handle(IntTools_Context) aCtx = new IntTools_Context();
  // Tangency criteria
  Standard_Real aSinTol = Abs(Sin(myAngle));

  // Make blocks of connected edges
  TopTools_ListOfListOfShape                aLCB;
  TopTools_IndexedDataMapOfShapeListOfShape aMVEMap;

  AlgoTools::MakeConnexityBlocks(aCETangent, TopAbs_VERTEX, TopAbs_EDGE, aLCB, aMVEMap);

  // Analyze each block to find co-planar edges
  Message_ProgressScope aPS3(aPSOuter.Next(),
                             "Analyzing blocks to find co-planar edges",
                             aLCB.Size());
  for (TopTools_ListOfListOfShape::Iterator itLCB(aLCB); itLCB.More(); itLCB.Next(), aPS3.Next())
  {
    if (!aPS3.More())
    {
      return;
    }
    const ShapeList& aCB = itLCB.Value();

    TopTools_MapOfShape aMFence;
    for (ShapeList::Iterator itCB1(aCB); itCB1.More(); itCB1.Next())
    {
      const TopoEdge& aE1 = TopoDS::Edge(itCB1.Value());
      if (!aMFence.Add(aE1))
        continue;

      TopoCompound aBlock;
      ShapeBuilder().MakeCompound(aBlock);
      ShapeBuilder().Add(aBlock, aE1.Oriented(TopAbs_FORWARD));

      Standard_Real               anOffset = anEdgeOffsetMap.Find(aE1);
      const ShapeList& aLF1     = Ancestors(aE1);

      Dir3d aDN1;
      AlgoTools3D::GetNormalToFaceOnEdge(aE1, TopoDS::Face(aLF1.First()), aDN1);

      ShapeList::Iterator itCB2 = itCB1;
      for (itCB2.Next(); itCB2.More(); itCB2.Next())
      {
        const TopoEdge& aE2 = TopoDS::Edge(itCB2.Value());
        if (aMFence.Contains(aE2))
          continue;

        const ShapeList& aLF2 = Ancestors(aE2);

        Dir3d aDN2;
        AlgoTools3D::GetNormalToFaceOnEdge(aE2, TopoDS::Face(aLF2.First()), aDN2);

        if (aDN1.XYZ().Crossed(aDN2.XYZ()).Modulus() < aSinTol)
        {
          ShapeBuilder().Add(aBlock, aE2.Oriented(TopAbs_FORWARD));
          aMFence.Add(aE2);
          anOffset = Max(anOffset, anEdgeOffsetMap.Find(aE2));
        }
      }

      // Make the prism
      BRepPrimAPI_MakePrism aMP(aBlock, Vector3d(aDN1.XYZ()) * anOffset);
      if (!aMP.IsDone())
        continue;

      TopTools_IndexedDataMapOfShapeListOfShape aPrismAncestors;
      TopExp1::MapShapesAndAncestors(aMP.Shape(), TopAbs_EDGE, TopAbs_FACE, aPrismAncestors);
      TopExp1::MapShapesAndAncestors(aMP.Shape(), TopAbs_VERTEX, TopAbs_EDGE, aPrismAncestors);

      for (TopoDS_Iterator itE(aBlock); itE.More(); itE.Next())
      {
        const TopoEdge&          aE    = TopoDS::Edge(itE.Value());
        const ShapeList& aLG   = aMP.Generated(aE);
        TopoFace                 aFNew = TopoDS::Face(aLG.First());

        ShapeList& aLA = myAncestors.ChangeFromKey(aE);

        TopoShape aF1 = aLA.First();
        TopoShape aF2 = aLA.Last();

        const Standard_Real* pOffsetVal1  = myFaceOffsetMap.Seek(aF1);
        const Standard_Real* pOffsetVal2  = myFaceOffsetMap.Seek(aF2);
        const Standard_Real  anOffsetVal1 = pOffsetVal1 ? Abs(*pOffsetVal1) : myOffset;
        const Standard_Real  anOffsetVal2 = pOffsetVal2 ? Abs(*pOffsetVal2) : myOffset;

        const TopoShape& aFToRemove = anOffsetVal1 > anOffsetVal2 ? aF1 : aF2;
        const TopoShape& aFOpposite = anOffsetVal1 > anOffsetVal2 ? aF2 : aF1;

        // Orient the face so its normal is directed to smaller offset face
        {
          // get normal of the new face
          Dir3d aDN;
          AlgoTools3D::GetNormalToFaceOnEdge(aE, aFNew, aDN);

          // get bi-normal for the aFOpposite
          TopoEdge aEInF;
          for (ShapeExplorer aExpE(aFOpposite, TopAbs_EDGE); aExpE.More(); aExpE.Next())
          {
            if (aE.IsSame(aExpE.Current()))
            {
              aEInF = TopoDS::Edge(aExpE.Current());
              break;
            }
          }

          gp_Pnt2d                  aP2d;
          Point3d                    aPInF;
          Standard_Real             f, l;
          const Handle(GeomCurve3d)& aC3D  = BRepInspector::Curve(aEInF, f, l);
          Point3d                    aPOnE = aC3D->Value((f + l) / 2.);
          AlgoTools3D::PointNearEdge(aEInF,
                                              TopoDS::Face(aFOpposite),
                                              (f + l) / 2.,
                                              1.e-5,
                                              aP2d,
                                              aPInF);

          Vector3d aBN(aPOnE, aPInF);

          if (aBN.Dot(aDN) < 0)
            aFNew.Reverse();
        }

        // Remove the face with bigger offset value from edge ancestors
        for (ShapeList::Iterator itA(aLA); itA.More(); itA.Next())
        {
          if (itA.Value().IsSame(aFToRemove))
          {
            aLA.Remove(itA);
            break;
          }
        }
        aLA.Append(aFNew);

        myMapEdgeType(aE).Clear();
        // Analyze edge again
        EdgeAnalyse(aE, TopoDS::Face(aFOpposite), aFNew, aSinTol, myMapEdgeType(aE));

        // Analyze vertices
        TopTools_MapOfShape aFNewEdgeMap;
        aFNewEdgeMap.Add(aE);
        for (TopoDS_Iterator itV(aE); itV.More(); itV.Next())
        {
          const TopoShape& aV = itV.Value();
          // Add Side edge to map of Ancestors with the correct orientation
          TopoEdge aEG = TopoDS::Edge(aMP.Generated(aV).First());
          myGenerated.Bind(aV, aEG);
          {
            for (ShapeExplorer anExpEg(aFNew, TopAbs_EDGE); anExpEg.More(); anExpEg.Next())
            {
              if (anExpEg.Current().IsSame(aEG))
              {
                aEG = TopoDS::Edge(anExpEg.Current());
                break;
              }
            }
          }

          if (aDMVEMin.IsBound(aV))
          {
            const ShapeList* pSA = aDMVFAnc.Seek(aV);
            if (pSA && pSA->Extent() == 1)
            {
              // Adjust orientation of generated edge to its new ancestor
              TopoEdge aEMin = TopoDS::Edge(aDMVEMin.Find(aV));
              for (ShapeExplorer expEx(pSA->First(), TopAbs_EDGE); expEx.More(); expEx.Next())
              {
                if (expEx.Current().IsSame(aEMin))
                {
                  aEMin = TopoDS::Edge(expEx.Current());
                  break;
                }
              }

              TopAbs_Orientation anOriInEMin(TopAbs_FORWARD), anOriInEG(TopAbs_FORWARD);

              for (TopoDS_Iterator itx(aEMin); itx.More(); itx.Next())
              {
                if (itx.Value().IsSame(aV))
                {
                  anOriInEMin = itx.Value().Orientation();
                  break;
                }
              }

              for (TopoDS_Iterator itx(aEG); itx.More(); itx.Next())
              {
                if (itx.Value().IsSame(aV))
                {
                  anOriInEG = itx.Value().Orientation();
                  break;
                }
              }

              if (anOriInEG == anOriInEMin)
                aEG.Reverse();
            }
          }

          ShapeList& aLVA = myAncestors.ChangeFromKey(aV);
          if (!aLVA.Contains(aEG))
            aLVA.Append(aEG);
          aFNewEdgeMap.Add(aEG);

          ShapeList& aLEGA =
            myAncestors(myAncestors.Add(aEG, aPrismAncestors.FindFromKey(aEG)));
          {
            // Add ancestors from the shape
            const ShapeList* pSA = aDMVFAnc.Seek(aV);
            if (pSA && !pSA->IsEmpty())
            {
              ShapeList aLSA = *pSA;
              aLEGA.Append(aLSA);
            }
          }

          myMapEdgeType.Bind(aEG, BRepOffset_ListOfInterval());
          if (aLEGA.Extent() == 2)
          {
            EdgeAnalyse(aEG,
                        TopoDS::Face(aLEGA.First()),
                        TopoDS::Face(aLEGA.Last()),
                        aSinTol,
                        myMapEdgeType(aEG));
          }
        }

        // Find an edge opposite to tangential one and add ancestors for it
        TopoEdge aEOpposite;
        for (ShapeExplorer anExpE(aFNew, TopAbs_EDGE); anExpE.More(); anExpE.Next())
        {
          if (!aFNewEdgeMap.Contains(anExpE.Current()))
          {
            aEOpposite = TopoDS::Edge(anExpE.Current());
            break;
          }
        }

        {
          // Find it in aFOpposite
          for (ShapeExplorer anExpE(aFToRemove, TopAbs_EDGE); anExpE.More(); anExpE.Next())
          {
            const TopoShape& aEInFToRem = anExpE.Current();
            if (aE.IsSame(aEInFToRem))
            {
              if (AlgoTools::IsSplitToReverse(aEOpposite, aEInFToRem, aCtx))
                aEOpposite.Reverse();
              break;
            }
          }
        }

        ShapeList aLFOpposite;
        aLFOpposite.Append(aFNew);
        aLFOpposite.Append(aFToRemove);
        myAncestors.Add(aEOpposite, aLFOpposite);
        myMapEdgeType.Bind(aEOpposite, BRepOffset_ListOfInterval());
        EdgeAnalyse(aEOpposite,
                    aFNew,
                    TopoDS::Face(aFToRemove),
                    aSinTol,
                    myMapEdgeType(aEOpposite));

        TopTools_DataMapOfShapeShape* pEEMap = myReplacement.ChangeSeek(aFToRemove);
        if (!pEEMap)
          pEEMap = myReplacement.Bound(aFToRemove, TopTools_DataMapOfShapeShape());
        pEEMap->Bind(aE, aEOpposite);

        // Add ancestors for the vertices
        for (TopoDS_Iterator itV(aEOpposite); itV.More(); itV.Next())
        {
          const TopoShape&         aV   = itV.Value();
          const ShapeList& aLVA = aPrismAncestors.FindFromKey(aV);
          myAncestors.Add(aV, aLVA);
        }

        myNewFaces.Append(aFNew);
        myGenerated.Bind(aE, aFNew);
      }
    }
  }
}

//=================================================================================================

const TopoEdge& BRepOffset_Analyse::EdgeReplacement(const TopoFace& theF,
                                                       const TopoEdge& theE) const
{
  const TopTools_DataMapOfShapeShape* pEE = myReplacement.Seek(theF);
  if (!pEE)
    return theE;

  const TopoShape* pE = pEE->Seek(theE);
  if (!pE)
    return theE;

  return TopoDS::Edge(*pE);
}

//=================================================================================================

TopoShape BRepOffset_Analyse::Generated(const TopoShape& theS) const
{
  static TopoShape aNullShape;
  const TopoShape* pGenS = myGenerated.Seek(theS);
  return pGenS ? *pGenS : aNullShape;
}

//=================================================================================================

const ShapeList* BRepOffset_Analyse::Descendants(const TopoShape&    theS,
                                                            const Standard_Boolean theUpdate) const
{
  if (myDescendants.IsEmpty() || theUpdate)
  {
    myDescendants.Clear();
    const Standard_Integer aNbA = myAncestors.Extent();
    for (Standard_Integer i = 1; i <= aNbA; ++i)
    {
      const TopoShape&         aSS = myAncestors.FindKey(i);
      const ShapeList& aLA = myAncestors(i);

      for (ShapeList::Iterator it(aLA); it.More(); it.Next())
      {
        const TopoShape& aSA = it.Value();

        ShapeList* pLD = myDescendants.ChangeSeek(aSA);
        if (!pLD)
          pLD = myDescendants.Bound(aSA, ShapeList());
        if (!pLD->Contains(aSS))
          pLD->Append(aSS);
      }
    }
  }

  return myDescendants.Seek(theS);
}

//=================================================================================================

void BRepOffset_Analyse::Clear()
{
  myDone = Standard_False;
  myShape.Nullify();
  myMapEdgeType.Clear();
  myAncestors.Clear();
  myFaceOffsetMap.Clear();
  myReplacement.Clear();
  myDescendants.Clear();
  myNewFaces.Clear();
  myGenerated.Clear();
}

//=======================================================================
// function : BRepOffset_ListOfInterval&
// purpose  :
//=======================================================================
const BRepOffset_ListOfInterval& BRepOffset_Analyse::Type(const TopoEdge& E) const
{
  return myMapEdgeType(E);
}

//=================================================================================================

void BRepOffset_Analyse::Edges(const TopoVertex&         V,
                               const ChFiDS_TypeOfConcavity T,
                               ShapeList&        LE) const
{
  LE.Clear();
  const ShapeList&        L = Ancestors(V);
  TopTools_ListIteratorOfListOfShape it(L);

  for (; it.More(); it.Next())
  {
    const TopoEdge&               E          = TopoDS::Edge(it.Value());
    const BRepOffset_ListOfInterval* pIntervals = myMapEdgeType.Seek(E);
    if (pIntervals && pIntervals->Extent() > 0)
    {
      TopoVertex V1, V2;
      BRepOffset_Tool::EdgeVertices(E, V1, V2);
      if (V1.IsSame(V))
      {
        if (pIntervals->Last().Type() == T)
          LE.Append(E);
      }
      if (V2.IsSame(V))
      {
        if (pIntervals->First().Type() == T)
          LE.Append(E);
      }
    }
  }
}

//=================================================================================================

void BRepOffset_Analyse::Edges(const TopoFace&           F,
                               const ChFiDS_TypeOfConcavity T,
                               ShapeList&        LE) const
{
  LE.Clear();
  ShapeExplorer exp(F, TopAbs_EDGE);

  for (; exp.More(); exp.Next())
  {
    const TopoEdge& E = TopoDS::Edge(exp.Current());

    const BRepOffset_ListOfInterval&        Lint = Type(E);
    BRepOffset_ListIteratorOfListOfInterval it(Lint);
    for (; it.More(); it.Next())
    {
      if (it.Value().Type() == T)
        LE.Append(E);
    }
  }
}

//=================================================================================================

void BRepOffset_Analyse::TangentEdges(const TopoEdge&    Edge,
                                      const TopoVertex&  Vertex,
                                      ShapeList& Edges) const
{
  Vector3d V, VRef;

  Standard_Real     U, URef;
  BRepAdaptor_Curve C3d, C3dRef;

  URef   = BRepInspector::Parameter(Vertex, Edge);
  C3dRef = BRepAdaptor_Curve(Edge);
  VRef   = C3dRef.DN(URef, 1);
  CorrectOrientationOfTangent(VRef, Vertex, Edge);
  if (VRef.SquareMagnitude() < gp::Resolution())
    return;

  Edges.Clear();

  const ShapeList&        Anc = Ancestors(Vertex);
  TopTools_ListIteratorOfListOfShape it(Anc);
  for (; it.More(); it.Next())
  {
    const TopoEdge& CurE = TopoDS::Edge(it.Value());
    if (CurE.IsSame(Edge))
      continue;
    U   = BRepInspector::Parameter(Vertex, CurE);
    C3d = BRepAdaptor_Curve(CurE);
    V   = C3d.DN(U, 1);
    CorrectOrientationOfTangent(V, Vertex, CurE);
    if (V.SquareMagnitude() < gp::Resolution())
      continue;
    if (V.IsOpposite(VRef, myAngle))
    {
      Edges.Append(CurE);
    }
  }
}

//=================================================================================================

void BRepOffset_Analyse::Explode(ShapeList& List, const ChFiDS_TypeOfConcavity T) const
{
  List.Clear();
  ShapeBuilder        B;
  TopTools_MapOfShape Map;

  ShapeExplorer Fexp;
  for (Fexp.Init(myShape, TopAbs_FACE); Fexp.More(); Fexp.Next())
  {
    if (Map.Add(Fexp.Current()))
    {
      TopoFace     Face = TopoDS::Face(Fexp.Current());
      TopoCompound Co;
      B.MakeCompound(Co);
      B.Add(Co, Face);
      // add to Co all faces from the cloud of faces
      // G1 created from <Face>
      AddFaces(Face, Co, Map, T);
      List.Append(Co);
    }
  }
}

//=================================================================================================

void BRepOffset_Analyse::Explode(ShapeList&        List,
                                 const ChFiDS_TypeOfConcavity T1,
                                 const ChFiDS_TypeOfConcavity T2) const
{
  List.Clear();
  ShapeBuilder        B;
  TopTools_MapOfShape Map;

  ShapeExplorer Fexp;
  for (Fexp.Init(myShape, TopAbs_FACE); Fexp.More(); Fexp.Next())
  {
    if (Map.Add(Fexp.Current()))
    {
      TopoFace     Face = TopoDS::Face(Fexp.Current());
      TopoCompound Co;
      B.MakeCompound(Co);
      B.Add(Co, Face);
      // add to Co all faces from the cloud of faces
      // G1 created from  <Face>
      AddFaces(Face, Co, Map, T1, T2);
      List.Append(Co);
    }
  }
}

//=================================================================================================

void BRepOffset_Analyse::AddFaces(const TopoFace&           Face,
                                  TopoCompound&             Co,
                                  TopTools_MapOfShape&         Map,
                                  const ChFiDS_TypeOfConcavity T) const
{
  ShapeBuilder                B;
  const ShapeList* pLE = Descendants(Face);
  if (!pLE)
    return;
  for (ShapeList::Iterator it(*pLE); it.More(); it.Next())
  {
    const TopoEdge&               E  = TopoDS::Edge(it.Value());
    const BRepOffset_ListOfInterval& LI = Type(E);
    if (!LI.IsEmpty() && LI.First().Type() == T)
    {
      // so <NewFace> is attached to G1 by <Face>
      const ShapeList& L = Ancestors(E);
      if (L.Extent() == 2)
      {
        TopoFace F1 = TopoDS::Face(L.First());
        if (F1.IsSame(Face))
          F1 = TopoDS::Face(L.Last());
        if (Map.Add(F1))
        {
          B.Add(Co, F1);
          AddFaces(F1, Co, Map, T);
        }
      }
    }
  }
}

//=================================================================================================

void BRepOffset_Analyse::AddFaces(const TopoFace&           Face,
                                  TopoCompound&             Co,
                                  TopTools_MapOfShape&         Map,
                                  const ChFiDS_TypeOfConcavity T1,
                                  const ChFiDS_TypeOfConcavity T2) const
{
  ShapeBuilder                B;
  const ShapeList* pLE = Descendants(Face);
  if (!pLE)
    return;
  for (ShapeList::Iterator it(*pLE); it.More(); it.Next())
  {
    const TopoEdge&               E  = TopoDS::Edge(it.Value());
    const BRepOffset_ListOfInterval& LI = Type(E);
    if (!LI.IsEmpty() && (LI.First().Type() == T1 || LI.First().Type() == T2))
    {
      // so <NewFace> is attached to G1 by <Face>
      const ShapeList& L = Ancestors(E);
      if (L.Extent() == 2)
      {
        TopoFace F1 = TopoDS::Face(L.First());
        if (F1.IsSame(Face))
          F1 = TopoDS::Face(L.Last());
        if (Map.Add(F1))
        {
          B.Add(Co, F1);
          AddFaces(F1, Co, Map, T1, T2);
        }
      }
    }
  }
}
