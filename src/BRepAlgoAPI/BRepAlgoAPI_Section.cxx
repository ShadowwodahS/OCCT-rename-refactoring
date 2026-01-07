// Created on: 1994-02-18
// Created by: Remi LEQUETTE
// Copyright (c) 1994-1999 Matra Datavision
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

// modified by Michael KLOKOV  Wed Mar  6 15:01:25 2002
// modified by  Eugeny MALTCHIKOV Wed Jul 04 11:13:01 2012

#include <BRepAlgoAPI_Section.hxx>

#include <BOPAlgo_PaveFiller.hxx>

#include <BOPDS_DS.hxx>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeShell.hxx>

#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>

#include <gp_Pln.hxx>

#include <TopoDS_Shape.hxx>

//
static TopoShape MakeShape(const Handle(GeomSurface)&);

static Standard_Boolean HasAncestorFaces(const BOPAlgo_PPaveFiller&,
                                         const TopoShape&,
                                         TopoShape&,
                                         TopoShape&);
static Standard_Boolean HasAncestorFace(const BOPAlgo_PPaveFiller&,
                                        Standard_Integer,
                                        const TopoShape&,
                                        TopoShape&);

//
//=================================================================================================

BRepAlgoAPI_Section::BRepAlgoAPI_Section()
    : BRepAlgoAPI_BooleanOperation()
{
  Init(Standard_False);
}

//=================================================================================================

BRepAlgoAPI_Section::BRepAlgoAPI_Section(const BooleanPaveFiller& aPF)
    : BRepAlgoAPI_BooleanOperation(aPF)
{
  Init(Standard_False);
}

//=================================================================================================

BRepAlgoAPI_Section::BRepAlgoAPI_Section(const TopoShape&    Sh1,
                                         const TopoShape&    Sh2,
                                         const Standard_Boolean PerformNow)
    : BRepAlgoAPI_BooleanOperation(Sh1, Sh2, BOPAlgo_SECTION)
{
  Init(PerformNow);
}

//=================================================================================================

BRepAlgoAPI_Section::BRepAlgoAPI_Section(const TopoShape&       aS1,
                                         const TopoShape&       aS2,
                                         const BooleanPaveFiller& aDSF,
                                         const Standard_Boolean    PerformNow)
    : BRepAlgoAPI_BooleanOperation(aS1, aS2, aDSF, BOPAlgo_SECTION)
{
  Init(PerformNow);
}

//=================================================================================================

BRepAlgoAPI_Section::BRepAlgoAPI_Section(const TopoShape&    Sh,
                                         const gp_Pln&          Pl,
                                         const Standard_Boolean PerformNow)
    : BRepAlgoAPI_BooleanOperation(Sh, MakeShape(new GeomPlane(Pl)), BOPAlgo_SECTION)
{
  Init(PerformNow);
}

//=================================================================================================

BRepAlgoAPI_Section::BRepAlgoAPI_Section(const TopoShape&         Sh,
                                         const Handle(GeomSurface)& Sf,
                                         const Standard_Boolean      PerformNow)
    : BRepAlgoAPI_BooleanOperation(Sh, MakeShape(Sf), BOPAlgo_SECTION)
{
  Init(PerformNow);
}

//=================================================================================================

BRepAlgoAPI_Section::BRepAlgoAPI_Section(const Handle(GeomSurface)& Sf,
                                         const TopoShape&         Sh,
                                         const Standard_Boolean      PerformNow)
    : BRepAlgoAPI_BooleanOperation(MakeShape(Sf), Sh, BOPAlgo_SECTION)
{
  Init(PerformNow);
}

//=================================================================================================

BRepAlgoAPI_Section::BRepAlgoAPI_Section(const Handle(GeomSurface)& Sf1,
                                         const Handle(GeomSurface)& Sf2,
                                         const Standard_Boolean      PerformNow)
    : BRepAlgoAPI_BooleanOperation(MakeShape(Sf1), MakeShape(Sf2), BOPAlgo_SECTION)
{
  Init(PerformNow);
}

//=================================================================================================

BRepAlgoAPI_Section::~BRepAlgoAPI_Section() {}

//=================================================================================================

void BRepAlgoAPI_Section::Init(const Standard_Boolean bFlag)
{
  myOperation      = BOPAlgo_SECTION;
  myApprox         = Standard_False;
  myComputePCurve1 = Standard_False;
  myComputePCurve2 = Standard_False;
  //
  if (bFlag)
  {
    Build();
  }
}

//=================================================================================================

void BRepAlgoAPI_Section::Init1(const TopoShape& S1)
{
  myArguments.Clear();
  myArguments.Append(S1);
}

//=================================================================================================

void BRepAlgoAPI_Section::Init1(const gp_Pln& Pl)
{
  Init1(MakeShape(new GeomPlane(Pl)));
}

//=================================================================================================

void BRepAlgoAPI_Section::Init1(const Handle(GeomSurface)& Sf)
{
  Init1(MakeShape(Sf));
}

//=================================================================================================

void BRepAlgoAPI_Section::Init2(const TopoShape& S2)
{
  myTools.Clear();
  myTools.Append(S2);
}

//=================================================================================================

void BRepAlgoAPI_Section::Init2(const gp_Pln& Pl)
{
  Init2(MakeShape(new GeomPlane(Pl)));
}

//=================================================================================================

void BRepAlgoAPI_Section::Init2(const Handle(GeomSurface)& Sf)
{
  Init2(MakeShape(Sf));
}

//=================================================================================================

void BRepAlgoAPI_Section::Approximation(const Standard_Boolean B)
{
  myApprox = B;
}

//=================================================================================================

void BRepAlgoAPI_Section::ComputePCurveOn1(const Standard_Boolean B)
{
  myComputePCurve1 = B;
}

//=================================================================================================

void BRepAlgoAPI_Section::ComputePCurveOn2(const Standard_Boolean B)
{
  myComputePCurve2 = B;
}

//=================================================================================================

void BRepAlgoAPI_Section::SetAttributes()
{
  SectionAttribute theSecAttr(myApprox, myComputePCurve1, myComputePCurve2);
  myDSFiller->SetSectionAttribute(theSecAttr);
}

//=================================================================================================

void BRepAlgoAPI_Section::Build(const Message_ProgressRange& theRange)
{
  BRepAlgoAPI_BooleanOperation::Build(theRange);
}

//=================================================================================================

Standard_Boolean BRepAlgoAPI_Section::HasAncestorFaceOn1(const TopoShape& aE,
                                                         TopoShape&       aF) const
{
  Standard_Boolean bRes;
  //
  bRes = HasAncestorFace(myDSFiller, 1, aE, aF);
  return bRes;
}

//=================================================================================================

Standard_Boolean BRepAlgoAPI_Section::HasAncestorFaceOn2(const TopoShape& aE,
                                                         TopoShape&       aF) const
{
  Standard_Boolean bRes;
  //
  bRes = HasAncestorFace(myDSFiller, 2, aE, aF);
  return bRes;
}

//=================================================================================================

Standard_Boolean HasAncestorFace(const BOPAlgo_PPaveFiller& pPF,
                                 Standard_Integer           aIndex,
                                 const TopoShape&        aE,
                                 TopoShape&              aF)
{
  Standard_Boolean bRes;
  //
  bRes = Standard_False;
  if (aE.IsNull())
  {
    return bRes;
  }
  if (aE.ShapeType() != TopAbs_EDGE)
  {
    return bRes;
  }
  //
  TopoShape aF1, aF2;
  //
  bRes = HasAncestorFaces(pPF, aE, aF1, aF2);
  if (!bRes)
  {
    return bRes;
  }
  //
  aF = (aIndex == 1) ? aF1 : aF2;
  return bRes;
}

//=================================================================================================

Standard_Boolean HasAncestorFaces(const BOPAlgo_PPaveFiller& pPF,
                                  const TopoShape&        aEx,
                                  TopoShape&              aF1,
                                  TopoShape&              aF2)
{

  Standard_Integer                    aNbFF, i, j, nE, nF1, nF2, aNbVC;
  BOPDS_ListIteratorOfListOfPaveBlock aItLPB;
  //
  const BOPDS_PDS&        pDS  = pPF->PDS();
  BOPDS_VectorOfInterfFF& aFFs = pDS->InterfFF();
  //
  // section edges
  aNbFF = aFFs.Length();
  for (i = 0; i < aNbFF; ++i)
  {
    BOPDS_InterfFF& aFFi = aFFs(i);
    aFFi.Indices(nF1, nF2);
    //
    const BOPDS_VectorOfCurve& aVC = aFFi.Curves();
    aNbVC                          = aVC.Length();
    for (j = 0; j < aNbVC; j++)
    {
      const BOPDS_Curve& aBC = aVC(j);
      //
      const BOPDS_ListOfPaveBlock& aLPB = aBC.PaveBlocks();
      //
      aItLPB.Initialize(aLPB);
      for (; aItLPB.More(); aItLPB.Next())
      {
        const Handle(BOPDS_PaveBlock)& aPB = aItLPB.Value();
        nE                                 = aPB->Edge();
        if (nE < 0)
        {
          continue;
        }
        //
        const TopoShape aE = pDS->Shape(nE);
        if (aEx.IsSame(aE))
        {
          aF1 = pDS->Shape(nF1);
          aF2 = pDS->Shape(nF2);
          return Standard_True;
        }
      }
    }
  }
  return Standard_False;
}

//=================================================================================================

TopoShape MakeShape(const Handle(GeomSurface)& S)
{
  GeomAbs_Shape c = S->Continuity();
  if (c >= GeomAbs_C2)
  {
    return FaceMaker(S, Precision::Confusion());
  }
  return BRepBuilderAPI_MakeShell(S);
}
