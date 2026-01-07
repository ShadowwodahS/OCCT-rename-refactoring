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

#include "RWStepBasic_RWSiUnit.pxx"
#include <StepBasic_DimensionalExponents.hxx>
#include <StepBasic_SiUnit.hxx>
#include <StepData_StepReaderData.hxx>
#include <StepData_StepWriter.hxx>
#include <TCollection_AsciiString.hxx>

// --- Enum : SiPrefix ---
static AsciiString1 spExa(".EXA.");
static AsciiString1 spPico(".PICO.");
static AsciiString1 spMega(".MEGA.");
static AsciiString1 spFemto(".FEMTO.");
static AsciiString1 spAtto(".ATTO.");
static AsciiString1 spCenti(".CENTI.");
static AsciiString1 spNano(".NANO.");
static AsciiString1 spHecto(".HECTO.");
static AsciiString1 spMicro(".MICRO.");
static AsciiString1 spTera(".TERA.");
static AsciiString1 spGiga(".GIGA.");
static AsciiString1 spMilli(".MILLI.");
static AsciiString1 spPeta(".PETA.");
static AsciiString1 spDeci(".DECI.");
static AsciiString1 spKilo(".KILO.");
static AsciiString1 spDeca(".DECA.");

// --- Enum : SiUnitName ---
static AsciiString1 sunHertz(".HERTZ.");
static AsciiString1 sunDegreeCelsius(".DEGREE_CELSIUS.");
static AsciiString1 sunSiemens(".SIEMENS.");
static AsciiString1 sunSievert(".SIEVERT.");
static AsciiString1 sunLux(".LUX.");
static AsciiString1 sunWatt(".WATT.");
static AsciiString1 sunOhm(".OHM.");
static AsciiString1 sunSecond(".SECOND.");
static AsciiString1 sunBecquerel(".BECQUEREL.");
static AsciiString1 sunPascal(".PASCAL.");
static AsciiString1 sunHenry(".HENRY.");
static AsciiString1 sunTesla(".TESLA.");
static AsciiString1 sunVolt(".VOLT.");
static AsciiString1 sunJoule(".JOULE.");
static AsciiString1 sunKelvin(".KELVIN.");
static AsciiString1 sunAmpere(".AMPERE.");
static AsciiString1 sunGram(".GRAM.");
static AsciiString1 sunSteradian(".STERADIAN.");
static AsciiString1 sunMole(".MOLE.");
static AsciiString1 sunLumen(".LUMEN.");
static AsciiString1 sunGray(".GRAY.");
static AsciiString1 sunCandela(".CANDELA.");
static AsciiString1 sunFarad(".FARAD.");
static AsciiString1 sunRadian(".RADIAN.");
static AsciiString1 sunNewton(".NEWTON.");
static AsciiString1 sunMetre(".METRE.");
static AsciiString1 sunWeber(".WEBER.");
static AsciiString1 sunCoulomb(".COULOMB.");

RWStepBasic_RWSiUnit::RWStepBasic_RWSiUnit() {}

void RWStepBasic_RWSiUnit::ReadStep(const Handle(StepData_StepReaderData)& data,
                                    const Standard_Integer                 num,
                                    Handle(Interface_Check)&               ach,
                                    const Handle(StepBasic_SiUnit)&        ent) const
{
  // --- Number of Parameter Control ---
  if (!data->CheckNbParams(num, 3, ach, "si_unit"))
    return;

  // --- inherited field : dimensions ---
  // --- this field is redefined ---
  // szv#4:S4163:12Mar99 `Standard_Boolean stat1 =` not needed
  data->CheckDerived(num, 1, "dimensions", ach, Standard_False);

  // --- own field : prefix ---
  StepBasic_SiPrefix aPrefix    = StepBasic_spExa;
  Standard_Boolean   hasAprefix = Standard_False;
  if (data->IsParamDefined(num, 2))
  {
    if (data->ParamType(num, 2) == Interface_ParamEnum)
    {
      Standard_CString text = data->ParamCValue(num, 2);
      hasAprefix            = DecodePrefix(aPrefix, text);
      if (!hasAprefix)
        ach->AddFail("Enumeration si_prefix has not an allowed value");
    }
    else
      ach->AddFail("Parameter #2 (prefix) is not an enumeration");
  }

  // --- own field : name ---
  StepBasic_SiUnitName aName = StepBasic_sunMetre;
  if (data->ParamType(num, 3) == Interface_ParamEnum)
  {
    Standard_CString text = data->ParamCValue(num, 3);
    if (!DecodeName(aName, text))
      ach->AddFail("Enumeration si_unit_name has not an allowed value");
  }
  else
    ach->AddFail("Parameter #3 (name) is not an enumeration");

  //--- Initialisation of the read entity ---
  ent->Init(hasAprefix, aPrefix, aName);
}

void RWStepBasic_RWSiUnit::WriteStep(StepData_StepWriter&            SW,
                                     const Handle(StepBasic_SiUnit)& ent) const
{

  // --- inherited field dimensions ---
  SW.SendDerived();

  // --- own field : prefix ---
  Standard_Boolean hasAprefix = ent->HasPrefix();
  if (hasAprefix)
    SW.SendEnum(EncodePrefix(ent->Prefix()));
  else
    SW.SendUndef();

  // --- own field : name ---
  SW.SendEnum(EncodeName(ent->Name()));
}

Standard_Boolean RWStepBasic_RWSiUnit::DecodePrefix(StepBasic_SiPrefix&    aPrefix,
                                                    const Standard_CString text) const
{
  if (spExa.IsEqual(text))
    aPrefix = StepBasic_spExa;
  else if (spPico.IsEqual(text))
    aPrefix = StepBasic_spPico;
  else if (spMega.IsEqual(text))
    aPrefix = StepBasic_spMega;
  else if (spFemto.IsEqual(text))
    aPrefix = StepBasic_spFemto;
  else if (spAtto.IsEqual(text))
    aPrefix = StepBasic_spAtto;
  else if (spCenti.IsEqual(text))
    aPrefix = StepBasic_spCenti;
  else if (spNano.IsEqual(text))
    aPrefix = StepBasic_spNano;
  else if (spHecto.IsEqual(text))
    aPrefix = StepBasic_spHecto;
  else if (spMicro.IsEqual(text))
    aPrefix = StepBasic_spMicro;
  else if (spTera.IsEqual(text))
    aPrefix = StepBasic_spTera;
  else if (spGiga.IsEqual(text))
    aPrefix = StepBasic_spGiga;
  else if (spMilli.IsEqual(text))
    aPrefix = StepBasic_spMilli;
  else if (spPeta.IsEqual(text))
    aPrefix = StepBasic_spPeta;
  else if (spDeci.IsEqual(text))
    aPrefix = StepBasic_spDeci;
  else if (spKilo.IsEqual(text))
    aPrefix = StepBasic_spKilo;
  else if (spDeca.IsEqual(text))
    aPrefix = StepBasic_spDeca;
  else
    return Standard_False;
  return Standard_True;
}

Standard_Boolean RWStepBasic_RWSiUnit::DecodeName(StepBasic_SiUnitName&  aName,
                                                  const Standard_CString text) const
{
  if (sunHertz.IsEqual(text))
    aName = StepBasic_sunHertz;
  else if (sunDegreeCelsius.IsEqual(text))
    aName = StepBasic_sunDegreeCelsius;
  else if (sunSiemens.IsEqual(text))
    aName = StepBasic_sunSiemens;
  else if (sunSievert.IsEqual(text))
    aName = StepBasic_sunSievert;
  else if (sunLux.IsEqual(text))
    aName = StepBasic_sunLux;
  else if (sunWatt.IsEqual(text))
    aName = StepBasic_sunWatt;
  else if (sunOhm.IsEqual(text))
    aName = StepBasic_sunOhm;
  else if (sunSecond.IsEqual(text))
    aName = StepBasic_sunSecond;
  else if (sunBecquerel.IsEqual(text))
    aName = StepBasic_sunBecquerel;
  else if (sunPascal.IsEqual(text))
    aName = StepBasic_sunPascal;
  else if (sunHenry.IsEqual(text))
    aName = StepBasic_sunHenry;
  else if (sunTesla.IsEqual(text))
    aName = StepBasic_sunTesla;
  else if (sunVolt.IsEqual(text))
    aName = StepBasic_sunVolt;
  else if (sunJoule.IsEqual(text))
    aName = StepBasic_sunJoule;
  else if (sunKelvin.IsEqual(text))
    aName = StepBasic_sunKelvin;
  else if (sunAmpere.IsEqual(text))
    aName = StepBasic_sunAmpere;
  else if (sunGram.IsEqual(text))
    aName = StepBasic_sunGram;
  else if (sunSteradian.IsEqual(text))
    aName = StepBasic_sunSteradian;
  else if (sunMole.IsEqual(text))
    aName = StepBasic_sunMole;
  else if (sunLumen.IsEqual(text))
    aName = StepBasic_sunLumen;
  else if (sunGray.IsEqual(text))
    aName = StepBasic_sunGray;
  else if (sunCandela.IsEqual(text))
    aName = StepBasic_sunCandela;
  else if (sunFarad.IsEqual(text))
    aName = StepBasic_sunFarad;
  else if (sunRadian.IsEqual(text))
    aName = StepBasic_sunRadian;
  else if (sunNewton.IsEqual(text))
    aName = StepBasic_sunNewton;
  else if (sunMetre.IsEqual(text))
    aName = StepBasic_sunMetre;
  else if (sunWeber.IsEqual(text))
    aName = StepBasic_sunWeber;
  else if (sunCoulomb.IsEqual(text))
    aName = StepBasic_sunCoulomb;
  else
    return Standard_False;
  return Standard_True;
}

AsciiString1 RWStepBasic_RWSiUnit::EncodePrefix(const StepBasic_SiPrefix aPrefix) const
{
  switch (aPrefix)
  {
    case StepBasic_spExa:
      return spExa;
    case StepBasic_spPico:
      return spPico;
    case StepBasic_spMega:
      return spMega;
    case StepBasic_spFemto:
      return spFemto;
    case StepBasic_spAtto:
      return spAtto;
    case StepBasic_spCenti:
      return spCenti;
    case StepBasic_spNano:
      return spNano;
    case StepBasic_spHecto:
      return spHecto;
    case StepBasic_spMicro:
      return spMicro;
    case StepBasic_spTera:
      return spTera;
    case StepBasic_spGiga:
      return spGiga;
    case StepBasic_spMilli:
      return spMilli;
    case StepBasic_spPeta:
      return spPeta;
    case StepBasic_spDeci:
      return spDeci;
    case StepBasic_spKilo:
      return spKilo;
    case StepBasic_spDeca:
      return spDeca;
  }
  return AsciiString1("");
}

AsciiString1 RWStepBasic_RWSiUnit::EncodeName(const StepBasic_SiUnitName aName) const
{
  switch (aName)
  {
    case StepBasic_sunHertz:
      return sunHertz;
    case StepBasic_sunDegreeCelsius:
      return sunDegreeCelsius;
    case StepBasic_sunSiemens:
      return sunSiemens;
    case StepBasic_sunSievert:
      return sunSievert;
    case StepBasic_sunLux:
      return sunLux;
    case StepBasic_sunWatt:
      return sunWatt;
    case StepBasic_sunOhm:
      return sunOhm;
    case StepBasic_sunSecond:
      return sunSecond;
    case StepBasic_sunBecquerel:
      return sunBecquerel;
    case StepBasic_sunPascal:
      return sunPascal;
    case StepBasic_sunHenry:
      return sunHenry;
    case StepBasic_sunTesla:
      return sunTesla;
    case StepBasic_sunVolt:
      return sunVolt;
    case StepBasic_sunJoule:
      return sunJoule;
    case StepBasic_sunKelvin:
      return sunKelvin;
    case StepBasic_sunAmpere:
      return sunAmpere;
    case StepBasic_sunGram:
      return sunGram;
    case StepBasic_sunSteradian:
      return sunSteradian;
    case StepBasic_sunMole:
      return sunMole;
    case StepBasic_sunLumen:
      return sunLumen;
    case StepBasic_sunGray:
      return sunGray;
    case StepBasic_sunCandela:
      return sunCandela;
    case StepBasic_sunFarad:
      return sunFarad;
    case StepBasic_sunRadian:
      return sunRadian;
    case StepBasic_sunNewton:
      return sunNewton;
    case StepBasic_sunMetre:
      return sunMetre;
    case StepBasic_sunWeber:
      return sunWeber;
    case StepBasic_sunCoulomb:
      return sunCoulomb;
  }
  return AsciiString1("");
}
