// Created on: 1995-12-08
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

#include <GeomFill_CornerState.hxx>

//=================================================================================================

CornerState::CornerState()
    : gap(RealLast()),
      tgtang(0.0),
      isconstrained(0),
      norang(0.0),
      scal(1.),
      coonscnd(1)
{
}

//=================================================================================================

Standard_Real CornerState::Gap() const
{
  return gap;
}

//=================================================================================================

void CornerState::Gap(const Standard_Real G)
{
  gap = G;
}

//=================================================================================================

Standard_Real CornerState::TgtAng() const
{
  return tgtang;
}

//=================================================================================================

void CornerState::TgtAng(const Standard_Real Ang)
{
  tgtang = Ang;
}

//=================================================================================================

Standard_Boolean CornerState::HasConstraint() const
{
  return isconstrained;
}

//=================================================================================================

void CornerState::Constraint()
{
  isconstrained = 1;
}

//=================================================================================================

Standard_Real CornerState::NorAng() const
{
  return norang;
}

//=================================================================================================

void CornerState::NorAng(const Standard_Real Ang)
{
  norang = Ang;
}

//=================================================================================================

Standard_Boolean CornerState::IsToKill(Standard_Real& Scal) const
{
  Scal = scal;
  if (!isconstrained)
    return 0;
  return !coonscnd;
}

//=================================================================================================

void CornerState::DoKill(const Standard_Real Scal)
{
  scal     = Scal;
  coonscnd = 0;
}
