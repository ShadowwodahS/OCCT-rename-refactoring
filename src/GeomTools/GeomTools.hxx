// Created on: 1992-08-28
// Created by: Remi LEQUETTE
// Copyright (c) 1992-1999 Matra Datavision
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

#ifndef _GeomTools_HeaderFile
#define _GeomTools_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_OStream.hxx>
#include <Standard_IStream.hxx>
#include <Standard_Real.hxx>
class GeomSurface;
class GeomCurve3d;
class GeomCurve2d;
class UndefinedTypeHandler;

//! The GeomTools1 package provides  utilities for Geometry1.
//!
//! *  SurfaceSet,  CurveSet, Curve2dSet  : Tools used
//! for dumping, writing and reading.
//!
//! * Methods to dump, write, read curves and surfaces.
class GeomTools1
{
public:
  DEFINE_STANDARD_ALLOC

  //! A set of Curves from Geom2d.
  //! Dumps the surface on the stream.
  Standard_EXPORT static void Dump(const Handle(GeomSurface)& S, Standard_OStream& OS);

  //! Writes the surface on the stream.
  Standard_EXPORT static void Write(const Handle(GeomSurface)& S, Standard_OStream& OS);

  //! Reads the surface from the stream.
  Standard_EXPORT static void Read(Handle(GeomSurface)& S, Standard_IStream& IS);

  //! Dumps the Curve on the stream.
  Standard_EXPORT static void Dump(const Handle(GeomCurve3d)& C, Standard_OStream& OS);

  //! Writes the Curve on the stream.
  Standard_EXPORT static void Write(const Handle(GeomCurve3d)& C, Standard_OStream& OS);

  //! Reads the Curve from the stream.
  Standard_EXPORT static void Read(Handle(GeomCurve3d)& C, Standard_IStream& IS);

  //! Dumps the Curve on the stream.
  Standard_EXPORT static void Dump(const Handle(GeomCurve2d)& C, Standard_OStream& OS);

  //! Writes the Curve on the stream.
  Standard_EXPORT static void Write(const Handle(GeomCurve2d)& C, Standard_OStream& OS);

  //! Reads the Curve from the stream.
  Standard_EXPORT static void Read(Handle(GeomCurve2d)& C, Standard_IStream& IS);

  Standard_EXPORT static void SetUndefinedTypeHandler(
    const Handle(UndefinedTypeHandler)& aHandler);

  Standard_EXPORT static Handle(UndefinedTypeHandler) GetUndefinedTypeHandler();

  //! Reads the Standard_Real value from the stream. Zero is read
  //! in case of error
  Standard_EXPORT static void GetReal(Standard_IStream& IS, Standard_Real& theValue);
};

#endif // _GeomTools_HeaderFile
