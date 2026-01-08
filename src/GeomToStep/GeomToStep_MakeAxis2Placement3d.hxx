// Created on: 1993-06-14
// Created by: Martine LANGLOIS
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

#ifndef _GeomToStep_MakeAxis2Placement3d_HeaderFile
#define _GeomToStep_MakeAxis2Placement3d_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <GeomToStep_Root.hxx>
#include <StepData_StepModel.hxx>
class StepGeom_Axis2Placement3d;
class Frame3d;
class Ax3;
class Transform3d;
class Geom_Axis2Placement;

//! This class implements the mapping between classes
//! Axis2Placement from Geom and Ax2, Ax3 from gp1, and the class
//! Axis2Placement3d from StepGeom which describes an
//! axis2_placement_3d from Prostep.
class GeomToStep_MakeAxis2Placement3d : public Root1
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT GeomToStep_MakeAxis2Placement3d(
    const ConversionFactors& theLocalFactors = ConversionFactors());

  Standard_EXPORT GeomToStep_MakeAxis2Placement3d(
    const Frame3d&           A,
    const ConversionFactors& theLocalFactors = ConversionFactors());

  Standard_EXPORT GeomToStep_MakeAxis2Placement3d(
    const Ax3&           A,
    const ConversionFactors& theLocalFactors = ConversionFactors());

  Standard_EXPORT GeomToStep_MakeAxis2Placement3d(
    const Transform3d&          T,
    const ConversionFactors& theLocalFactors = ConversionFactors());

  Standard_EXPORT GeomToStep_MakeAxis2Placement3d(
    const Handle(Geom_Axis2Placement)& A,
    const ConversionFactors&            theLocalFactors = ConversionFactors());

  Standard_EXPORT const Handle(StepGeom_Axis2Placement3d)& Value() const;

protected:
private:
  Handle(StepGeom_Axis2Placement3d) theAxis2Placement3d;
};

#endif // _GeomToStep_MakeAxis2Placement3d_HeaderFile
