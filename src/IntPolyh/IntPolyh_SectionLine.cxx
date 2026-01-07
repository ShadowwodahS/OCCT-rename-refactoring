// Created on: 1999-04-06
// Created by: Fabrice SERVANT
// Copyright (c) 1999-1999 Matra Datavision
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

// modified by Edward AGAPOV (eap) Thu Feb 14 2002 (occ139)
// Add Prepend(), replace array with sequence

#include <IntPolyh_SectionLine.hxx>
#include <IntPolyh_StartPoint.hxx>
#include <IntPolyh_Triangle.hxx>

#include <stdio.h>

//=================================================================================================

SectionLine::SectionLine() /*: n(0),nbstartpoints(0),ptr(0)*/ {}

//=================================================================================================

SectionLine::SectionLine(const Standard_Integer N) /* : nbstartpoints(0)*/
{
  Init(N);
}

//=================================================================================================

void SectionLine::Init(const Standard_Integer /*N*/)
{
  //   ptr = (void*) (new StartPoint [N]);
  //   n=N;
  if (!mySeqOfSPoints.Length())
    IncrementNbStartPoints();
}

//=================================================================================================

Standard_Integer SectionLine::GetN() const
{
  // return(n);
  return mySeqOfSPoints.Length();
}

//=================================================================================================

Standard_Integer SectionLine::NbStartPoints() const
{
  //  return(nbstartpoints);
  return mySeqOfSPoints.Length() - 1;
}

//=================================================================================================

void SectionLine::IncrementNbStartPoints()
{
  //  nbstartpoints++;
  StartPoint aSP;
  mySeqOfSPoints.Append(aSP);
}

//=================================================================================================

const StartPoint& SectionLine::Value(const Standard_Integer Index) const
{
  return mySeqOfSPoints(Index + 1);
}

//=================================================================================================

StartPoint& SectionLine::ChangeValue(const Standard_Integer Index)
{
  return mySeqOfSPoints(Index + 1);
}

//=================================================================================================

void SectionLine::Destroy()
{
  //   if(n) {
  //     if(ptr) {
  //       StartPoint *ptrstpoint = (StartPoint *)ptr;
  //       delete [] ptrstpoint;
  //       ptr=0;
  //       n=0;
  //     }
  //   }
}

//=================================================================================================

SectionLine& SectionLine::Copy(const SectionLine& Other)
{
  //   if(ptr==Other.ptr) return(*this);
  //   Destroy();
  //   n=Other.n;
  //   ptr = (void *) (new StartPoint[n]);
  //   for(Standard_Integer i=0;i<=n;i++) {
  //     (*this)[i]=Other[i];
  //   }
  mySeqOfSPoints = Other.mySeqOfSPoints;
  return (*this);
}

//=================================================================================================

void SectionLine::Dump() const
{
  printf("\n SectionLine 0-> %d", /*nbstartpoints*/ NbStartPoints() - 1);
  for (Standard_Integer i = 0; i < NbStartPoints(); i++)
  {
    //(*this)[i].Dump(i);
    Value(i).Dump(i);
    //     const StartPoint& SP = Value(i);
    //     std::cout << "point P" << i << " " << SP.X() << " " << SP.Y() << " " << SP.Z() <<
    //     std::endl;
  }
  printf("\n");
}

//=================================================================================================

void SectionLine::Prepend(const StartPoint& SP)
{
  mySeqOfSPoints.Prepend(SP);
}
