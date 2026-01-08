// Created on: 1995-02-07
// Created by: Arnaud BOUZY
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

#ifndef _DsgPrs_AnglePresentation_HeaderFile
#define _DsgPrs_AnglePresentation_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <Prs3d_Drawer.hxx>
#include <Standard_Real.hxx>
#include <DsgPrs_ArrowSide.hxx>
#include <Prs3d_Presentation.hxx>

class UtfString;
class gp_Circ;
class Point3d;
class Dir3d;
class Axis3d;

//! A framework for displaying angles.
class AnglePresentation
{
public:
  DEFINE_STANDARD_ALLOC

  //! Draws the presentation of the full angle of a cone.
  //! VminCircle - a circle at V parameter = Vmin
  //! VmaxCircle - a circle at V parameter = Vmax
  //! aCircle - a circle at V parameter from projection of aPosition to axis of the cone
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const Standard_Real               aVal,
                                  const UtfString& aText,
                                  const gp_Circ&                    aCircle,
                                  const Point3d&                     aPosition,
                                  const Point3d&                     Apex,
                                  const gp_Circ&                    VminCircle,
                                  const gp_Circ&                    VmaxCircle,
                                  const Standard_Real               aArrowSize);

  //! Draws the representation of the angle
  //! defined by dir1 and dir2, centered on
  //! CenterPoint, using the offset point OffsetPoint.
  //! Lines are drawn to points AttachmentPoint1 and AttachmentPoint2
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const Standard_Real               theval,
                                  const Point3d&                     CenterPoint,
                                  const Point3d&                     AttachmentPoint1,
                                  const Point3d&                     AttachmentPoint2,
                                  const Dir3d&                     dir1,
                                  const Dir3d&                     dir2,
                                  const Point3d&                     OffsetPoint);

  //! Same  as above, but <thevalstring> contains conversion
  //! in Session units....
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const Standard_Real               theval,
                                  const UtfString& thevalstring,
                                  const Point3d&                     CenterPoint,
                                  const Point3d&                     AttachmentPoint1,
                                  const Point3d&                     AttachmentPoint2,
                                  const Dir3d&                     dir1,
                                  const Dir3d&                     dir2,
                                  const Point3d&                     OffsetPoint);

  //! Same  as above, may add one  or
  //! two Arrows  according to  <ArrowSide>  value
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const Standard_Real               theval,
                                  const UtfString& thevalstring,
                                  const Point3d&                     CenterPoint,
                                  const Point3d&                     AttachmentPoint1,
                                  const Point3d&                     AttachmentPoint2,
                                  const Dir3d&                     dir1,
                                  const Dir3d&                     dir2,
                                  const Point3d&                     OffsetPoint,
                                  const DsgPrs_ArrowSide            ArrowSide);

  //! Same  as above, but axisdir contains the axis direction
  //! useful for Revol that can be opened with 180 degrees
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const Standard_Real               theval,
                                  const UtfString& thevalstring,
                                  const Point3d&                     CenterPoint,
                                  const Point3d&                     AttachmentPoint1,
                                  const Point3d&                     AttachmentPoint2,
                                  const Dir3d&                     dir1,
                                  const Dir3d&                     dir2,
                                  const Dir3d&                     axisdir,
                                  const Point3d&                     OffsetPoint);

  //! Same  as above,may add one  or
  //! two Arrows  according to  <ArrowSide>  value
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const Standard_Real               theval,
                                  const UtfString& thevalstring,
                                  const Point3d&                     CenterPoint,
                                  const Point3d&                     AttachmentPoint1,
                                  const Point3d&                     AttachmentPoint2,
                                  const Dir3d&                     dir1,
                                  const Dir3d&                     dir2,
                                  const Dir3d&                     axisdir,
                                  const Standard_Boolean            isPlane,
                                  const Axis3d&                     AxisOfSurf,
                                  const Point3d&                     OffsetPoint,
                                  const DsgPrs_ArrowSide            ArrowSide);

  //! simple representation of a poor lonesome angle dimension
  //! Draw1 a line from <theCenter>   to <AttachmentPoint1>, then operates
  //! a rotation around the perpmay add one  or
  //! two Arrows  according to  <ArrowSide>  value.  The
  //! attributes (color,arrowsize,...) are driven by the Drawer.
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const Standard_Real               theval,
                                  const Point3d&                     theCenter,
                                  const Point3d&                     AttachmentPoint1,
                                  const Axis3d&                     theAxe,
                                  const DsgPrs_ArrowSide            ArrowSide);

protected:
private:
};

#endif // _DsgPrs_AnglePresentation_HeaderFile
