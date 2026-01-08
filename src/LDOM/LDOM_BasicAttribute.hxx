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

#ifndef LDOM_BasicAttribute_HeaderFile
#define LDOM_BasicAttribute_HeaderFile

#include <LDOM_BasicNode.hxx>
#include <LDOMBasicString.hxx>

class LDOM_Attr;

//  Class BasicAttribute
//

class BasicAttribute : public BasicNode
{
public:
  // ---------- PUBLIC METHODS ----------

  BasicAttribute()
      : BasicNode(LDOM_Node::UNKNOWN),
        myName(NULL)
  {
  }

  //    Empty constructor

  BasicAttribute& operator=(const LDOM_NullPtr* aNull);

  //    Nullify

  const char* GetName() const { return myName; }

  const LDOMBasicString1& GetValue() const { return myValue; }

  void SetValue(const LDOMBasicString1& aValue, const Handle(MemoryManager)& aDoc)
  {
    myValue = LDOMString(aValue, aDoc);
  }

private:
  friend class LDOM_Node;
  friend class LDOM_Attr;
  friend class LDOM_Element;
  friend class BasicElement;
  friend class LDOM_XmlReader;

  // ---------- PRIVATE METHODS ----------

  BasicAttribute(const LDOMBasicString1& aName)
      : BasicNode(LDOM_Node::ATTRIBUTE_NODE),
        myName(aName.GetString())
  {
  }

  //    Constructor

  static BasicAttribute& Create(const LDOMBasicString1&         theName,
                                     const Handle(MemoryManager)& theDoc,
                                     Standard_Integer&              theHashIndex);

  BasicAttribute(const LDOM_Attr& anAttr);

private:
  // ---------- PRIVATE FIELDS ----------

  //  LDOMBasicString1       myName;
  const char*     myName;
  LDOMBasicString1 myValue;
};

#endif
