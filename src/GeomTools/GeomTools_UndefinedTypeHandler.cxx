// Created on: 1999-10-29
// Created by: Pavel DURANDIN
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

#include <Geom2d_Curve.hxx>
#include <Geom_Surface.hxx>
#include <GeomTools_UndefinedTypeHandler.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(GeomTools_UndefinedTypeHandler, RefObject)

GeomTools_UndefinedTypeHandler::GeomTools_UndefinedTypeHandler() {}

//=================================================================================================

void GeomTools_UndefinedTypeHandler::PrintCurve(const Handle(GeomCurve3d)& /*C*/,
                                                Standard_OStream&      OS,
                                                const Standard_Boolean compact) const
{
  if (!compact)
    OS << "****** UNKNOWN CURVE TYPE ******\n";
  else
    std::cout << "****** UNKNOWN CURVE TYPE ******" << std::endl;
}

//=================================================================================================

Standard_IStream& GeomTools_UndefinedTypeHandler::ReadCurve(const Standard_Integer /*ctype*/,
                                                            Standard_IStream& IS,
                                                            Handle(GeomCurve3d)& /*C*/) const
{
  return IS;
}

//=================================================================================================

void GeomTools_UndefinedTypeHandler::PrintCurve2d(const Handle(GeomCurve2d)& /*C*/,
                                                  Standard_OStream&      OS,
                                                  const Standard_Boolean compact) const
{
  if (!compact)
    OS << "****** UNKNOWN CURVE2d TYPE ******\n";
  else
    std::cout << "****** UNKNOWN CURVE2d TYPE ******" << std::endl;
}

//=================================================================================================

Standard_IStream& GeomTools_UndefinedTypeHandler::ReadCurve2d(const Standard_Integer /*ctype*/,
                                                              Standard_IStream& IS,
                                                              Handle(GeomCurve2d)& /*C*/) const
{
  return IS;
}

//=================================================================================================

void GeomTools_UndefinedTypeHandler::PrintSurface(const Handle(GeomSurface)& /*S*/,
                                                  Standard_OStream&      OS,
                                                  const Standard_Boolean compact) const
{
  if (!compact)
    OS << "****** UNKNOWN SURFACE TYPE ******\n";
  else
    std::cout << "****** UNKNOWN SURFACE TYPE ******" << std::endl;
}

Standard_IStream& GeomTools_UndefinedTypeHandler::ReadSurface(const Standard_Integer /*ctype*/,
                                                              Standard_IStream& IS,
                                                              Handle(GeomSurface)& /*S*/) const
{
  return IS;
}
