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

// AGV 140202: Repl.(const char *) for (LDOMBasicString1) => myTagName

#ifndef LDOM_BasicElement_HeaderFile
#define LDOM_BasicElement_HeaderFile

#include <LDOM_BasicNode.hxx>
#include <LDOMBasicString.hxx>
#include <LDOM_Node.hxx>

class LDOM_NodeList;
class BasicAttribute;

//  Class BasicElement
//

class BasicElement : public BasicNode
{
public:
  // ---------- PUBLIC METHODS ----------

  BasicElement()
      : BasicNode(LDOM_Node::UNKNOWN),
        myTagName(NULL),
        myAttributeMask(0),
        myFirstChild(NULL)
  {
  }

  //    Empty constructor

  static BasicElement& Create(const char*                    aName,
                                   const Standard_Integer         aLength,
                                   const Handle(MemoryManager)& aDoc);

  //  Standard_EXPORT BasicElement (const BasicElement& theOther);
  //    Copy constructor

  Standard_EXPORT BasicElement& operator=(const LDOM_NullPtr* aNull);
  //    Nullify

  Standard_EXPORT ~BasicElement();

  //    Destructor

  const char* GetTagName() const { return myTagName; }

  const BasicNode* GetFirstChild() const { return myFirstChild; }

  Standard_EXPORT const BasicNode* GetLastChild() const;

  Standard_EXPORT const BasicAttribute& GetAttribute(const LDOMBasicString1& aName,
                                                          const BasicNode*  aLastCh) const;
  //    Search for attribute name, using or setting myFirstAttribute

protected:
  // ---------- PROTECTED METHODS ----------

  //  BasicElement (const LDOM_Element& anElement);
  //    Constructor

  Standard_EXPORT const BasicNode* AddAttribute(const LDOMBasicString1&         anAttrName,
                                                     const LDOMBasicString1&         anAttrValue,
                                                     const Handle(MemoryManager)& aDoc,
                                                     const BasicNode*          aLastCh);
  //    add or replace an attribute to the element

  Standard_EXPORT const BasicNode* RemoveAttribute(const LDOMBasicString1& aName,
                                                        const BasicNode*  aLastCh) const;

  Standard_EXPORT void RemoveChild(const BasicNode* aChild) const;
  //    remove a child element

  Standard_EXPORT void AppendChild(const BasicNode*  aChild,
                                   const BasicNode*& aLastCh) const;
  //    append a child node to the end of the list

private:
  friend class LDOMParser;
  friend class LDOM_XmlReader;
  friend class LDOM_Document;
  friend class LDOM_Element;
  friend class LDOM_Node;
  // ---------- PRIVATE METHODS ----------

  const BasicAttribute* GetFirstAttribute(const BasicNode*&  aLastCh,
                                               const BasicNode**& thePrN) const;

  void RemoveNodes();

  void ReplaceElement(const BasicElement& anOther, const Handle(MemoryManager)& aDoc);
  //    remark: recursive

  void AddElementsByTagName(LDOM_NodeList& aList, const LDOMBasicString1& aTagName) const;
  //    remark: recursive

  void AddAttributes(LDOM_NodeList& aList, const BasicNode* aLastCh) const;
  //    add attributes to list

private:
  // ---------- PRIVATE FIELDS ----------

  //  LDOMBasicString1       myTagName;
  const char*     myTagName;
  unsigned long   myAttributeMask;
  BasicNode* myFirstChild;
};

#endif
