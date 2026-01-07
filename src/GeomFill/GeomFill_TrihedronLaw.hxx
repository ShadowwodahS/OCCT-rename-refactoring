// Created on: 1997-12-02
// Created by: Philippe MANGIN
// Copyright (c) 1997-1999 Matra Datavision
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

#ifndef _GeomFill_TrihedronLaw_HeaderFile
#define _GeomFill_TrihedronLaw_HeaderFile

#include <Adaptor3d_Curve.hxx>
#include <GeomFill_PipeError.hxx>
#include <GeomAbs_Shape.hxx>
#include <TColStd_Array1OfReal.hxx>

class Vector3d;

class GeomFill_TrihedronLaw;
DEFINE_STANDARD_HANDLE(GeomFill_TrihedronLaw, RefObject)

//! To define Trihedron along one Curve
class GeomFill_TrihedronLaw : public RefObject
{

public:
  //! initialize curve of trihedron law
  //! @return Standard_True
  Standard_EXPORT virtual Standard_Boolean SetCurve(const Handle(Adaptor3d_Curve)& C);

  Standard_EXPORT virtual Handle(GeomFill_TrihedronLaw) Copy() const = 0;

  //! Give a status to the Law1
  //! Returns PipeOk (default implementation)
  Standard_EXPORT virtual GeomFill_PipeError ErrorStatus() const;

  //! compute Triedrhon on curve at parameter <Param>
  Standard_EXPORT virtual Standard_Boolean D0(const Standard_Real Param,
                                              Vector3d&             Tangent,
                                              Vector3d&             Normal,
                                              Vector3d&             BiNormal) = 0;

  //! compute Triedrhon and  derivative Trihedron  on curve
  //! at parameter <Param>
  //! Warning : It used only for C1 or C2 approximation
  Standard_EXPORT virtual Standard_Boolean D1(const Standard_Real Param,
                                              Vector3d&             Tangent,
                                              Vector3d&             DTangent,
                                              Vector3d&             Normal,
                                              Vector3d&             DNormal,
                                              Vector3d&             BiNormal,
                                              Vector3d&             DBiNormal);

  //! compute  Trihedron on curve
  //! first and seconde  derivatives.
  //! Warning : It used only for C2 approximation
  Standard_EXPORT virtual Standard_Boolean D2(const Standard_Real Param,
                                              Vector3d&             Tangent,
                                              Vector3d&             DTangent,
                                              Vector3d&             D2Tangent,
                                              Vector3d&             Normal,
                                              Vector3d&             DNormal,
                                              Vector3d&             D2Normal,
                                              Vector3d&             BiNormal,
                                              Vector3d&             DBiNormal,
                                              Vector3d&             D2BiNormal);

  //! Returns  the number  of  intervals for  continuity
  //! <S>.
  //! May be one if Continuity(me) >= <S>
  Standard_EXPORT virtual Standard_Integer NbIntervals(const GeomAbs_Shape S) const = 0;

  //! Stores in <T> the  parameters bounding the intervals
  //! of continuity <S>.
  //!
  //! The array must provide  enough room to  accommodate
  //! for the parameters. i.e. T.Length() > NbIntervals()
  Standard_EXPORT virtual void Intervals(TColStd_Array1OfReal& T, const GeomAbs_Shape S) const = 0;

  //! Sets the bounds of the parametric interval on
  //! the function
  //! This determines the derivatives in these values if the
  //! function is not Cn.
  Standard_EXPORT virtual void SetInterval(const Standard_Real First, const Standard_Real Last);

  //! Gets the bounds of the parametric interval on
  //! the function
  Standard_EXPORT void GetInterval(Standard_Real& First, Standard_Real& Last);

  //! Get average value of M(t) and V(t) it is usfull to
  //! make fast approximation of rational  surfaces.
  Standard_EXPORT virtual void GetAverageLaw(Vector3d& ATangent,
                                             Vector3d& ANormal,
                                             Vector3d& ABiNormal) = 0;

  //! Say if the law is Constant
  Standard_EXPORT virtual Standard_Boolean IsConstant() const;

  //! Say if the law is defined, only by the 3d Geometry of
  //! the set Curve
  //! Return False by Default.
  Standard_EXPORT virtual Standard_Boolean IsOnlyBy3dCurve() const;

  DEFINE_STANDARD_RTTIEXT(GeomFill_TrihedronLaw, RefObject)

protected:
  Handle(Adaptor3d_Curve) myCurve;
  Handle(Adaptor3d_Curve) myTrimmed;

private:
};

#endif // _GeomFill_TrihedronLaw_HeaderFile
