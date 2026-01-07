// Created on: 2011-06-30
// Created by: jgv@ROLEX
// Copyright (c) 2011-2014 OPEN CASCADE SAS
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

#include <GeometryTest.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>

#include <DBRep_DrawableShape.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_Appli.hxx>
#include <DrawTrSurf.hxx>
#include <Draw_Marker3D.hxx>

#include <stdio.h>
#ifdef _WIN32
Standard_IMPORT DrawViewer dout;
#endif

//=================================================================================================

static Standard_Integer xdistcc(DrawInterpreter&, Standard_Integer n, const char** a)
{
  if (n < 5)
  {
    std::cout << " Use xdistcc c1 c2 t1 t2 nbp" << std::endl;
    return 0;
  }

  Standard_Integer      i, aNbP, iSize;
  Standard_Real         aD, aT, aT1, aT2, dT;
  Point3d                aP1, aP2;
  Handle(GeomCurve3d)    aC1, aC2;
  Handle(Draw_Marker3D) aMr;
  DrawColor            aColor(Draw_rouge);

  aC1 = DrawTrSurf1::GetCurve(a[1]);
  if (aC1.IsNull())
  {
    std::cout << a[1] << " is null curve" << std::endl;
    return 0;
  }

  aC2 = DrawTrSurf1::GetCurve(a[2]);
  if (aC2.IsNull())
  {
    std::cout << a[2] << " is null curve" << std::endl;
    return 0;
  }

  aT1 = Draw1::Atof(a[3]);
  aT2 = Draw1::Atof(a[4]);

  aNbP = 10;
  if (n > 4)
  {
    aNbP = Draw1::Atoi(a[5]);
  }

  iSize = 3;

  Standard_Real aMaxParam = 0.0;
  Standard_Real aMaxDist  = 0.0;

  dT = (aT2 - aT1) / (aNbP - 1);
  for (i = 0; i < aNbP; ++i)
  {
    aT = aT1 + i * dT;
    if (i == aNbP - 1)
      aT = aT2;

    aC1->D0(aT, aP1);
    aC2->D0(aT, aP2);

    aD = aP1.Distance(aP2);

    if (aD > aMaxDist)
    {
      aMaxParam = aT;
      aMaxDist  = aD;
    }

    printf(" T=%lg\tD=%lg\n", aT, aD);

    aMr = new Draw_Marker3D(aP1, Draw_Plus, aColor, iSize);
    dout << aMr;
  }

  std::cout << "Max distance = " << aMaxDist << std::endl;
  std::cout << "Param = " << aMaxParam << std::endl;

  return 0;
}

//=================================================================================================

static Standard_Integer xdistc2dc2dss(DrawInterpreter&, Standard_Integer n, const char** a)
{
  if (n < 7)
  {
    std::cout << " Use xdistc2dc2dss c2d_1 c2d_2 s1 s2 t1 t2 nbp" << std::endl;
    return 0;
  }

  Standard_Integer      i, aNbP, iSize;
  Standard_Real         aD, aT, aT1, aT2, dT;
  Point3d                aP1, aP2;
  gp_Pnt2d              aP2d1, aP2d2;
  Handle(GeomCurve2d)  aC2d1, aC2d2;
  Handle(GeomSurface)  aS1, aS2;
  Handle(Draw_Marker3D) aMr;
  DrawColor            aColor(Draw_rouge);

  aC2d1 = DrawTrSurf1::GetCurve2d(a[1]);
  if (aC2d1.IsNull())
  {
    std::cout << a[1] << " is null 2dcurve" << std::endl;
    return 0;
  }

  aC2d2 = DrawTrSurf1::GetCurve2d(a[2]);
  if (aC2d2.IsNull())
  {
    std::cout << a[2] << " is null 2dcurve" << std::endl;
    return 0;
  }

  aS1 = DrawTrSurf1::GetSurface(a[3]);
  if (aS1.IsNull())
  {
    std::cout << a[3] << " is null surface" << std::endl;
    return 0;
  }

  aS2 = DrawTrSurf1::GetSurface(a[4]);
  if (aS2.IsNull())
  {
    std::cout << a[4] << " is null surface" << std::endl;
    return 0;
  }

  aT1 = Draw1::Atof(a[5]);
  aT2 = Draw1::Atof(a[6]);

  aNbP = 10;
  if (n > 6)
  {
    aNbP = Draw1::Atoi(a[7]);
  }

  iSize = 3;

  Standard_Real aMaxParam = 0.0;
  Standard_Real aMaxDist  = 0.0;

  dT = (aT2 - aT1) / (aNbP - 1);
  for (i = 0; i < aNbP; ++i)
  {
    aT = aT1 + i * dT;
    if (i == aNbP - 1)
      aT = aT2;

    aC2d1->D0(aT, aP2d1);
    aS1->D0(aP2d1.X(), aP2d1.Y(), aP1);

    aC2d2->D0(aT, aP2d2);
    aS2->D0(aP2d2.X(), aP2d2.Y(), aP2);

    aD = aP1.Distance(aP2);

    if (aD > aMaxDist)
    {
      aMaxParam = aT;
      aMaxDist  = aD;
    }

    printf(" T=%lg\tD=%lg\n", aT, aD);

    aMr = new Draw_Marker3D(aP1, Draw_Plus, aColor, iSize);
    dout << aMr;
  }

  std::cout << "Max distance = " << aMaxDist << std::endl;
  std::cout << "Param = " << aMaxParam << std::endl;

  return 0;
}

//=================================================================================================

static Standard_Integer xdistcc2ds(DrawInterpreter&, Standard_Integer n, const char** a)
{
  if (n < 6)
  {
    std::cout << " Use xdistcc2ds c c2d s t1 t2 nbp" << std::endl;
    return 0;
  }

  Standard_Integer      i, aNbP, iSize;
  Standard_Real         aD, aT, aT1, aT2, dT;
  Point3d                aP, aPOnS;
  gp_Pnt2d              aP2d;
  Handle(GeomCurve3d)    aC;
  Handle(GeomCurve2d)  aC2d;
  Handle(GeomSurface)  aS;
  Handle(Draw_Marker3D) aMr;
  DrawColor            aColor(Draw_rouge);

  aC = DrawTrSurf1::GetCurve(a[1]);
  if (aC.IsNull())
  {
    std::cout << a[1] << " is null curve" << std::endl;
    return 0;
  }

  aC2d = DrawTrSurf1::GetCurve2d(a[2]);
  if (aC2d.IsNull())
  {
    std::cout << a[2] << " is null 2dcurve" << std::endl;
    return 0;
  }

  aS = DrawTrSurf1::GetSurface(a[3]);
  if (aS.IsNull())
  {
    std::cout << a[3] << " is null surface" << std::endl;
    return 0;
  }

  aT1 = Draw1::Atof(a[4]);
  aT2 = Draw1::Atof(a[5]);

  aNbP = 10;
  if (n > 5)
  {
    aNbP = Draw1::Atoi(a[6]);
  }

  iSize = 3;

  Standard_Real aMaxParam = 0.0;
  Standard_Real aMaxDist  = 0.0;

  dT = (aT2 - aT1) / (aNbP - 1);
  for (i = 0; i < aNbP; ++i)
  {
    aT = aT1 + i * dT;
    if (i == aNbP - 1)
      aT = aT2;

    aC->D0(aT, aP);

    aC2d->D0(aT, aP2d);
    aS->D0(aP2d.X(), aP2d.Y(), aPOnS);

    aD = aP.Distance(aPOnS);

    if (aD > aMaxDist)
    {
      aMaxParam = aT;
      aMaxDist  = aD;
    }

    printf(" T=%lg\tD=%lg\n", aT, aD);

    aMr = new Draw_Marker3D(aP, Draw_Plus, aColor, iSize);
    dout << aMr;
  }

  std::cout << "Max distance = " << aMaxDist << std::endl;
  std::cout << "Param = " << aMaxParam << std::endl;

  return 0;
}

//=================================================================================================

static Standard_Integer xdistcs(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 6)
  {
    std::cout << "Use: xdistcs curve surface t1 t2 nbpoints [tol [warn_tol]]" << std::endl;
    std::cout << "Measures distances from curve to surface by nbpoints probing points on a curve"
              << std::endl;
    std::cout << "Error will be reported for points where distance is greater than tol"
              << std::endl;
    std::cout << "Warning will be reported for points where distance is greater than warn_tol"
              << std::endl;
    return 0;
  }
  //
  Standard_Boolean           bRet;
  Standard_Integer           i, aNbP, iSize;
  Standard_Real              aTol, aD, aT, aT1, aT2, dT;
  Point3d                     aP;
  Handle(GeomCurve3d)         aC;
  Handle(GeomSurface)       aS;
  PointOnSurfProjector aPPS;
  Handle(Draw_Marker3D)      aMr;
  DrawColor                 aColor(Draw_rouge);
  //
  aTol = 1.e-7;
  //
  aC = DrawTrSurf1::GetCurve(a[1]);
  if (aC.IsNull())
  {
    di << "Error: " << a[1] << " is not a curve!\n";
    return 0;
  }
  //
  aS = DrawTrSurf1::GetSurface(a[2]);
  if (aS.IsNull())
  {
    di << "Error: " << a[2] << " is not a surface!\n";
    return 0;
  }
  //
  aT1 = Draw1::Atof(a[3]);
  aT2 = Draw1::Atof(a[4]);
  //
  aNbP = 10;
  if (n > 5)
  {
    aNbP = Draw1::Atoi(a[5]);
  }
  Standard_Real anErrTol = (n > 6 ? Draw1::Atof(a[6]) : RealLast());
  Standard_Real aWarnTol = (n > 7 ? Draw1::Atof(a[7]) : RealLast());
  //
  iSize = 3;
  //
  dT = (aT2 - aT1) / (aNbP - 1);

  Standard_Real aMaxParam = 0.0;
  Standard_Real aMaxDist  = 0.0;
  for (i = 0; i < aNbP; ++i)
  {
    aT = aT1 + i * dT;
    if (i == aNbP - 1)
    {
      aT = aT2;
    }
    //
    aC->D0(aT, aP);
    aPPS.Init(aP, aS, aTol);
    bRet = aPPS.IsDone();
    if (!bRet)
    {
      di << "Error: PointOnSurfProjector failed\n";
      return 0;
    }
    //
    aD = aPPS.LowerDistance();
    // report error or warning if distance is greater than tolerance
    if (aD > anErrTol)
    {
      di << "Error in " << a[1] << ":";
    }
    else if (aD > aWarnTol)
    {
      di << "Attention (critical value of tolerance) :";
    }
    char aMsg[256];
    sprintf(aMsg, " T=%lg\tD=%lg\n", aT, aD);
    di << aMsg;
    //
    aMr = new Draw_Marker3D(aP, Draw_Plus, aColor, iSize);
    dout << aMr;

    if (aD > aMaxDist)
    {
      aMaxParam = aT;
      aMaxDist  = aD;
    }
  }

  di << "Max distance = " << aMaxDist << "\n";
  di << "Param = " << aMaxParam << "\n";
  //
  return 0;
}

//=================================================================================================

void GeometryTest::TestProjCommands(DrawInterpreter& theCommands)
{

  static Standard_Boolean loaded = Standard_False;
  if (loaded)
    return;
  loaded = Standard_True;

  DrawTrSurf1::BasicCommands(theCommands);

  const char* g;

  g = "Testing of projection (geometric objects)";

  theCommands.Add("xdistcs",
                  "xdistcs curve surface t1 t2 nbpoints [tol [warn_tol]]",
                  __FILE__,
                  xdistcs,
                  g);
  theCommands.Add("xdistcc2ds", "xdistcc2ds c c2d s t1 t2 nbp", __FILE__, xdistcc2ds, g);
  theCommands.Add("xdistc2dc2dss",
                  "xdistc2dc2dss c2d_1 c2d_2 s1 s2 t1 t2 nbp",
                  __FILE__,
                  xdistc2dc2dss,
                  g);
  theCommands.Add("xdistcc", "xdistcc c1 c2 t1 t2 nbp", __FILE__, xdistcc, g);
}
