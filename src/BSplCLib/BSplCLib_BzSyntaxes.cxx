// Created on: 1995-09-08
// Created by: Laurent BOURESCHE
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

#include <BSplCLib.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>

#define No_Standard_RangeError
#define No_Standard_OutOfRange

//=======================================================================
// struct : BSplCLib_BezierArrays
// purpose: Auxiliary structure providing standard definitions of bspline
//         knots for bezier (using stack allocation)
//=======================================================================

class BSplCLib_BezierArrays
{
public:
  BSplCLib_BezierArrays(Standard_Integer Degree)
      : aKnots{0., 1.},
        aMults{Degree + 1, Degree + 1},
        knots(aKnots[0], 1, 2),
        mults(aMults[0], 1, 2)
  {
  }

private:
  Standard_Real    aKnots[2];
  Standard_Integer aMults[2];

public:
  TColStd_Array1OfReal    knots;
  TColStd_Array1OfInteger mults;
};

//=================================================================================================

void BSplCLib1::IncreaseDegree(const Standard_Integer      NewDegree,
                              const TColgp_Array1OfPnt&   Poles,
                              const TColStd_Array1OfReal* Weights,
                              TColgp_Array1OfPnt&         NewPoles,
                              TColStd_Array1OfReal*       NewWeights)
{
  Standard_Integer      deg = Poles.Length() - 1;
  BSplCLib_BezierArrays bzarr(deg);
  BSplCLib1::IncreaseDegree(deg,
                           NewDegree,
                           0,
                           Poles,
                           Weights,
                           bzarr.knots,
                           bzarr.mults,
                           NewPoles,
                           NewWeights,
                           bzarr.knots,
                           bzarr.mults);
}

//=================================================================================================

void BSplCLib1::IncreaseDegree(const Standard_Integer      NewDegree,
                              const TColgp_Array1OfPnt2d& Poles,
                              const TColStd_Array1OfReal* Weights,
                              TColgp_Array1OfPnt2d&       NewPoles,
                              TColStd_Array1OfReal*       NewWeights)
{
  Standard_Integer      deg = Poles.Length() - 1;
  BSplCLib_BezierArrays bzarr(deg);
  BSplCLib1::IncreaseDegree(deg,
                           NewDegree,
                           0,
                           Poles,
                           Weights,
                           bzarr.knots,
                           bzarr.mults,
                           NewPoles,
                           NewWeights,
                           bzarr.knots,
                           bzarr.mults);
}

//=================================================================================================

void BSplCLib1::PolesCoefficients(const TColgp_Array1OfPnt&   Poles,
                                 const TColStd_Array1OfReal* Weights,
                                 TColgp_Array1OfPnt&         CachePoles,
                                 TColStd_Array1OfReal*       CacheWeights)
{
  Standard_Integer     deg = Poles.Length() - 1;
  TColStd_Array1OfReal bidflatknots(FlatBezierKnots(deg), 1, 2 * (deg + 1));
  BSplCLib1::BuildCache(0., 1., 0, deg, bidflatknots, Poles, Weights, CachePoles, CacheWeights);
}

//=================================================================================================

void BSplCLib1::PolesCoefficients(const TColgp_Array1OfPnt2d& Poles,
                                 const TColStd_Array1OfReal* Weights,
                                 TColgp_Array1OfPnt2d&       CachePoles,
                                 TColStd_Array1OfReal*       CacheWeights)
{
  Standard_Integer     deg = Poles.Length() - 1;
  TColStd_Array1OfReal bidflatknots(FlatBezierKnots(deg), 1, 2 * (deg + 1));
  BSplCLib1::BuildCache(0., 1., 0, deg, bidflatknots, Poles, Weights, CachePoles, CacheWeights);
}

//=================================================================================================

void BSplCLib1::D0(const Standard_Real         U,
                  const TColgp_Array1OfPnt&   Poles,
                  const TColStd_Array1OfReal* Weights,
                  Point3d&                     P)
{
  Standard_Integer      deg = Poles.Length() - 1;
  BSplCLib_BezierArrays bzarr(deg);
  BSplCLib1::D0(U, 1, deg, 0, Poles, Weights, bzarr.knots, &bzarr.mults, P);
}

//=================================================================================================

void BSplCLib1::D0(const Standard_Real         U,
                  const TColgp_Array1OfPnt2d& Poles,
                  const TColStd_Array1OfReal* Weights,
                  gp_Pnt2d&                   P)
{
  Standard_Integer      deg = Poles.Length() - 1;
  BSplCLib_BezierArrays bzarr(deg);
  BSplCLib1::D0(U, 1, deg, 0, Poles, Weights, bzarr.knots, &bzarr.mults, P);
}

//=================================================================================================

void BSplCLib1::D1(const Standard_Real         U,
                  const TColgp_Array1OfPnt&   Poles,
                  const TColStd_Array1OfReal* Weights,
                  Point3d&                     P,
                  Vector3d&                     V)
{
  Standard_Integer      deg = Poles.Length() - 1;
  BSplCLib_BezierArrays bzarr(deg);
  BSplCLib1::D1(U, 1, deg, 0, Poles, Weights, bzarr.knots, &bzarr.mults, P, V);
}

//=================================================================================================

void BSplCLib1::D1(const Standard_Real         U,
                  const TColgp_Array1OfPnt2d& Poles,
                  const TColStd_Array1OfReal* Weights,
                  gp_Pnt2d&                   P,
                  gp_Vec2d&                   V)
{
  Standard_Integer      deg = Poles.Length() - 1;
  BSplCLib_BezierArrays bzarr(deg);
  BSplCLib1::D1(U, 1, deg, 0, Poles, Weights, bzarr.knots, &bzarr.mults, P, V);
}

//=================================================================================================

void BSplCLib1::D2(const Standard_Real         U,
                  const TColgp_Array1OfPnt&   Poles,
                  const TColStd_Array1OfReal* Weights,
                  Point3d&                     P,
                  Vector3d&                     V1,
                  Vector3d&                     V2)
{
  Standard_Integer      deg = Poles.Length() - 1;
  BSplCLib_BezierArrays bzarr(deg);
  BSplCLib1::D2(U, 1, deg, 0, Poles, Weights, bzarr.knots, &bzarr.mults, P, V1, V2);
}

//=================================================================================================

void BSplCLib1::D2(const Standard_Real         U,
                  const TColgp_Array1OfPnt2d& Poles,
                  const TColStd_Array1OfReal* Weights,
                  gp_Pnt2d&                   P,
                  gp_Vec2d&                   V1,
                  gp_Vec2d&                   V2)
{
  Standard_Integer      deg = Poles.Length() - 1;
  BSplCLib_BezierArrays bzarr(deg);
  BSplCLib1::D2(U, 1, deg, 0, Poles, Weights, bzarr.knots, &bzarr.mults, P, V1, V2);
}

//=================================================================================================

void BSplCLib1::D3(const Standard_Real         U,
                  const TColgp_Array1OfPnt&   Poles,
                  const TColStd_Array1OfReal* Weights,
                  Point3d&                     P,
                  Vector3d&                     V1,
                  Vector3d&                     V2,
                  Vector3d&                     V3)
{
  Standard_Integer      deg = Poles.Length() - 1;
  BSplCLib_BezierArrays bzarr(deg);
  BSplCLib1::D3(U, 1, deg, 0, Poles, Weights, bzarr.knots, &bzarr.mults, P, V1, V2, V3);
}

//=================================================================================================

void BSplCLib1::D3(const Standard_Real         U,
                  const TColgp_Array1OfPnt2d& Poles,
                  const TColStd_Array1OfReal* Weights,
                  gp_Pnt2d&                   P,
                  gp_Vec2d&                   V1,
                  gp_Vec2d&                   V2,
                  gp_Vec2d&                   V3)
{
  Standard_Integer      deg = Poles.Length() - 1;
  BSplCLib_BezierArrays bzarr(deg);
  BSplCLib1::D3(U, 1, deg, 0, Poles, Weights, bzarr.knots, &bzarr.mults, P, V1, V2, V3);
}
