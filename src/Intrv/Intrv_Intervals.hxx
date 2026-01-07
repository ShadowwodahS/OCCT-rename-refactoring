// Created on: 1991-12-13
// Created by: Christophe MARION
// Copyright (c) 1991-1999 Matra Datavision
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

#ifndef _Intrv_Intervals_HeaderFile
#define _Intrv_Intervals_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Intrv_SequenceOfInterval.hxx>
class Interval2;

//! The class  Intervals is a  sorted  sequence of non
//! overlapping  Real Intervals.
class Intervals
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates a void sequence of intervals.
  Standard_EXPORT Intervals();

  //! Creates a sequence of one interval.
  Standard_EXPORT Intervals(const Interval2& Int);

  //! Intersects the intervals with the interval <Tool>.
  Standard_EXPORT void Intersect(const Interval2& Tool);

  //! Intersects the intervals with the intervals in the
  //! sequence  <Tool>.
  Standard_EXPORT void Intersect(const Intervals& Tool);

  Standard_EXPORT void Subtract(const Interval2& Tool);

  Standard_EXPORT void Subtract(const Intervals& Tool);

  Standard_EXPORT void Unite(const Interval2& Tool);

  Standard_EXPORT void Unite(const Intervals& Tool);

  Standard_EXPORT void XUnite(const Interval2& Tool);

  Standard_EXPORT void XUnite(const Intervals& Tool);

  Standard_Integer NbIntervals() const;

  const Interval2& Value(const Standard_Integer Index) const;

protected:
private:
  Intrv_SequenceOfInterval myInter;
};

#include <Intrv_Intervals.lxx>

#endif // _Intrv_Intervals_HeaderFile
