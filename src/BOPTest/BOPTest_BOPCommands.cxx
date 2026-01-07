// Created on: 2000-03-16
// Created by: Peter KURNEV
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#include <BOPAlgo_BOP.hxx>
#include <BOPAlgo_MakerVolume.hxx>
#include <BOPAlgo_Operation.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BOPAlgo_Section.hxx>
#include <BOPDS_DS.hxx>
#include <BOPTest.hxx>
#include <BOPTest_Objects.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepTest_Objects.hxx>
#include <DBRep.hxx>
#include <Draw.hxx>
#include <DrawTrSurf.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <IntTools_Curve.hxx>
#include <IntTools_FaceFace.hxx>
#include <IntTools_PntOn2Faces.hxx>
#include <NCollection_BaseAllocator.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <Draw_ProgressIndicator.hxx>

#include <stdio.h>
//
//
static BooleanPaveFiller* pPF = NULL;
//

static Standard_Integer bopsmt(DrawInterpreter&       di,
                               Standard_Integer        n,
                               const char**            a,
                               const BOPAlgo_Operation aOp);
//
static Standard_Integer bsmt(DrawInterpreter&       di,
                             Standard_Integer        n,
                             const char**            a,
                             const BOPAlgo_Operation aOp);
//
static Standard_Integer bop(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bopsection(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer boptuc(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bopcut(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bopfuse(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bopcommon(DrawInterpreter&, Standard_Integer, const char**);
//
static Standard_Integer bsection(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer btuc(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bcut(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bfuse(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bcommon(DrawInterpreter&, Standard_Integer, const char**);
//
static Standard_Integer bopcurves(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer mkvolume(DrawInterpreter&, Standard_Integer, const char**);

//=================================================================================================

void BOPTest::BOPCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;
  // Chapter's name
  const char* g = "BOPTest commands";
  // Commands

  theCommands.Add("bop", "use bop s1 s2", __FILE__, bop, g);
  theCommands.Add("bopcommon", "use bopcommon r", __FILE__, bopcommon, g);
  theCommands.Add("bopfuse", "use bopfuse r", __FILE__, bopfuse, g);
  theCommands.Add("bopcut", "use bopcut r", __FILE__, bopcut, g);
  theCommands.Add("boptuc", "use boptuc r", __FILE__, boptuc, g);
  theCommands.Add("bopsection", "use bopsection r", __FILE__, bopsection, g);
  //
  theCommands.Add("bcommon", "use bcommon r s1 s2", __FILE__, bcommon, g);
  theCommands.Add("bfuse", "use bfuse r s1 s2", __FILE__, bfuse, g);
  theCommands.Add("bcut", "use bcut r s1 s2", __FILE__, bcut, g);
  theCommands.Add("btuc", "use btuc r s1 s2", __FILE__, btuc, g);
  theCommands.Add("bsection",
                  "use bsection r s1 s2 [-n2d/-n2d1/-n2d2] [-na]"
                  "Builds section between shapes. Options:\n"
                  "-n2d/-n2d1/-n2d2 - disable the PCurve construction;\n"
                  "-na - disables the approximation of the section curves.\n",
                  __FILE__,
                  bsection,
                  g);
  //
  theCommands.Add("bopcurves",
                  "use bopcurves F1 F2 [-2d/-2d1/-2d2] "
                  "[-p u1 v1 u2 v2 (to add start points] [-v (for extended output)]",
                  __FILE__,
                  bopcurves,
                  g);
  theCommands.Add("mkvolume",
                  "make solids from set of shapes.\nmkvolume r b1 b2 ... [-c] [-ni] [-ai]",
                  __FILE__,
                  mkvolume,
                  g);
}

//=================================================================================================

Standard_Integer bop(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  Standard_Boolean     bRunParallel, bNonDestructive;
  Standard_Real        aTol;
  TopoShape         aS1, aS2;
  ShapeList aLC;
  //
  if (n != 3)
  {
    di << " use bop s1 s2 \n";
    return 0;
  }
  //
  aS1 = DBRep1::Get(a[1]);
  aS2 = DBRep1::Get(a[2]);
  //
  if (aS1.IsNull() || aS2.IsNull())
  {
    di << " null shapes are not allowed \n";
    return 0;
  }
  //
  aTol                                     = BOPTest_Objects::FuzzyValue();
  bRunParallel                             = BOPTest_Objects::RunParallel();
  bNonDestructive                          = BOPTest_Objects::NonDestructive();
  BOPAlgo_GlueEnum               aGlue     = BOPTest_Objects::Glue();
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(di, 1);
  //
  aLC.Append(aS1);
  aLC.Append(aS2);
  //
  if (pPF != NULL)
  {
    delete pPF;
    pPF = NULL;
  }
  Handle(NCollection_BaseAllocator) aAL = NCollection_BaseAllocator::CommonBaseAllocator();
  pPF                                   = new BooleanPaveFiller(aAL);
  //
  pPF->SetArguments(aLC);
  pPF->SetFuzzyValue(aTol);
  pPF->SetRunParallel(bRunParallel);
  pPF->SetNonDestructive(bNonDestructive);
  pPF->SetGlue(aGlue);
  pPF->SetUseOBB(BOPTest_Objects::UseOBB());
  //
  pPF->Perform(aProgress->Start());
  BOPTest::ReportAlerts(pPF->GetReport());
  //
  return 0;
}

//=================================================================================================

Standard_Integer bopcommon(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  return bopsmt(di, n, a, BOPAlgo_COMMON);
}

//=================================================================================================

Standard_Integer bopfuse(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  return bopsmt(di, n, a, BOPAlgo_FUSE);
}

//=================================================================================================

Standard_Integer bopcut(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  return bopsmt(di, n, a, BOPAlgo_CUT);
}

//=================================================================================================

Standard_Integer boptuc(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  return bopsmt(di, n, a, BOPAlgo_CUT21);
}

//=================================================================================================

Standard_Integer bopsmt(DrawInterpreter&       di,
                        Standard_Integer        n,
                        const char**            a,
                        const BOPAlgo_Operation aOp)
{
  if (n < 2)
  {
    di << " use bopsmt r\n";
    return 0;
  }
  //
  if (!pPF)
  {
    di << " prepare PaveFiller first\n";
    return 0;
  }
  //
  if (pPF->HasErrors())
  {
    di << " PaveFiller has not been done\n";
    return 0;
  }
  //
  Standard_Boolean bRunParallel;
  Standard_Integer aNb;
  BOPAlgo_BOP      aBOP;
  //
  const ShapeList& aLC = pPF->Arguments();
  aNb                             = aLC.Extent();
  if (aNb != 2)
  {
    di << " wrong number of arguments " << aNb << '\n';
    return 0;
  }
  //
  bRunParallel                             = BOPTest_Objects::RunParallel();
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(di, 1);
  //
  const TopoShape& aS1 = aLC.First();
  const TopoShape& aS2 = aLC.Last();
  //
  aBOP.AddArgument(aS1);
  aBOP.AddTool(aS2);
  aBOP.SetOperation(aOp);
  aBOP.SetRunParallel(bRunParallel);
  aBOP.SetCheckInverted(BOPTest_Objects::CheckInverted());
  aBOP.SetToFillHistory(BRepTest_Objects::IsHistoryNeeded());
  //
  aBOP.PerformWithFiller(*pPF, aProgress->Start());
  BOPTest::ReportAlerts(aBOP.GetReport());

  // Store the history of Boolean operation into the session
  if (BRepTest_Objects::IsHistoryNeeded())
    BRepTest_Objects::SetHistory(aBOP.History());

  if (aBOP.HasErrors())
  {
    return 0;
  }
  //
  const TopoShape& aR = aBOP.Shape();
  if (aR.IsNull())
  {
    di << " null shape\n";
    return 0;
  }
  //
  DBRep1::Set(a[1], aR);
  return 0;
}

//=================================================================================================

Standard_Integer bopsection(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
  {
    di << " use bopsection r\n";
    return 0;
  }
  //
  if (!pPF)
  {
    di << " prepare PaveFiller first\n";
    return 0;
  }
  //
  if (pPF->HasErrors())
  {
    di << " PaveFiller has not been done\n";
    return 0;
  }
  //
  Standard_Boolean bRunParallel;
  Standard_Integer aNb;
  BOPAlgo_Section  aBOP;
  //
  const ShapeList& aLC = pPF->Arguments();
  aNb                             = aLC.Extent();
  if (aNb != 2)
  {
    di << " wrong number of arguments " << aNb << '\n';
    return 0;
  }
  //
  bRunParallel = BOPTest_Objects::RunParallel();
  //
  const TopoShape& aS1 = aLC.First();
  const TopoShape& aS2 = aLC.Last();
  //
  aBOP.AddArgument(aS1);
  aBOP.AddArgument(aS2);
  aBOP.SetRunParallel(bRunParallel);
  aBOP.SetCheckInverted(BOPTest_Objects::CheckInverted());
  aBOP.SetToFillHistory(BRepTest_Objects::IsHistoryNeeded());
  //
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(di, 1);
  aBOP.PerformWithFiller(*pPF, aProgress->Start());
  BOPTest::ReportAlerts(aBOP.GetReport());

  // Store the history of Section operation into the session
  if (BRepTest_Objects::IsHistoryNeeded())
    BRepTest_Objects::SetHistory(aBOP.History());

  if (aBOP.HasErrors())
  {
    return 0;
  }
  //
  const TopoShape& aR = aBOP.Shape();
  if (aR.IsNull())
  {
    di << " null shape\n";
    return 0;
  }
  //
  DBRep1::Set(a[1], aR);
  return 0;
}

//=================================================================================================

Standard_Integer bcommon(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  return bsmt(di, n, a, BOPAlgo_COMMON);
}

//=================================================================================================

Standard_Integer bfuse(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  return bsmt(di, n, a, BOPAlgo_FUSE);
}

//=================================================================================================

Standard_Integer bcut(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  return bsmt(di, n, a, BOPAlgo_CUT);
}

//=================================================================================================

Standard_Integer btuc(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  return bsmt(di, n, a, BOPAlgo_CUT21);
}

//=================================================================================================

Standard_Integer bsection(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 4)
  {
    di << "use bsection r s1 s2 [-n2d/-n2d1/-n2d2] [-na] [tol]\n";
    return 0;
  }
  //
  TopoShape aS1, aS2;
  //
  aS1 = DBRep1::Get(a[2]);
  aS2 = DBRep1::Get(a[3]);
  if (aS1.IsNull() || aS2.IsNull())
  {
    di << " Null shapes are not allowed \n";
    return 0;
  }
  //
  Standard_Boolean bRunParallel, bNonDestructive, bApp, bPC1, bPC2;
  Standard_Real    aTol;
  //
  bApp                   = Standard_True;
  bPC1                   = Standard_True;
  bPC2                   = Standard_True;
  aTol                   = BOPTest_Objects::FuzzyValue();
  bRunParallel           = BOPTest_Objects::RunParallel();
  bNonDestructive        = BOPTest_Objects::NonDestructive();
  BOPAlgo_GlueEnum aGlue = BOPTest_Objects::Glue();
  //
  for (Standard_Integer i = 4; i < n; ++i)
  {
    if (!strcmp(a[i], "-n2d"))
    {
      bPC1 = Standard_False;
      bPC2 = Standard_False;
    }
    else if (!strcmp(a[i], "-n2d1"))
    {
      bPC1 = Standard_False;
    }
    else if (!strcmp(a[i], "-n2d2"))
    {
      bPC2 = Standard_False;
    }
    else if (!strcmp(a[i], "-na"))
    {
      bApp = Standard_False;
    }
  }
  //
  BRepAlgoAPI_Section aSec(aS1, aS2, Standard_False);
  //
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(di, 1);
  aSec.Approximation(bApp);
  aSec.ComputePCurveOn1(bPC1);
  aSec.ComputePCurveOn2(bPC2);
  //
  aSec.SetFuzzyValue(aTol);
  aSec.SetRunParallel(bRunParallel);
  aSec.SetNonDestructive(bNonDestructive);
  aSec.SetGlue(aGlue);
  aSec.SetUseOBB(BOPTest_Objects::UseOBB());
  //
  aSec.Build(aProgress->Start());
  // Store the history of Section operation into the session
  if (BRepTest_Objects::IsHistoryNeeded())
    BRepTest_Objects::SetHistory(aSec.History());

  //
  if (aSec.HasWarnings())
  {
    Standard_SStream aSStream;
    aSec.DumpWarnings(aSStream);
    di << aSStream;
  }
  //
  if (!aSec.IsDone())
  {
    Standard_SStream aSStream;
    aSec.DumpErrors(aSStream);
    di << aSStream;
    return 0;
  }
  //
  const TopoShape& aR = aSec.Shape();
  if (aR.IsNull())
  {
    di << " null shape\n";
    return 0;
  }
  DBRep1::Set(a[1], aR);
  return 0;
}

//=================================================================================================

Standard_Integer bsmt(DrawInterpreter&       di,
                      Standard_Integer        n,
                      const char**            a,
                      const BOPAlgo_Operation aOp)
{
  TopoShape         aS1, aS2;
  ShapeList aLC;
  //
  if (n != 4)
  {
    di << " use bx r s1 s2\n";
    return 0;
  }
  //
  aS1 = DBRep1::Get(a[2]);
  aS2 = DBRep1::Get(a[3]);
  //
  if (aS1.IsNull() || aS2.IsNull())
  {
    di << " null shapes are not allowed \n";
    return 0;
  }
  aLC.Append(aS1);
  aLC.Append(aS2);
  //
  Handle(NCollection_BaseAllocator) aAL = NCollection_BaseAllocator::CommonBaseAllocator();
  //
  BOPAlgo_BOP aBOP(aAL);
  aBOP.AddArgument(aS1);
  aBOP.AddTool(aS2);
  aBOP.SetOperation(aOp);
  // set options
  aBOP.SetGlue(BOPTest_Objects::Glue());
  aBOP.SetFuzzyValue(BOPTest_Objects::FuzzyValue());
  aBOP.SetNonDestructive(BOPTest_Objects::NonDestructive());
  aBOP.SetRunParallel(BOPTest_Objects::RunParallel());
  aBOP.SetUseOBB(BOPTest_Objects::UseOBB());
  aBOP.SetCheckInverted(BOPTest_Objects::CheckInverted());
  aBOP.SetToFillHistory(BRepTest_Objects::IsHistoryNeeded());
  //
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(di, 1);
  aBOP.Perform(aProgress->Start());
  BOPTest::ReportAlerts(aBOP.GetReport());

  // Store the history of Boolean operation into the session
  if (BRepTest_Objects::IsHistoryNeeded())
    BRepTest_Objects::SetHistory(aBOP.PDS()->Arguments(), aBOP);

  if (aBOP.HasErrors())
  {
    return 0;
  }
  const TopoShape& aR = aBOP.Shape();
  if (aR.IsNull())
  {
    di << " null shape\n";
    return 0;
  }
  //
  DBRep1::Set(a[1], aR);
  return 0;
}

//=================================================================================================

Standard_Integer bopcurves(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
  {
    di << "Usage: bopcurves F1 F2 [-2d/-2d1/-2d2] "
          "[-p u1 v1 u2 v2 (to add start points] [-v (for extended output)]\n";
    return 1;
  }
  //
  TopoShape     S1 = DBRep1::Get(a[1]);
  TopoShape     S2 = DBRep1::Get(a[2]);
  TopAbs_ShapeEnum aType;
  //
  if (S1.IsNull() || S2.IsNull())
  {
    di << " Null shapes are not allowed \n";
    return 1;
  }
  //
  aType = S1.ShapeType();
  if (aType != TopAbs_FACE)
  {
    di << " Type mismatch F1\n";
    return 1;
  }
  aType = S2.ShapeType();
  if (aType != TopAbs_FACE)
  {
    di << " Type mismatch F2\n";
    return 1;
  }
  //
  const TopoFace& aF1 = *(TopoFace*)(&S1);
  const TopoFace& aF2 = *(TopoFace*)(&S2);
  //
  Standard_Boolean        aToApproxC3d, aToApproxC2dOnS1, aToApproxC2dOnS2, anIsDone;
  Standard_Integer        aNbCurves, aNbPoints;
  Standard_Real           anAppTol;
  IntSurf_ListOfPntOn2S   aListOfPnts;
  AsciiString1 aNm("c_"), aNp("p_");
  //
  anAppTol         = 0.0000001;
  aToApproxC3d     = Standard_True;
  aToApproxC2dOnS1 = Standard_False;
  aToApproxC2dOnS2 = Standard_False;

  //
  Standard_Boolean bExtOut = Standard_False;
  for (Standard_Integer i = 3; i < n; i++)
  {
    if (!strcasecmp(a[i], "-2d"))
    {
      aToApproxC2dOnS1 = Standard_True;
      aToApproxC2dOnS2 = Standard_True;
    }
    else if (!strcasecmp(a[i], "-2d1"))
    {
      aToApproxC2dOnS1 = Standard_True;
    }
    else if (!strcasecmp(a[i], "-2d2"))
    {
      aToApproxC2dOnS2 = Standard_True;
    }
    else if (!strcasecmp(a[i], "-p"))
    {
      IntSurf_PntOn2S     aPt;
      const Standard_Real aU1 = Draw1::Atof(a[++i]);
      const Standard_Real aV1 = Draw1::Atof(a[++i]);
      const Standard_Real aU2 = Draw1::Atof(a[++i]);
      const Standard_Real aV2 = Draw1::Atof(a[++i]);

      aPt.SetValue(aU1, aV1, aU2, aV2);
      aListOfPnts.Append(aPt);
    }
    else if (!strcasecmp(a[i], "-v"))
    {
      bExtOut = Standard_True;
    }
    else
    {
      di << "Wrong key.\n";
      di << "To build 2d curves use one of the following keys: -2d/-2d1/-2d2\n";
      di << "To add start points use the following key: -p u1 v1 u2 v2\n";
      di << "For extended output use the following key: -v\n";
      return 1;
    }
  }
  //
  IntTools_FaceFace aFF;
  //
  aFF.SetParameters(aToApproxC3d, aToApproxC2dOnS1, aToApproxC2dOnS2, anAppTol);
  aFF.SetList(aListOfPnts);
  aFF.SetFuzzyValue(BOPTest_Objects::FuzzyValue());
  //
  aFF.Perform(aF1, aF2, BOPTest_Objects::RunParallel());
  //
  anIsDone = aFF.IsDone();
  if (!anIsDone)
  {
    di << "Error: Intersection failed\n";
    return 0;
  }
  //
  aFF.PrepareLines3D(Standard_False);
  const IntTools_SequenceOfCurves&      aSCs = aFF.Lines();
  const IntTools_SequenceOfPntOn2Faces& aSPs = aFF.Points();
  //
  aNbCurves = aSCs.Length();
  aNbPoints = aSPs.Length();
  if (!aNbCurves && !aNbPoints)
  {
    di << " has no 3d curves\n";
    di << " has no 3d points\n";
    return 0;
  }
  //
  // curves
  if (aNbCurves)
  {
    Standard_Real aTolR = 0.;
    if (!bExtOut)
    {
      // find maximal tolerance
      for (Standard_Integer i = 1; i <= aNbCurves; i++)
      {
        const IntTools_Curve& anIC = aSCs(i);
        if (aTolR < anIC.Tolerance())
        {
          aTolR = anIC.Tolerance();
        }
      }
      di << "Tolerance Reached=" << aTolR << "\n";
    }
    //
    di << aNbCurves << " curve(s) found.\n";
    //
    for (Standard_Integer i = 1; i <= aNbCurves; i++)
    {
      const IntTools_Curve& anIC = aSCs(i);

      const Handle(GeomCurve3d)& aC3D = anIC.Curve();

      if (aC3D.IsNull())
      {
        di << " has Null 3d curve# " << i << "\n";
        continue;
      }

      AsciiString1 anIndx(i), aNmx;
      aNmx = aNm + anIndx;

      Standard_CString nameC = aNmx.ToCString();

      DrawTrSurf1::Set(nameC, aC3D);
      di << nameC << " ";
      //
      const Handle(GeomCurve2d)& aPC1 = anIC.FirstCurve2d();
      const Handle(GeomCurve2d)& aPC2 = anIC.SecondCurve2d();
      //
      if (!aPC1.IsNull() || !aPC2.IsNull())
      {
        di << "(";
        //
        if (!aPC1.IsNull())
        {
          AsciiString1 pc1N("c2d1_"), pc1Nx;
          pc1Nx                     = pc1N + anIndx;
          Standard_CString nameC2d1 = pc1Nx.ToCString();
          //
          DrawTrSurf1::Set(nameC2d1, aPC1);
          di << nameC2d1;
        }
        //
        if (!aPC2.IsNull())
        {
          AsciiString1 pc2N("c2d2_"), pc2Nx;
          pc2Nx                     = pc2N + anIndx;
          Standard_CString nameC2d2 = pc2Nx.ToCString();
          //
          DrawTrSurf1::Set(nameC2d2, aPC2);
          //
          if (!aPC1.IsNull())
          {
            di << ", ";
          }
          di << nameC2d2;
        }
        di << ") ";
      }
      //
      if (bExtOut)
      {
        di << "\nTolerance: " << anIC.Tolerance() << "\n";
        di << "Tangential tolerance: " << anIC.TangentialTolerance() << "\n";
        di << "\n";
      }
    }
    if (!bExtOut)
    {
      di << "\n";
    }
  }
  //
  // points
  if (aNbPoints)
  {
    di << aNbPoints << " point(s) found.\n";
    //
    for (Standard_Integer i = 1; i <= aNbPoints; i++)
    {
      const IntTools_PntOn2Faces& aPi = aSPs(i);
      const Point3d&               aP  = aPi.P1().Pnt();
      //
      AsciiString1 anIndx(i), aNmx;
      aNmx                   = aNp + anIndx;
      Standard_CString nameP = aNmx.ToCString();
      //
      DrawTrSurf1::Set(nameP, aP);
      di << nameP << " ";
    }
    di << "\n";
  }
  //
  return 0;
}

//=================================================================================================

Standard_Integer mkvolume(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
  {
    di << "Usage: mkvolume r b1 b2 ... [-c] [-ni] [-ai]\n";
    di << "Options:\n";
    di << " -c  - use this option if the arguments are compounds\n";
    di << "       containing shapes that should be interfered;\n";
    di << " -ni - use this option if the arguments should not be interfered;\n";
    di << " -ai - use this option to avoid internal for solids shapes in the result.\n";
    return 1;
  }
  //
  const char* usage = "Type mkvolume without arguments for the usage of the command.\n";
  //
  Standard_Boolean     bToIntersect, bRunParallel, bNonDestructive;
  Standard_Boolean     bCompounds, bAvoidInternal;
  Standard_Integer     i;
  Standard_Real        aTol;
  TopoShape         aS;
  ShapeList aLS;
  //
  aTol                   = BOPTest_Objects::FuzzyValue();
  bRunParallel           = BOPTest_Objects::RunParallel();
  bNonDestructive        = BOPTest_Objects::NonDestructive();
  BOPAlgo_GlueEnum aGlue = BOPTest_Objects::Glue();
  //
  bToIntersect   = Standard_True;
  bCompounds     = Standard_False;
  bAvoidInternal = Standard_False;
  //
  for (i = 2; i < n; ++i)
  {
    aS = DBRep1::Get(a[i]);
    if (!aS.IsNull())
    {
      aLS.Append(aS);
    }
    else
    {
      if (!strcmp(a[i], "-c"))
      {
        bCompounds = Standard_True;
      }
      else if (!strcmp(a[i], "-ni"))
      {
        bToIntersect = Standard_False;
      }
      else if (!strcmp(a[i], "-ai"))
      {
        bAvoidInternal = Standard_True;
      }
    }
  }
  //
  if (aLS.IsEmpty())
  {
    di << "No shapes to process.\n";
    di << usage;
    return 1;
  }
  //
  // treat list of arguments for the case of compounds
  if (bToIntersect && bCompounds)
  {
    ShapeList               aLSx;
    TopTools_ListIteratorOfListOfShape aItLS;
    //
    aItLS.Initialize(aLS);
    for (; aItLS.More(); aItLS.Next())
    {
      const TopoShape& aSx = aItLS.Value();
      TopoDS_Iterator     aItS(aSx);
      for (; aItS.More(); aItS.Next())
      {
        const TopoShape& aSxS = aItS.Value();
        aLSx.Append(aSxS);
      }
    }
    //
    aLS.Clear();
    aLS.Assign(aLSx);
  }
  //
  BOPAlgo_MakerVolume aMV;
  aMV.SetArguments(aLS);
  aMV.SetIntersect(bToIntersect);
  aMV.SetRunParallel(bRunParallel);
  aMV.SetFuzzyValue(aTol);
  aMV.SetNonDestructive(bNonDestructive);
  aMV.SetAvoidInternalShapes(bAvoidInternal);
  aMV.SetGlue(aGlue);
  aMV.SetUseOBB(BOPTest_Objects::UseOBB());
  aMV.SetToFillHistory(BRepTest_Objects::IsHistoryNeeded());
  //
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(di, 1);
  aMV.Perform(aProgress->Start());
  BOPTest::ReportAlerts(aMV.GetReport());

  // Store the history of Volume Maker into the session
  if (BRepTest_Objects::IsHistoryNeeded())
    BRepTest_Objects::SetHistory(aLS, aMV);

  if (aMV.HasErrors())
  {
    return 0;
  }
  //
  const TopoShape& aR = aMV.Shape();
  //
  DBRep1::Set(a[1], aR);
  //
  return 0;
}
