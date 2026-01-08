// Created on: 1995-04-05
// Created by: Christophe MARION
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

#include <HLRTest.hxx>

#include <DBRep.hxx>
#include <Draw_Appli.hxx>
#include <HLRAppli_ReflectLines.hxx>
#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <HLRTest_OutLiner.hxx>
#include <HLRTest_Projector.hxx>
#include <HLRTopoBRep_OutLiner.hxx>
#include <TopoDS_Shape.hxx>
#include <BRep_Builder.hxx>

static Handle(HLRBRep_Algo) hider;
#ifdef _WIN32
Standard_IMPORT DrawViewer dout;
#endif

#include <BRepTopAdaptor_MapOfShapeTool.hxx>

//=================================================================================================

void HLRTest::Set(const Standard_CString Name, const HLRAlgoProjector& P)
{
  Draw1::Set(Name, new HLRTest_Projector(P));
}

//=================================================================================================

Standard_Boolean HLRTest::GetProjector(Standard_CString& Name, HLRAlgoProjector& P)
{
  Handle(HLRTest_Projector) HP = Handle(HLRTest_Projector)::DownCast(Draw1::Get(Name));
  if (HP.IsNull())
    return Standard_False;
  P = HP->Projector();
  return Standard_True;
}

//=================================================================================================

void HLRTest::Set(const Standard_CString Name, const TopoShape& S)
{
  Draw1::Set(Name, new HLRTest_OutLiner(S));
}

//=================================================================================================

Handle(HLRTopoBRep_OutLiner) HLRTest::GetOutLiner(Standard_CString& Name)
{
  Handle(Draw_Drawable3D)  D  = Draw1::Get(Name);
  Handle(HLRTest_OutLiner) HS = Handle(HLRTest_OutLiner)::DownCast(D);
  if (!HS.IsNull())
    return HS->OutLiner();
  Handle(HLRTopoBRep_OutLiner) HO;
  return HO;
}

//=================================================================================================

static Standard_Integer hprj(DrawInterpreter&, Standard_Integer n, const char** a)
{
  if (n < 2)
    return 1;
  //
  Frame3d anAx2 = gp1::XOY();
  if (n == 11)
  {
    Standard_Real x = Draw1::Atof(a[2]);
    Standard_Real y = Draw1::Atof(a[3]);
    Standard_Real z = Draw1::Atof(a[4]);

    Standard_Real dx = Draw1::Atof(a[5]);
    Standard_Real dy = Draw1::Atof(a[6]);
    Standard_Real dz = Draw1::Atof(a[7]);

    Standard_Real dx1 = Draw1::Atof(a[8]);
    Standard_Real dy1 = Draw1::Atof(a[9]);
    Standard_Real dz1 = Draw1::Atof(a[10]);

    Point3d anOrigin(x, y, z);
    Dir3d aNormal(dx, dy, dz);
    Dir3d aDX(dx1, dy1, dz1);
    anAx2 = Frame3d(anOrigin, aNormal, aDX);
  }

  HLRAlgoProjector P(anAx2);
  HLRTest::Set(a[1], P);
  return 0;
}

//=================================================================================================

static Standard_Integer hout(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
    return 1;
  const char*  name = a[2];
  TopoShape S    = DBRep1::Get(name);
  if (S.IsNull())
  {
    di << name << " is not a shape.\n";
    return 1;
  }
  HLRTest::Set(a[1], S);
  return 0;
}

//=================================================================================================

static Standard_Integer hfil(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  Standard_Integer nbIso = 0;
  if (n < 3)
    return 1;
  if (n > 3)
    nbIso = Draw1::Atoi(a[3]);
  const char*                  name1 = a[1];
  Handle(HLRTopoBRep_OutLiner) HS    = HLRTest::GetOutLiner(name1);
  if (HS.IsNull())
  {
    di << name1 << " is not an OutLiner.\n";
    return 1;
  }
  const char*       name2 = a[2];
  HLRAlgoProjector P;
  if (!HLRTest::GetProjector(name2, P))
  {
    di << name2 << " is not a projector.\n";
    return 1;
  }
  BRepTopAdaptor_MapOfShapeTool MST;
  HS->Fill(P, MST, nbIso);
  return 0;
}

//=================================================================================================

static Standard_Integer sori(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
    return 1;
  const char*                  name1 = a[1];
  const char*                  name2 = a[2];
  Handle(HLRTopoBRep_OutLiner) HS    = HLRTest::GetOutLiner(name2);
  if (HS.IsNull())
  {
    di << name2 << " is not an OutLiner.\n";
    return 1;
  }
  DBRep1::Set(name1, HS->OriginalShape());
  return 0;
}

//=================================================================================================

static Standard_Integer sout(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
    return 1;
  const char*                  name1 = a[1];
  const char*                  name2 = a[2];
  Handle(HLRTopoBRep_OutLiner) HS    = HLRTest::GetOutLiner(name2);
  if (HS.IsNull())
  {
    di << name2 << " is not an OutLiner.\n";
    return 1;
  }
  if (HS->OutLinedShape().IsNull())
  {
    di << name2 << " has no OutLinedShape.\n";
    return 1;
  }
  DBRep1::Set(name1, HS->OutLinedShape());
  return 0;
}

//=================================================================================================

static Standard_Integer hloa(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
    return 1;
  const char*                  name1 = a[1];
  Handle(HLRTopoBRep_OutLiner) HS    = HLRTest::GetOutLiner(name1);
  if (HS.IsNull())
  {
    di << name1 << " is not an OutLiner.\n";
    return 1;
  }
  hider->Load(HS);
  return 0;
}

//=================================================================================================

static Standard_Integer hrem(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n > 1)
  {
    const char*                  name = a[1];
    Standard_Integer             index;
    Handle(HLRTopoBRep_OutLiner) HS = HLRTest::GetOutLiner(name);
    if (HS.IsNull())
    {
      TopoShape S = DBRep1::Get(name);
      if (S.IsNull())
      {
        di << name << " is not an OutLiner and not a shape.\n";
        return 1;
      }
      else
      {
        index = hider->Index(S);
        if (index == 0)
        {
          di << name << " not loaded shape.\n";
          return 1;
        }
      }
    }
    else
    {
      index = hider->Index(HS->OriginalShape());
      if (index == 0)
      {
        di << name << " not loaded outliner.\n";
        return 1;
      }
    }
    hider->Remove(index);
    di << name << " removed\n";
  }
  else
  {
    while (hider->NbShapes1() > 0)
    {
      hider->Remove(1);
    }
    di << " all shapes removed\n";
  }
  return 0;
}

//=================================================================================================

static Standard_Integer sprj(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
    return 1;
  const char*       name = a[1];
  HLRAlgoProjector P;
  if (!HLRTest::GetProjector(name, P))
  {
    di << name << " is not a projector.\n";
    return 1;
  }
  hider->Projector(P);
  return 0;
}

//=================================================================================================

static Standard_Integer upda(DrawInterpreter&, Standard_Integer, const char**)
{
  hider->Update();
  return 0;
}

//=================================================================================================

static Standard_Integer hide(DrawInterpreter&, Standard_Integer, const char**)
{
  hider->Hide();
  return 0;
}

//=================================================================================================

static Standard_Integer show(DrawInterpreter&, Standard_Integer, const char**)
{
  hider->ShowAll();
  return 0;
}

//=================================================================================================

static Standard_Integer hdbg(DrawInterpreter& di, Standard_Integer, const char**)
{
  hider->Debug(!hider->Debug());
  if (hider->Debug())
    di << "debug\n";
  else
    di << "no debug\n";
  return 0;
}

//=================================================================================================

static Standard_Integer hnul(DrawInterpreter&, Standard_Integer, const char**)
{
  hider->OutLinedShapeNullify();
  return 0;
}

//=================================================================================================

static Standard_Integer hres(DrawInterpreter&, Standard_Integer n, const char** a)
{
  TopoShape S, V, V1, VN, VO, VI, H, H1, HN, HO, HI;
  if (n > 1)
  {
    const char* name = a[1];
    S                = DBRep1::Get(name);
  }
  HLRBRep_HLRToShape HS(hider);

  if (S.IsNull())
  {
    V  = HS.VCompound();
    V1 = HS.Rg1LineVCompound();
    VN = HS.RgNLineVCompound();
    VO = HS.OutLineVCompound();
    VI = HS.IsoLineVCompound();
    H  = HS.HCompound();
    H1 = HS.Rg1LineHCompound();
    HN = HS.RgNLineHCompound();
    HO = HS.OutLineHCompound();
    HI = HS.IsoLineHCompound();
  }
  else
  {
    V  = HS.VCompound(S);
    V1 = HS.Rg1LineVCompound(S);
    VN = HS.RgNLineVCompound(S);
    VO = HS.OutLineVCompound(S);
    VI = HS.IsoLineVCompound(S);
    H  = HS.HCompound(S);
    H1 = HS.Rg1LineHCompound(S);
    HN = HS.RgNLineHCompound(S);
    HO = HS.OutLineHCompound(S);
    HI = HS.IsoLineHCompound(S);
  }
  if (!V.IsNull())
    DBRep1::Set("vl", V);
  if (!V1.IsNull())
    DBRep1::Set("v1l", V1);
  if (!VN.IsNull())
    DBRep1::Set("vnl", VN);
  if (!VO.IsNull())
    DBRep1::Set("vol", VO);
  if (!VI.IsNull())
    DBRep1::Set("vil", VI);
  if (!H.IsNull())
    DBRep1::Set("hl", H);
  if (!H1.IsNull())
    DBRep1::Set("h1l", H1);
  if (!HN.IsNull())
    DBRep1::Set("hnl", HN);
  if (!HO.IsNull())
    DBRep1::Set("hol", HO);
  if (!HI.IsNull())
    DBRep1::Set("hil", HI);
  return 0;
}

//=================================================================================================

static Standard_Integer reflectlines(DrawInterpreter&, Standard_Integer n, const char** a)
{
  if (n < 6)
    return 1;

  TopoShape aShape = DBRep1::Get(a[2]);
  if (aShape.IsNull())
    return 1;

  Standard_Real anAISViewProjX = atof(a[3]);
  Standard_Real anAISViewProjY = atof(a[4]);
  Standard_Real anAISViewProjZ = atof(a[5]);

  Point3d anOrigin(0., 0., 0.);
  Dir3d aNormal(anAISViewProjX, anAISViewProjY, anAISViewProjZ);
  Frame3d theAxes(anOrigin, aNormal);
  Dir3d aDX = theAxes.XDirection();

  HLRAppli_ReflectLines Reflector(aShape);

  Reflector.SetAxes(aNormal.X(),
                    aNormal.Y(),
                    aNormal.Z(),
                    anOrigin.X(),
                    anOrigin.Y(),
                    anOrigin.Z(),
                    aDX.X(),
                    aDX.Y(),
                    aDX.Z());

  Reflector.Perform();

  TopoShape Result = Reflector.GetResult();
  DBRep1::Set(a[1], Result);

  return 0;
}

//=================================================================================================

static Standard_Integer hlrin3d(DrawInterpreter&, Standard_Integer n, const char** a)
{
  if (n < 6)
    return 1;

  TopoShape aShape = DBRep1::Get(a[2]);
  if (aShape.IsNull())
    return 1;

  Standard_Real anAISViewProjX = atof(a[3]);
  Standard_Real anAISViewProjY = atof(a[4]);
  Standard_Real anAISViewProjZ = atof(a[5]);

  Point3d anOrigin(0., 0., 0.);
  Dir3d aNormal(anAISViewProjX, anAISViewProjY, anAISViewProjZ);
  Frame3d theAxes(anOrigin, aNormal);
  Dir3d aDX = theAxes.XDirection();

  HLRAppli_ReflectLines Reflector(aShape);

  Reflector.SetAxes(aNormal.X(),
                    aNormal.Y(),
                    aNormal.Z(),
                    anOrigin.X(),
                    anOrigin.Y(),
                    anOrigin.Z(),
                    aDX.X(),
                    aDX.Y(),
                    aDX.Z());

  Reflector.Perform();

  TopoCompound Result;
  ShapeBuilder    BB;
  BB.MakeCompound(Result);

  TopoShape SharpEdges =
    Reflector.GetCompoundOf3dEdges(HLRBRep_Sharp, Standard_True, Standard_True);
  if (!SharpEdges.IsNull())
    BB.Add(Result, SharpEdges);
  TopoShape OutLines =
    Reflector.GetCompoundOf3dEdges(HLRBRep_OutLine, Standard_True, Standard_True);
  if (!OutLines.IsNull())
    BB.Add(Result, OutLines);
  TopoShape SmoothEdges =
    Reflector.GetCompoundOf3dEdges(HLRBRep_Rg1Line, Standard_True, Standard_True);
  if (!SmoothEdges.IsNull())
    BB.Add(Result, SmoothEdges);

  DBRep1::Set(a[1], Result);

  return 0;
}

//=================================================================================================

static Standard_Integer hlrin2d(DrawInterpreter&, Standard_Integer n, const char** a)
{
  if (n < 9)
    return 1;

  TopoShape aShape = DBRep1::Get(a[2]);
  if (aShape.IsNull())
    return 1;

  Standard_Real anAISViewProjX = atof(a[3]);
  Standard_Real anAISViewProjY = atof(a[4]);
  Standard_Real anAISViewProjZ = atof(a[5]);

  Standard_Real Eye_X = atof(a[6]);
  Standard_Real Eye_Y = atof(a[7]);
  Standard_Real Eye_Z = atof(a[8]);

  Point3d anOrigin(0., 0., 0.);
  Dir3d aNormal(anAISViewProjX, anAISViewProjY, anAISViewProjZ);
  Dir3d aDX(Eye_X, Eye_Y, Eye_Z);

  HLRAppli_ReflectLines Reflector(aShape);

  Reflector.SetAxes(aNormal.X(),
                    aNormal.Y(),
                    aNormal.Z(),
                    anOrigin.X(),
                    anOrigin.Y(),
                    anOrigin.Z(),
                    aDX.X(),
                    aDX.Y(),
                    aDX.Z());

  Reflector.Perform();

  TopoCompound Result;
  ShapeBuilder    BB;
  BB.MakeCompound(Result);

  TopoShape SharpEdges =
    Reflector.GetCompoundOf3dEdges(HLRBRep_Sharp, Standard_True, Standard_False);
  if (!SharpEdges.IsNull())
    BB.Add(Result, SharpEdges);
  TopoShape OutLines =
    Reflector.GetCompoundOf3dEdges(HLRBRep_OutLine, Standard_True, Standard_False);
  if (!OutLines.IsNull())
    BB.Add(Result, OutLines);
  TopoShape SmoothEdges =
    Reflector.GetCompoundOf3dEdges(HLRBRep_Rg1Line, Standard_True, Standard_False);
  if (!SmoothEdges.IsNull())
    BB.Add(Result, SmoothEdges);

  DBRep1::Set(a[1], Result);

  return 0;
}

//=================================================================================================

void HLRTest::Commands(DrawInterpreter& theCommands)
{
  // Register save/restore tool
  HLRTest_Projector::RegisterFactory();

  const char* g = "ADVALGOS HLR Commands";

  theCommands.Add("hprj", "hprj name [view-id = 1]", __FILE__, hprj, g);
  theCommands.Add("houtl", "houtl name shape", __FILE__, hout, g);
  theCommands.Add("hfill", "hfill name proj [nbIso]", __FILE__, hfil, g);
  theCommands.Add("hsin", "hsin name outliner", __FILE__, sori, g);
  theCommands.Add("hsout", "hsout name outliner", __FILE__, sout, g);
  theCommands.Add("hload", "hload outliner", __FILE__, hloa, g);
  theCommands.Add("hremove", "hremove [name]", __FILE__, hrem, g);
  theCommands.Add("hsetprj", "hsetprj [name]", __FILE__, sprj, g);
  theCommands.Add("hupdate", "hupdate", __FILE__, upda, g);
  theCommands.Add("hhide", "hhide", __FILE__, hide, g);
  theCommands.Add("hshowall", "hshowall", __FILE__, show, g);
  theCommands.Add("hdebug", "hdebug", __FILE__, hdbg, g);
  theCommands.Add("hnullify", "hnullify", __FILE__, hnul, g);
  theCommands.Add("hres2d", "hres2d", __FILE__, hres, g);

  theCommands.Add("reflectlines",
                  "reflectlines res shape proj_X proj_Y proj_Z",
                  __FILE__,
                  reflectlines,
                  g);

  theCommands.Add("hlrin3d", "hlrin3d res shape proj_X proj_Y proj_Z", __FILE__, hlrin3d, g);

  theCommands.Add("hlrin2d",
                  "hlrin2d res shape proj_X proj_Y proj_Z eye_x eye_y eye_z",
                  __FILE__,
                  hlrin2d,
                  g);

  hider = new HLRBRep_Algo();
}
