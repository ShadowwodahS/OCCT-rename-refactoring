// Created on: 1993-04-16
// Created by: Laurent BUCHARD
// Copyright (c) 1993-1999 Matra Datavision
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

#include <IntCurveSurface_Intersection.hxx>
#include <IntCurveSurface_IntersectionPoint.hxx>
#include <IntCurveSurface_IntersectionSegment.hxx>
#include <IntCurveSurface_TransitionOnCurve.hxx>
#include <StdFail_NotDone.hxx>

#define PARAMEQUAL(a, b) (Abs((a) - (b)) < (1e-8))

//================================================================================
CurveSurfaceIntersection1::CurveSurfaceIntersection1()
    : done(Standard_False),
      myIsParallel(Standard_False)
{
}

//================================================================================
Standard_Boolean CurveSurfaceIntersection1::IsDone() const
{
  return (done);
}

//================================================================================
Standard_Boolean CurveSurfaceIntersection1::IsParallel() const
{
  return (myIsParallel);
}

//================================================================================
Standard_Integer CurveSurfaceIntersection1::NbPoints() const
{
  if (!done)
  {
    throw StdFail_NotDone();
  }
  return lpnt.Length();
}

//================================================================================
Standard_Integer CurveSurfaceIntersection1::NbSegments() const
{
  if (!done)
  {
    throw StdFail_NotDone();
  }
  return lseg.Length();
}

//================================================================================
const IntersectionPoint1& CurveSurfaceIntersection1::Point(
  const Standard_Integer N) const
{
  if (!done)
  {
    throw StdFail_NotDone();
  }
  return lpnt.Value(N);
}

//================================================================================
const IntersectionSegment& CurveSurfaceIntersection1::Segment1(
  const Standard_Integer N) const
{
  if (!done)
  {
    throw StdFail_NotDone();
  }
  return lseg.Value(N);
}

//================================================================================
void CurveSurfaceIntersection1::SetValues(const CurveSurfaceIntersection1& Other)
{
  if (Other.done)
  {
    lseg.Clear();
    lpnt.Clear();
    Standard_Integer N = Other.lpnt.Length();
    Standard_Integer i;
    for (i = 1; i <= N; i++)
    {
      lpnt.Append(Other.lpnt.Value(i));
    }
    N = Other.lseg.Length();
    for (i = 1; i <= N; i++)
    {
      lseg.Append(Other.lseg.Value(i));
    }
    done = Standard_True;
  }
  else
  {
    done = Standard_False;
  }
}

//================================================================================
void CurveSurfaceIntersection1::Append(const CurveSurfaceIntersection1& Other,
                                          //					  const Standard_Real a,
                                          const Standard_Real,
                                          //					  const Standard_Real b)
                                          const Standard_Real)
{
  Standard_Integer i, ni;
  if (Other.done)
  {
    ni = Other.lpnt.Length();
    for (i = 1; i <= ni; i++)
    {
      Append(Other.Point(i));
    }
    ni = Other.lseg.Length();
    for (i = 1; i <= ni; i++)
    {
      Append(Other.Segment1(i));
    }
  }
}

//================================================================================
void CurveSurfaceIntersection1::Append(const IntersectionPoint1& OtherPoint)
{
  Standard_Integer                  i, ni;
  Standard_Real                     anu, anv, anw, u, v, w;
  IntCurveSurface_TransitionOnCurve TrOnCurve, anTrOnCurve;
  Point3d                            P, anP;
  ni = lpnt.Length();
  for (i = 1; i <= ni; i++)
  {
    OtherPoint.Values(P, u, v, w, TrOnCurve);
    lpnt(i).Values(anP, anu, anv, anw, anTrOnCurve);
    if (PARAMEQUAL(u, anu))
    {
      if (PARAMEQUAL(v, anv))
      {
        if (PARAMEQUAL(w, anw))
        {
          if (anTrOnCurve == TrOnCurve)
          {
            return;
          }
        }
      }
    }
  }
  lpnt.Append(OtherPoint);
}

//================================================================================
void CurveSurfaceIntersection1::Append(const IntersectionSegment& OtherSegment)
{
  lseg.Append(OtherSegment);
}

//================================================================================
void CurveSurfaceIntersection1::ResetFields()
{
  if (done)
  {
    lseg.Clear();
    lpnt.Clear();
    done         = Standard_False;
    myIsParallel = Standard_False;
  }
}

//================================================================================
void CurveSurfaceIntersection1::Dump() const
{
  if (done)
  {
    Standard_Integer i, ni;
    ni = lpnt.Length();
    for (i = 1; i <= ni; i++)
    {
      Point(i).Dump();
    }
    ni = lseg.Length();
    for (i = 1; i <= ni; i++)
    {
      Segment1(i).Dump();
    }
  }
  else
  {
    std::cout << " Intersection NotDone" << std::endl;
  }
}
