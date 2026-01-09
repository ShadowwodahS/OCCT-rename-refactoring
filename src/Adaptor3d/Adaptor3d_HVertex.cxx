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

#include <Adaptor3d_HVertex.hxx>

#include <Adaptor2d_Curve2d.hxx>
#include <ElCLib.hxx>
#include <gp_Pnt2d.hxx>
#include <Precision.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(HandleVertex, RefObject)

HandleVertex::HandleVertex()
    : myTol(0.0)
{
}

HandleVertex::HandleVertex(const gp_Pnt2d&          P,
                                     const TopAbs_Orientation Or,
                                     const Standard_Real      Resolution)
    : myPnt(P),
      myTol(Resolution),
      myOri(Or)
{
}

gp_Pnt2d HandleVertex::Value()
{
  return myPnt;
}

Standard_Real HandleVertex::Parameter(const Handle(Adaptor2d_Curve2d)& C)
{
  return ElCLib1::Parameter(C->Line(), myPnt);
}

Standard_Real HandleVertex::Resolution(const Handle(Adaptor2d_Curve2d)&)
{
  return myTol;
}

TopAbs_Orientation HandleVertex::Orientation()
{
  return myOri;
}

Standard_Boolean HandleVertex::IsSame(const Handle(HandleVertex)& Other)
{
  return (myPnt.Distance(Other->Value()) <= Precision1::Confusion());
}
