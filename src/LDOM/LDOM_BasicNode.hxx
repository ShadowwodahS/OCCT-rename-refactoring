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

#ifndef LDOM_BasicNode_HeaderFile
#define LDOM_BasicNode_HeaderFile

#include <LDOM_Node.hxx>

class LDOM_NullPtr;

//  Block1 of comments describing class BasicNode
//

class BasicNode
{
public:
  DEFINE_STANDARD_ALLOC

public:
  Standard_Boolean isNull() const { return myNodeType == LDOM_Node::UNKNOWN; }

  LDOM_Node::NodeType getNodeType() const { return myNodeType; }

  Standard_EXPORT const BasicNode* GetSibling() const;

protected:
  // ---------- PROTECTED METHODS ----------

  BasicNode()
      : myNodeType(LDOM_Node::UNKNOWN),
        mySibling(NULL)
  {
  }

  //    Empty constructor

  BasicNode(LDOM_Node::NodeType aType)
      : myNodeType(aType),
        mySibling(NULL)
  {
  }

  //    Constructor

  BasicNode(const BasicNode& anOther)
      : myNodeType(anOther.getNodeType()),
        mySibling(anOther.GetSibling())
  {
  }

  //    Copy constructor

  BasicNode& operator=(const LDOM_NullPtr*)
  {
    myNodeType = LDOM_Node::UNKNOWN;
    return *this;
  }

  Standard_EXPORT BasicNode& operator=(const BasicNode& anOther);

  void SetSibling(const BasicNode* anOther) { mySibling = anOther; }

protected:
  friend class BasicElement;
  friend class LDOM_Node;
  friend class LDOMParser;
  // ---------- PROTECTED FIELDSS ----------

  LDOM_Node::NodeType   myNodeType;
  const BasicNode* mySibling;
};

#endif
