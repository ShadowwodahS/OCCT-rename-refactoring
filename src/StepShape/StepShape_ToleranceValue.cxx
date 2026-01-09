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

#include <StepBasic_MeasureWithUnit.hxx>
#include <StepShape_ToleranceValue.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ToleranceValue1, RefObject)

ToleranceValue1::ToleranceValue1() {}

void ToleranceValue1::Init(const Handle(RefObject)& lower_bound,
                                    const Handle(RefObject)& upper_bound)
{
  theLowerBound = lower_bound;
  theUpperBound = upper_bound;
}

Handle(RefObject) ToleranceValue1::LowerBound() const
{
  return theLowerBound;
}

void ToleranceValue1::SetLowerBound(const Handle(RefObject)& lower_bound)
{
  theLowerBound = lower_bound;
}

Handle(RefObject) ToleranceValue1::UpperBound() const
{
  return theUpperBound;
}

void ToleranceValue1::SetUpperBound(const Handle(RefObject)& upper_bound)
{
  theUpperBound = upper_bound;
}
