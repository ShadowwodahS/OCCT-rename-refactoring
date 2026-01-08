// Created on: 1999-09-16
// Created by: Edward AGAPOV
// Copyright (c) 1999 Matra Datavision
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

#include <Standard_NotImplemented.hxx>
#include <ElCLib.hxx>
#include <Extrema_ExtPElC.hxx>
#include <Extrema_ExtPExtS.hxx>
#include <Extrema_POnCurv.hxx>
#include <Extrema_POnSurf.hxx>
#include <Precision.hxx>
#include <gp.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <math_FunctionSetRoot.hxx>
#include <math_Vector.hxx>
#include <GeomAdaptor_SurfaceOfLinearExtrusion.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Extrema_ExtPExtS, RefObject)

static Frame3d GetPosition(const Handle(Adaptor3d_Curve)& C);

static void PerformExtPElC(PointElCCurveExtrema&               E,
                           const Point3d&                  P,
                           const Handle(Adaptor3d_Curve)& C,
                           const Standard_Real            Tol);

static Standard_Boolean IsCaseAnalyticallyComputable(const GeomAbs_CurveType& theType,
                                                     const Frame3d&            theCurvePos,
                                                     const Dir3d&            theSurfaceDirection);

static Point3d GetValue(const Standard_Real U, const Handle(Adaptor3d_Curve)& C);

//=======================================================================
// function : Project
// purpose  : Returns the projection of a point <Point> on a plane
//           <ThePlane>  along a direction <TheDir>.
//=======================================================================
static Point3d ProjectPnt(const Frame3d& ThePlane, const Dir3d& TheDir, const Point3d& Point)
{
  Vector3d        PO(Point, ThePlane.Location());
  Standard_Real Alpha = PO * Vector3d(ThePlane.Direction());
  Alpha /= TheDir * ThePlane.Direction();
  Point3d P;
  P.SetXYZ(Point.XYZ() + Alpha * TheDir.XYZ());
  return P;
}

//=================================================================================================

static Standard_Boolean IsOriginalPnt(const Point3d&          P,
                                      const PointOnSurface1* Points,
                                      const Standard_Integer NbPoints)
{
  for (Standard_Integer i = 1; i <= NbPoints; i++)
  {
    if (Points[i - 1].Value().IsEqual(P, Precision1::Confusion()))
    {
      return Standard_False;
    }
  }
  return Standard_True;
}

//=================================================================================================

void Extrema_ExtPExtS::MakePreciser(Standard_Real&         U,
                                    const Point3d&          P,
                                    const Standard_Boolean isMin,
                                    const Frame3d&          OrtogSection) const
{
  if (U > myusup)
  {
    U = myusup;
  }
  else if (U < myuinf)
  {
    U = myuinf;
  }
  else
  {

    Standard_Real step = (myusup - myuinf) / 30, D2e, D2next, D2prev;
    Point3d        Pe   = ProjectPnt(OrtogSection, myDirection, GetValue(U, myC)),
           Pprev       = ProjectPnt(OrtogSection, myDirection, GetValue(U - step, myC)),
           Pnext       = ProjectPnt(OrtogSection, myDirection, GetValue(U + step, myC));
    D2e = P.SquareDistance(Pe), D2next = P.SquareDistance(Pnext), D2prev = P.SquareDistance(Pprev);
    Standard_Boolean notFound;
    if (isMin)
      notFound = (D2e > D2prev || D2e > D2next);
    else
      notFound = (D2e < D2prev || D2e < D2next);

    if (notFound && (D2e < D2next && isMin))
    {
      step   = -step;
      D2next = D2prev;
      Pnext  = Pprev;
    }
    while (notFound)
    {
      U = U + step;
      if (U > myusup)
      {
        U = myusup;
        break;
      }
      if (U < myuinf)
      {
        U = myuinf;
        break;
      }
      D2e    = D2next;
      Pe     = Pnext;
      Pnext  = ProjectPnt(OrtogSection, myDirection, GetValue(U + step, myC));
      D2next = P.SquareDistance(Pnext);
      if (isMin)
        notFound = D2e > D2next;
      else
        notFound = D2e < D2next;
    }
  }
}

//=============================================================================

Extrema_ExtPExtS::Extrema_ExtPExtS()
    : myuinf(0.0),
      myusup(0.0),
      mytolu(0.0),
      myvinf(0.0),
      myvsup(0.0),
      mytolv(0.0),
      myIsAnalyticallyComputable(Standard_False),
      myDone(Standard_False),
      myNbExt(0)
{
  for (size_t anIdx = 0; anIdx < sizeof(mySqDist) / sizeof(mySqDist[0]); anIdx++)
  {
    mySqDist[anIdx] = RealLast();
  }
}

//=============================================================================

Extrema_ExtPExtS::Extrema_ExtPExtS(const Point3d&                                       theP,
                                   const Handle(GeomAdaptor_SurfaceOfLinearExtrusion)& theS,
                                   const Standard_Real                                 theUmin,
                                   const Standard_Real                                 theUsup,
                                   const Standard_Real                                 theVmin,
                                   const Standard_Real                                 theVsup,
                                   const Standard_Real                                 theTolU,
                                   const Standard_Real                                 theTolV)
    : myuinf(theUmin),
      myusup(theUsup),
      mytolu(theTolU),
      myvinf(theVmin),
      myvsup(theVsup),
      mytolv(theTolV),
      myS(theS),
      myIsAnalyticallyComputable(Standard_False),
      myDone(Standard_False),
      myNbExt(0)
{
  for (size_t anIdx = 0; anIdx < sizeof(mySqDist) / sizeof(mySqDist[0]); anIdx++)
  {
    mySqDist[anIdx] = RealLast();
  }
  Initialize(theS, theUmin, theUsup, theVmin, theVsup, theTolU, theTolV);

  Perform(theP);
}

//=============================================================================

Extrema_ExtPExtS::Extrema_ExtPExtS(const Point3d&                                       theP,
                                   const Handle(GeomAdaptor_SurfaceOfLinearExtrusion)& theS,
                                   const Standard_Real                                 theTolU,
                                   const Standard_Real                                 theTolV)
    : myuinf(theS->FirstUParameter()),
      myusup(theS->LastUParameter()),
      mytolu(theTolU),
      myvinf(theS->FirstVParameter()),
      myvsup(theS->LastVParameter()),
      mytolv(theTolV),
      myS(theS),
      myIsAnalyticallyComputable(Standard_False),
      myDone(Standard_False),
      myNbExt(0)
{
  for (size_t anIdx = 0; anIdx < sizeof(mySqDist) / sizeof(mySqDist[0]); anIdx++)
  {
    mySqDist[anIdx] = RealLast();
  }
  Initialize(theS,
             theS->FirstUParameter(),
             theS->LastUParameter(),
             theS->FirstVParameter(),
             theS->LastVParameter(),
             theTolU,
             theTolV);

  Perform(theP);
}

//=================================================================================================

void Extrema_ExtPExtS::Initialize(const Handle(GeomAdaptor_SurfaceOfLinearExtrusion)& theS,
                                  const Standard_Real                                 theUinf,
                                  const Standard_Real                                 theUsup,
                                  const Standard_Real                                 theVinf,
                                  const Standard_Real                                 theVsup,
                                  const Standard_Real                                 theTolU,
                                  const Standard_Real                                 theTolV)
{
  myuinf = theUinf;
  myusup = theUsup;
  mytolu = theTolU;

  myvinf = theVinf;
  myvsup = theVsup;
  mytolv = theTolV;

  myIsAnalyticallyComputable = Standard_False;
  myDone                     = Standard_False;
  myNbExt                    = 0;

  Handle(Adaptor3d_Curve) anACurve = theS->BasisCurve();

  myF.Initialize(*theS);
  myC                        = anACurve;
  myS                        = theS;
  myPosition                 = GetPosition(myC);
  myDirection                = theS->Direction();
  myIsAnalyticallyComputable = // Standard_False;
    IsCaseAnalyticallyComputable(myC->GetType(), myPosition, myDirection);

  if (!myIsAnalyticallyComputable)
  {
    myExtPS.Initialize(*theS, 32, 32, theUinf, theUsup, theVinf, theVsup, theTolU, theTolV);
  }
}

//=================================================================================================

void Extrema_ExtPExtS::Perform(const Point3d& P)
{
  const Standard_Integer NbExtMax = 4; // dimension of arrays
                                       // myPoint[] and mySqDist[]
                                       // For "analytical" case
  myDone  = Standard_False;
  myNbExt = 0;

  if (!myIsAnalyticallyComputable)
  {
    myExtPS.Perform(P);
    myDone = myExtPS.IsDone();
    //  modified by NIZHNY-EAP Wed Nov 17 12:59:08 1999 ___BEGIN___
    myNbExt = myExtPS.NbExt();
    //  modified by NIZHNY-EAP Wed Nov 17 12:59:09 1999 ___END___
    return;
  }

  Point3d          Pe, Pp = ProjectPnt(myPosition, myDirection, P);
  PointElCCurveExtrema anExt;
  PerformExtPElC(anExt, Pp, myC, mytolu);
  if (!anExt.IsDone())
    return;

  Frame3d           anOrtogSection(P, myDirection);
  Standard_Real    U, V;
  Standard_Boolean isMin,
    isSimpleCase = myDirection.IsParallel(myPosition.Direction(), Precision1::Angular());
  Standard_Integer i, aNbExt = anExt.NbExt();
  math_Vector      UV(1, 2), Tol(1, 2), UVinf(1, 2), UVsup(1, 2);
  Tol(1)   = mytolu;
  Tol(2)   = mytolv;
  UVinf(1) = myuinf;
  UVinf(2) = myvinf;
  UVsup(1) = myusup;
  UVsup(2) = myvsup;

  for (i = 1; i <= aNbExt; i++)
  {
    PointOnCurve1 POC = anExt.Point(i);
    U                   = POC.Parameter();
    //// modified by jgv, 23.12.2008 for OCC17194 ////
    if (myC->IsPeriodic())
    {
      Standard_Real U2 = U;
      ElCLib1::AdjustPeriodic(myuinf, myuinf + 2. * M_PI, Precision1::PConfusion(), U, U2);
    }
    //////////////////////////////////////////////////
    Point3d E = POC.Value();
    Pe       = ProjectPnt(anOrtogSection, myDirection, E);

    if (isSimpleCase)
    {
      V = Vector3d(E, Pe) * Vector3d(myDirection);
      // modified by NIZHNY-MKK  Thu Sep 18 14:46:14 2003.BEGIN
      //       myPoint[++myNbExt] = PointOnSurface1(U, V, Pe);
      //       myValue[myNbExt] = anExt.Value(i);
      myPoint[myNbExt]  = PointOnSurface1(U, V, Pe);
      mySqDist[myNbExt] = anExt.SquareDistance(i);
      myNbExt++;
      if (myNbExt == NbExtMax)
      {
        break;
      }
      // modified by NIZHNY-MKK  Thu Sep 18 14:46:18 2003.END
    }
    else
    {
      myF.SetPoint(P);
      isMin = anExt.IsMin(i); //( Pp.Distance(GetValue(U+10,myC)) > anExt.Value(i) );

      MakePreciser(U, P, isMin, anOrtogSection);
      E  = GetValue(U, myC);
      Pe = ProjectPnt(anOrtogSection, myDirection, E), V = Vector3d(E, Pe) * Vector3d(myDirection);
      UV(1) = U;
      UV(2) = V;
      FunctionSetRoot aFSR(myF, Tol);
      aFSR.Perform(myF, UV, UVinf, UVsup);
      //      for (Standard_Integer k=1 ; k <= myF.NbExt();
      Standard_Integer k;
      for (k = 1; k <= myF.NbExt(); k++)
      {
        if (IsOriginalPnt(myF.Point(k).Value(), myPoint, myNbExt))
        {
          // modified by NIZHNY-MKK  Thu Sep 18 14:46:41 2003.BEGIN
          // 	  myPoint[++myNbExt] = myF.Point(k);
          // 	  myValue[myNbExt] = myF.Value(k);
          myPoint[myNbExt]  = myF.Point(k);
          mySqDist[myNbExt] = myF.SquareDistance(k);
          myNbExt++;
          if (myNbExt == NbExtMax)
          {
            break;
          }
          // modified by NIZHNY-MKK  Thu Sep 18 14:46:43 2003.END
        }
      }
      if (myNbExt == NbExtMax)
      {
        break;
      }
      // try symmetric point
      myF.SetPoint(P); // To clear previous solutions
      U *= -1;
      MakePreciser(U, P, isMin, anOrtogSection);
      E  = GetValue(U, myC);
      Pe = ProjectPnt(anOrtogSection, myDirection, E), V = Vector3d(E, Pe) * Vector3d(myDirection);
      UV(1) = U;
      UV(2) = V;

      aFSR.Perform(myF, UV, UVinf, UVsup);

      for (k = 1; k <= myF.NbExt(); k++)
      {
        if (myF.SquareDistance(k) > Precision1::Confusion() * Precision1::Confusion())
        {
          // Additional checking solution: FSR sometimes is wrong
          // when starting point is far from solution.
          Standard_Real          dist = Sqrt(myF.SquareDistance(k));
          math_Vector            Vals(1, 2);
          const PointOnSurface1& PonS = myF.Point(k);
          Standard_Real          u, v;
          PonS.Parameter(u, v);
          UV(1) = u;
          UV(2) = v;
          myF.Value(UV, Vals);
          Vector3d du, dv;
          myS->D1(u, v, Pe, du, dv);
          Standard_Real mdu = du.Magnitude();
          Standard_Real mdv = dv.Magnitude();
          u                 = Abs(Vals(1));
          v                 = Abs(Vals(2));
          if (mdu > Precision1::PConfusion())
          {
            if (u / dist / mdu > Precision1::PConfusion())
            {
              continue;
            }
          }
          if (mdv > Precision1::PConfusion())
          {
            if (v / dist / mdv > Precision1::PConfusion())
            {
              continue;
            }
          }
        }
        if (IsOriginalPnt(myF.Point(k).Value(), myPoint, myNbExt))
        {
          // modified by NIZHNY-MKK  Thu Sep 18 14:46:59 2003.BEGIN
          // 	  myPoint[++myNbExt] = myF.Point(k);
          // 	  myValue[myNbExt] = myF.Value(k);
          myPoint[myNbExt]  = myF.Point(k);
          mySqDist[myNbExt] = myF.SquareDistance(k);
          myNbExt++;
          if (myNbExt == NbExtMax)
          {
            break;
          }
          // modified by NIZHNY-MKK  Thu Sep 18 14:47:04 2003.END
        }
      }
      if (myNbExt == NbExtMax)
      {
        break;
      }
    }
  }
  myDone = Standard_True;
  return;
}

//=============================================================================

Standard_Boolean Extrema_ExtPExtS::IsDone() const
{
  return myDone;
}

//=============================================================================

Standard_Integer Extrema_ExtPExtS::NbExt() const
{
  if (!IsDone())
  {
    throw StdFail_NotDone();
  }
  if (myIsAnalyticallyComputable)
    return myNbExt;
  else
    return myExtPS.NbExt();
}

//=============================================================================

Standard_Real Extrema_ExtPExtS::SquareDistance(const Standard_Integer N) const
{
  if ((N < 1) || (N > NbExt()))
  {
    throw Standard_OutOfRange();
  }
  if (myIsAnalyticallyComputable)
    // modified by NIZHNY-MKK  Thu Sep 18 14:48:39 2003.BEGIN
    //     return myValue[N];
    return mySqDist[N - 1];
  // modified by NIZHNY-MKK  Thu Sep 18 14:48:42 2003.END
  else
    return myExtPS.SquareDistance(N);
}

//=============================================================================

const PointOnSurface1& Extrema_ExtPExtS::Point(const Standard_Integer N) const
{
  if ((N < 1) || (N > NbExt()))
  {
    throw Standard_OutOfRange();
  }
  if (myIsAnalyticallyComputable)
  {
    // modified by NIZHNY-MKK  Thu Sep 18 14:47:40 2003.BEGIN
    //     return myPoint[N];
    return myPoint[N - 1];
  }
  // modified by NIZHNY-MKK  Thu Sep 18 14:47:43 2003.END
  else
    return myExtPS.Point(N);
}

//=============================================================================

static Frame3d GetPosition(const Handle(Adaptor3d_Curve)& C)
{
  switch (C->GetType())
  {
    case GeomAbs_Line: {
      gp_Lin L   = C->Line();
      gp_Pln Pln = gp_Pln(L.Location(), L.Direction());
      //: abv 30.05.02: OCC  - use constructor instead of Set...s() to avoid exception
      Frame3d Pos(Pln.Location(), Pln.Position1().Direction(), Pln.Position1().XDirection());
      //     Pos.SetAxis(Pln.XAxis());
      //     Pos.SetXDirection(Pln.YAxis().Direction());
      //     Pos.SetYDirection(Pln.Position1().Direction());
      return Pos;
    }
    case GeomAbs_Circle:
      return C->Circle().Position1();
    case GeomAbs_Ellipse:
      return C->Ellipse().Position1();
    case GeomAbs_Hyperbola:
      return C->Hyperbola().Position1();
    case GeomAbs_Parabola:
      return C->Parabola().Position1();
    default:
      return Frame3d();
  }
}

//=============================================================================

static void PerformExtPElC(PointElCCurveExtrema&               E,
                           const Point3d&                  P,
                           const Handle(Adaptor3d_Curve)& C,
                           const Standard_Real            Tol)
{
  switch (C->GetType())
  {
    case GeomAbs_Hyperbola:
      E.Perform(P, C->Hyperbola(), Tol, -Precision1::Infinite(), Precision1::Infinite());
      return;
    case GeomAbs_Line:
      E.Perform(P, C->Line(), Tol, -Precision1::Infinite(), Precision1::Infinite());
      return;
    case GeomAbs_Circle:
      E.Perform(P, C->Circle(), Tol, 0.0, 2.0 * M_PI);
      return;
    case GeomAbs_Ellipse:
      E.Perform(P, C->Ellipse(), Tol, 0.0, 2.0 * M_PI);
      return;
    case GeomAbs_Parabola:
      E.Perform(P, C->Parabola(), Tol, -Precision1::Infinite(), Precision1::Infinite());
      return;
    default:
      return;
  }
}

//=================================================================================================

static Standard_Boolean IsCaseAnalyticallyComputable(const GeomAbs_CurveType& theType,
                                                     const Frame3d&            theCurvePos,
                                                     const Dir3d&            theSurfaceDirection)
{
  // check type
  switch (theType)
  {
    case GeomAbs_Line:
    case GeomAbs_Circle:
    case GeomAbs_Ellipse:
    case GeomAbs_Hyperbola:
    case GeomAbs_Parabola:
      break;
    default:
      return Standard_False;
  }
  // check if it is a plane
  if (Abs(theCurvePos.Direction() * theSurfaceDirection) <= gp1::Resolution())
    return Standard_False;
  else
    return Standard_True;
}

//=================================================================================================

static Point3d GetValue(const Standard_Real U, const Handle(Adaptor3d_Curve)& C)
{
  switch (C->GetType())
  {
    case GeomAbs_Line:
      return ElCLib1::Value(U, C->Line());
    case GeomAbs_Circle:
      return ElCLib1::Value(U, C->Circle());
    case GeomAbs_Ellipse:
      return ElCLib1::Value(U, C->Ellipse());
    case GeomAbs_Hyperbola:
      return ElCLib1::Value(U, C->Hyperbola());
    case GeomAbs_Parabola:
      return ElCLib1::Value(U, C->Parabola());
    default:
      return Point3d();
  }
}

//=================================================================================================

// #ifdef OCCT_DEBUG
// static Standard_Real GetU(const Vector3d& vec,
//			  const Point3d& P,
//			  const Handle(Adaptor3d_Curve)& C)
//{
//  switch (C->GetType()) {
//  case GeomAbs_Line:
//    return ElCLib1::Parameter(C->Line().Translated(vec), P);
//  case GeomAbs_Circle:
//    return ElCLib1::Parameter(C->Circle().Translated(vec), P);
//  case GeomAbs_Ellipse:
//    return ElCLib1::Parameter(C->Ellipse().Translated(vec), P);
//  case GeomAbs_Hyperbola:
//    return ElCLib1::Parameter(C->Hyperbola().Translated(vec), P);
//  case GeomAbs_Parabola:
//    return ElCLib1::Parameter(C->Parabola().Translated(vec), P);
//  default:
//    return 0;
//  }
//}
// #endif
