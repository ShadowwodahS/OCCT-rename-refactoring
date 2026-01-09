// Created on: 1993-03-10
// Created by: JCV
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

#include <Geom_Transformation.hxx>

#include <Standard_Dump.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Transformation1, RefObject)

Transformation1::Transformation1() {}

Transformation1::Transformation1(const Transform3d& T)
    : gpTrsf(T)
{
}

Handle(Transformation1) Transformation1::Copy() const
{

  Handle(Transformation1) T;
  T = new Transformation1(gpTrsf);
  return T;
}

Handle(Transformation1) Transformation1::Inverted() const
{

  return new Transformation1(gpTrsf.Inverted());
}

Handle(Transformation1) Transformation1::Multiplied(
  const Handle(Transformation1)& Other) const
{

  return new Transformation1(gpTrsf.Multiplied(Other->Trsf()));
}

Handle(Transformation1) Transformation1::Powered(const Standard_Integer N) const
{

  Transform3d T = gpTrsf;
  T.Power(N);
  return new Transformation1(T);
}

void Transformation1::PreMultiply(const Handle(Transformation1)& Other)
{

  gpTrsf.PreMultiply(Other->Trsf());
}

void Transformation1::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &gpTrsf)
}
