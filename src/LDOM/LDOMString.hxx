// Created on: 2001-06-25
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

#ifndef LDOMString_HeaderFile
#define LDOMString_HeaderFile

#include <LDOMBasicString.hxx>

class MemoryManager;

//  Class LDOMString
//  Represents various object types which can be mapped to XML strings
//  LDOMString is not an independent type: you must be sure that the owner
//  LDOM_Document is never lost during the lifetime of its LDOMStrings - for
//  that it is necessary to keep at least one LDOM_Document or LDOM_Node alive
//  before all LDOMString's (LDOM_AsciiDoc type) are destroyed.

class LDOMString : public LDOMBasicString1
{
public:
  // ---------- PUBLIC METHODS ----------

  LDOMString()
      : myPtrDoc(NULL)
  {
  }

  //    Empty constructor

  LDOMString(const LDOMString& anOther)
      : LDOMBasicString1(anOther),
        myPtrDoc(anOther.myPtrDoc)
  {
  }

  //    Copy constructor

  LDOMString(const Standard_Integer aValue)
      : LDOMBasicString1(aValue),
        myPtrDoc(NULL)
  {
  }

  //    Integer1 => LDOMString

  //  Standard_EXPORT LDOMString (const Standard_Real aValue);

  LDOMString(const char* aValue)
      : LDOMBasicString1(aValue),
        myPtrDoc(NULL)
  {
  }

  //    Create LDOM_AsciiFree

  const MemoryManager& getOwnerDocument() const { return *myPtrDoc; }

  LDOMString& operator=(const LDOM_NullPtr* aNull)
  {
    LDOMBasicString1::operator=(aNull);
    return *this;
  }

  LDOMString& operator=(const LDOMString& anOther)
  {
    myPtrDoc = anOther.myPtrDoc;
    LDOMBasicString1::operator=(anOther);
    return *this;
  }

private:
  friend class LDOM_Document;
  friend class LDOM_Node;
  friend class LDOM_Element;
  friend class BasicElement;
  friend class BasicAttribute;
  friend class BasicText;

  static LDOMString CreateDirectString(const char* aValue, const MemoryManager& aDoc);

  LDOMString(const LDOMBasicString1& anOther, const MemoryManager& aDoc)
      : LDOMBasicString1(anOther),
        myPtrDoc(&aDoc)
  {
  }

  //    Plain copy from LDOMBasicString1

  LDOMString(const LDOMBasicString1& anOther, const Handle(MemoryManager)& aDoc);
  //    Copy from another string with allocation in the document space

private:
  // ---------- PRIVATE FIELDS -------------
  const MemoryManager* myPtrDoc;
};

#endif
