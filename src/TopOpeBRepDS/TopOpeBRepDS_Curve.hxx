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

#ifndef _TopOpeBRepDS_Curve_HeaderFile
#define _TopOpeBRepDS_Curve_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopoDS_Shape.hxx>
#include <Standard_Integer.hxx>
class GeomCurve3d;
class TopOpeBRepDS_Interference;
class GeomCurve2d;

//! A Geom curve and a tolerance.
class TopOpeBRepDS_Curve
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TopOpeBRepDS_Curve();

  Standard_EXPORT TopOpeBRepDS_Curve(const Handle(GeomCurve3d)& P,
                                     const Standard_Real       T,
                                     const Standard_Boolean    IsWalk = Standard_False);

  Standard_EXPORT void DefineCurve(const Handle(GeomCurve3d)& P,
                                   const Standard_Real       T,
                                   const Standard_Boolean    IsWalk);

  //! Update the tolerance
  Standard_EXPORT void Tolerance(const Standard_Real tol);

  //! define the interferences face/curve.
  Standard_EXPORT void SetSCI(const Handle(TopOpeBRepDS_Interference)& I1,
                              const Handle(TopOpeBRepDS_Interference)& I2);

  Standard_EXPORT const Handle(TopOpeBRepDS_Interference)& GetSCI1() const;

  Standard_EXPORT const Handle(TopOpeBRepDS_Interference)& GetSCI2() const;

  Standard_EXPORT void GetSCI(Handle(TopOpeBRepDS_Interference)& I1,
                              Handle(TopOpeBRepDS_Interference)& I2) const;

  Standard_EXPORT void SetShapes(const TopoShape& S1, const TopoShape& S2);

  Standard_EXPORT void GetShapes(TopoShape& S1, TopoShape& S2) const;

  Standard_EXPORT const TopoShape& Shape1() const;

  Standard_EXPORT TopoShape& ChangeShape1();

  Standard_EXPORT const TopoShape& Shape2() const;

  Standard_EXPORT TopoShape& ChangeShape2();

  Standard_EXPORT const Handle(GeomCurve3d)& Curve() const;

  Standard_EXPORT void SetRange(const Standard_Real First, const Standard_Real Last);

  Standard_EXPORT Standard_Boolean Range(Standard_Real& First, Standard_Real& Last) const;

  Standard_EXPORT Standard_Real Tolerance() const;

  Standard_EXPORT Handle(GeomCurve3d)& ChangeCurve();

  Standard_EXPORT void Curve(const Handle(GeomCurve3d)& C3D, const Standard_Real Tol);

  Standard_EXPORT const Handle(GeomCurve2d)& Curve1() const;

  Standard_EXPORT void Curve1(const Handle(GeomCurve2d)& PC1);

  Standard_EXPORT const Handle(GeomCurve2d)& Curve2() const;

  Standard_EXPORT void Curve2(const Handle(GeomCurve2d)& PC2);

  Standard_EXPORT Standard_Boolean IsWalk() const;

  Standard_EXPORT void ChangeIsWalk(const Standard_Boolean B);

  Standard_EXPORT Standard_Boolean Keep() const;

  Standard_EXPORT void ChangeKeep(const Standard_Boolean B);

  Standard_EXPORT Standard_Integer Mother() const;

  Standard_EXPORT void ChangeMother(const Standard_Integer I);

  Standard_EXPORT Standard_Integer DSIndex() const;

  Standard_EXPORT void ChangeDSIndex(const Standard_Integer I);

protected:
private:
  Handle(GeomCurve3d)                myCurve;
  Standard_Real                     myFirst;
  Standard_Real                     myLast;
  Standard_Boolean                  myRangeDefined;
  Standard_Real                     myTolerance;
  Standard_Boolean                  myIsWalk;
  TopoShape                      myS1;
  TopoShape                      myS2;
  Handle(TopOpeBRepDS_Interference) mySCI1;
  Handle(TopOpeBRepDS_Interference) mySCI2;
  Standard_Boolean                  myKeep;
  Standard_Integer                  myMother;
  Standard_Integer                  myDSIndex;
};

#endif // _TopOpeBRepDS_Curve_HeaderFile
