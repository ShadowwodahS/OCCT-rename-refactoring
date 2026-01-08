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

#include <Standard_Type.hxx>
#include <StepBasic_DimensionalExponents.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DimensionalExponents, RefObject)

DimensionalExponents::DimensionalExponents() {}

void DimensionalExponents::Init(const Standard_Real aLengthExponent,
                                          const Standard_Real aMassExponent,
                                          const Standard_Real aTimeExponent,
                                          const Standard_Real aElectricCurrentExponent,
                                          const Standard_Real aThermodynamicTemperatureExponent,
                                          const Standard_Real aAmountOfSubstanceExponent,
                                          const Standard_Real aLuminousIntensityExponent)
{
  // --- classe own fields ---
  lengthExponent                   = aLengthExponent;
  massExponent                     = aMassExponent;
  timeExponent                     = aTimeExponent;
  electricCurrentExponent          = aElectricCurrentExponent;
  thermodynamicTemperatureExponent = aThermodynamicTemperatureExponent;
  amountOfSubstanceExponent        = aAmountOfSubstanceExponent;
  luminousIntensityExponent        = aLuminousIntensityExponent;
}

void DimensionalExponents::SetLengthExponent(const Standard_Real aLengthExponent)
{
  lengthExponent = aLengthExponent;
}

Standard_Real DimensionalExponents::LengthExponent() const
{
  return lengthExponent;
}

void DimensionalExponents::SetMassExponent(const Standard_Real aMassExponent)
{
  massExponent = aMassExponent;
}

Standard_Real DimensionalExponents::MassExponent() const
{
  return massExponent;
}

void DimensionalExponents::SetTimeExponent(const Standard_Real aTimeExponent)
{
  timeExponent = aTimeExponent;
}

Standard_Real DimensionalExponents::TimeExponent() const
{
  return timeExponent;
}

void DimensionalExponents::SetElectricCurrentExponent(
  const Standard_Real aElectricCurrentExponent)
{
  electricCurrentExponent = aElectricCurrentExponent;
}

Standard_Real DimensionalExponents::ElectricCurrentExponent() const
{
  return electricCurrentExponent;
}

void DimensionalExponents::SetThermodynamicTemperatureExponent(
  const Standard_Real aThermodynamicTemperatureExponent)
{
  thermodynamicTemperatureExponent = aThermodynamicTemperatureExponent;
}

Standard_Real DimensionalExponents::ThermodynamicTemperatureExponent() const
{
  return thermodynamicTemperatureExponent;
}

void DimensionalExponents::SetAmountOfSubstanceExponent(
  const Standard_Real aAmountOfSubstanceExponent)
{
  amountOfSubstanceExponent = aAmountOfSubstanceExponent;
}

Standard_Real DimensionalExponents::AmountOfSubstanceExponent() const
{
  return amountOfSubstanceExponent;
}

void DimensionalExponents::SetLuminousIntensityExponent(
  const Standard_Real aLuminousIntensityExponent)
{
  luminousIntensityExponent = aLuminousIntensityExponent;
}

Standard_Real DimensionalExponents::LuminousIntensityExponent() const
{
  return luminousIntensityExponent;
}
