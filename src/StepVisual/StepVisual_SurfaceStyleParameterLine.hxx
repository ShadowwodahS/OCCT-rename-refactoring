// Created on: 1995-12-01
// Created by: EXPRESS->CDL V0.2 Translator
// Copyright (c) 1995-1999 Matra Datavision
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

#ifndef _StepVisual_SurfaceStyleParameterLine_HeaderFile
#define _StepVisual_SurfaceStyleParameterLine_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <StepVisual_HArray1OfDirectionCountSelect.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Integer.hxx>
class StepVisual_CurveStyle;
class DirectionCountSelect;

class StepVisual_SurfaceStyleParameterLine;
DEFINE_STANDARD_HANDLE(StepVisual_SurfaceStyleParameterLine, RefObject)

class StepVisual_SurfaceStyleParameterLine : public RefObject
{

public:
  //! Returns a SurfaceStyleParameterLine
  Standard_EXPORT StepVisual_SurfaceStyleParameterLine();

  Standard_EXPORT void Init(
    const Handle(StepVisual_CurveStyle)&                    aStyleOfParameterLines,
    const Handle(HArray1OfDirectionCount)& aDirectionCounts);

  Standard_EXPORT void SetStyleOfParameterLines(
    const Handle(StepVisual_CurveStyle)& aStyleOfParameterLines);

  Standard_EXPORT Handle(StepVisual_CurveStyle) StyleOfParameterLines() const;

  Standard_EXPORT void SetDirectionCounts(
    const Handle(HArray1OfDirectionCount)& aDirectionCounts);

  Standard_EXPORT Handle(HArray1OfDirectionCount) DirectionCounts() const;

  Standard_EXPORT DirectionCountSelect
    DirectionCountsValue(const Standard_Integer num) const;

  Standard_EXPORT Standard_Integer NbDirectionCounts() const;

  DEFINE_STANDARD_RTTIEXT(StepVisual_SurfaceStyleParameterLine, RefObject)

protected:
private:
  Handle(StepVisual_CurveStyle)                    styleOfParameterLines;
  Handle(HArray1OfDirectionCount) directionCounts;
};

#endif // _StepVisual_SurfaceStyleParameterLine_HeaderFile
