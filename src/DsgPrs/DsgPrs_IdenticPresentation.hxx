// Created on: 1997-01-03
// Created by: Stagiaire Flore Lautheanne
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

#ifndef _DsgPrs_IdenticPresentation_HeaderFile
#define _DsgPrs_IdenticPresentation_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <Prs3d_Drawer.hxx>
#include <Prs3d_Presentation.hxx>

class UtfString;
class Point3d;
class Frame3d;
class gp_Elips;

class DsgPrs_IdenticPresentation
{
public:
  DEFINE_STANDARD_ALLOC

  //! draws a line between <aPntAttach> and
  //! <aPntOffset>.
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const UtfString& aText,
                                  const Point3d&                     aPntAttach,
                                  const Point3d&                     aPntOffset);

  //! draws the 'identic' presentation by
  //! drawing a line between <aFAttach> and
  //! <aSAttach> , and a linkimg segment
  //! between <aPntOffset> and its projection
  //! on the precedent line.
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const UtfString& aText,
                                  const Point3d&                     aFAttach,
                                  const Point3d&                     aSAttach,
                                  const Point3d&                     aPntOffset);

  //! draws the 'identic' presentation in the case of
  //! circles : draws an arc of circle between
  //! <aFAttach> and <aSAttach> of center <aCenter>
  //! and of radius dist(aCenter, aFAttach), and
  //! draws a segment between <aPntOffset> and
  //! its projection on the arc.
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const UtfString& aText,
                                  const Frame3d&                     aAx2,
                                  const Point3d&                     aCenter,
                                  const Point3d&                     aFAttach,
                                  const Point3d&                     aSAttach,
                                  const Point3d&                     aPntOffset);

  //! draws the 'identic' presentation in the case of
  //! circles : draws an arc of circle between
  //! <aFAttach> and <aSAttach> of center <aCenter>
  //! and of radius dist(aCenter, aFAttach), and
  //! draws a segment between <aPntOffset> and <aPntOnCirc>
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const UtfString& aText,
                                  const Frame3d&                     aAx2,
                                  const Point3d&                     aCenter,
                                  const Point3d&                     aFAttach,
                                  const Point3d&                     aSAttach,
                                  const Point3d&                     aPntOffset,
                                  const Point3d&                     aPntOnCirc);

  //! draws the 'identic' presentation in the case of
  //! ellipses: draws an arc of the anEllipse
  //! between <aFAttach> and <aSAttach> and
  //! draws a segment between <aPntOffset> and <aPntOnElli>
  Standard_EXPORT static void Add(const Handle(Prs3d_Presentation)& aPresentation,
                                  const Handle(StyleDrawer)&       aDrawer,
                                  const UtfString& aText,
                                  const gp_Elips&                   anEllipse,
                                  const Point3d&                     aFAttach,
                                  const Point3d&                     aSAttach,
                                  const Point3d&                     aPntOffset,
                                  const Point3d&                     aPntOnElli);

protected:
private:
};

#endif // _DsgPrs_IdenticPresentation_HeaderFile
