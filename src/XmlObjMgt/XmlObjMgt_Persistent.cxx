// Created on: 2001-07-17
// Created by: Julia DOROVSKIKH
// Copyright (c) 2001-2014 OPEN CASCADE SAS
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

// AGV 130202: Changed prototype LDOM_Node::getOwnerDocument()

#include <XmlObjMgt.hxx>
#include <XmlObjMgt_Document.hxx>
#include <XmlObjMgt_Persistent.hxx>

//=======================================================================
// function : PersistentStorage
// purpose  : empty constructor
//=======================================================================
PersistentStorage::PersistentStorage()
    : myID(0)
{
}

//=================================================================================================

PersistentStorage::PersistentStorage(const XmlObjMgt_Element& theElement)
    : myElement(theElement),
      myID(0)
{
  if (theElement != NULL)
    theElement.getAttribute(XmlObjMgt1::IdString()).GetInteger(myID);
}

//=================================================================================================

PersistentStorage::PersistentStorage(const XmlObjMgt_Element&   theElement,
                                           const XmlObjMgt_DOMString& theRef)
    : myID(0)
{
  if (theElement != NULL)
  {
    Standard_Integer aRefID;
    if (theElement.getAttribute(theRef).GetInteger(aRefID))
    {
      myElement = XmlObjMgt1::FindChildElement(theElement, aRefID);
      if (myElement != NULL)
        myElement.getAttribute(XmlObjMgt1::IdString()).GetInteger(myID);
    }
  }
}

//=======================================================================
// function : CreateElement
// purpose  : <theType id="theID"/>
//=======================================================================
void PersistentStorage::CreateElement(XmlObjMgt_Element&         theParent,
                                         const XmlObjMgt_DOMString& theType,
                                         const Standard_Integer     theID)
{
  // AGV  XmlObjMgt_Document& anOwnerDoc =
  // AGV    (XmlObjMgt_Document&)theParent.getOwnerDocument();
  XmlObjMgt_Document anOwnerDoc = XmlObjMgt_Document(theParent.getOwnerDocument());
  myElement                     = anOwnerDoc.createElement(theType);
  theParent.appendChild(myElement);
  SetId(theID);
}

//=================================================================================================

void PersistentStorage::SetId(const Standard_Integer theId)
{
  myID = theId;
  myElement.setAttribute(XmlObjMgt1::IdString(), theId);
}
