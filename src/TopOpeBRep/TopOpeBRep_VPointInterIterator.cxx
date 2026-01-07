// Created on: 1993-11-16
// Created by: Jean Yves LEBEY
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

#include <TopOpeBRep_LineInter.hxx>
#include <TopOpeBRep_VPointInter.hxx>
#include <TopOpeBRep_VPointInterIterator.hxx>

//=================================================================================================

VPointIntersectionIterator::VPointIntersectionIterator()
    : myLineInter(NULL),
      myVPointIndex(0),
      myVPointNb(0),
      mycheckkeep(Standard_False)
{
}

//=================================================================================================

VPointIntersectionIterator::VPointIntersectionIterator(const TopOpeBRep_LineInter& LI)
{
  Init(LI);
}

//=================================================================================================

void VPointIntersectionIterator::Init(const TopOpeBRep_LineInter& LI,
                                          const Standard_Boolean      checkkeep)
{
  myLineInter = (TopOpeBRep_LineInter*)&LI;
  mycheckkeep = checkkeep;
  Init();
}

//=================================================================================================

void VPointIntersectionIterator::Init()
{
  myVPointIndex = 1;
  myVPointNb    = myLineInter->NbVPoint();
  if (mycheckkeep)
  {
    while (More())
    {
      const TopOpeBRep_VPointInter& VP = CurrentVP();
      if (VP.Keep())
        break;
      else
        myVPointIndex++;
    }
  }
}

//=================================================================================================

Standard_Boolean VPointIntersectionIterator::More() const
{
  return (myVPointIndex <= myVPointNb);
}

//=================================================================================================

void VPointIntersectionIterator::Next()
{
  myVPointIndex++;
  if (mycheckkeep)
  {
    while (More())
    {
      const TopOpeBRep_VPointInter& VP = CurrentVP();
      if (VP.Keep())
        break;
      else
        myVPointIndex++;
    }
  }
}

//=================================================================================================

const TopOpeBRep_VPointInter& VPointIntersectionIterator::CurrentVP()
{
  if (!More())
    throw Standard_ProgramError("VPointIntersectionIterator::CurrentVP");
  const TopOpeBRep_VPointInter& VP = myLineInter->VPoint(myVPointIndex);
  return VP;
}

//=================================================================================================

TopOpeBRep_VPointInter& VPointIntersectionIterator::ChangeCurrentVP()
{
  if (!More())
    throw Standard_ProgramError("VPointIntersectionIterator::ChangeCurrentVP");
  TopOpeBRep_VPointInter& VP = myLineInter->ChangeVPoint(myVPointIndex);
  return VP;
}

//=================================================================================================

Standard_Integer VPointIntersectionIterator::CurrentVPIndex() const
{
  if (!More())
    throw Standard_ProgramError("VPointIntersectionIterator::CurrentVPIndex");
  return myVPointIndex;
}

TopOpeBRep_PLineInter VPointIntersectionIterator::PLineInterDummy() const
{
  return myLineInter;
}
