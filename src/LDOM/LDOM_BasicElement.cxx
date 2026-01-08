// Created on: 2001-06-26
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

// AGV 140202: Replace(const char *) for (LDOMBasicString1)=>myTagName

#include <LDOM_BasicElement.hxx>
#include <LDOM_BasicAttribute.hxx>
#include <LDOM_BasicText.hxx>
#include <LDOM_MemManager.hxx>
#include <LDOM_NodeList.hxx>

//=======================================================================
// function : Create
// purpose  : construction in the Document's data pool
//=======================================================================

BasicElement& BasicElement::Create(const char*                    aName,
                                             const Standard_Integer         aLen,
                                             const Handle(MemoryManager)& aDoc)
{
  if (aName == NULL)
  {
    static BasicElement aVoidElement;
    aVoidElement = BasicElement();
    return aVoidElement;
  }
  void*              aMem     = aDoc->Allocate(sizeof(BasicElement));
  BasicElement* aNewElem = new (aMem) BasicElement;

  Standard_Integer aHash;
  //  aDoc -> HashedAllocate (aString, strlen(aString), aNewElem -> myTagName);
  aNewElem->myTagName = aDoc->HashedAllocate(aName, aLen, aHash);

  aNewElem->myNodeType = LDOM_Node::ELEMENT_NODE;
  return *aNewElem;
}

//=================================================================================================

void BasicElement::RemoveNodes()
{
  const BasicNode* aNode = (const BasicNode*)myFirstChild;
  while (aNode)
  {
    const BasicNode* aNext = aNode->GetSibling();
    switch (aNode->getNodeType())
    {
      case LDOM_Node::ELEMENT_NODE: {
        BasicElement& anElement = *(BasicElement*)aNode;
        anElement                    = NULL;
        break;
      }
      case LDOM_Node::ATTRIBUTE_NODE: {
        BasicAttribute& anAttr = *(BasicAttribute*)aNode;
        anAttr                      = NULL;
        break;
      }
      case LDOM_Node::TEXT_NODE:
      case LDOM_Node::COMMENT_NODE:
      case LDOM_Node::CDATA_SECTION_NODE: {
        BasicText& aTxt = *(BasicText*)aNode;
        aTxt                 = NULL;
        break;
      }
      default:;
    }
    aNode = aNext;
  }
  myFirstChild = NULL;
}

//=======================================================================
// function : operator =
// purpose  : Nullify
//=======================================================================

BasicElement& BasicElement::operator=(const LDOM_NullPtr* aNull)
{
  myTagName = NULL;
  RemoveNodes();
  BasicNode::operator=(aNull);
  return *this;
}

//=================================================================================================

/*
BasicElement::BasicElement (const LDOM_Element& anElement)
     : BasicNode   (LDOM_Node::ELEMENT_NODE),
       myAttributeMask  (0),
       myFirstChild     (NULL)
{
//  LDOMString aNewTagName (anElement.getTagName(), anElement.myDocument);
//  myTagName = aNewTagName;
  const BasicElement& anOther =
    (const BasicElement&) anElement.Origin();
  myTagName = anOther.GetTagName();
}
*/
//=================================================================================================

BasicElement::~BasicElement()
{
  myTagName = NULL;
  RemoveNodes();
}

//=================================================================================================

const BasicNode* BasicElement::GetLastChild() const
{
  const BasicNode* aNode = myFirstChild;
  if (aNode)
  {
    if (aNode->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
      aNode = NULL;
    else
      while (aNode->mySibling)
      {
        if (aNode->mySibling->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
          break;
        aNode = aNode->mySibling;
      }
  }
  return aNode;
}

//=================================================================================================

const BasicAttribute& BasicElement::GetAttribute(const LDOMBasicString1& aName,
                                                           const BasicNode*  aLastCh) const
{
  const BasicNode* aNode;
  if (aLastCh)
    aNode = aLastCh->GetSibling();
  else
    aNode = myFirstChild;
  const char* aNameStr = aName.GetString();
  while (aNode)
  {
    if (aNode->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
    {
      const BasicAttribute* anAttr = (const BasicAttribute*)aNode;
      if (!strcmp(aNameStr, anAttr->GetName()))
        return *anAttr;
    }
    aNode = aNode->mySibling;
  }
  static const BasicAttribute aNullAttribute;
  return aNullAttribute;
}

//=======================================================================
// function : GetFirstAttribute
// purpose  : private method
//=======================================================================

const BasicAttribute* BasicElement::GetFirstAttribute(
  const BasicNode*&  theLastCh,
  const BasicNode**& thePrevNode) const
{
  //  Find the First Attribute as well as the Last Child among siblings
  const BasicNode*  aFirstAttr;
  const BasicNode** aPrevNode;
  if (theLastCh)
  {
    aFirstAttr = theLastCh->mySibling;
    aPrevNode  = (const BasicNode**)&(theLastCh->mySibling);
    while (aFirstAttr)
    {
      if (aFirstAttr->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
        break;
      aPrevNode  = (const BasicNode**)&(aFirstAttr->mySibling);
      aFirstAttr = aFirstAttr->mySibling;
    }
  }
  else
  {
    aFirstAttr = myFirstChild;
    aPrevNode  = (const BasicNode**)&myFirstChild;
    while (aFirstAttr)
    {
      if (aFirstAttr->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
        break;
      if (aFirstAttr->isNull() == Standard_False)
        theLastCh = aFirstAttr;
      aPrevNode  = (const BasicNode**)&(aFirstAttr->mySibling);
      aFirstAttr = aFirstAttr->mySibling;
    }
  }
  thePrevNode = aPrevNode;
  return (BasicAttribute*)aFirstAttr;
}

//=======================================================================
// function : AddAttribute
// purpose  : Add or replace an attribute
//=======================================================================

const BasicNode* BasicElement::AddAttribute(const LDOMBasicString1&         anAttrName,
                                                      const LDOMBasicString1&         anAttrValue,
                                                      const Handle(MemoryManager)& aDocument,
                                                      const BasicNode*          aLastCh)
{
  //  Create attribute
  Standard_Integer     aHash;
  BasicAttribute& anAttr = BasicAttribute::Create(anAttrName, aDocument, aHash);
  anAttr.myValue              = anAttrValue;

  //  Initialize the loop of attribute name search
  const BasicNode**     aPrNode;
  const BasicAttribute* aFirstAttr = GetFirstAttribute(aLastCh, aPrNode);
  const char*                aNameStr   = anAttrName.GetString();

  //  Check attribute hash value against the current mask
  const unsigned int  anAttrMaskValue = aHash & (8 * sizeof(myAttributeMask) - 1);
  const unsigned long anAttributeMask = (1 << anAttrMaskValue);
#ifdef OCCT_DEBUG_MASK
  anAttributeMask = 0xffffffff;
#endif
  if ((myAttributeMask & anAttributeMask) == 0)
  {
    // this is new attribute, OK
    myAttributeMask |= anAttributeMask;
    *aPrNode = &anAttr;
    anAttr.SetSibling(aFirstAttr);
  }
  else
  {
    // this attribute may have already been installed
    BasicAttribute* aCurrentAttr = (BasicAttribute*)aFirstAttr;
    while (aCurrentAttr)
    {
      if (aCurrentAttr->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
        if (MemoryManager::CompareStrings(aNameStr, aHash, aCurrentAttr->GetName()))
        {
          aCurrentAttr->SetValue(anAttrValue, aDocument);
          break;
        }
      aCurrentAttr = (BasicAttribute*)aCurrentAttr->mySibling;
    }
    if (aCurrentAttr == NULL)
    {
      // this is new attribute, OK
      *aPrNode = &anAttr;
      anAttr.SetSibling(aFirstAttr);
    }
  }
  return aLastCh;
}

//=======================================================================
// function : RemoveAttribute
// purpose  : Find and delete an attribute from list
//=======================================================================

const BasicNode* BasicElement::RemoveAttribute(const LDOMBasicString1& aName,
                                                         const BasicNode*  aLastCh) const
{
  //  Check attribute hash value against the current mask
  const char* const      aNameStr = aName.GetString();
  const Standard_Integer aHash =
    MemoryManager::Hash(aNameStr, (Standard_Integer)strlen(aNameStr));
  const unsigned int  anAttrMaskValue = aHash & (8 * sizeof(myAttributeMask) - 1);
  const unsigned long anAttributeMask = (1 << anAttrMaskValue);
#ifdef OCCT_DEBUG_MASK
  anAttributeMask = 0xffffffff;
#endif
  if ((myAttributeMask & anAttributeMask) == 0)
  {
    ; // maybe cause for exception
  }
  else
  {
    const BasicNode**     aPrevNode; // dummy
    const BasicAttribute* anAttr = GetFirstAttribute(aLastCh, aPrevNode);
    while (anAttr)
    {
      if (anAttr->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
        if (MemoryManager::CompareStrings(aNameStr, aHash, anAttr->GetName()))
        {
          anAttr = NULL;
          break;
        }
      anAttr = (const BasicAttribute*)anAttr->mySibling;
    }
  }
  return aLastCh;
}

//=================================================================================================

void BasicElement::RemoveChild(const BasicNode* aChild) const
{
  const BasicNode*  aNode     = myFirstChild;
  const BasicNode** aPrevNode = (const BasicNode**)&myFirstChild;
  while (aNode)
  {
    if (aNode->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
      break;
    if (aNode == aChild)
    {
      *aPrevNode               = aNode->GetSibling();
      *(BasicNode*)aChild = NULL;
      break;
    }
    aPrevNode = (const BasicNode**)&(aNode->mySibling);
    aNode     = aNode->GetSibling();
  }
  // here may be the cause to throw an exception
}

//=================================================================================================

void BasicElement::AppendChild(const BasicNode*  aChild,
                                    const BasicNode*& aLastChild) const
{
  if (aLastChild)
  {
    (const BasicNode*&)aChild->mySibling     = aLastChild->mySibling;
    (const BasicNode*&)aLastChild->mySibling = aChild;
  }
  else
  {
    const BasicNode*  aNode     = myFirstChild;
    const BasicNode** aPrevNode = (const BasicNode**)&myFirstChild;
    while (aNode)
    {
      if (aNode->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
      {
        (const BasicNode*&)aChild->mySibling = aNode;
        break;
      }
      aPrevNode = (const BasicNode**)&(aNode->mySibling);
      aNode     = aNode->mySibling;
    }
    *aPrevNode = aChild;
  }
  aLastChild = aChild;
}

//=======================================================================
// function : AddElementsByTagName
// purpose  : Add to the List all sub-elements with the given name (recursive)
//=======================================================================

void BasicElement::AddElementsByTagName(LDOM_NodeList&         aList,
                                             const LDOMBasicString1& aTagName) const
{
  const BasicNode* aNode      = myFirstChild;
  const char*           aTagString = aTagName.GetString();
  while (aNode)
  {
    if (aNode->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
      break;
    if (aNode->getNodeType() == LDOM_Node::ELEMENT_NODE)
    {
      BasicElement& anElement = *(BasicElement*)aNode;
      //      if (anElement.GetTagName().equals(aTagName))
      if (strcmp(anElement.GetTagName(), aTagString) == 0)
        aList.Append(anElement);
      anElement.AddElementsByTagName(aList, aTagName);
    }
    aNode = aNode->GetSibling();
  }
}

//=================================================================================================

void BasicElement::AddAttributes(LDOM_NodeList& aList, const BasicNode* aLastChild) const
{
  const BasicNode* aBNode;
  if (aLastChild)
    aBNode = aLastChild->GetSibling();
  else
    aBNode = GetFirstChild();
  while (aBNode)
  {
    if (aBNode->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
      aList.Append(*aBNode);
    aBNode = aBNode->GetSibling();
  }
}

//=======================================================================
// function : ReplaceElement
// purpose  : Copy data and children into this node from another one
//           The only preserved data is mySibling
//=======================================================================

void BasicElement::ReplaceElement(const BasicElement&       anOtherElem,
                                       const Handle(MemoryManager)& aDocument)
{
  myTagName                        = anOtherElem.GetTagName();
  myAttributeMask                  = anOtherElem.myAttributeMask;
  myFirstChild                     = NULL;
  const BasicNode* aBNode     = anOtherElem.GetFirstChild();
  const BasicNode* aLastChild = NULL;

  // Loop on children (non-attributes)
  for (; aBNode != NULL; aBNode = aBNode->GetSibling())
  {
    if (aBNode->isNull())
      continue;
    BasicNode*           aNewBNode;
    const LDOM_Node::NodeType aNewNodeType = aBNode->getNodeType();
    switch (aNewNodeType)
    {
      case LDOM_Node::ELEMENT_NODE: {
        const BasicElement& aBNodeElem = *(const BasicElement*)aBNode;
        const char*              aTagString = aBNodeElem.GetTagName();
        BasicElement&       aNewBNodeElem =
          BasicElement::Create(aTagString, (Standard_Integer)strlen(aTagString), aDocument);
        aNewBNodeElem.ReplaceElement(aBNodeElem, aDocument); // reccur
        aNewBNode = &aNewBNodeElem;
        break;
      }
      case LDOM_Node::ATTRIBUTE_NODE:
        goto loop_attr;
      case LDOM_Node::TEXT_NODE:
      case LDOM_Node::COMMENT_NODE:
      case LDOM_Node::CDATA_SECTION_NODE: {
        const BasicText& aBNodeText = *(const BasicText*)aBNode;
        aNewBNode                        = &BasicText::Create(aNewNodeType,
                                            LDOMString(aBNodeText.GetData(), aDocument),
                                            aDocument);
        break;
      }
      default:
        continue;
    }
    if (GetFirstChild())
      (const BasicNode*&)aLastChild->mySibling = aNewBNode;
    else
      (const BasicNode*&)myFirstChild = aNewBNode;
    (const BasicNode*&)aLastChild = aNewBNode;
  }

  // Loop on attributes (in the end of the list of children)
loop_attr:
  BasicNode* aLastAttr = (BasicNode*)aLastChild;
  for (; aBNode != NULL; aBNode = aBNode->GetSibling())
  {
    Standard_Integer aHash;
    if (aBNode->isNull())
      continue;
    const BasicAttribute* aBNodeAtt = (const BasicAttribute*)aBNode;
    BasicAttribute*       aNewAtt =
      &BasicAttribute::Create(aBNodeAtt->GetName(), aDocument, aHash);
    aNewAtt->SetValue(aBNodeAtt->myValue, aDocument);
    if (aLastAttr)
      aLastAttr->SetSibling(aNewAtt);
    else
      myFirstChild = aNewAtt;
    aLastAttr = aNewAtt;
  }
}
