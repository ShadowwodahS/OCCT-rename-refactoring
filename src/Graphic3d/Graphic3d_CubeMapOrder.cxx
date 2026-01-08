// Author: Ilya Khramov
// Copyright (c) 2019 OPEN CASCADE SAS
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

#include <Graphic3d_CubeMapOrder.hxx>

#include <Standard_Failure.hxx>

#include <bitset>

//=================================================================================================

CubeMapOrder::CubeMapOrder()
    : myConvolution(0),
      myHasOverflows(false)
{
}

//=================================================================================================

CubeMapOrder::CubeMapOrder(unsigned char thePosXLocation,
                                               unsigned char theNegXLocation,
                                               unsigned char thePosYLocation,
                                               unsigned char theNegYLocation,
                                               unsigned char thePosZLocation,
                                               unsigned char theNegZLocation)
    : myConvolution(0),
      myHasOverflows(false)
{
  Set(Graphic3d_CMS_POS_X, thePosXLocation);
  Set(Graphic3d_CMS_NEG_X, theNegXLocation);
  Set(Graphic3d_CMS_POS_Y, thePosYLocation);
  Set(Graphic3d_CMS_NEG_Y, theNegYLocation);
  Set(Graphic3d_CMS_POS_Z, thePosZLocation);
  Set(Graphic3d_CMS_NEG_Z, theNegZLocation);
}

//=================================================================================================

CubeMapOrder::CubeMapOrder(const Graphic3d_ValidatedCubeMapOrder& theOrder)
    : myConvolution(theOrder.Order.myConvolution),
      myHasOverflows(theOrder.Order.myHasOverflows)
{
}

//=================================================================================================

CubeMapOrder& CubeMapOrder::Set(const CubeMapOrder& theOrder)
{
  myConvolution  = theOrder.myConvolution;
  myHasOverflows = theOrder.myHasOverflows;
  return *this;
}

// =======================================================================
// function : operator=
// purpose  :
// =======================================================================
Graphic3d_ValidatedCubeMapOrder CubeMapOrder::Validated() const
{
  if (!IsValid())
  {
    throw ExceptionBase(
      "Try of Graphic3d_ValidatedCubeMapOrder creation using invalid CubeMapOrder");
  }

  return *this;
}

//=================================================================================================

CubeMapOrder& CubeMapOrder::Set(Graphic3d_CubeMapSide theCubeMapSide,
                                                    unsigned char         theValue)
{
  if (theValue > 5)
  {
    myHasOverflows = true;
    return *this;
  }
  set(theCubeMapSide, theValue);
  return *this;
}

//=================================================================================================

unsigned char CubeMapOrder::Get(Graphic3d_CubeMapSide theCubeMapSide) const
{
  return get(static_cast<unsigned char>(theCubeMapSide));
}

// =======================================================================
// function : operator[]
// purpose  :
// =======================================================================
unsigned char CubeMapOrder::operator[](Graphic3d_CubeMapSide theCubeMapSide) const
{
  return Get(theCubeMapSide);
}

//=================================================================================================

CubeMapOrder& CubeMapOrder::SetDefault()
{
  for (unsigned char i = 0; i < 6; ++i)
  {
    set(Graphic3d_CubeMapSide(i), i);
  }
  return *this;
}

//=================================================================================================

CubeMapOrder& CubeMapOrder::Permute(
  const Graphic3d_ValidatedCubeMapOrder& thePermutation)
{
  for (unsigned char i = 0; i < 6; ++i)
  {
    set(i, thePermutation->get(get(i)));
  }

  return *this;
}

//=================================================================================================

CubeMapOrder CubeMapOrder::Permuted(
  const Graphic3d_ValidatedCubeMapOrder& thePermutation) const
{
  CubeMapOrder anOrder = *this;
  anOrder.Permute(thePermutation);
  return anOrder;
}

//=================================================================================================

CubeMapOrder& CubeMapOrder::Swap(Graphic3d_CubeMapSide theFirstSide,
                                                     Graphic3d_CubeMapSide theSecondSide)
{
  unsigned char aTmp = Get(theFirstSide);
  set(theFirstSide, Get(theSecondSide));
  set(theSecondSide, aTmp);
  return *this;
}

//=================================================================================================

CubeMapOrder CubeMapOrder::Swapped(Graphic3d_CubeMapSide theFirstSide,
                                                       Graphic3d_CubeMapSide theSecondSide) const
{
  CubeMapOrder anOrder = *this;
  anOrder.Swap(theFirstSide, theSecondSide);
  return anOrder;
}

//=================================================================================================

CubeMapOrder& CubeMapOrder::Clear()
{
  myConvolution  = 0;
  myHasOverflows = false;
  return *this;
}

//=================================================================================================

bool CubeMapOrder::IsEmpty() const
{
  return myConvolution == 0;
}

//=================================================================================================

bool CubeMapOrder::HasRepetitions() const
{
  std::bitset<6> aBitSet;
  for (unsigned char i = 0; i < 6; ++i)
  {
    std::bitset<6>::reference aFlag = aBitSet[get(i)];
    if (aFlag)
    {
      return true;
    }
    aFlag = true;
  }
  return false;
}

//=================================================================================================

bool CubeMapOrder::HasOverflows() const
{
  return myHasOverflows;
}

//=================================================================================================

bool CubeMapOrder::IsValid() const
{
  return !HasRepetitions() && !HasOverflows();
}

//=================================================================================================

unsigned char CubeMapOrder::get(unsigned char theCubeMapSide) const
{
  return (myConvolution / (1 << (theCubeMapSide * 3))) % (1 << 3);
}

//=================================================================================================

void CubeMapOrder::set(unsigned char theCubeMapSide, unsigned char theValue)
{
  unsigned int aValuePlace = 1 << (theCubeMapSide * 3);
  myConvolution -= aValuePlace * get(theCubeMapSide);
  myConvolution += aValuePlace * theValue;
}

//=================================================================================================

void CubeMapOrder::set(Graphic3d_CubeMapSide theCubeMapSide, unsigned char theValue)
{
  set(static_cast<unsigned char>(theCubeMapSide), theValue);
}

//=================================================================================================

const Graphic3d_ValidatedCubeMapOrder& CubeMapOrder::Default()
{
  static const Graphic3d_ValidatedCubeMapOrder aCubeMapOrder =
    CubeMapOrder().SetDefault().Validated();
  return aCubeMapOrder;
}