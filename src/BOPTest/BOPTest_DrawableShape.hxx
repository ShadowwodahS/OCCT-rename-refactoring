// Created on: 2000-05-25
// Created by: Peter KURNEV
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#ifndef _BOPTest_DrawableShape_HeaderFile
#define _BOPTest_DrawableShape_HeaderFile

#include <Standard.hxx>

#include <DBRep_DrawableShape.hxx>
#include <Standard_Real.hxx>
#include <Standard_Integer.hxx>
class Draw_Text3D;
class TopoShape;
class Point3d;
class DrawDisplay;

class BOPTest_DrawableShape;
DEFINE_STANDARD_HANDLE(BOPTest_DrawableShape, DBRep_DrawableShape)

class BOPTest_DrawableShape : public DBRep_DrawableShape
{

public:
  Standard_EXPORT BOPTest_DrawableShape(const TopoShape&    S,
                                        const DrawColor&      FreeCol,
                                        const DrawColor&      ConnCol,
                                        const DrawColor&      EdgeCol,
                                        const DrawColor&      IsosCol,
                                        const Standard_Real    size,
                                        const Standard_Integer nbisos,
                                        const Standard_Integer discret,
                                        const Standard_CString Text,
                                        const DrawColor&      TextColor);

  Standard_EXPORT BOPTest_DrawableShape(const TopoShape&    S,
                                        const Standard_CString Text,
                                        const DrawColor&      TextColor);

  Standard_EXPORT virtual void DrawOn(DrawDisplay& dis) const Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(BOPTest_DrawableShape, DBRep_DrawableShape)

protected:
private:
  Standard_EXPORT Point3d Pnt() const;

  Handle(Draw_Text3D) myText;
  DrawColor          myTextColor;
};

#endif // _BOPTest_DrawableShape_HeaderFile
