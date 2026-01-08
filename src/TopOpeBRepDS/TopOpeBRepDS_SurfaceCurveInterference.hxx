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

#ifndef _TopOpeBRepDS_SurfaceCurveInterference_HeaderFile
#define _TopOpeBRepDS_SurfaceCurveInterference_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <TopOpeBRepDS_Interference.hxx>
#include <TopOpeBRepDS_Kind.hxx>
#include <Standard_Integer.hxx>
class GeomCurve2d;
class StateTransition;

class TopOpeBRepDS_SurfaceCurveInterference;
DEFINE_STANDARD_HANDLE(TopOpeBRepDS_SurfaceCurveInterference, TopOpeBRepDS_Interference)

//! an interference with a 2d curve
class TopOpeBRepDS_SurfaceCurveInterference : public TopOpeBRepDS_Interference
{

public:
  Standard_EXPORT TopOpeBRepDS_SurfaceCurveInterference();

  Standard_EXPORT TopOpeBRepDS_SurfaceCurveInterference(const StateTransition& Transition,
                                                        const TopOpeBRepDS_Kind        SupportType,
                                                        const Standard_Integer         Support,
                                                        const TopOpeBRepDS_Kind        GeometryType,
                                                        const Standard_Integer         Geometry1,
                                                        const Handle(GeomCurve2d)&    PC);

  Standard_EXPORT TopOpeBRepDS_SurfaceCurveInterference(const Handle(TopOpeBRepDS_Interference)& I);

  Standard_EXPORT const Handle(GeomCurve2d)& PCurve() const;

  Standard_EXPORT void PCurve(const Handle(GeomCurve2d)& PC);

  DEFINE_STANDARD_RTTIEXT(TopOpeBRepDS_SurfaceCurveInterference, TopOpeBRepDS_Interference)

protected:
private:
  Handle(GeomCurve2d) myPCurve;
};

#endif // _TopOpeBRepDS_SurfaceCurveInterference_HeaderFile
