// Created on: 2001-10-01
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

#include <LDOM_OSStream.hxx>
#include <NCollection_IncAllocator.hxx>
#include <Standard_Assert.hxx>

#include <string.h>

//=================================================================================================

StringBuffer::StringElement::StringElement(const int                                theLength,
                                               const Handle(NCollection_BaseAllocator)& theAlloc)
    : buf(reinterpret_cast<char*>(theAlloc->Allocate(theLength))),
      len(0),
      next(0)
{
}

//=================================================================================================

StringBuffer::StringBuffer(const Standard_Integer theMaxBuf)
    : myMaxBuf(theMaxBuf),
      myLength(0),
      myAlloc(new NCollection_IncAllocator)
{
  myFirstString = new (myAlloc) StringElement(theMaxBuf, myAlloc);
  myCurString   = myFirstString;
}

//=================================================================================================

StringBuffer::~StringBuffer()
{
  // no destruction is required as IncAllocator is used
}

//=================================================================================================

void StringBuffer::Clear()
{
  myAlloc       = new NCollection_IncAllocator;
  myFirstString = new (myAlloc) StringElement(myMaxBuf, myAlloc);
  myLength      = 0;
  myCurString   = myFirstString;
}

//=================================================================================================

Standard_CString StringBuffer::str() const
{
  char* aRetStr = new char[myLength + 1];

  StringElement* aCurElem = myFirstString;
  int              aCurLen  = 0;
  while (aCurElem)
  {
    strncpy(aRetStr + aCurLen, aCurElem->buf, aCurElem->len);
    aCurLen += aCurElem->len;
    aCurElem = aCurElem->next;
  }
  *(aRetStr + myLength) = '\0';

  return aRetStr;
}

//=======================================================================
// function : overflow()
// purpose  : redefined virtual
//=======================================================================
int StringBuffer::overflow(int c)
{
  char cc = (char)c;
  xsputn(&cc, 1);
  return c;
}

//=======================================================================
// function : underflow
// purpose  : redefined virtual
//=======================================================================

int StringBuffer::underflow()
{
  return EOF;
}

// int StringBuffer::uflow()
//{ return EOF; }

//=======================================================================
// function : xsputn()
// purpose  : redefined virtual
//=======================================================================
std::streamsize StringBuffer::xsputn(const char* aStr, std::streamsize n)
{
  Standard_ASSERT_RAISE(n < IntegerLast(),
                        "StringBuffer cannot work with strings greater than 2 Gb");

  Standard_Integer aLen    = static_cast<int>(n) + 1;
  Standard_Integer freeLen = myMaxBuf - myCurString->len - 1;
  if (freeLen >= n)
  {
    strncpy(myCurString->buf + myCurString->len, aStr, aLen);
  }
  else if (freeLen <= 0)
  {
    StringElement* aNextElem = new (myAlloc) StringElement(Max(aLen, myMaxBuf), myAlloc);
    myCurString->next          = aNextElem;
    myCurString                = aNextElem;
    strncpy(myCurString->buf + myCurString->len, aStr, aLen);
  }
  else // 0 < freeLen < n
  {
    // copy string by parts
    strncpy(myCurString->buf + myCurString->len, aStr, freeLen);
    myCurString->len += freeLen;
    *(myCurString->buf + myCurString->len) = '\0';
    aLen -= freeLen;
    StringElement* aNextElem = new (myAlloc) StringElement(Max(aLen, myMaxBuf), myAlloc);
    myCurString->next          = aNextElem;
    myCurString                = aNextElem;
    strncpy(myCurString->buf + myCurString->len, aStr + freeLen, aLen);
  }
  myCurString->len += aLen - 1;
  *(myCurString->buf + myCurString->len) = '\0';

  myLength += static_cast<int>(n);
  return n;
}

// streamsize StringBuffer::xsgetn(char* s, streamsize n)
//{ return _IO_default_xsgetn(this, s, n); }

//=================================================================================================

OutputStream::OutputStream(const Standard_Integer theMaxBuf)
    : Standard_OStream(&myBuffer),
      myBuffer(theMaxBuf)
{
  init(&myBuffer);
}

//=======================================================================
// function : ~OutputStream()
// purpose  : Destructor - for g++ vtable generation in *this* translation unit
//=======================================================================
OutputStream::~OutputStream() {}
