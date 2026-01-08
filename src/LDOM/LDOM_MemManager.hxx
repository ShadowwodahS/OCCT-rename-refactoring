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

#ifndef LDOM_MemManager_HeaderFile
#define LDOM_MemManager_HeaderFile

#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>

class LDOM_BasicElement;
class LDOM_MemManager;
class LDOMBasicString1;

// Define handle class for LDOM_MemManager
DEFINE_STANDARD_HANDLE(LDOM_MemManager, RefObject)

//  Class LDOM_MemManager (underlying structure of LDOM_Document)
//

class LDOM_MemManager : public RefObject
{
public:
  // ---------- PUBLIC METHODS ----------

  Standard_EXPORT LDOM_MemManager(const Standard_Integer aBlockSize);
  // Constructor

  Standard_EXPORT ~LDOM_MemManager();
  // Destructor

  Standard_EXPORT void* Allocate(const Standard_Integer aSize);
  // General Memory allocator

  const char* HashedAllocate(const char*            aString,
                             const Standard_Integer theLen,
                             Standard_Integer&      theHash);
  // Memory allocation with access via hash table. No new allocation
  // if already present

  void HashedAllocate(const char*            aString,
                      const Standard_Integer theLen,
                      LDOMBasicString1&       theResult);

  // Memory allocation with access via hash table. No new allocation
  // if already present

  static Standard_Integer Hash(const char* theString, const Standard_Integer theLen)
  {
    return HashTable1::Hash(theString, theLen);
  }

  static Standard_Boolean CompareStrings(const char*            theString,
                                         const Standard_Integer theHashValue,
                                         const char*            theHashedStr);

  //  LDOM_Document           Doc           () const
  //                                { return LDOM_Document (* this); }

  const LDOM_MemManager& Self() const { return *this; }

  const LDOM_BasicElement* RootElement() const { return myRootElement; }

private:
  friend class LDOM_Document;
  friend class LDOMParser;

  // ---- CLASS MemBlock1 ----
  class MemBlock1
  {
    friend class LDOM_MemManager;
    inline MemBlock1(const Standard_Integer aSize, MemBlock1* aFirst);
    inline void* Allocate(const Standard_Integer aSize);
    void*        AllocateAndCheck(const Standard_Integer aSize, const MemBlock1*&);
    ~MemBlock1();

    MemBlock1* Next() { return myNext; }

    Standard_Integer  mySize;
    Standard_Integer* myBlock;
    Standard_Integer* myEndBlock;
    Standard_Integer* myFreeSpace;
    MemBlock1*         myNext;
  };

  // ---- CLASS HashTable1 ----
  class HashTable1
  {
    friend class LDOM_MemManager;
    HashTable1(/* const Standard_Integer theMask, */
              LDOM_MemManager& theMemManager);
    const char*             AddString(const char*            theString,
                                      const Standard_Integer theLen,
                                      Standard_Integer&      theHashIndex);
    static Standard_Integer Hash(const char* theString, const Standard_Integer theLen);

    struct TableItem1
    {
      char*             str;
      struct TableItem1* next;
    }* myTable;

    LDOM_MemManager& myManager;
    void             operator=(const HashTable1&);
  };

  // ---- PROHIBITED (PRIVATE) METHODS ----
  LDOM_MemManager(const LDOM_MemManager& theOther);
  // Copy constructor

  LDOM_MemManager& operator=(const LDOM_MemManager& theOther);
  // Assignment

  // ---------- PRIVATE FIELDS ----------

  const LDOM_BasicElement* myRootElement;
  MemBlock1*                myFirstBlock;
  MemBlock1*                myFirstWithoutRoom;
  Standard_Integer         myBlockSize;
  HashTable1*               myHashTable;

public:
  // CASCADE RTTI
  DEFINE_STANDARD_RTTIEXT(LDOM_MemManager, RefObject)
};

#endif
