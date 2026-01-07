// Created on: 1993-08-10
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

#ifndef _BRep_PointOnCurveOnSurface_HeaderFile
#define _BRep_PointOnCurveOnSurface_HeaderFile

#include <Standard.hxx>

#include <BRep_PointsOnSurface.hxx>
#include <Standard_Real.hxx>
class GeomCurve2d;
class GeomSurface;
class TopLoc_Location;

class BRep_PointOnCurveOnSurface;
DEFINE_STANDARD_HANDLE(BRep_PointOnCurveOnSurface, BRep_PointsOnSurface)

//! Representation by   a parameter on  a curve   on a
//! surface.
class BRep_PointOnCurveOnSurface : public BRep_PointsOnSurface
{

public:
  Standard_EXPORT BRep_PointOnCurveOnSurface(const Standard_Real         P,
                                             const Handle(GeomCurve2d)& C,
                                             const Handle(GeomSurface)& S,
                                             const TopLoc_Location&      L);

  //! Returns True
  Standard_EXPORT virtual Standard_Boolean IsPointOnCurveOnSurface() const Standard_OVERRIDE;

  Standard_EXPORT virtual Standard_Boolean IsPointOnCurveOnSurface(const Handle(GeomCurve2d)& PC,
                                                                   const Handle(GeomSurface)& S,
                                                                   const TopLoc_Location& L) const
    Standard_OVERRIDE;

  Standard_EXPORT virtual const Handle(GeomCurve2d)& PCurve() const Standard_OVERRIDE;

  Standard_EXPORT virtual void PCurve(const Handle(GeomCurve2d)& C) Standard_OVERRIDE;

  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson(Standard_OStream& theOStream,
                                        Standard_Integer  theDepth = -1) const Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(BRep_PointOnCurveOnSurface, BRep_PointsOnSurface)

protected:
private:
  Handle(GeomCurve2d) myPCurve;
};

#endif // _BRep_PointOnCurveOnSurface_HeaderFile
