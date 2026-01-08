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

#include <LDOM_BasicNode.hxx>

#include <LDOM_BasicAttribute.hxx>
#include <LDOM_BasicElement.hxx>
#include <LDOM_BasicText.hxx>

//=======================================================================
// function : operator =
// purpose  : Assignment
//=======================================================================

BasicNode& BasicNode::operator=(const BasicNode& anOther)
{
  myNodeType = anOther.getNodeType();
  mySibling  = anOther.GetSibling();
  return *this;
}

//=======================================================================
// function : GetSibling
// purpose  : also detaches NULL siblings
//=======================================================================

const BasicNode* BasicNode::GetSibling() const
{
  while (mySibling)
    if (mySibling->isNull())
      (const BasicNode*&)mySibling = mySibling->mySibling;
    else
      break;
  return mySibling;
}

#ifdef OCCT_DEBUG
  #ifndef _MSC_VER
    //=======================================================================
    // Debug Function for DBX: use "print -p <Variable> or pp <Variable>"
    //=======================================================================
    #include <LDOM_OSStream.hxx>
    #define FLITERAL 0x10
    #define MAX_SIBLINGS 8

char* db_pretty_print(const BasicNode* aBNode, int fl, char*)
{
  OutputStream out(128);
  switch (aBNode->getNodeType())
  {
    case LDOM_Node::ELEMENT_NODE: {
      const BasicElement& aBElem = *(const BasicElement*)aBNode;
      if ((fl & FLITERAL) == 0)
        out << "BasicElement: ";
      out << '<' << aBElem.GetTagName();
      aBNode = aBElem.GetFirstChild();
      while (aBNode)
      {
        if (aBNode->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
          out << ' ' << db_pretty_print((const BasicAttribute*)aBNode, FLITERAL, 0);
        aBNode = aBNode->GetSibling();
      }
      out << '>';
      if ((fl & FLITERAL) == 0)
      {
        aBNode      = aBElem.GetFirstChild();
        int counter = MAX_SIBLINGS;
        if (aBNode)
        {
          out << std::endl << " == Children:" << std::endl;
          while (aBNode && counter--)
          {
            if (aBNode->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
              break;
            out << "  *(BasicNode*)" << aBNode << " : " << db_pretty_print(aBNode, FLITERAL, 0)
                << std::endl;
            aBNode = aBNode->GetSibling();
          }
        }
        aBNode = aBElem.GetSibling();
        if (aBNode)
        {
          out << " == Siblings:" << std::endl;
          counter = MAX_SIBLINGS;
          while (aBNode && counter--)
          {
            if (aBNode->getNodeType() == LDOM_Node::ATTRIBUTE_NODE)
              break;
            out << "  *(BasicNode*)" << aBNode << " : " << db_pretty_print(aBNode, FLITERAL, 0)
                << std::endl;
            aBNode = aBNode->GetSibling();
          }
        }
      }
      out << std::ends;
      break;
    }
    case LDOM_Node::ATTRIBUTE_NODE: {
      const BasicAttribute& aBAtt = *(const BasicAttribute*)aBNode;
      if ((fl & FLITERAL) == 0)
        out << "BasicAttribute: ";
      out << aBAtt.GetName() << '=' << db_pretty_print(&aBAtt.GetValue(), FLITERAL, 0) << std::ends;
      break;
    }
    case LDOM_Node::TEXT_NODE:
    case LDOM_Node::COMMENT_NODE:
    case LDOM_Node::CDATA_SECTION_NODE: {
      const BasicText& aBText = *(const BasicText*)aBNode;
      if ((fl & FLITERAL) == 0)
        out << "BasicText: ";
      out << db_pretty_print(&aBText.GetData(), FLITERAL, 0) << std::ends;
      break;
    }
    default:
      out << "UNKNOWN" << std::ends;
      break;
  }
  return (char*)out.str();
}

char* db_pretty_print(const LDOM_Node* aBNode, int fl, char* c)
{
  return db_pretty_print(&aBNode->Origin(), fl, c);
}

  #endif
#endif
