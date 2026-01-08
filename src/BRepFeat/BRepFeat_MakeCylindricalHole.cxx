// Created on: 1995-05-30
// Created by: Jacques GOUSSARD
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

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepFeat_MakeCylindricalHole.hxx>
#include <BRepPrim_Cylinder.hxx>
#include <ElCLib.hxx>
#include <Geom_Curve.hxx>
#include <gp_Ax1.hxx>
#include <LocOpe_CurveShapeIntersector.hxx>
#include <LocOpe_PntFace.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <CSLib.hxx>

static void Baryc(const TopoShape&, Point3d&);

static void BoxParameters(const TopoShape&, const Axis3d&, Standard_Real&, Standard_Real&);

static Standard_Boolean GetOffset(const LocOpe_PntFace& PntInfo,
                                  const Standard_Real   Radius,
                                  const Axis3d&         Axis,
                                  Standard_Real&        outOff);

static void CreateCyl(const LocOpe_PntFace& PntInfoFirst,
                      const LocOpe_PntFace& PntInfoLast,
                      const Standard_Real   Radius,
                      const Axis3d&         Axis,
                      TopoShell&         Cyl,
                      TopoFace&          CylTopF,
                      TopoFace&          CylBottF);

//=================================================================================================

void BRepFeat_MakeCylindricalHole::Perform(const Standard_Real Radius)
{
  const TopoShape& aObject = myArguments.First();
  if (aObject.IsNull() || !myAxDef)
  {
    throw Standard_ConstructionError();
  }

  myIsBlind = Standard_False;
  myStatus  = BRepFeat_NoError;

  CurveShapeIntersector theASI(myAxis, aObject);
  if (!theASI.IsDone() || theASI.NbPoints() <= 0)
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  // It is not possible to use infinite cylinder for topological operations.
  Standard_Real PMin, PMax;
  BoxParameters(aObject, myAxis, PMin, PMax);
  Standard_Real Heigth  = 2. * (PMax - PMin);
  Coords3d        theOrig = myAxis.Location().XYZ();
  theOrig += ((3. * PMin - PMax) / 2.) * myAxis.Direction().XYZ();
  Point3d            p1_ao1(theOrig);
  Frame3d            a1_ao1(p1_ao1, myAxis.Direction());
  BRepPrim_Cylinder theCylinder(a1_ao1, Radius, Heigth);

  // Probably it is better to make cut directly

  ShapeBuilder B;
  TopoSolid theTool;
  B.MakeSolid(theTool);
  B.Add(theTool, theCylinder.Shell());

  myTopFace  = theCylinder.TopFace();
  myBotFace  = theCylinder.BottomFace();
  myValidate = Standard_False;

  //  BRepTools1::Dump(theTool,std::cout);
  Standard_Boolean Fuse = Standard_False;
  //
  AddTool(theTool);
  SetOperation(Fuse);
  BOPAlgo_BOP::Perform();
}

//=================================================================================================

void BRepFeat_MakeCylindricalHole::PerformThruNext(const Standard_Real    Radius,
                                                   const Standard_Boolean Cont)
{
  //
  const TopoShape& aObject = myArguments.First();
  if (aObject.IsNull() || !myAxDef)
  {
    throw Standard_ConstructionError();
  }

  myIsBlind  = Standard_False;
  myValidate = Cont;
  myStatus   = BRepFeat_NoError;

  CurveShapeIntersector theASI(myAxis, aObject);
  if (!theASI.IsDone())
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  Standard_Integer   IndFrom, IndTo;
  TopAbs_Orientation theOr;
  LocOpe_PntFace     PntInfoFirst, PntInfoLast;
  Standard_Boolean   ok = theASI.LocalizeAfter(0., theOr, IndFrom, IndTo);
  if (ok)
  {
    if (theOr == TopAbs_FORWARD)
    {
      PntInfoFirst = theASI.Point(IndFrom);
      ok           = theASI.LocalizeAfter(IndTo, theOr, IndFrom, IndTo);
      if (ok)
      {
        if (theOr != TopAbs_REVERSED)
        {
          ok = Standard_False;
        }
        else
        {
          PntInfoLast = theASI.Point(IndTo);
        }
      }
    }
    else
    { // TopAbs_REVERSED
      PntInfoLast = theASI.Point(IndTo);
      ok          = theASI.LocalizeBefore(IndFrom, theOr, IndFrom, IndTo);
      if (ok)
      {
        if (theOr != TopAbs_FORWARD)
        {
          ok = Standard_False;
        }
        else
        {
          PntInfoFirst = theASI.Point(IndFrom);
        }
      }
    }
  }
  if (!ok)
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  TopoShell Cyl;
  CreateCyl(PntInfoFirst, PntInfoLast, Radius, myAxis, Cyl, myTopFace, myBotFace);

  ShapeBuilder B;
  TopoSolid theTool;
  B.MakeSolid(theTool);
  B.Add(theTool, Cyl);

  Standard_Boolean Fuse = Standard_False;
  AddTool(theTool);
  SetOperation(Fuse);
  BOPAlgo_BOP::Perform();
  ShapeList parts;
  PartsOfTool(parts);

  Standard_Integer                   nbparts = 0;
  TopTools_ListIteratorOfListOfShape its(parts);
  for (; its.More(); its.Next())
  {
    nbparts++;
  }
  if (nbparts == 0)
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  if (nbparts >= 2)
  { // preserve the smallest as parameter
    // along the axis

    Standard_Real First = PntInfoFirst.Parameter();
    Standard_Real Last  = PntInfoLast.Parameter();
    TopoShape  tokeep;
    Standard_Real parbar, parmin = Last;
    Point3d        Barycentre;
    for (its.Initialize(parts); its.More(); its.Next())
    {
      Baryc(its.Value(), Barycentre);
      parbar = ElCLib1::LineParameter(myAxis, Barycentre);
      if (parbar >= First && parbar <= Last && parbar <= parmin)
      {
        parmin = parbar;
        tokeep = its.Value();
      }
    }

    if (tokeep.IsNull())
    { // preserve the closest interval

      Standard_Real dmin = RealLast();
      for (its.Initialize(parts); its.More(); its.Next())
      {
        Baryc(its.Value(), Barycentre);
        parbar = ElCLib1::LineParameter(myAxis, Barycentre);
        if (parbar < First)
        {
          if (First - parbar < dmin)
          {
            dmin   = First - parbar;
            tokeep = its.Value();
          }
          else
          { // parbar > Last
            if (parbar - Last < dmin)
            {
              dmin   = parbar - Last;
              tokeep = its.Value();
            }
          }
        }
      }
    }
    for (its.Initialize(parts); its.More(); its.Next())
    {
      if (tokeep.IsSame(its.Value()))
      {
        KeepPart(its.Value());
        break;
      }
    }
  }
}

//=================================================================================================

void BRepFeat_MakeCylindricalHole::PerformUntilEnd(const Standard_Real    Radius,
                                                   const Standard_Boolean Cont)
{
  //
  const TopoShape& aObject = myArguments.First();
  if (aObject.IsNull() || !myAxDef)
  {
    throw Standard_ConstructionError();
  }

  myIsBlind  = Standard_False;
  myValidate = Cont;
  myStatus   = BRepFeat_NoError;

  CurveShapeIntersector theASI(myAxis, aObject);
  if (!theASI.IsDone())
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  Standard_Integer   IndFrom, IndTo;
  TopAbs_Orientation theOr;
  Standard_Boolean   ok = theASI.LocalizeAfter(0., theOr, IndFrom, IndTo);
  LocOpe_PntFace     PntInfoFirst, PntInfoLast;

  if (ok)
  {
    if (theOr == TopAbs_REVERSED)
    {
      ok = theASI.LocalizeBefore(IndFrom, theOr, IndFrom, IndTo); // on reset
      // It is possible to search for the next.
    }
    if (ok && theOr == TopAbs_FORWARD)
    {
      PntInfoFirst = theASI.Point(IndFrom);
      ok           = theASI.LocalizeBefore(theASI.NbPoints() + 1, theOr, IndFrom, IndTo);
      if (ok)
      {
        if (theOr != TopAbs_REVERSED)
        {
          ok = Standard_False;
        }
        else
        {
          PntInfoLast = theASI.Point(IndTo);
        }
      }
    }
  }
  if (!ok)
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  TopoShell Cyl;
  CreateCyl(PntInfoFirst, PntInfoLast, Radius, myAxis, Cyl, myTopFace, myBotFace);

  ShapeBuilder B;
  TopoSolid theTool;
  B.MakeSolid(theTool);
  B.Add(theTool, Cyl);

  Standard_Boolean Fuse = Standard_False;
  AddTool(theTool);
  SetOperation(Fuse);
  BOPAlgo_BOP::Perform();
  ShapeList parts;
  PartsOfTool(parts);

  Standard_Integer                   nbparts = 0;
  TopTools_ListIteratorOfListOfShape its(parts);
  for (; its.More(); its.Next())
  {
    nbparts++;
  }
  if (nbparts == 0)
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  if (nbparts >= 2)
  { // preserve everything above the First
    Standard_Real parbar;
    Point3d        Barycentre;
    for (its.Initialize(parts); its.More(); its.Next())
    {
      Baryc(its.Value(), Barycentre);
      parbar = ElCLib1::LineParameter(myAxis, Barycentre);
      if (parbar > PntInfoFirst.Parameter())
      {
        KeepPart(its.Value());
      }
    }
  }
}

//=================================================================================================

void BRepFeat_MakeCylindricalHole::Perform(const Standard_Real    Radius,
                                           const Standard_Real    PFrom,
                                           const Standard_Real    PTo,
                                           const Standard_Boolean Cont)
{
  //
  const TopoShape& aObject = myArguments.First();
  if (aObject.IsNull() || !myAxDef)
  {
    throw Standard_ConstructionError();
  }

  myIsBlind  = Standard_False;
  myValidate = Cont;
  myStatus   = BRepFeat_NoError;

  CurveShapeIntersector theASI(myAxis, aObject);
  if (!theASI.IsDone())
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  Standard_Real thePFrom, thePTo;
  if (PFrom < PTo)
  {
    thePFrom = PFrom;
    thePTo   = PTo;
  }
  else
  {
    thePFrom = PTo;
    thePTo   = PFrom;
  }

  // Standard_Real First=0,Last=0,prm;
  LocOpe_PntFace     PntInfoFirst, PntInfoLast;
  Standard_Integer   IndFrom, IndTo;
  TopAbs_Orientation theOr;
  Standard_Boolean   ok = theASI.LocalizeAfter(thePFrom, theOr, IndFrom, IndTo);
  if (ok)
  {
    if (theOr == TopAbs_REVERSED)
    {
      ok = theASI.LocalizeBefore(IndFrom, theOr, IndFrom, IndTo); // reset
      // It is possible to find the next.
    }
    if (ok && theOr == TopAbs_FORWARD)
    {
      PntInfoFirst = theASI.Point(IndFrom);
      ok           = theASI.LocalizeBefore(thePTo, theOr, IndFrom, IndTo);
      if (ok)
      {
        if (theOr == TopAbs_FORWARD)
        {
          ok = theASI.LocalizeAfter(IndTo, theOr, IndFrom, IndTo);
        }
        if (ok && theOr == TopAbs_REVERSED)
        {
          PntInfoLast = theASI.Point(IndTo);
        }
      }
    }
  }

  if (!ok)
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  TopoShell Cyl;
  CreateCyl(PntInfoFirst, PntInfoLast, Radius, myAxis, Cyl, myTopFace, myBotFace);

  ShapeBuilder B;
  TopoSolid theTool;
  B.MakeSolid(theTool);
  B.Add(theTool, Cyl);

  Standard_Boolean Fuse = Standard_False;
  AddTool(theTool);
  SetOperation(Fuse);
  BOPAlgo_BOP::Perform();
  ShapeList parts;
  PartsOfTool(parts);

  Standard_Integer                   nbparts = 0;
  TopTools_ListIteratorOfListOfShape its(parts);
  for (; its.More(); its.Next())
  {
    nbparts++;
  }
  if (nbparts == 0)
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  if (nbparts >= 2)
  { // preserve parts between First and Last

    TopoShape  tokeep;
    Standard_Real parbar;
    Point3d        Barycentre;
    for (its.Initialize(parts); its.More(); its.Next())
    {
      Baryc(its.Value(), Barycentre);
      parbar = ElCLib1::LineParameter(myAxis, Barycentre);
      if (!(parbar < PntInfoFirst.Parameter() || parbar > PntInfoLast.Parameter()))
      {
        KeepPart(its.Value());
      }
    }
  }
}

//=================================================================================================

void BRepFeat_MakeCylindricalHole::PerformBlind(const Standard_Real    Radius,
                                                const Standard_Real    Length,
                                                const Standard_Boolean Cont)
{
  //
  const TopoShape& aObject = myArguments.First();
  if (aObject.IsNull() || !myAxDef || Length <= 0.)
  {
    throw Standard_ConstructionError();
  }

  myIsBlind  = Standard_True;
  myValidate = Cont;
  myStatus   = BRepFeat_NoError;

  CurveShapeIntersector theASI(myAxis, aObject);
  if (!theASI.IsDone())
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  Standard_Real      First;
  Standard_Integer   IndFrom, IndTo;
  TopAbs_Orientation theOr;
  Standard_Boolean   ok = theASI.LocalizeAfter(0., theOr, IndFrom, IndTo);

  if (ok)
  {
    if (theOr == TopAbs_REVERSED)
    {
      ok = theASI.LocalizeBefore(IndFrom, theOr, IndFrom, IndTo); // reset
      // it is possible to find the next
    }
    ok = ok && theOr == TopAbs_FORWARD;
  }
  if (!ok)
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  // check a priori the length of the hole
  Standard_Integer IFNext, ITNext;
  ok = theASI.LocalizeAfter(IndTo, theOr, IFNext, ITNext);
  if (!ok)
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }
  if (theASI.Point(IFNext).Parameter() <= Length)
  {
    myStatus = BRepFeat_HoleTooLong;
    return;
  }

  ShapeList theList;

  // version for advanced control
  for (Standard_Integer i = IndFrom; i <= ITNext; i++)
  {
    theList.Append(theASI.Point(i).Face());
  }

  First = theASI.Point(IndFrom).Parameter();

  //// It is not possible to use infinite cylinder for topological operations.
  Standard_Real PMin, PMax;
  BoxParameters(aObject, myAxis, PMin, PMax);
  if (PMin > Length)
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  Standard_Real Heigth  = 3. * (Length - PMin) / 2.;
  Coords3d        theOrig = myAxis.Location().XYZ();
  theOrig += ((3. * PMin - Length) / 2.) * myAxis.Direction().XYZ();
  Point3d            p5_ao1(theOrig);
  Frame3d            a5_ao1(p5_ao1, myAxis.Direction());
  BRepPrim_Cylinder theCylinder(a5_ao1, Radius, Heigth);

  ShapeBuilder B;
  TopoSolid theTool;
  B.MakeSolid(theTool);
  B.Add(theTool, theCylinder.Shell());

  myTopFace = theCylinder.TopFace();
  myBotFace.Nullify();

  //  BRepTools1::Dump(theTool,std::cout);
  Standard_Boolean Fuse = Standard_False;
  // myBuilder.Perform(theTool,theList,Fuse);
  // myBuilder.BuildPartsOfTool();
  AddTool(theTool);
  SetOperation(Fuse);
  BOPAlgo_BOP::Perform();
  ShapeList parts;
  PartsOfTool(parts);

  Standard_Integer                   nbparts = 0;
  TopTools_ListIteratorOfListOfShape its(parts);
  for (; its.More(); its.Next())
  {
    nbparts++;
  }
  if (nbparts == 0)
  {
    myStatus = BRepFeat_InvalidPlacement;
    return;
  }

  if (nbparts >= 2)
  { // preserve the smallest as parameter along the axis
    TopoShape  tokeep;
    Standard_Real parbar, parmin = RealLast();
    Point3d        Barycentre;
    for (its.Initialize(parts); its.More(); its.Next())
    {
      Baryc(its.Value(), Barycentre);
      parbar = ElCLib1::LineParameter(myAxis, Barycentre);
      if (parbar >= First && parbar <= parmin)
      {
        parmin = parbar;
        tokeep = its.Value();
      }
    }

    if (tokeep.IsNull())
    { // preserve the closest interval

      Standard_Real dmin = RealLast();
      for (its.Initialize(parts); its.More(); its.Next())
      {
        Baryc(its.Value(), Barycentre);
        parbar = ElCLib1::LineParameter(myAxis, Barycentre);
        if (Abs(First - parbar) < dmin)
        {
          dmin   = Abs(First - parbar);
          tokeep = its.Value();
        }
      }
    }
    for (its.Initialize(parts); its.More(); its.Next())
    {
      if (tokeep.IsSame(its.Value()))
      {
        KeepPart(its.Value());
        break;
      }
    }
  }
}

//=================================================================================================

void BRepFeat_MakeCylindricalHole::Build()
{
  if (myStatus == BRepFeat_NoError)
  {
    PerformResult();
    if (!HasErrors())
    {
      myStatus = (myValidate) ? Validate() : BRepFeat_NoError;
      if (myStatus == BRepFeat_NoError)
      {
        myShape = Shape();
      }
    }
    else
    {
      myStatus = BRepFeat_InvalidPlacement; // why not
    }
  }
}

//=================================================================================================

BRepFeat_Status BRepFeat_MakeCylindricalHole::Validate()
{
  BRepFeat_Status thestat = BRepFeat_NoError;
  ShapeExplorer ex(Shape(), TopAbs_FACE);
  if (myIsBlind)
  { // limit of the hole
    for (; ex.More(); ex.Next())
    {
      if (ex.Current().IsSame(myTopFace))
      {
        break;
      }
    }
    if (!ex.More())
    {
      thestat = BRepFeat_HoleTooLong;
    }
  }
  else
  {
    for (; ex.More(); ex.Next())
    {
      if (ex.Current().IsSame(myTopFace))
      {
        return BRepFeat_InvalidPlacement;
      }
    }
    for (ex.ReInit(); ex.More(); ex.Next())
    {
      if (ex.Current().IsSame(myBotFace))
      {
        return BRepFeat_InvalidPlacement;
      }
    }
  }
  return thestat;
}

void Baryc(const TopoShape& S, Point3d& B)
{
  ShapeExplorer    exp(S, TopAbs_EDGE);
  Coords3d             Bar(0., 0., 0.);
  TopLoc_Location    L;
  Handle(GeomCurve3d) C;
  Standard_Real      prm, First, Last;

  Standard_Integer i, nbp = 0;
  for (; exp.More(); exp.Next())
  {
    // Calculate points by non-degenerated edges
    const TopoEdge& E = TopoDS::Edge(exp.Current());
    if (!BRepInspector::Degenerated(E))
    {
      C = BRepInspector::Curve(E, L, First, Last);
      C = Handle(GeomCurve3d)::DownCast(C->Transformed(L.Transformation()));
      for (i = 1; i <= 11; i++)
      {
        prm = ((11 - i) * First + (i - 1) * Last) / 10.;
        Bar += C->Value(prm).XYZ();
        nbp++;
      }
    }
  }
  Bar.Divide((Standard_Real)nbp);
  B.SetXYZ(Bar);
}

void BoxParameters(const TopoShape& S,
                   const Axis3d&       Axis,
                   Standard_Real&      parmin,
                   Standard_Real&      parmax)
{

  // calculate the parameters of a bounding box in the direction of the axis of the hole
  Box2 B;
  BRepBndLib::Add(S, B);
  Standard_Real c[6];
  B.Get(c[0], c[2], c[4], c[1], c[3], c[5]);
  Point3d           P;
  Standard_Integer i, j, k;
  parmin = RealLast();
  parmax = RealFirst();
  Standard_Real param;
  for (i = 0; i <= 1; i++)
  {
    P.SetX(c[i]);
    for (j = 2; j <= 3; j++)
    {
      P.SetY(c[j]);
      for (k = 4; k <= 5; k++)
      {
        P.SetZ(c[k]);
        param  = ElCLib1::LineParameter(Axis, P);
        parmin = Min(param, parmin);
        parmax = Max(param, parmax);
      }
    }
  }
}

Standard_Boolean GetOffset(const LocOpe_PntFace& PntInfo,
                           const Standard_Real   Radius,
                           const Axis3d&         Axis,
                           Standard_Real&        outOff)
{
  const TopoFace&  FF = PntInfo.Face();
  BRepAdaptor_Surface FFA(FF);

  Standard_Real Up = PntInfo.UParameter();
  Standard_Real Vp = PntInfo.VParameter();
  Point3d        PP;
  Vector3d        D1U, D1V;
  FFA.D1(Up, Vp, PP, D1U, D1V);
  Dir3d             NormF;
  CSLib_NormalStatus stat;
  CSLib1::Normal(D1U, D1V, Precision::Angular(), stat, NormF);
  if (stat != CSLib_Defined)
    return Standard_False;
  Standard_Real angle = Axis.Direction().Angle(NormF);
  if (Abs(M_PI / 2. - angle) < Precision::Angular())
    return Standard_False;
  outOff = Radius * Abs(tan(angle));
  return Standard_True;
}

void CreateCyl(const LocOpe_PntFace& PntInfoFirst,
               const LocOpe_PntFace& PntInfoLast,
               const Standard_Real   Radius,
               const Axis3d&         Axis,
               TopoShell&         Cyl,
               TopoFace&          CylTopF,
               TopoFace&          CylBottF)
{
  Standard_Real First = 0, Last = 0;
  double        offF = 0., offL = 0.;
  Last                 = PntInfoLast.Parameter();
  First                = PntInfoFirst.Parameter();
  Standard_Real Heigth = Last - First;

  if (!GetOffset(PntInfoFirst, Radius, Axis, offF))
    offF = Radius;
  if (!GetOffset(PntInfoLast, Radius, Axis, offL))
    offL = Radius;

  // create cylinder along the axis (myAxis);
  // from 'First - offF' to 'Last + offL' params
  Coords3d            theOrig = PntInfoFirst.Pnt().XYZ() - offF * Axis.Direction().XYZ();
  Point3d            p2_ao1(theOrig);
  Frame3d            a2_ao1(p2_ao1, Axis.Direction());
  BRepPrim_Cylinder theCylinder(a2_ao1, Radius, Heigth + offF + offL);
  Cyl      = theCylinder.Shell();
  CylTopF  = theCylinder.TopFace();
  CylBottF = theCylinder.BottomFace();
}
