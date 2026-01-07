// Created on: 2008-03-05
// Created by: Vlad ROMASHKO
// Copyright (c) 2008-2014 OPEN CASCADE SAS
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

#include <Message_Messenger.hxx>
#include <Standard_Type.hxx>
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>
#include <TFunction_Scope.hxx>
#include <XmlMFunction_ScopeDriver.hxx>
#include <XmlObjMgt.hxx>
#include <XmlObjMgt_Document.hxx>
#include <XmlObjMgt_Persistent.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XmlMFunction_ScopeDriver, XmlMDF_ADriver)
IMPLEMENT_DOMSTRING(LastIDIndex, "lastid")
IMPLEMENT_DOMSTRING(LastLabelIndex, "lastlabel")

IMPLEMENT_DOMSTRING(ExtString, "string")

//=================================================================================================

XmlMFunction_ScopeDriver::XmlMFunction_ScopeDriver(const Handle(Message_Messenger)& theMsgDriver)
    : XmlMDF_ADriver(theMsgDriver, NULL)
{
}

//=================================================================================================

Handle(TDF_Attribute) XmlMFunction_ScopeDriver::NewEmpty() const
{
  return (new TFunction_Scope());
}

//=======================================================================
// function : Paste
// purpose  : persistent -> transient (retrieve)
//=======================================================================
Standard_Boolean XmlMFunction_ScopeDriver::Paste(const XmlObjMgt_Persistent&  theSource,
                                                 const Handle(TDF_Attribute)& theTarget,
                                                 XmlObjMgt_RRelocationTable&) const
{
  Handle(TFunction_Scope) S = Handle(TFunction_Scope)::DownCast(theTarget);
  TColStd_ListOfInteger   IDs;
  TDF_LabelList           Labels;

  Standard_Integer         aFirstInd, aLastInd, aValue, ind, nbIDs = 0, nbLabels = 0;
  const XmlObjMgt_Element& anElement = theSource;

  // IDs
  // ===

  // Read the FirstIndex; if the attribute is absent initialize to 1
  aFirstInd = 1;

  // Read the LastIndex; the attribute should present
  if (!anElement.getAttribute(::LastIDIndex()).GetInteger(aLastInd))
  {
    UtfString aMessageString =
      UtfString("Cannot retrieve the last index"
                                 " for Scope attribute");
    myMessageDriver->Send(aMessageString, Message_Fail);
    return Standard_False;
  }
  nbIDs = aLastInd - aFirstInd + 1;

  if (aFirstInd == aLastInd)
  {
    Standard_Integer anInteger;
    if (!XmlObjMgt::GetStringValue(anElement).GetInteger(anInteger))
    {
      UtfString aMessageString =
        UtfString("Cannot retrieve integer member"
                                   " for Scope attribute as \"");
      myMessageDriver->Send(aMessageString, Message_Fail);
      return Standard_False;
    }
    IDs.Append(anInteger);
  }
  else
  {
    Standard_CString aValueStr = Standard_CString(XmlObjMgt::GetStringValue(anElement).GetString());

    for (ind = aFirstInd; ind <= aLastInd; ind++)
    {
      if (!XmlObjMgt::GetInteger(aValueStr, aValue))
      {
        UtfString aMessageString =
          UtfString("Cannot retrieve integer member"
                                     " for Scope attribute as \"")
          + aValueStr + "\"";
        myMessageDriver->Send(aMessageString, Message_Fail);
        return Standard_False;
      }
      IDs.Append(aValue);
    }
  }

  // Labels
  // ======

  aFirstInd = 1;

  // Read the LastIndex; the attribute should present
  if (!anElement.getAttribute(::LastLabelIndex()).GetInteger(aLastInd))
  {
    UtfString aMessageString =
      UtfString("Cannot retrieve the last index"
                                 " for Scope attribute");
    myMessageDriver->Send(aMessageString, Message_Fail);
    return Standard_False;
  }
  nbLabels = aLastInd - aFirstInd + 1;

  if (!anElement.hasChildNodes())
  {
    UtfString aMessageString =
      UtfString("Cannot retrieve an array of labels");
    myMessageDriver->Send(aMessageString, Message_Fail);
    return Standard_False;
  }

  LDOM_Node           aCurNode = anElement.getFirstChild() /*.getNextSibling().getNextSibling()*/;
  LDOM_Element*       aCurElement = (LDOM_Element*)&aCurNode;
  XmlObjMgt_DOMString aValueStr;
  while (*aCurElement != anElement.getLastChild())
  {
    aValueStr = XmlObjMgt::GetStringValue(*aCurElement);
    if (aValueStr == NULL)
    {
      aCurNode    = aCurElement->getNextSibling();
      aCurElement = (LDOM_Element*)&aCurNode;
      continue;
    }
    AsciiString1 anEntry;
    if (XmlObjMgt::GetTagEntryString(aValueStr, anEntry) == Standard_False)
    {
      UtfString aMessage =
        UtfString("Cannot retrieve reference from \"") + aValueStr + '\"';
      myMessageDriver->Send(aMessage, Message_Fail);
      return Standard_False;
    }
    // Find label by entry
    DataLabel tLab; // Null label.
    if (anEntry.Length() > 0)
    {
      Tool3::Label(S->Label().Data(), anEntry, tLab, Standard_True);
    }
    Labels.Append(tLab);
    aCurNode    = aCurElement->getNextSibling();
    aCurElement = (LDOM_Element*)&aCurNode;
  }

  // Last reference
  aValueStr = XmlObjMgt::GetStringValue(*aCurElement);
  if (aValueStr == NULL)
  {
    myMessageDriver->Send("Cannot retrieve reference string from element", Message_Fail);
    return Standard_False;
  }
  AsciiString1 anEntry;
  if (XmlObjMgt::GetTagEntryString(aValueStr, anEntry) == Standard_False)
  {
    UtfString aMessage =
      UtfString("Cannot retrieve reference from \"") + aValueStr + '\"';
    myMessageDriver->Send(aMessage, Message_Fail);
    return Standard_False;
  }
  // Find label by entry
  DataLabel tLab; // Null label.
  if (anEntry.Length() > 0)
  {
    Tool3::Label(S->Label().Data(), anEntry, tLab, Standard_True);
  }
  Labels.Append(tLab);

  // Check equality of lengths of the list of IDs & Labels.
  if (nbIDs != nbLabels)
  {
    UtfString aMessage =
      UtfString("Numbers of IDs & Labels are different");
    myMessageDriver->Send(aMessage, Message_Fail);
    return Standard_False;
  }

  // Set IDs & Labels into the Scope attribute
  int                                 freeID = 0;
  TColStd_ListIteratorOfListOfInteger itri(IDs);
  TDF_ListIteratorOfLabelList         itrl(Labels);
  for (; itri.More(); itri.Next(), itrl.Next())
  {
    int ID = itri.Value();
    if (ID > freeID)
      freeID = ID;
    S->ChangeFunctions().Bind(ID, itrl.Value());
  }
  freeID++;
  S->SetFreeID(freeID);

  return Standard_True;
}

//=======================================================================
// function : Paste
// purpose  : transient -> persistent (store)
//=======================================================================
void XmlMFunction_ScopeDriver::Paste(const Handle(TDF_Attribute)& theSource,
                                     XmlObjMgt_Persistent&        theTarget,
                                     XmlObjMgt_SRelocationTable&) const
{
  Handle(TFunction_Scope) S = Handle(TFunction_Scope)::DownCast(theSource);

  // IDs
  // ===

  theTarget.Element().setAttribute(::LastIDIndex(), S->GetFunctions().Extent());

  AsciiString1                              aValueStr;
  TFunction_DoubleMapIteratorOfDoubleMapOfIntegerLabel itrd(S->GetFunctions());
  for (; itrd.More(); itrd.Next())
  {
    const Standard_Integer ID = itrd.Key1();
    aValueStr += AsciiString1(ID);
    aValueStr += ' ';
  }
  aValueStr += "\n";

  XmlObjMgt::SetStringValue(theTarget, aValueStr.ToCString(), Standard_True);

  // Labels
  // ======

  XmlObjMgt_Element& anElement = theTarget;
  anElement.setAttribute(::LastLabelIndex(), S->GetFunctions().Extent());

  XmlObjMgt_Document aDoc(anElement.getOwnerDocument());

  for (itrd.Initialize(S->GetFunctions()); itrd.More(); itrd.Next())
  {
    DataLabel L = itrd.Key2();

    AsciiString1 anEntry;
    Tool3::Entry(L, anEntry);

    XmlObjMgt_DOMString aDOMString;
    XmlObjMgt::SetTagEntryString(aDOMString, anEntry);
    XmlObjMgt_Element aCurTarget = aDoc.createElement(::ExtString());
    XmlObjMgt::SetStringValue(aCurTarget, aDOMString, Standard_True);
    anElement.appendChild(aCurTarget);
  }
}
