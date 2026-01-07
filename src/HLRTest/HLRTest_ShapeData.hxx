// Created on: 1992-08-21
// Created by: Christophe MARION
// Copyright (c) 1992-1999 Matra Datavision
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

#ifndef _HLRTest_ShapeData_HeaderFile
#define _HLRTest_ShapeData_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <Draw_Color.hxx>
#include <Standard_Transient.hxx>

class HLRTest_ShapeData;
DEFINE_STANDARD_HANDLE(HLRTest_ShapeData, RefObject)

//! Contains the colors of a shape.
class HLRTest_ShapeData : public RefObject
{

public:
  Standard_EXPORT HLRTest_ShapeData(const DrawColor& CVis,
                                    const DrawColor& COVis,
                                    const DrawColor& CIVis,
                                    const DrawColor& CHid,
                                    const DrawColor& COHid,
                                    const DrawColor& CIHid);

  void VisibleColor(const DrawColor& CVis);

  void VisibleOutLineColor(const DrawColor& COVis);

  void VisibleIsoColor(const DrawColor& CIVis);

  void HiddenColor(const DrawColor& CHid);

  void HiddenOutLineColor(const DrawColor& COHid);

  void HiddenIsoColor(const DrawColor& CIHid);

  DrawColor VisibleColor() const;

  DrawColor VisibleOutLineColor() const;

  DrawColor VisibleIsoColor() const;

  DrawColor HiddenColor() const;

  DrawColor HiddenOutLineColor() const;

  DrawColor HiddenIsoColor() const;

  DEFINE_STANDARD_RTTIEXT(HLRTest_ShapeData, RefObject)

protected:
private:
  DrawColor myVColor;
  DrawColor myVOColor;
  DrawColor myVIColor;
  DrawColor myHColor;
  DrawColor myHOColor;
  DrawColor myHIColor;
};

#include <HLRTest_ShapeData.lxx>

#endif // _HLRTest_ShapeData_HeaderFile
