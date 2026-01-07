// Created on: 1994-12-13
// Created by: Jacques GOUSSARD
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

#include <Standard_Stream.hxx>
#include <Standard_Macro.hxx>

#include <BRepTest.hxx>

#include <DBRep.hxx>
#include <Draw_Appli.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_Box.hxx>

#include <BRepBuilderAPI.hxx>
#include <BRepBuilderAPI_FindPlane.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_NurbsConvert.hxx>
#include <gp_Ax2.hxx>
#include <gp_Mat.hxx>
#include <gp_GTrsf.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <BRepOffsetAPI_NormalProjection.hxx>
#include <BRepLib.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <Bnd_Box2d.hxx>
#include <Message.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRepTools_WireExplorer.hxx>

#include <GCPnts_QuasiUniformAbscissa.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <ProjLib_ComputeApproxOnPolarSurface.hxx>
#include <DrawTrSurf.hxx>
#include <Geom_Plane.hxx>

#include <Draw_Segment3D.hxx>
#include <Draw_Marker3D.hxx>
#include <Draw_MarkerShape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepTools_PurgeLocations.hxx>
#include <BRepTools.hxx>
#include <Standard_Dump.hxx>

Standard_IMPORT DrawViewer dout;

//=======================================================================
// function : ConvertBndToShape
// purpose  : Creates TopoSolid from theBox
//=======================================================================
static void ConvertBndToShape(const OrientedBox& theBox, const char* const theName)
{
  if (theBox.IsVoid())
  {
    DBRep1::Set(theName, TopoShape());
    return;
  }

  const Point3d& aBaryCenter = theBox.Center();
  const gp_XYZ &aXDir = theBox.XDirection(), &aYDir = theBox.YDirection(),
               &aZDir  = theBox.ZDirection();
  Standard_Real aHalfX = theBox.XHSize(), aHalfY = theBox.YHSize(), aHalfZ = theBox.ZHSize();

  Frame3d anAxes(aBaryCenter, aZDir, aXDir);
  anAxes.SetLocation(aBaryCenter.XYZ() - aHalfX * aXDir - aHalfY * aYDir - aHalfZ * aZDir);
  TopoSolid aBox = BoxMaker(anAxes, 2.0 * aHalfX, 2.0 * aHalfY, 2.0 * aHalfZ);
  DBRep1::Set(theName, aBox);
}

//=======================================================================
// addpcurve
//=======================================================================

static Standard_Integer addpcurve(DrawInterpreter&, Standard_Integer n, const char** a)
{
  if (n < 4)
    return 1;
  TopoShape E = DBRep1::Get(a[1]);
  if (E.IsNull())
    return 1;
  Handle(GeomCurve2d) PC  = DrawTrSurf1::GetCurve2d(a[2]);
  TopoShape         F   = DBRep1::Get(a[3]);
  Standard_Real        tol = 1.e-7;
  if (n > 4)
  {
    tol = Draw1::Atof(a[4]);
  }
  ShapeBuilder BB;
  BB.UpdateEdge(TopoDS::Edge(E), PC, TopoDS::Face(F), tol);
  DBRep1::Set(a[1], E);
  return 0;
}

//=======================================================================
// transform
//=======================================================================

static Standard_Integer transform(DrawInterpreter&, Standard_Integer n, const char** a)
{
  if (n <= 1)
    return 1;

  Transform3d          T;
  Standard_Integer last  = n;
  const char*      aName = a[0];

  Standard_Boolean isBasic    = Standard_False;
  Standard_Boolean isForced   = Standard_False;
  Standard_Boolean isCopy     = Standard_False;
  Standard_Boolean isCopyMesh = Standard_False;

  // Check "copymesh" flag.
  if (!strcmp(a[n - 1], "-copymesh"))
  {
    isCopyMesh = Standard_True;
    last       = --n;
  }
  // Check "copy" flag.
  if (!strcmp(a[n - 1], "-copy"))
  {
    isCopy = Standard_True;
    last   = --n;
  }

  if (!strcmp(aName, "reset"))
  {
  }
  else
  {
    isBasic  = (aName[0] == 'b');
    isForced = (aName[0] == 'f');

    aName++;

    if (!strcmp(aName, "move"))
    {
      if (n < 3)
        return 1;
      TopoShape SL = DBRep1::Get(a[n - 1]);
      if (SL.IsNull())
        return 0;
      T       = SL.Location().Transformation();
      last    = n - 1;
      isBasic = Standard_True;
    }
    else if (!strcmp(aName, "translate"))
    {
      if (n < 5)
        return 1;
      T.SetTranslation(Vector3d(Draw1::Atof(a[n - 3]), Draw1::Atof(a[n - 2]), Draw1::Atof(a[n - 1])));
      last = n - 3;
    }
    else if (!strcmp(aName, "rotate"))
    {
      if (n < 9)
        return 1;
      T.SetRotation(
        Axis3d(Point3d(Draw1::Atof(a[n - 7]), Draw1::Atof(a[n - 6]), Draw1::Atof(a[n - 5])),
               Vector3d(Draw1::Atof(a[n - 4]), Draw1::Atof(a[n - 3]), Draw1::Atof(a[n - 2]))),
        Draw1::Atof(a[n - 1]) * (M_PI / 180.0));
      last = n - 7;
    }
    else if (!strcmp(aName, "mirror"))
    {
      if (n < 8)
        return 1;
      T.SetMirror(Frame3d(Point3d(Draw1::Atof(a[n - 6]), Draw1::Atof(a[n - 5]), Draw1::Atof(a[n - 4])),
                         Vector3d(Draw1::Atof(a[n - 3]), Draw1::Atof(a[n - 2]), Draw1::Atof(a[n - 1]))));

      last = n - 6;
    }
    else if (!strcmp(aName, "scale"))
    {
      if (n < 6)
        return 1;
      T.SetScale(Point3d(Draw1::Atof(a[n - 4]), Draw1::Atof(a[n - 3]), Draw1::Atof(a[n - 2])),
                 Draw1::Atof(a[n - 1]));
      last = n - 4;
    }
  }

  if (T.Form() == gp_Identity || isBasic || isForced)
  {
    Standard_Boolean isExeption = Standard_True;
    if (isForced)
    {
      isExeption = Standard_False;
    }
    TopLoc_Location L(T);
    for (Standard_Integer i = 1; i < last; i++)
    {
      TopoShape S = DBRep1::Get(a[i]);
      if (S.IsNull())
      {
        Message::SendFail() << "Error: " << a[i] << " is not a valid shape";
        return 1;
      }
      else
      {
        try
        {
          if (!strcmp(aName, "move") || !strcmp(aName, "reset"))
          {
            DBRep1::Set(a[i], S.Located(L, isExeption));
          }
          else
          {
            DBRep1::Set(a[i], S.Moved(L, isExeption));
          }
        }
        catch (const Standard_DomainError&)
        {
          AsciiString1 aScale(T.ScaleFactor());
          Message::SendWarning() << "Operation is not done: " << aName
                                 << " is not a valid transformation - scale = " << aScale;
          return 0;
        }
      }
    }
  }
  else
  {
    BRepBuilderAPI_Transform trf(T);
    for (Standard_Integer i = 1; i < last; i++)
    {
      TopoShape S = DBRep1::Get(a[i]);
      if (S.IsNull())
      {
        Message::SendFail() << "Error: " << a[i] << " is not a valid shape";
        return 1;
      }
      else
      {
        trf.Perform(S, isCopy, isCopyMesh);
        if (!trf.IsDone())
          return 1;
        DBRep1::Set(a[i], trf.Shape());
      }
    }
  }
  return 0;
}

///=======================================================================
// gtransform
//=======================================================================

static Standard_Integer deform(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 6)
  {
    di << "Syntax error: wrong number of arguments";
    return 1;
  }

  Transform3d  T;
  gp_GTrsf GT(T);

  //  gp_Mat rot(Draw1::Atof(a[last-3]),0,0,0,Draw1::Atof(a[last-2]),0,0,0,Draw1::Atof(a[last-1]));
  gp_Mat rot(Draw1::Atof(a[3]), 0, 0, 0, Draw1::Atof(a[4]), 0, 0, 0, Draw1::Atof(a[5]));
  GT.SetVectorialPart(rot);
  BRepBuilderAPI_GTransform   gtrf(GT);
  BRepBuilderAPI_NurbsConvert nbscv;
  //  Standard_Integer last = n - 3;
  //  for (Standard_Integer i = 1; i < last; i++) {
  //    TopoShape aShape = DBRep1::Get(a[i]);
  TopoShape aShape = DBRep1::Get(a[2]);
  if (aShape.IsNull())
  {
    di << "Syntax error: '" << a[2] << "' is not a valid shape";
    return 1;
  }

  gtrf.Perform(aShape);
  if (!gtrf.IsDone())
  {
    di << "Error: transformation failed";
    return 1;
  }

  DBRep1::Set(a[1], gtrf.Shape());
  return 0;
}

//=======================================================================
// tcopy
//=======================================================================

static Standard_Integer tcopy(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  Standard_Boolean copyGeom = Standard_True;
  Standard_Boolean copyMesh = Standard_False;
  Standard_Integer iFirst   = 1; // index of first shape argument

  if (n > 1)
  {
    for (Standard_Integer i = 1; i <= 2; i++)
    {
      if (a[i][0] != '-')
        break;
      if (a[i][1] == 'n')
      {
        copyGeom = Standard_False;
        iFirst++;
      }
      else if (a[i][1] == 'm')
      {
        copyMesh = Standard_True;
        iFirst++;
      }
    }
  }

  if (n < 3 || (n - iFirst) % 2)
  {
    Message::SendFail() << "Use: " << a[0]
                        << " [-n(ogeom)] [-m(esh)] shape1 copy1 [shape2 copy2 [...]]\n"
                        << "Option -n forbids copying of geometry (it will be shared)\n"
                        << "Option -m forces copying of mesh (disabled by default)";
    return 1;
  }

  BRepBuilderAPI_Copy cop;
  Standard_Integer    nbPairs = (n - iFirst) / 2;
  for (Standard_Integer i = 0; i < nbPairs; i++)
  {
    cop.Perform(DBRep1::Get(a[i + iFirst]), copyGeom, copyMesh);
    DBRep1::Set(a[i + iFirst + 1], cop.Shape());
    di << a[i + iFirst + 1] << " ";
  }
  return 0;
}

//=======================================================================
// NurbsConvert
//=======================================================================

static Standard_Integer nurbsconvert(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
    return 1;
  if ((n - 1) % 2 != 0)
    return 1;
  BRepBuilderAPI_NurbsConvert nbscv;
  for (Standard_Integer i = 0; i < (n - 1) / 2; i++)
  {
    TopoShape S = DBRep1::Get(a[2 * i + 2]);
    if (S.IsNull())
    {
      // std::cout << a[2*i+2] << " is not a valid shape" << std::endl;
      di << a[2 * i + 2] << " is not a valid shape\n";
    }
    else
    {
      nbscv.Perform(S);
      if (nbscv.IsDone())
      {
        DBRep1::Set(a[2 * i + 1], nbscv.Shape());
      }
      else
      {
        return 1;
      }
    }
  }

  return 0;
}

//=======================================================================
// make a 3D edge curve
//=======================================================================

static Standard_Integer mkedgecurve(DrawInterpreter&, Standard_Integer n, const char** a)
{

  if (n < 3)
    return 1;
  Standard_Real Tolerance = Draw1::Atof(a[2]);

  TopoShape S = DBRep1::Get(a[1]);

  if (S.IsNull())
    return 1;

  BRepLib::BuildCurves3d(S, Tolerance);
  return 0;
}

//=======================================================================
// sameparameter
//=======================================================================

static Standard_Integer sameparameter(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
  {
    di << "Use sameparameter [result] shape [toler]\n";
    di << "shape is an initial shape\n";
    di << "result is a result shape. if skipped = > initial shape will be modified\n";
    di << "toler is tolerance (default is 1.e-7)";
    return 1;
  }
  Standard_Real    aTol  = 1.e-7;
  Standard_Boolean force = !strcmp(a[0], "fsameparameter");

  Standard_Real    aTol1    = Draw1::Atof(a[n - 1]);
  Standard_Boolean IsUseTol = aTol1 > 0;
  if (IsUseTol)
    aTol = aTol1;

  TopoShape anInpS = DBRep1::Get(IsUseTol ? a[n - 2] : a[n - 1]);
  if (anInpS.IsNull())
    return 1;

  if ((n == 4 && IsUseTol) || (n == 3 && !IsUseTol))
  {
    TopoShape      aResultSh;
    BRepTools_ReShape aResh;
    BRepLib::SameParameter(anInpS, aResh, aTol, force);
    aResultSh = aResh.Apply(anInpS);
    DBRep1::Set(a[1], aResultSh);
  }
  else
  {
    BRepLib::SameParameter(anInpS, aTol, force);
    DBRep1::Set(a[1], anInpS);
  }

  return 0;
}

//=================================================================================================

static Standard_Integer updatetol(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
  {
    di << "Use updatetololerance [result] shape [param]\n";
    di << "shape is an initial shape\n";
    di << "result is a result shape. if skipped = > initial shape will be modified\n";
    di << "if [param] is absent - not verify of face tolerance, else - perform it";
    return 1;
  }
  TopoShape     aSh1 = DBRep1::Get(a[n - 1]);
  Standard_Boolean IsF  = aSh1.IsNull();

  TopoShape anInpS = IsF ? DBRep1::Get(a[n - 2]) : aSh1;
  if (anInpS.IsNull())
    return 1;

  if ((n == 4 && IsF) || (n == 3 && !IsF))
  {
    TopoShape      aResultSh;
    BRepTools_ReShape aResh;
    BRepLib::UpdateTolerances(anInpS, aResh, IsF);
    aResultSh = aResh.Apply(anInpS);
    DBRep1::Set(a[1], aResultSh);
  }
  else
  {
    BRepLib::UpdateTolerances(anInpS, IsF);
    DBRep1::Set(a[1], anInpS);
  }

  return 0;
}

//=================================================================================================

static Standard_Integer orientsolid(DrawInterpreter&, Standard_Integer n, const char** a)
{
  if (n < 2)
    return 1;

  TopoShape S = DBRep1::Get(a[1]);
  if (S.IsNull())
    return 1;
  if (S.ShapeType() != TopAbs_SOLID)
    return 1;

  BRepLib::OrientClosedSolid(TopoDS::Solid(S));

  DBRep1::Set(a[1], S);
  return 0;
}

//=================================================================================================

static Standard_Integer getcoords(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
    return 1;

  for (Standard_Integer i = 1; i < n; i++)
  {
    const TopoShape aShape = DBRep1::Get(a[i]);

    if (aShape.IsNull())
      continue;

    if (aShape.ShapeType() == TopAbs_VERTEX)
    {
      const TopoVertex& aVertex = TopoDS::Vertex(aShape);
      Point3d               aPnt    = BRepInspector::Pnt(aVertex);

      di << a[i] << " (x,y,z) : " << aPnt.X() << " " << aPnt.Y() << " " << aPnt.Z() << "\n";
    }
  }

  return 0;
}

//! Parse 6 real values for defining AABB.
static Standard_Boolean parseMinMax(const char** theArgVec, Box2& theBox)
{
  const AsciiString1 aMin[3] = {theArgVec[0], theArgVec[1], theArgVec[2]};
  const AsciiString1 aMax[3] = {theArgVec[3], theArgVec[4], theArgVec[5]};
  if (!aMin[0].IsRealValue() || !aMin[1].IsRealValue() || !aMin[2].IsRealValue()
      || !aMax[0].IsRealValue() || !aMax[1].IsRealValue() || !aMax[2].IsRealValue())
  {
    return Standard_False;
  }

  const Point3d aPntMin(aMin[0].RealValue(), aMin[1].RealValue(), aMin[2].RealValue());
  const Point3d aPntMax(aMax[0].RealValue(), aMax[1].RealValue(), aMax[2].RealValue());
  theBox.SetVoid();
  theBox.Add(aPntMin);
  theBox.Add(aPntMax);
  return Standard_True;
}

//=================================================================================================

static Standard_Integer BoundBox(DrawInterpreter& theDI,
                                 Standard_Integer  theNArg,
                                 const char**      theArgVal)
{
  // 1. Parse arguments

  TopoShape aShape;
  Box2      anAABB;

  Standard_Boolean doPrint            = Standard_False;
  Standard_Boolean doDumpJson         = Standard_False;
  Standard_Boolean useOldSyntax       = Standard_False;
  Standard_Boolean isOBB              = Standard_False;
  Standard_Boolean isTriangulationReq = Standard_True;
  Standard_Boolean isOptimal          = Standard_False;
  Standard_Boolean isTolerUsed        = Standard_False;
  Standard_Boolean isFinitePart       = Standard_False;
  Standard_Boolean hasToDraw          = Standard_True;

  AsciiString1 anOutVars[6];
  AsciiString1 aResShapeName;
  for (Standard_Integer anArgIter = 1; anArgIter < theNArg; ++anArgIter)
  {
    AsciiString1 anArgCase(theArgVal[anArgIter]);
    anArgCase.LowerCase();
    if (anArgCase == "-obb")
    {
      isOBB = Standard_True;
    }
    else if (anArgCase == "-aabb")
    {
      isOBB = Standard_False;
    }
    else if (anArgCase == "-shape" && anArgIter + 1 < theNArg && aResShapeName.IsEmpty())
    {
      aResShapeName = theArgVal[++anArgIter];
      hasToDraw     = Standard_False;
    }
    else if (anArgCase == "-dump" || anArgCase == "-print")
    {
      doPrint = Standard_True;
    }
    else if (anArgCase == "-dumpjson")
    {
      doDumpJson = Standard_True;
    }
    else if (anArgCase == "-save" && anArgIter + 6 < theNArg && anOutVars[0].IsEmpty())
    {
      for (int aCompIter = 0; aCompIter < 6; ++aCompIter)
      {
        anOutVars[aCompIter] = theArgVal[anArgIter + aCompIter + 1];
      }
      anArgIter += 6;
    }
    else if (anArgCase == "-notriangulation")
    {
      isTriangulationReq = Standard_False;
    }
    else if (anArgCase == "-optimal")
    {
      isOptimal = Standard_True;
    }
    else if (anArgCase == "-exttoler")
    {
      isTolerUsed = Standard_True;
    }
    else if (anArgCase == "-nodraw")
    {
      hasToDraw = Standard_False;
    }
    else if (anArgCase == "-finite" || anArgCase == "-finitepart")
    {
      isFinitePart = Standard_True;
    }
    else if (aShape.IsNull() && !DBRep1::Get(theArgVal[anArgIter]).IsNull())
    {
      aShape = DBRep1::Get(theArgVal[anArgIter]);
    }
    else if (anAABB.IsVoid() && anArgIter + 5 < theNArg
             && parseMinMax(theArgVal + anArgIter, anAABB))
    {
      anArgIter += 5;
    }
    else
    {
      Message::SendFail() << "Syntax error at argument '" << theArgVal[anArgIter] << "'";
      return 1;
    }
  }

  if (anAABB.IsVoid() && aShape.IsNull())
  {
    Message::SendFail() << "Syntax error: input is not specified (neither shape nor coordinates)";
    return 1;
  }
  else if (!anAABB.IsVoid() && (isOBB || isOptimal || isTolerUsed))
  {
    Message::SendFail() << "Syntax error: Options -obb, -optimal and -extToler cannot be used for "
                           "explicitly defined AABB";
    return 1;
  }
  else if (isOBB && !anOutVars[0].IsEmpty())
  {
    Message::SendFail() << "Error: Option -save works only with axes-aligned boxes";
    return 1;
  }

  // enable printing (old syntax) if neither saving to shape nor to DRAW variables is requested
  if (!doPrint && !doDumpJson && anOutVars[0].IsEmpty() && aResShapeName.IsEmpty())
  {
    doPrint      = Standard_True;
    useOldSyntax = Standard_True;
  }

  // 2. Compute box and save results
  Handle(Draw_Box) aDB;
  if (isOBB)
  {
    OrientedBox anOBB;
    BRepBndLib::AddOBB(aShape, anOBB, isTriangulationReq, isOptimal, isTolerUsed);

    if (anOBB.IsVoid())
    {
      theDI << "Void box.\n";
    }
    else if (doPrint)
    {
      const Point3d& aBaryCenter = anOBB.Center();
      const gp_XYZ &aXDir = anOBB.XDirection(), &aYDir = anOBB.YDirection(),
                   &aZDir = anOBB.ZDirection();
      theDI << "Oriented bounding box\n";
      theDI << "Center: " << aBaryCenter.X() << " " << aBaryCenter.Y() << " " << aBaryCenter.Z()
            << "\n";
      theDI << "X-axis: " << aXDir.X() << " " << aXDir.Y() << " " << aXDir.Z() << "\n";
      theDI << "Y-axis: " << aYDir.X() << " " << aYDir.Y() << " " << aYDir.Z() << "\n";
      theDI << "Z-axis: " << aZDir.X() << " " << aZDir.Y() << " " << aZDir.Z() << "\n";
      theDI << "Half X: " << anOBB.XHSize() << "\n"
            << "Half Y: " << anOBB.YHSize() << "\n"
            << "Half Z: " << anOBB.ZHSize() << "\n";
    }

    if (doDumpJson)
    {
      Standard_SStream aStream;
      anOBB.DumpJson(aStream);

      theDI << "Oriented bounding box\n";
      theDI << Standard_Dump::FormatJson(aStream);
    }

    if (hasToDraw && !anOBB.IsVoid())
    {
      aDB = new Draw_Box(anOBB, Draw_orange);
    }

    if (!aResShapeName.IsEmpty())
    {
      ConvertBndToShape(anOBB, aResShapeName.ToCString());
    }
  }
  else // if(!isOBB)
  {
    if (!aShape.IsNull())
    {
      anAABB.SetVoid();
      if (isOptimal)
      {
        BRepBndLib::AddOptimal(aShape, anAABB, isTriangulationReq, isTolerUsed);
      }
      else
      {
        BRepBndLib::Add(aShape, anAABB, isTriangulationReq);
      }
    }

    if (anAABB.IsVoid())
    {
      theDI << "Void box.\n";
    }
    else
    {
      if (isFinitePart && anAABB.IsOpen())
      {
        anAABB = anAABB.FinitePart();
      }
      const Point3d aMin = anAABB.CornerMin();
      const Point3d aMax = anAABB.CornerMax();

      // print to DRAW
      if (doPrint)
      {
        if (useOldSyntax)
        {
          theDI << aMin.X() << " " << aMin.Y() << " " << aMin.Z() << " " << aMax.X() << " "
                << aMax.Y() << " " << aMax.Z() << "\n";
        }
        else
        {
          theDI << "Axes-aligned bounding box\n";
          theDI << "X-range: " << aMin.X() << " " << aMax.X() << "\n"
                << "Y-range: " << aMin.Y() << " " << aMax.Y() << "\n"
                << "Z-range: " << aMin.Z() << " " << aMax.Z() << "\n";
          if (anAABB.IsOpen() && anAABB.HasFinitePart())
          {
            Box2      aFinitAabb = anAABB.FinitePart();
            const Point3d aFinMin    = aFinitAabb.CornerMin();
            const Point3d aFinMax    = aFinitAabb.CornerMax();
            theDI << "Finite part\n";
            theDI << "X-range: " << aFinMin.X() << " " << aFinMax.X() << "\n"
                  << "Y-range: " << aFinMin.Y() << " " << aFinMax.Y() << "\n"
                  << "Z-range: " << aFinMin.Z() << " " << aFinMax.Z() << "\n";
          }
        }
      }

      if (doDumpJson)
      {
        Standard_SStream aStream;
        anAABB.DumpJson(aStream);

        theDI << "Bounding box\n";
        theDI << Standard_Dump::FormatJson(aStream);
      }

      // save DRAW variables
      if (!anOutVars[0].IsEmpty())
      {
        Draw1::Set(anOutVars[0].ToCString(), aMin.X());
        Draw1::Set(anOutVars[1].ToCString(), aMin.Y());
        Draw1::Set(anOutVars[2].ToCString(), aMin.Z());
        Draw1::Set(anOutVars[3].ToCString(), aMax.X());
        Draw1::Set(anOutVars[4].ToCString(), aMax.Y());
        Draw1::Set(anOutVars[5].ToCString(), aMax.Z());
      }

      // add presentation to DRAW viewer
      if (hasToDraw)
      {
        aDB = new Draw_Box(anAABB, Draw_orange);
      }
    }

    // save as shape
    if (!aResShapeName.IsEmpty())
    {
      ConvertBndToShape(anAABB, aResShapeName.ToCString());
    }
  }

  if (!aDB.IsNull())
  {
    dout << aDB;
  }
  return 0;
}

//=================================================================================================

static Standard_Integer IsBoxesInterfered(DrawInterpreter& theDI,
                                          Standard_Integer  theNArg,
                                          const char**      theArgVal)
{
  if (theNArg < 2)
  {
    theDI << "Use: isbbinterf shape1 shape2 [-o].\n";
    return 1;
  }

  const TopoShape aShape1 = DBRep1::Get(theArgVal[1]);
  const TopoShape aShape2 = DBRep1::Get(theArgVal[2]);

  Standard_Boolean isOBB = (theNArg > 3) && (!strcmp(theArgVal[3], "-o"));

  if (isOBB)
  {
    OrientedBox anOBB1, anOBB2;
    BRepBndLib::AddOBB(aShape1, anOBB1);
    BRepBndLib::AddOBB(aShape2, anOBB2);

    if (anOBB1.IsOut(anOBB2))
    {
      theDI << "The shapes are NOT interfered by OBB.\n";
    }
    else
    {
      theDI << "The shapes are interfered by OBB.\n";
    }
  }
  else
  {
    Box2 anAABB1, anAABB2;
    BRepBndLib::Add(aShape1, anAABB1);
    BRepBndLib::Add(aShape2, anAABB2);

    if (anAABB1.IsOut(anAABB2))
    {
      theDI << "The shapes are NOT interfered by AABB.\n";
    }
    else
    {
      theDI << "The shapes are interfered by AABB.\n";
    }
  }

  return 0;
}

//=================================================================================================

#include <BndLib_AddSurface.hxx>
#include <BndLib_Add3dCurve.hxx>
#include <BndLib_Add2dCurve.hxx>
#include <Draw_Segment2D.hxx>

static Standard_Integer gbounding(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 2 && n != 3)
  {
    di << "Usage: gbounding surf/curve/curve2d [-o] \n";
    di << "[-o] turn on Optimal mode ('off' by default) \n";
    return 1;
  }
  else
  {
    Standard_Boolean IsOptimal = Standard_False;
    if (n == 3 && !strcmp(a[2], "-o"))
      IsOptimal = Standard_True;

    Standard_Real        axmin, aymin, azmin, axmax, aymax, azmax;
    Box2              B;
    Bnd_Box2d            B2d;
    Handle(Draw_Box)     DB;
    Standard_Boolean     Is3d = Standard_True;
    Handle(GeomCurve3d)   C;
    Handle(GeomSurface) S;
    Handle(GeomCurve2d) C2d;
    S = DrawTrSurf1::GetSurface(a[1]);
    if (!S.IsNull())
    {
      // add surf
      GeomAdaptor_Surface aGAS(S);
      if (IsOptimal)
        AddSurface::AddOptimal(aGAS, Precision::Confusion(), B);
      else
        AddSurface::Add(aGAS, Precision::Confusion(), B);
    }
    else
    {
      C = DrawTrSurf1::GetCurve(a[1]);
      if (!C.IsNull())
      {
        // add cur
        GeomAdaptor_Curve aGAC(C);
        if (IsOptimal)
          Add3dCurve::AddOptimal(aGAC, Precision::Confusion(), B);
        else
          Add3dCurve::Add(aGAC, Precision::Confusion(), B);
      }
      else
      {
        C2d = DrawTrSurf1::GetCurve2d(a[1]);
        if (!C2d.IsNull())
        {
          // add cur2d
          Is3d = Standard_False;
          if (IsOptimal)
            Add2dCurve::AddOptimal(C2d,
                                          C2d->FirstParameter(),
                                          C2d->LastParameter(),
                                          Precision::Confusion(),
                                          B2d);
          else
            Add2dCurve::Add(C2d,
                                   C2d->FirstParameter(),
                                   C2d->LastParameter(),
                                   Precision::Confusion(),
                                   B2d);
        }
        else
        {
          di << "Wrong argument \n";
          return 1;
        }
      }
    }

    if (Is3d)
    {
      B.Get(axmin, aymin, azmin, axmax, aymax, azmax);
      DB = new Draw_Box(B, Draw_vert);
      dout << DB;
      di << axmin << " " << aymin << " " << azmin << " " << axmax << " " << aymax << " " << azmax;
    }
    else
    {
      B2d.Get(axmin, aymin, axmax, aymax);
      gp_Pnt2d        p1(axmin, aymin);
      gp_Pnt2d        p2(axmax, aymin);
      gp_Pnt2d        p3(axmax, aymax);
      gp_Pnt2d        p4(axmin, aymax);
      Draw_Segment2D* S1 = new Draw_Segment2D(p1, p2, Draw_vert);
      Draw_Segment2D* S2 = new Draw_Segment2D(p2, p3, Draw_vert);
      Draw_Segment2D* S3 = new Draw_Segment2D(p3, p4, Draw_vert);
      Draw_Segment2D* S4 = new Draw_Segment2D(p4, p1, Draw_vert);
      dout << S1 << S2 << S3 << S4;
      di << axmin << " " << aymin << " " << axmax << " " << aymax;
    }
  }
  return 0;
}

//=================================================================================================

static Standard_Integer findplane(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
    return 1;
  TopoShape S = DBRep1::Get(a[1]);
  if (S.IsNull())
    return 1;
  Standard_Real            tolerance = 1.0e-5;
  BRepBuilderAPI_FindPlane a_plane_finder(S, tolerance);
  if (a_plane_finder.Found())
  {
    // std::cout << " a plane is found "   ;
    di << " a plane is found \n";
    const Handle(Geom_Geometry) aSurf = a_plane_finder.Plane(); // to avoid ambiguity
    DrawTrSurf1::Set(a[2], aSurf);
  }
  return 0;
}

//=================================================================================================

static Standard_Integer precision(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  n--;

  if (n == 0)
  {
    // std::cout << " Current Precision = " << BRepBuilderAPI::Precision() << std::endl;
    di << " Current Precision = " << BRepBuilderAPI::Precision() << "\n";
  }
  else
  {
    BRepBuilderAPI::Precision(Draw1::Atof(a[1]));
  }
  return 0;
}

//=======================================================================
// function : reperage shape (Int lin Shape) + pointe double click   + maxtol
// purpose  :
//=======================================================================
#include <IntCurvesFace_ShapeIntersector.hxx>

static Standard_Integer reperageshape(DrawInterpreter& di, Standard_Integer narg, const char** a)
{
  Standard_Integer details = 0;
  if (narg < 2)
    return 1;
  if (narg == 3)
    details = 1;
  const char*  id1       = a[1];
  TopoShape TheShape1 = DBRep1::Get(id1);

  // std::cout << "Pick positions with button "<<std::endl;
  di << "Pick positions with button \n";
  Standard_Integer id, X, Y, b;
  Transform3d          T;
  Point3d           P1, P2;
  dout.Select(id, X, Y, b);

  dout.GetTrsf(id, T);
  T.Invert();
  Standard_Real z = dout.Zoom(id);
  P2.SetCoord((Standard_Real)X / z, (Standard_Real)Y / z, 0.0);
  P2.Transform(T);
  P1.SetCoord((Standard_Real)X / z, (Standard_Real)Y / z, -1.0);
  P1.Transform(T);

  Axis3d                         Axe(P1, Vector3d(P1, P2));
  IntCurvesFace_ShapeIntersector Inter;
  Inter.Load(TheShape1, 1e-7);

  Inter.Perform(Axe, -RealLast(), RealLast());

  // std::cout<<"\n --> ";
  di << "\n --> ";
  if (Inter.NbPnt())
  {
    for (Standard_Integer i = 1; i <= Inter.NbPnt(); i++)
    {
      Standard_Integer numface = 1;
      ShapeExplorer  ExF;
      for (ExF.Init(TheShape1, TopAbs_FACE); ExF.More(); ExF.Next(), numface++)
      {
        TopoFace Face = TopoDS::Face(ExF.Current());
        if (Face.IsEqual(Inter.Face(i)))
        {
          // std::cout<<" "<<a[1]<<"_"<<numface;
          di << " " << a[1] << "_" << numface;
          continue;
        }
      }
      const Point3d& P    = Inter.Pnt(i);
      Standard_Real PMin = Inter.WParameter(i);
      if (details)
      {
        // std::cout<<" w:"<<PMin<<std::endl;
        di << " w:" << PMin << "\n";
      }
      if (Inter.Transition(i) == IntCurveSurface_In)
      {
        if (Inter.State(i) == TopAbs_IN)
        {
          Handle(Draw_Marker3D) p = new Draw_Marker3D(P, Draw_Square, Draw_rouge, 2);
          dout << p;
          dout.Flush();
        }
        else if (Inter.State(i) == TopAbs_ON)
        {
          Handle(Draw_Marker3D) p = new Draw_Marker3D(P, Draw_Square, Draw_vert, 2);
          dout << p;
          dout.Flush();
        }
      }
      else
      {
        if (Inter.Transition(i) == IntCurveSurface_Out)
        {
          if (Inter.State(i) == TopAbs_IN)
          {
            Handle(Draw_Marker3D) p = new Draw_Marker3D(P, Draw_X, Draw_rouge, 2);
            dout << p;
            dout.Flush();
          }
          else if (Inter.State(i) == TopAbs_ON)
          {
            Handle(Draw_Marker3D) p = new Draw_Marker3D(P, Draw_X, Draw_vert, 2);
            dout << p;
            dout.Flush();
          }
        }
      }
    }
  }
  // std::cout<<std::endl;
  di << "\n";
  return (0);
}

static Standard_Integer maxtolerance(DrawInterpreter& theCommands,
                                     Standard_Integer  n,
                                     const char**      a)
{
  if (n < 2)
    return (1);
  TopoShape TheShape = DBRep1::Get(a[1]);
  if (TheShape.IsNull())
    return (1);

  Standard_Real    T, TMF, TME, TMV, TmF, TmE, TmV;
  Standard_Integer nbF, nbE, nbV;
  TMF = TME = TMV = -RealLast();
  TmF = TmE = TmV = RealLast();

  TopTools_MapOfShape mapS;
  mapS.Clear();

  for (ShapeExplorer ex(TheShape, TopAbs_FACE); ex.More(); ex.Next())
  {
    TopoFace Face = TopoDS::Face(ex.Current());
    T                = BRepInspector::Tolerance(Face);
    if (T > TMF)
      TMF = T;
    if (T < TmF)
      TmF = T;
    mapS.Add(Face);
  }

  nbF = mapS.Extent();
  mapS.Clear();

  for (ShapeExplorer ex(TheShape, TopAbs_EDGE); ex.More(); ex.Next())
  {
    TopoEdge Edge = TopoDS::Edge(ex.Current());
    T                = BRepInspector::Tolerance(Edge);
    if (T > TME)
      TME = T;
    if (T < TmE)
      TmE = T;
    mapS.Add(Edge);
  }

  nbE = mapS.Extent();
  mapS.Clear();

  for (ShapeExplorer ex(TheShape, TopAbs_VERTEX); ex.More(); ex.Next())
  {
    TopoVertex Vertex = TopoDS::Vertex(ex.Current());
    T                    = BRepInspector::Tolerance(Vertex);
    if (T > TMV)
      TMV = T;
    if (T < TmV)
      TmV = T;
    mapS.Add(Vertex);
  }

  nbV = mapS.Extent();

  Standard_SStream sss;
  sss << "\n## Tolerances on the shape " << a[1] << "  (nbFaces:" << nbF << "  nbEdges:" << nbE
      << " nbVtx:" << nbV << ")\n";
  sss.precision(5);
  sss.setf(std::ios::scientific);
  if (TmF <= TMF)
    sss << "\n    Face   : Min " << std::setw(8) << TmF << "    Max  " << std::setw(8) << TMF
        << " \n ";
  if (TmE <= TME)
    sss << "\n    Edge   : Min " << std::setw(8) << TmE << "    Max  " << std::setw(8) << TME
        << " \n ";
  if (TmV <= TMV)
    sss << "\n    Vertex : Min " << std::setw(8) << TmV << "    Max  " << std::setw(8) << TMV
        << " \n ";
  theCommands << sss;

  return 0;
}

static Standard_Integer vecdc(DrawInterpreter& di, Standard_Integer, const char**)
{
  // std::cout << "Pick positions with button "<<std::endl;
  di << "Pick positions with button \n";

  Standard_Integer id, X, Y, b;
  Transform3d          T;
  Point3d           P1, P2, PP1, PP2;

  //-----------------------------------------------------------
  dout.Select(id, X, Y, b);
  dout.GetTrsf(id, T);
  T.Invert();
  Standard_Real z = dout.Zoom(id);
  P1.SetCoord((Standard_Real)X / z, (Standard_Real)Y / z, 0.0);
  P1.Transform(T);

  dout.Select(id, X, Y, b);
  dout.GetTrsf(id, T);
  T.Invert();
  z = dout.Zoom(id);

  P2.SetCoord((Standard_Real)X / z, (Standard_Real)Y / z, 0.0);
  P2.Transform(T);
  Standard_Real xa, ya, za;
  if (Abs(P1.X()) > Abs(P2.X()))
    xa = P1.X();
  else
    xa = P2.X();
  if (Abs(P1.Y()) > Abs(P2.Y()))
    ya = P1.Y();
  else
    ya = P2.Y();
  if (Abs(P1.Z()) > Abs(P2.Z()))
    za = P1.Z();
  else
    za = P2.Z();
  P1.SetCoord(xa, ya, za);
  Handle(Draw_Marker3D) D0 =
    new Draw_Marker3D(Point3d(P1.X(), P1.Y(), P1.Z()), Draw_Square, Draw_blanc, 1);

  dout << D0;
  dout.Flush();
  //-----------------------------------------------------------
  dout.Select(id, X, Y, b);
  dout.GetTrsf(id, T);
  T.Invert();
  z = dout.Zoom(id);
  PP1.SetCoord((Standard_Real)X / z, (Standard_Real)Y / z, 0.0);
  PP1.Transform(T);
  dout.Select(id, X, Y, b);
  dout.GetTrsf(id, T);
  T.Invert();
  z = dout.Zoom(id);
  PP2.SetCoord((Standard_Real)X / z, (Standard_Real)Y / z, 0.0);
  PP2.Transform(T);
  if (Abs(PP1.X()) > Abs(PP2.X()))
    xa = PP1.X();
  else
    xa = PP2.X();
  if (Abs(PP1.Y()) > Abs(PP2.Y()))
    ya = PP1.Y();
  else
    ya = PP2.Y();
  if (Abs(PP1.Z()) > Abs(PP2.Z()))
    za = PP1.Z();
  else
    za = PP2.Z();
  PP1.SetCoord(xa, ya, za);
  Handle(Draw_Segment3D) d = new Draw_Segment3D(P1, PP1, Draw_blanc);
  dout << d;
  dout.Flush();
  // std::cout<<"\nttran   "<<PP1.X()-P1.X()<<" "<<PP1.Y()-P1.Y()<<" "<<PP1.Z()-P1.Z()<<std::endl;
  di << "\nttran   " << PP1.X() - P1.X() << " " << PP1.Y() - P1.Y() << " " << PP1.Z() - P1.Z()
     << "\n";

  static Standard_Integer nboxvecdp = 0;
  // std::cout<<"\nbox  b"<<++nboxvecdp<<" "<<Min(P1.X(),PP1.X())<<" "<<Min(P1.Y(),PP1.Y())<<"
  // "<<Min(PP1.Z(),P1.Z()); std::cout<<"  "<<Abs(PP1.X()-P1.X())<<" "<<Abs(PP1.Y()-P1.Y())<<"
  // "<<Abs(PP1.Z()-P1.Z())<<std::endl;

  // std::cout<<"\nDistance :"<<sqrt( (PP1.X()-P1.X())*(PP1.X()-P1.X())
  //		     +(PP1.Y()-P1.Y())*(PP1.Y()-P1.Y())
  //		     +(PP1.Z()-P1.Z())*(PP1.Z()-P1.Z()))<<std::endl;

  di << "\nbox  b" << ++nboxvecdp << " " << Min(P1.X(), PP1.X()) << " " << Min(P1.Y(), PP1.Y())
     << " " << Min(PP1.Z(), P1.Z());
  di << "  " << Abs(PP1.X() - P1.X()) << " " << Abs(PP1.Y() - P1.Y()) << " "
     << Abs(PP1.Z() - P1.Z()) << "\n";

  di << "\nDistance :"
     << sqrt((PP1.X() - P1.X()) * (PP1.X() - P1.X()) + (PP1.Y() - P1.Y()) * (PP1.Y() - P1.Y())
             + (PP1.Z() - P1.Z()) * (PP1.Z() - P1.Z()))
     << "\n";
  return (0);
}

//=======================================================================
// nproject
//=======================================================================

#include <TopTools_SequenceOfShape.hxx>

static Standard_Integer nproject(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 4)
    return 1;
  TopoShape             InpShape;
  Standard_Integer         arg = 2, i;
  TopTools_SequenceOfShape Args;

  Standard_Real    Tol = 1.e-4;
  Standard_Real    Tol2d;
  Standard_Real    MaxDistance = 1.e-3;
  GeomAbs_Shape    Continuity  = GeomAbs_C1;
  Standard_Integer MaxDeg      = 14;
  Standard_Integer MaxSeg      = 16;

  while ((n > arg) && !(InpShape = DBRep1::Get(a[arg])).IsNull())
  {
    Args.Append(InpShape);
    arg++;
  }
  if (Args.Length() < 2)
    return 1;

  BRepOffsetAPI_NormalProjection OrtProj(Args.Last());

  for (i = 1; i < Args.Length(); i++)
    OrtProj.Add(Args(i));

  if (n > arg)
    if (!strcmp(a[arg], "-g"))
    {
      OrtProj.SetLimit(Standard_False);
      arg++;
    }

  if (n > arg)
    if (!strcmp(a[arg], "-d"))
    {
      arg++;
      if (n > arg)
        MaxDistance = Draw1::Atof(a[arg++]);
      OrtProj.SetMaxDistance(MaxDistance);
    }
  if (n > arg)
  {
    Tol = Max(Draw1::Atof(a[arg++]), 1.e-10);
  }

  if (n > arg)
  {
    if (Draw1::Atoi(a[arg]) == 0)
      Continuity = GeomAbs_C0;
    else if (Draw1::Atoi(a[arg]) == 2)
      Continuity = GeomAbs_C2;
    arg++;
  }

  if (n > arg)
  {
    MaxDeg = Draw1::Atoi(a[arg++]);
    if (MaxDeg < 1 || MaxDeg > 14)
      MaxDeg = 14;
  }

  if (n > arg)
    MaxSeg = Draw1::Atoi(a[arg]);

  Tol2d = Pow(Tol, 2. / 3);

  OrtProj.SetParams(Tol, Tol2d, Continuity, MaxDeg, MaxSeg);
  OrtProj.Build();
  ShapeList Wire;
  Standard_Boolean     IsWire = OrtProj.BuildWire(Wire);
  if (IsWire)
  {
    // std::cout << " BuildWire OK " << std::endl;
    di << " BuildWire OK \n";
  }
  DBRep1::Set(a[1], OrtProj.Shape());
  return 0;
}

//==========================================================================
// function : wexplo
//           exploration of a wire
//==========================================================================
static Standard_Integer wexplo(DrawInterpreter&, Standard_Integer argc, const char** argv)
{
  char name[100];
  if (argc < 2)
    return 1;

  TopoShape C1 = DBRep1::Get(argv[1], TopAbs_WIRE);
  TopoShape C2;

  if (argc > 2)
    C2 = DBRep1::Get(argv[2], TopAbs_FACE);

  if (C1.IsNull())
    return 1;

  BRepTools_WireExplorer we;
  if (C2.IsNull())
    we.Init(TopoDS::Wire(C1));
  else
    we.Init(TopoDS::Wire(C1), TopoDS::Face(C2));

  Standard_Integer k = 1;
  while (we.More())
  {
    TopoEdge E = we.Current();
    Sprintf(name, "WEDGE_%d", k);
    DBRep1::Set(name, E);
    we.Next();
    k++;
  }

  return 0;
}

static Standard_Integer scalexyz(DrawInterpreter& /*di*/, Standard_Integer n, const char** a)
{
  if (n < 6)
    return 1;

  TopoShape aShapeBase = DBRep1::Get(a[2]);
  if (aShapeBase.IsNull())
    return 1;

  Standard_Real aFactorX = Draw1::Atof(a[3]);
  Standard_Real aFactorY = Draw1::Atof(a[4]);
  Standard_Real aFactorZ = Draw1::Atof(a[5]);

  gp_GTrsf aGTrsf;
  gp_Mat   rot(aFactorX, 0, 0, 0, aFactorY, 0, 0, 0, aFactorZ);
  aGTrsf.SetVectorialPart(rot);
  BRepBuilderAPI_GTransform aBRepGTrsf(aShapeBase, aGTrsf, Standard_False);
  if (!aBRepGTrsf.IsDone())
    throw Standard_ConstructionError("Scaling not done");
  TopoShape Result = aBRepGTrsf.Shape();

  DBRep1::Set(a[1], Result);
  return 0;
}

//=================================================================================================

static Standard_Integer compareshapes(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 3)
  {
    di << "Compare shapes. Usage: compare shape1 shape2\n";
    return 1;
  }
  // get shapes
  TopoShape aS1 = DBRep1::Get(a[1]);
  TopoShape aS2 = DBRep1::Get(a[2]);
  // compare shapes
  if (aS1.IsSame(aS2))
  {
    di << "same shapes\n";
    if (aS1.IsEqual(aS2))
    {
      di << "equal shapes\n";
    }
  }
  else
  {
    di << "shapes are not same\n";
  }
  return 0;
}

//=================================================================================================

static Standard_Integer issubshape(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 3)
  {
    di << "Check if the shape is sub-shape of other shape and get its index in the shape.\n";
    di << "Usage: issubshape subshape shape\n";
    return 1;
  }
  // get shapes
  TopoShape aSubShape = DBRep1::Get(a[1]);
  TopoShape aShape    = DBRep1::Get(a[2]);
  // check shapes
  if (aSubShape.IsNull() || aShape.IsNull())
  {
    di << "null shapes\n";
    return 0;
  }
  // find index of the sub-shape in the shape
  TopTools_MapOfShape aMShapes;
  // try to find the SubShape in Shape
  ShapeExplorer anExp(aShape, aSubShape.ShapeType());
  for (; anExp.More(); anExp.Next())
  {
    const TopoShape& aSS = anExp.Current();
    if (aMShapes.Add(aSS))
    {
      if (aSS.IsSame(aSubShape))
      {
        break;
      }
    }
  }
  //
  if (anExp.More())
  {
    di << a[1] << " is sub-shape of " << a[2] << ". Index in the shape: " << aMShapes.Extent()
       << ".\n";
  }
  else
  {
    di << a[1] << " is NOT sub-shape of " << a[2] << ".\n";
  }
  //
  return 0;
}

//=================================================================================================

static Standard_Integer purgeloc(DrawInterpreter& di, Standard_Integer /*n*/, const char** a)
{

  TopoShape aShapeBase = DBRep1::Get(a[2]);
  if (aShapeBase.IsNull())
    return 1;

  BRepTools_PurgeLocations aRemLoc;
  Standard_Boolean         isDone = aRemLoc.Perform(aShapeBase);
  TopoShape             Result = aRemLoc.GetResult();

  DBRep1::Set(a[1], Result);
  if (isDone)
  {
    di << "All problematic locations are purged \n";
  }
  else
  {
    di << "Not all problematic locations are purged \n";
  }
  return 0;
}

//=================================================================================================

static Standard_Integer checkloc(DrawInterpreter& di, Standard_Integer /*n*/, const char** a)
{

  TopoShape aShapeBase = DBRep1::Get(a[1]);
  if (aShapeBase.IsNull())
    return 1;

  ShapeList aLS;
  BRepTools1::CheckLocations(aShapeBase, aLS);
  if (aLS.IsEmpty())
  {
    di << "There are no problematic shapes" << "\n";
    return 0;
  }
  TopTools_ListIteratorOfListOfShape anIt(aLS);
  Standard_Integer                   i;
  for (i = 1; anIt.More(); anIt.Next(), ++i)
  {
    AsciiString1 aName(a[1]);
    aName += "_";
    aName.AssignCat(i);
    DBRep1::Set(aName.ToCString(), anIt.Value());
    di << aName << " ";
  }
  di << "\n";
  return 0;
}

void BRepTest::BasicCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;

  DBRep1::BasicCommands(theCommands);

  const char* g = "TOPOLOGY Basic shape commands";

  theCommands.Add("addpcurve",
                  "addpcurve edge 2dcurve face [tol (default 1.e-7)]",
                  __FILE__,
                  addpcurve,
                  g);

  theCommands.Add("reset", "reset name1 name2 ..., remove location", __FILE__, transform, g);

  theCommands.Add("tmove",
                  "tmove name1 name2 ... name, set location from name [-copy] [-copymesh]",
                  __FILE__,
                  transform,
                  g);

  theCommands.Add("ttranslate",
                  "ttranslate name1 name2 ... dx dy dz [-copy [-copymesh]]",
                  __FILE__,
                  transform,
                  g);

  theCommands.Add("trotate",
                  "trotate name1 name2 ... x y z dx dy dz angle [-copy [-copymesh]]",
                  __FILE__,
                  transform,
                  g);

  theCommands.Add("tmirror",
                  "tmirror name x y z dx dy dz [-copy] [-copymesh]",
                  __FILE__,
                  transform,
                  g);

  theCommands.Add("tscale", "tscale name x y z scale [-copy] [-copymesh]", __FILE__, transform, g);

  theCommands.Add("tcopy",
                  "tcopy [-n(ogeom)] [-m(esh)] name1 result1 [name2 result2 ...]",
                  __FILE__,
                  tcopy,
                  g);

  theCommands.Add("bmove",
                  "bmove name1 name2 ... name, set location from name",
                  __FILE__,
                  transform,
                  g);

  theCommands.Add("fmove",
                  "fmove name1 name2 ... name, set location from name",
                  __FILE__,
                  transform,
                  g);

  theCommands.Add("btranslate", "btranslate name1 name2 ... dx dy dz", __FILE__, transform, g);

  theCommands.Add("brotate",
                  "brotate name1 name2 ... x y z dx dy dz angle",
                  __FILE__,
                  transform,
                  g);

  theCommands.Add("bmirror", "bmirror name x y z dx dy dz", __FILE__, transform, g);

  theCommands.Add("fmirror", "fmirror name x y z dx dy dz", __FILE__, transform, g);

  theCommands.Add("bscale", "bscale name x y z scale", __FILE__, transform, g);

  theCommands.Add("fscale", "fscale name x y z scale", __FILE__, transform, g);

  theCommands.Add("precision", "precision [preci]", __FILE__, precision, g);

  theCommands.Add("mkedgecurve", "mkedgecurve name tolerance", __FILE__, mkedgecurve, g);

  theCommands.Add("fsameparameter",
                  "fsameparameter shapename [tol (default 1.e-7)], \nforce sameparameter on all "
                  "edges of the shape",
                  __FILE__,
                  sameparameter,
                  g);

  theCommands.Add("sameparameter",
                  "sameparameter [result] shape [tol]",
                  __FILE__,
                  sameparameter,
                  g);

  theCommands.Add("updatetolerance",
                  "updatetolerance [result] shape [param] \n  if [param] is absent - not verify of "
                  "face tolerance, else - perform it",
                  __FILE__,
                  updatetol,
                  g);

  theCommands.Add("solidorientation", "orientsolid myClosedSolid", __FILE__, orientsolid, g);

  theCommands.Add("getcoords",
                  "getcoords vertex1 vertex 2... ; shows coords of input vertices",
                  __FILE__,
                  getcoords,
                  g);

  theCommands.Add(
    "bounding",
    "bounding {shape | xmin ymin zmin xmax ymax zmax}"
    "\n\t\t:            [-obb] [-noTriangulation] [-optimal] [-extToler]"
    "\n\t\t:            [-dump] [-print] [-dumpJson] [-shape name] [-nodraw] [-finitePart]"
    "\n\t\t:            [-save xmin ymin zmin xmax ymax zmax]"
    "\n\t\t:"
    "\n\t\t: Computes a bounding box. Two types of the source data are supported:"
    "\n\t\t: a shape or AABB corners (xmin, ymin, zmin, xmax, ymax, zmax)."
    "\n\t\t:"
    "\n\t\t: Calculation options (applicable only if input is a shape):"
    "\n\t\t:  -obb     Compute Oriented Bounding Box1 (OBB) instead of AABB."
    "\n\t\t:  -noTriangulation Force use of exact geometry for calculation"
    "\n\t\t:                   even if triangulation is present."
    "\n\t\t:  -optimal Force calculation of optimal (more tight) AABB."
    "\n\t\t:           In case of OBB:"
    "\n\t\t:           - for PCA approach applies to initial AABB used in OBB calculation"
    "\n\t\t:           - for DiTo approach modifies the DiTo algorithm to check more axes."
    "\n\t\t:  -extToler Include tolerance of the shape in the resulting box."
    "\n\t\t:"
    "\n\t\t: Output options:"
    "\n\t\t:  -dump    Prints the information about computed Bounding Box1."
    "\n\t\t:  -print   Prints the information about computed Bounding Box1."
    "\n\t\t:           It is enabled by default (with plain old syntax for AABB)"
    "\n\t\t:           if neither -shape nor -save is specified."
    "\n\t\t:  -dumpJson Prints DumpJson information about Bounding Box1."
    "\n\t\t:  -shape   Stores computed box as solid in DRAW variable with specified name."
    "\n\t\t:  -save    Stores min and max coordinates of AABB in specified variables."
    "\n\t\t:  -noDraw  Avoid drawing resulting Bounding Box1 in DRAW viewer."
    "\n\t\t:  -finite  Return finite part of infinite box.",
    __FILE__,
    BoundBox,
    g);

  //
  theCommands.Add("gbounding", "gbounding surf/curve/curve2d [-o] ", __FILE__, gbounding, g);

  theCommands.Add("isbbinterf",
                  "isbbinterf shape1 shape2 [-o]\n"
                  "Checks whether the bounding-boxes created from "
                  "the given shapes are interfered. If \"-o\"-option "
                  "is switched on then the oriented boxes will be checked. "
                  "Otherwise, axes-aligned boxes will be checked.",
                  __FILE__,
                  IsBoxesInterfered,
                  g);

  theCommands.Add("nurbsconvert",
                  "nurbsconvert result name [result name]",
                  __FILE__,
                  nurbsconvert,
                  g);

  theCommands.Add("deform", "deform newname name CoeffX CoeffY CoeffZ", __FILE__, deform, g);

  theCommands.Add("findplane", "findplane name planename ", __FILE__, findplane, g);

  theCommands.Add("maxtolerance", "maxtolerance shape ", __FILE__, maxtolerance, g);

  theCommands.Add("reperageshape",
                  "reperage shape -> list of shape (result of interstion shape , line)",
                  __FILE__,
                  reperageshape,
                  g);

  theCommands.Add("vecdc", "vecdc + Pointe double click ", __FILE__, vecdc, g);

  theCommands.Add("nproject",
                  "nproject pj e1 e2 e3 ... surf -g -d [dmax] [Tol [continuity [maxdeg [maxseg]]]",
                  __FILE__,
                  nproject,
                  g);

  theCommands.Add("wexplo", "wexplo wire [face] create WEDGE_i", __FILE__, wexplo, g);

  theCommands.Add("scalexyz",
                  "scalexyz res shape factor_x factor_y factor_z",
                  __FILE__,
                  scalexyz,
                  g);

  theCommands.Add("compare",
                  "Compare shapes. Usage: compare shape1 shape2",
                  __FILE__,
                  compareshapes,
                  g);

  theCommands.Add(
    "issubshape",
    "issubshape subshape shape\n"
    "\t\tCheck if the shape is sub-shape of other shape and get its index in the shape.",
    __FILE__,
    issubshape,
    g);
  theCommands.Add("purgeloc", "purgeloc res shape ", __FILE__, purgeloc, g);

  theCommands.Add("checkloc", "checkloc shape ", __FILE__, checkloc, g);
}
