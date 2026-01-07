// Created on: 1996-09-18
// Created by: Jacques MINOT
// Copyright (c) 1996-1999 Matra Datavision
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

#ifndef _DsgPrs_OffsetPresentation_HeaderFile
#define _DsgPrs_OffsetPresentation_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <Prs3d_Drawer.hxx>
#include <Prs3d_Presentation.hxx>

class UtfString;
class Point3d;
class Dir3d;

//! A framework to define display of offsets.
class DsgPrs_OffsetPresentation
{
public:
  DEFINE_STANDARD_ALLOC

  //! Defines the display of elements showing offset shapes.
  //! These include the two points of attachment
  //! AttachmentPoint1 and AttachmentPoint1, the two
  //! directions aDirection and aDirection2, and the offset point OffsetPoint.
  //! These arguments are added to the presentation
  //! object aPresentation. Their display attributes are
  //! defined by the attribute manager aDrawer.
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const UtfString& aText,
                                  const Point3d&                     AttachmentPoint1,
                                  const Point3d&                     AttachmentPoint2,
                                  const Dir3d&                     aDirection,
                                  const Dir3d&                     aDirection2,
                                  const Point3d&                     OffsetPoint);

  //! draws the representation of axes alignment Constraint
  //! between the point AttachmentPoint1 and the
  //! point AttachmentPoint2, along direction
  //! aDirection, using the offset point OffsetPoint.
  Standard_EXPORT static void AddAxes(const Handle(Prs3d_Presentation)& aPresentation,
                                      const Handle(StyleDrawer)&       aDrawer,
                                      const UtfString& aText,
                                      const Point3d&                     AttachmentPoint1,
                                      const Point3d&                     AttachmentPoint2,
                                      const Dir3d&                     aDirection,
                                      const Dir3d&                     aDirection2,
                                      const Point3d&                     OffsetPoint);

protected:
private:
};

#endif // _DsgPrs_OffsetPresentation_HeaderFile
