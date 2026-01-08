// Created on: 2001-06-27
// Created by: Alexander GRIGORIEV
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

#include <LDOM_BasicAttribute.hxx>
#include <LDOM_BasicElement.hxx>
#include <LDOM_BasicText.hxx>

//=================================================================================================

const BasicNode& LDOM_Node::Origin() const
{
  if (myOrigin == NULL)
  {
    static BasicNode aNullNode;
    return aNullNode;
  }
  return *myOrigin;
}

//=================================================================================================

const MemoryManager& LDOM_Node::getOwnerDocument() const
{
  return myDocument->Self();
}

//=======================================================================
// function : operator =
// purpose  : Assignment
//=======================================================================

LDOM_Node& LDOM_Node::operator=(const LDOM_Node& theOther)
{
  myDocument  = theOther.myDocument;
  myOrigin    = theOther.myOrigin;
  myLastChild = theOther.myLastChild;
  return *this;
}

//=======================================================================
// function : operator =
// purpose  : Nullify
//=======================================================================

LDOM_Node& LDOM_Node::operator=(const LDOM_NullPtr* /*aNull*/)
{
  myDocument.Nullify();
  myOrigin    = NULL;
  myLastChild = NULL;
  return *this;
}

//=================================================================================================

Standard_Boolean LDOM_Node::isNull() const
{
  return myOrigin == NULL || myOrigin->isNull();
}

//=======================================================================
// function : operator ==
// purpose  : Compare two Nodes
//=======================================================================

Standard_Boolean LDOM_Node::operator==(const LDOM_Node& anOther) const
{
  if (isNull())
    return anOther.isNull();
  return myOrigin == anOther.myOrigin;
}

//=======================================================================
// function : operator !=
// purpose  : Compare two Nodes
//=======================================================================

Standard_Boolean LDOM_Node::operator!=(const LDOM_Node& anOther) const
{
  if (isNull())
    return !anOther.isNull();
  return myOrigin != anOther.myOrigin;
}

//=================================================================================================

LDOM_Node::NodeType LDOM_Node::getNodeType() const
{
  return myOrigin == NULL ? UNKNOWN : myOrigin->getNodeType();
}

//=================================================================================================

LDOMString LDOM_Node::getNodeName() const
{
  switch (getNodeType())
  {
    case ELEMENT_NODE: {
      const BasicElement& anElement = *(const BasicElement*)myOrigin;
      return LDOMString::CreateDirectString(anElement.GetTagName(), myDocument->Self());
    }
    case ATTRIBUTE_NODE: {
      const BasicAttribute& anAttr = *(const BasicAttribute*)myOrigin;
      return LDOMString::CreateDirectString(anAttr.GetName(), myDocument->Self());
    }
    default:;
  }
  return LDOMString();
}

//=================================================================================================

LDOMString LDOM_Node::getNodeValue() const
{
  switch (getNodeType())
  {
    case ATTRIBUTE_NODE: {
      const BasicAttribute& anAttr = *(const BasicAttribute*)myOrigin;
      return LDOMString(anAttr.GetValue(), myDocument->Self());
    }
    case TEXT_NODE:
    case CDATA_SECTION_NODE:
    case COMMENT_NODE: {
      const BasicText& aText = *(const BasicText*)myOrigin;
      return LDOMString(aText.GetData(), myDocument->Self());
    }
    default:;
  }
  return LDOMString();
}

//=================================================================================================

LDOM_Node LDOM_Node::getFirstChild() const
{
  const NodeType aType = getNodeType();
  if (aType == ELEMENT_NODE)
  {
    const BasicElement& anElement = *(const BasicElement*)myOrigin;
    const BasicNode*    aChild    = anElement.GetFirstChild();
    if (aChild)
      if (aChild->getNodeType() != LDOM_Node::ATTRIBUTE_NODE)
        return LDOM_Node(*aChild, myDocument);
  }
  return LDOM_Node();
}

//=================================================================================================

LDOM_Node LDOM_Node::getLastChild() const
{
  const NodeType aType = getNodeType();
  if (aType == ELEMENT_NODE)
  {
    if (myLastChild == NULL)
    {
      const BasicElement& anElement  = *(const BasicElement*)myOrigin;
      (const BasicNode*&)myLastChild = anElement.GetLastChild();
    }
    return LDOM_Node(*myLastChild, myDocument);
  }
  return LDOM_Node();
}

//=================================================================================================

LDOM_Node LDOM_Node::getNextSibling() const
{
  const BasicNode* aSibling = myOrigin->mySibling;
  if (aSibling)
    if (aSibling->getNodeType() != ATTRIBUTE_NODE)
      return LDOM_Node(*aSibling, myDocument);
  return LDOM_Node();
}

//=================================================================================================

void LDOM_Node::removeChild(const LDOM_Node& aChild)
{
  const NodeType aType = getNodeType();
  if (aType == ELEMENT_NODE)
  {
    const BasicElement& anElement = *(BasicElement*)myOrigin;
    if (aChild != NULL)
      anElement.RemoveChild(aChild.myOrigin);
    if (aChild.myOrigin == myLastChild)
      //      myLastChild = anElement.GetLastChild();
      myLastChild = NULL;
  }
}

//=================================================================================================

void LDOM_Node::appendChild(const LDOM_Node& aChild)
{
  const NodeType aType = getNodeType();
  if (aType == ELEMENT_NODE && aChild != NULL)
  {
    if (myLastChild)
    {
      aChild.myOrigin->SetSibling(myLastChild->mySibling);
      (const BasicNode*&)myLastChild->mySibling = aChild.myOrigin;
    }
    else
    {
      const BasicElement& anElement = *(BasicElement*)myOrigin;
      anElement.AppendChild(aChild.myOrigin, myLastChild);
    }
    myLastChild = aChild.myOrigin;
  }
}

//=================================================================================================

Standard_Boolean LDOM_Node::hasChildNodes() const
{
  const NodeType aType = getNodeType();
  if (aType == ELEMENT_NODE)
  {
    const BasicElement& anElement = *(const BasicElement*)myOrigin;
    const BasicNode*    aChild    = anElement.GetFirstChild();
    if (aChild)
      return !aChild->isNull();
  }
  return Standard_False;
}

//=================================================================================================

void LDOM_Node::SetValueClear() const
{
  LDOMBasicString1* aValue = NULL;
  switch (getNodeType())
  {
    case ATTRIBUTE_NODE: {
      const BasicAttribute& anAttr = *(const BasicAttribute*)myOrigin;
      aValue                            = (LDOMBasicString1*)&anAttr.GetValue();
      break;
    }
    case TEXT_NODE:
    case CDATA_SECTION_NODE:
    case COMMENT_NODE: {
      const BasicText& aText = *(const BasicText*)myOrigin;
      aValue                      = (LDOMBasicString1*)&aText.GetData();
      break;
    }
    default:
      return;
  }
  if (aValue->Type() == LDOMBasicString1::LDOM_AsciiDoc)
    aValue->myType = LDOMBasicString1::LDOM_AsciiDocClear;
}
