// Copyright (c) 2021 OPEN CASCADE SAS
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

#include <XmlMXCAFDoc_LengthUnitDriver.hxx>

#include <Message_Messenger.hxx>
#include <Standard_Type.hxx>
#include <XCAFDoc_LengthUnit.hxx>
#include <XmlObjMgt.hxx>
#include <XmlObjMgt_Persistent.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XmlMXCAFDoc_LengthUnitDriver, XmlMDF_ADriver)
IMPLEMENT_DOMSTRING(UnitScaleValue, "value")

//=================================================================================================

XmlMXCAFDoc_LengthUnitDriver::XmlMXCAFDoc_LengthUnitDriver(
  const Handle(Message_Messenger)& theMsgDriver)
    : XmlMDF_ADriver(theMsgDriver, "xcaf", "LengthUnit")
{
}

//=================================================================================================

Handle(TDF_Attribute) XmlMXCAFDoc_LengthUnitDriver::NewEmpty() const
{
  return (new XCAFDoc_LengthUnit());
}

//=======================================================================
// function : Paste
// purpose  : persistent -> transient (retrieve)
//=======================================================================
Standard_Boolean XmlMXCAFDoc_LengthUnitDriver::Paste(const PersistentStorage&  theSource,
                                                     const Handle(TDF_Attribute)& theTarget,
                                                     XmlObjMgt_RRelocationTable&) const
{
  XmlObjMgt_DOMString aNameStr = XmlObjMgt1::GetStringValue(theSource);

  if (aNameStr == NULL)
  {
    UtfString aMessageString =
      UtfString("Cannot retrieve LengthUnit attribute");
    myMessageDriver->Send(aMessageString, Message_Fail);
    return Standard_False;
  }
  const XmlObjMgt_Element& anElement       = theSource;
  XmlObjMgt_DOMString      aUnitScaleValue = anElement.getAttribute(::UnitScaleValue());
  if (aUnitScaleValue == NULL)
  {
    UtfString aMessageString("Cannot retrieve LengthUnit scale factor");
    myMessageDriver->Send(aMessageString, Message_Fail);
    return Standard_False;
  }
  AsciiString1 aScaleFactor(aUnitScaleValue.GetString());
  AsciiString1 anUnitName(aNameStr.GetString());
  if (!aScaleFactor.IsRealValue(true))
  {
    UtfString aMessageString("Cannot retrieve LengthUnit scale factor");
    myMessageDriver->Send(aMessageString, Message_Fail);
    return Standard_False;
  }

  Handle(XCAFDoc_LengthUnit) anInt = Handle(XCAFDoc_LengthUnit)::DownCast(theTarget);
  anInt->Set(anUnitName, aScaleFactor.RealValue());
  return Standard_True;
}

//=======================================================================
// function : Paste
// purpose  : transient -> persistent (store)
//=======================================================================
void XmlMXCAFDoc_LengthUnitDriver::Paste(const Handle(TDF_Attribute)& theSource,
                                         PersistentStorage&        theTarget,
                                         XmlObjMgt_SRelocationTable&) const
{
  Handle(XCAFDoc_LengthUnit) anAtt     = Handle(XCAFDoc_LengthUnit)::DownCast(theSource);
  XmlObjMgt_DOMString        aNameUnit = anAtt->GetUnitName().ToCString();
  XmlObjMgt_DOMString aValueUnit       = AsciiString1(anAtt->GetUnitValue()).ToCString();
  XmlObjMgt1::SetStringValue(theTarget, aNameUnit);
  theTarget.Element().setAttribute(::UnitScaleValue(), aValueUnit);
}
