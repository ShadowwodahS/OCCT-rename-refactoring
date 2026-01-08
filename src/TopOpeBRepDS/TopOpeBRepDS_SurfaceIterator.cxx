// Created on: 1994-06-07
// Created by: Jean Yves LEBEY
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

#include <TopOpeBRepDS_SurfaceIterator.hxx>

//=================================================================================================

SurfaceIterator::SurfaceIterator(const TopOpeBRepDS_ListOfInterference& L)
    : InterferenceIterator(L)
{
  InterferenceIterator::GeometryKind(TopOpeBRepDS_SURFACE);
}

//=================================================================================================

Standard_Integer SurfaceIterator::Current() const
{
  Handle(TopOpeBRepDS_Interference) i = Value();
  Standard_Integer                  g = i->Geometry1();
  return g;
}

//=================================================================================================

TopAbs_Orientation SurfaceIterator::Orientation(const TopAbs_State S) const
{
  Handle(TopOpeBRepDS_Interference) i = Value();
  const StateTransition&    t = i->Transition();
  TopAbs_Orientation                o = t.Orientation(S);
  return o;
}
