// Created on: 1993-01-21
// Created by: Remi LEQUETTE
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

#include <GeomTools.hxx>

#include <Geom2d_Curve.hxx>
#include <Geom_Surface.hxx>
#include <GeomTools_Curve2dSet.hxx>
#include <GeomTools_CurveSet.hxx>
#include <GeomTools_SurfaceSet.hxx>
#include <GeomTools_UndefinedTypeHandler.hxx>

static Handle(GeomTools_UndefinedTypeHandler) theActiveHandler = new GeomTools_UndefinedTypeHandler;

void GeomTools1::Dump(const Handle(GeomSurface)& S, Standard_OStream& OS)
{
  SurfaceSet1::PrintSurface(S, OS);
}

void GeomTools1::Write(const Handle(GeomSurface)& S, Standard_OStream& OS)
{
  SurfaceSet1::PrintSurface(S, OS, Standard_True);
}

void GeomTools1::Read(Handle(GeomSurface)& S, Standard_IStream& IS)
{
  S = SurfaceSet1::ReadSurface(IS);
}

void GeomTools1::Dump(const Handle(GeomCurve3d)& C, Standard_OStream& OS)
{
  CurveSet1::PrintCurve(C, OS);
}

void GeomTools1::Write(const Handle(GeomCurve3d)& C, Standard_OStream& OS)
{
  CurveSet1::PrintCurve(C, OS, Standard_True);
}

void GeomTools1::Read(Handle(GeomCurve3d)& C, Standard_IStream& IS)
{
  C = CurveSet1::ReadCurve(IS);
}

void GeomTools1::Dump(const Handle(GeomCurve2d)& C, Standard_OStream& OS)
{
  Curve2dSet1::PrintCurve2d(C, OS);
}

void GeomTools1::Write(const Handle(GeomCurve2d)& C, Standard_OStream& OS)
{
  Curve2dSet1::PrintCurve2d(C, OS, Standard_True);
}

void GeomTools1::Read(Handle(GeomCurve2d)& C, Standard_IStream& IS)
{
  C = Curve2dSet1::ReadCurve2d(IS);
}

//=================================================================================================

void GeomTools1::SetUndefinedTypeHandler(const Handle(GeomTools_UndefinedTypeHandler)& aHandler)
{
  if (!aHandler.IsNull())
    theActiveHandler = aHandler;
}

//=================================================================================================

Handle(GeomTools_UndefinedTypeHandler) GeomTools1::GetUndefinedTypeHandler()
{
  return theActiveHandler;
}

//=================================================================================================

void GeomTools1::GetReal(Standard_IStream& IS, Standard_Real& theValue)
{
  theValue = 0.;
  if (IS.eof())
  {
    return;
  }
  // According IEEE-754 Specification and standard stream parameters
  // the most optimal buffer length not less then 25
  constexpr size_t THE_BUFFER_SIZE = 32;
  char             aBuffer[THE_BUFFER_SIZE];

  aBuffer[0]                = '\0';
  std::streamsize anOldWide = IS.width(THE_BUFFER_SIZE - 1);
  IS >> aBuffer;
  IS.width(anOldWide);
  theValue = Strtod(aBuffer, nullptr);
}
