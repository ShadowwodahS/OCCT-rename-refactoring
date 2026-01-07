// Created on: 1994-03-17
// Created by: Bruno DUMORTIER
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

#include <Extrema_ExtPS.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <gp_Pnt.hxx>
#include <Precision.hxx>
#include <Standard_OutOfRange.hxx>
#include <StdFail_NotDone.hxx>

//=================================================================================================

PointOnSurfProjector::PointOnSurfProjector()
    : myIsDone(Standard_False),
      myIndex(0)
{
}

//=================================================================================================

PointOnSurfProjector::PointOnSurfProjector(const Point3d&               P,
                                                       const Handle(GeomSurface)& Surface,
                                                       const Extrema_ExtAlgo       theProjAlgo)
{
  Init(P, Surface, theProjAlgo);
}

//=================================================================================================

PointOnSurfProjector::PointOnSurfProjector(const Point3d&               P,
                                                       const Handle(GeomSurface)& Surface,
                                                       const Standard_Real         Tolerance,
                                                       const Extrema_ExtAlgo       theProjAlgo)
{
  Init(P, Surface, Tolerance, theProjAlgo);
}

//=================================================================================================

PointOnSurfProjector::PointOnSurfProjector(const Point3d&               P,
                                                       const Handle(GeomSurface)& Surface,
                                                       const Standard_Real         Umin,
                                                       const Standard_Real         Usup,
                                                       const Standard_Real         Vmin,
                                                       const Standard_Real         Vsup,
                                                       const Extrema_ExtAlgo       theProjAlgo)

{
  Init(P, Surface, Umin, Usup, Vmin, Vsup, theProjAlgo);
}

//=================================================================================================

PointOnSurfProjector::PointOnSurfProjector(const Point3d&               P,
                                                       const Handle(GeomSurface)& Surface,
                                                       const Standard_Real         Umin,
                                                       const Standard_Real         Usup,
                                                       const Standard_Real         Vmin,
                                                       const Standard_Real         Vsup,
                                                       const Standard_Real         Tolerance,
                                                       const Extrema_ExtAlgo       theProjAlgo)

{
  Init(P, Surface, Umin, Usup, Vmin, Vsup, Tolerance, theProjAlgo);
}

//=================================================================================================

void PointOnSurfProjector::Init()
{
  myIsDone = myExtPS.IsDone() && (myExtPS.NbExt() > 0);

  if (myIsDone)
  {
    // evaluate the lower distance and its index;
    Standard_Real Dist2, Dist2Min = myExtPS.SquareDistance(1);
    myIndex = 1;

    for (Standard_Integer i = 2; i <= myExtPS.NbExt(); i++)
    {
      Dist2 = myExtPS.SquareDistance(i);
      if (Dist2 < Dist2Min)
      {
        Dist2Min = Dist2;
        myIndex  = i;
      }
    }
  }
}

//=================================================================================================

void PointOnSurfProjector::Init(const Point3d&               P,
                                      const Handle(GeomSurface)& Surface,
                                      const Extrema_ExtAlgo       theProjAlgo)

{
  Init(P, Surface, Precision::Confusion(), theProjAlgo);
}

//=================================================================================================

void PointOnSurfProjector::Init(const Point3d&               P,
                                      const Handle(GeomSurface)& Surface,
                                      const Standard_Real         Tolerance,
                                      const Extrema_ExtAlgo       theProjAlgo)

{
  // modified by NIZNHY-PKV Thu Apr  4 10:37:55 2002 f
  // GeomAdaptor_Surface TheSurface (Surface);
  // myExtPS = Extrema_ExtPS (P, TheSurface, Tolerance, Tolerance);

  // modified by NIZNHY-PKV Mon Apr  8 11:13:37 2002 f XXX
  Standard_Real Umin, Usup, Vmin, Vsup;
  Surface->Bounds(Umin, Usup, Vmin, Vsup);
  myGeomAdaptor.Load(Surface, Umin, Usup, Vmin, Vsup);
  //
  // myExtPS = Extrema_ExtPS();
  myExtPS.SetAlgo(theProjAlgo);
  myExtPS.Initialize(myGeomAdaptor, Umin, Usup, Vmin, Vsup, Tolerance, Tolerance);
  myExtPS.Perform(P);
  // XXXmyExtPS = Extrema_ExtPS (P, myGeomAdaptor, Tolerance, Tolerance);
  // modified by NIZNHY-PKV Mon Apr  8 11:13:44 2002 t XXX

  // modified by NIZNHY-PKV Thu Apr  4 10:37:58 2002 t
  Init();
}

//=================================================================================================

void PointOnSurfProjector::Init(const Point3d&               P,
                                      const Handle(GeomSurface)& Surface,
                                      const Standard_Real         Umin,
                                      const Standard_Real         Usup,
                                      const Standard_Real         Vmin,
                                      const Standard_Real         Vsup,
                                      const Extrema_ExtAlgo       theProjAlgo)
{
  constexpr Standard_Real Tolerance = Precision::PConfusion();
  // modified by NIZNHY-PKV Thu Apr  4 10:38:23 2002 f
  // GeomAdaptor_Surface TheSurface (Surface,Umin,Usup,Vmin,Vsup);
  // myExtPS = Extrema_ExtPS (P, TheSurface, Tol, Tol);
  myGeomAdaptor.Load(Surface, Umin, Usup, Vmin, Vsup);
  // myExtPS = Extrema_ExtPS();
  myExtPS.SetAlgo(theProjAlgo);
  myExtPS.Initialize(myGeomAdaptor, Umin, Usup, Vmin, Vsup, Tolerance, Tolerance);
  myExtPS.Perform(P);
  // XXX myExtPS = Extrema_ExtPS (P, myGeomAdaptor, Tol, Tol);
  // modified by NIZNHY-PKV Thu Apr  4 10:38:30 2002 t
  Init();
}

//=================================================================================================

void PointOnSurfProjector::Init(const Point3d&               P,
                                      const Handle(GeomSurface)& Surface,
                                      const Standard_Real         Umin,
                                      const Standard_Real         Usup,
                                      const Standard_Real         Vmin,
                                      const Standard_Real         Vsup,
                                      const Standard_Real         Tolerance,
                                      const Extrema_ExtAlgo       theProjAlgo)
{
  // modified by NIZNHY-PKV Thu Apr  4 10:39:10 2002 f
  // GeomAdaptor_Surface TheSurface (Surface,Umin,Usup,Vmin,Vsup);
  // myExtPS = Extrema_ExtPS (P, TheSurface, Tolerance, Tolerance);
  myGeomAdaptor.Load(Surface, Umin, Usup, Vmin, Vsup);
  // myExtPS = Extrema_ExtPS();
  myExtPS.SetAlgo(theProjAlgo);
  myExtPS.Initialize(myGeomAdaptor, Umin, Usup, Vmin, Vsup, Tolerance, Tolerance);
  myExtPS.Perform(P);
  // XXX myExtPS = Extrema_ExtPS (P, myGeomAdaptor, Tolerance, Tolerance);
  // modified by NIZNHY-PKV Thu Apr  4 10:39:14 2002 t
  Init();
}

//=================================================================================================

void PointOnSurfProjector::Init(const Handle(GeomSurface)& Surface,
                                      const Standard_Real         Umin,
                                      const Standard_Real         Usup,
                                      const Standard_Real         Vmin,
                                      const Standard_Real         Vsup,
                                      const Extrema_ExtAlgo       theProjAlgo)
{
  constexpr Standard_Real Tolerance = Precision::PConfusion();
  // modified by NIZNHY-PKV Thu Apr  4 10:41:50 2002 f
  // GeomAdaptor_Surface TheSurface (Surface,Umin,Usup,Vmin,Vsup);
  myGeomAdaptor.Load(Surface, Umin, Usup, Vmin, Vsup);
  // modified by NIZNHY-PKV Thu Apr  4 10:42:29 2002 t
  // myExtPS = Extrema_ExtPS();
  // modified by NIZNHY-PKV Thu Apr  4 10:42:32 2002 f
  // myExtPS.Initialize(TheSurface, Umin, Usup, Vmin, Vsup, Tol, Tol);
  myExtPS.SetAlgo(theProjAlgo);
  myExtPS.Initialize(myGeomAdaptor, Umin, Usup, Vmin, Vsup, Tolerance, Tolerance);
  // modified by NIZNHY-PKV Thu Apr  4 10:42:39 2002 t
  myIsDone = Standard_False;
}

//=================================================================================================

void PointOnSurfProjector::Init(const Handle(GeomSurface)& Surface,
                                      const Standard_Real         Umin,
                                      const Standard_Real         Usup,
                                      const Standard_Real         Vmin,
                                      const Standard_Real         Vsup,
                                      const Standard_Real         Tolerance,
                                      const Extrema_ExtAlgo       theProjAlgo)
{
  // modified by NIZNHY-PKV Thu Apr  4 10:43:00 2002 f
  // GeomAdaptor_Surface TheSurface (Surface,Umin,Usup,Vmin,Vsup);
  myGeomAdaptor.Load(Surface, Umin, Usup, Vmin, Vsup);
  // modified by NIZNHY-PKV Thu Apr  4 10:43:16 2002 t
  // myExtPS = Extrema_ExtPS();
  // modified by NIZNHY-PKV Thu Apr  4 10:43:18 2002 f
  // myExtPS.Initialize(TheSurface, Umin, Usup, Vmin, Vsup, Tolerance, Tolerance);
  myExtPS.SetAlgo(theProjAlgo);
  myExtPS.Initialize(myGeomAdaptor, Umin, Usup, Vmin, Vsup, Tolerance, Tolerance);
  // modified by NIZNHY-PKV Thu Apr  4 10:43:26 2002 t
  myIsDone = Standard_False;
}

//=================================================================================================

void PointOnSurfProjector::Perform(const Point3d& P)
{
  myExtPS.Perform(P);
  Init();
}

//=================================================================================================

Standard_Boolean PointOnSurfProjector::IsDone() const
{
  return myIsDone;
}

//=================================================================================================

Standard_Integer PointOnSurfProjector::NbPoints() const
{
  if (myIsDone)
  {
    return myExtPS.NbExt();
  }
  else
  {
    return 0;
  }
}

//=================================================================================================

Point3d PointOnSurfProjector::Point(const Standard_Integer Index) const
{
  Standard_OutOfRange_Raise_if(Index < 1 || Index > NbPoints(),
                               "PointOnSurfProjector::Point");
  return (myExtPS.Point(Index)).Value();
}

//=================================================================================================

void PointOnSurfProjector::Parameters(const Standard_Integer Index,
                                            Standard_Real&         U,
                                            Standard_Real&         V) const
{
  Standard_OutOfRange_Raise_if(Index < 1 || Index > NbPoints(),
                               "PointOnSurfProjector::Parameter");
  (myExtPS.Point(Index)).Parameter(U, V);
}

//=================================================================================================

Standard_Real PointOnSurfProjector::Distance(const Standard_Integer Index) const
{
  Standard_OutOfRange_Raise_if(Index < 1 || Index > NbPoints(),
                               "PointOnSurfProjector::Distance");
  return sqrt(myExtPS.SquareDistance(Index));
}

//=================================================================================================

Point3d PointOnSurfProjector::NearestPoint() const
{
  StdFail_NotDone_Raise_if(!myIsDone, "PointOnSurfProjector::NearestPoint");

  return (myExtPS.Point(myIndex)).Value();
}

//=================================================================================================

PointOnSurfProjector::operator Standard_Integer() const
{
  return NbPoints();
}

//=================================================================================================

PointOnSurfProjector::operator Point3d() const
{
  return NearestPoint();
}

//=================================================================================================

void PointOnSurfProjector::LowerDistanceParameters(Standard_Real& U, Standard_Real& V) const
{
  StdFail_NotDone_Raise_if(!myIsDone, "PointOnSurfProjector::LowerDistanceParameters");

  (myExtPS.Point(myIndex)).Parameter(U, V);
}

//=================================================================================================

Standard_Real PointOnSurfProjector::LowerDistance() const
{
  StdFail_NotDone_Raise_if(!myIsDone, "PointOnSurfProjector::LowerDistance");

  return sqrt(myExtPS.SquareDistance(myIndex));
}

//=================================================================================================

PointOnSurfProjector::operator Standard_Real() const
{
  return LowerDistance();
}
