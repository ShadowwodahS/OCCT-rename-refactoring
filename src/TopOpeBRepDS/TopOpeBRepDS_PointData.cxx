// Created on: 1993-06-23
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

#include <TopOpeBRepDS_define.hxx>
#include <TopOpeBRepDS_Point.hxx>
#include <TopOpeBRepDS_PointData.hxx>

//=================================================================================================

PointData::PointData()
    : myS1(0),
      myS2(0)
{
}

//=================================================================================================

PointData::PointData(const Point1& P)
    : myPoint(P),
      myS1(0),
      myS2(0)
{
}

//=================================================================================================

PointData::PointData(const Point1& P,
                                               const Standard_Integer    I1,
                                               const Standard_Integer    I2)
    : myPoint(P),
      myS1(I1),
      myS2(I2)
{
}

//=================================================================================================

void PointData::SetShapes(const Standard_Integer I1, const Standard_Integer I2)
{
  myS1 = I1;
  myS2 = I2;
}

//=================================================================================================

void PointData::GetShapes(Standard_Integer& I1, Standard_Integer& I2) const
{
  I1 = myS1;
  I2 = myS2;
}
