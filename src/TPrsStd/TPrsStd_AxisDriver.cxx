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

#include <AIS_Axis.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <Geom_Line.hxx>
#include <gp_Lin.hxx>
#include <Standard_Type.hxx>
#include <TDataXtd_Axis.hxx>
#include <TDataXtd_Geometry.hxx>
#include <TDF_Label.hxx>
#include <TNaming_Tool.hxx>
#include <TopoDS_Shape.hxx>
#include <TPrsStd_AxisDriver.hxx>

IMPLEMENT_STANDARD_RTTIEXT(TPrsStd_AxisDriver, TPrsStd_Driver)

//=================================================================================================

TPrsStd_AxisDriver::TPrsStd_AxisDriver() {}

//=================================================================================================

Standard_Boolean TPrsStd_AxisDriver::Update(const DataLabel&               aLabel,
                                            Handle(VisualEntity)& anAISObject)
{

  Handle(TDataXtd_Axis) apAxis;
  if (!aLabel.FindAttribute(TDataXtd_Axis::GetID(), apAxis))
  {
    return Standard_False;
  }

  gp_Lin                     lin;
  Handle(ShapeAttribute) NS;
  if (aLabel.FindAttribute(ShapeAttribute::GetID(), NS))
  {
    if (Tool11::GetShape(NS).IsNull())
    {
      return Standard_False;
    }
  }

  Handle(VisualAxis) aistrihed;
  if (TDataXtd_Geometry::Line(aLabel, lin))
  {
    Handle(GeomLine) apt = new GeomLine(lin);

    //  Update de l'AIS1
    if (anAISObject.IsNull())
      aistrihed = new VisualAxis(apt);
    else
    {
      aistrihed = Handle(VisualAxis)::DownCast(anAISObject);
      if (aistrihed.IsNull())
        aistrihed = new VisualAxis(apt);
      else
      {
        aistrihed->SetComponent(apt);
        aistrihed->ResetTransformation();
        aistrihed->SetToUpdate();
        aistrihed->UpdateSelection();
      }
    }
    anAISObject = aistrihed;
    return Standard_True;
  }
  return Standard_False;
}
