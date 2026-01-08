// Created on: 2002-04-10
// Created by: Alexander KARTOMIN (akm)
// Copyright (c) 2002-2014 OPEN CASCADE SAS
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

#ifndef NCollection_BaseSequence_HeaderFile
#define NCollection_BaseSequence_HeaderFile

#include <Standard.hxx>
#include <NCollection_BaseAllocator.hxx>
#include <NCollection_DefineAlloc.hxx>

// **************************************** Class SeqNode ********************

class SequenceNode
{
public:
  // define new operator for use with NCollection allocators
  DEFINE_NCOLLECTION_ALLOC
public:
  SequenceNode()
      : myNext(NULL),
        myPrevious(NULL)
  {
  }

  SequenceNode* Next() const { return myNext; }

  SequenceNode* Previous() const { return myPrevious; }

  void SetNext(SequenceNode* theNext) { myNext = theNext; }

  void SetPrevious(SequenceNode* thePrev) { myPrevious = thePrev; }

private:
  SequenceNode* myNext;
  SequenceNode* myPrevious;
};

typedef void (*NCollection_DelSeqNode)(SequenceNode*,
                                       Handle(NCollection_BaseAllocator)& theAl);

/**
 * Purpose:     This  is  a base  class  for  the  Sequence.  It  deals with
 *              an indexed bidirectional list of SequenceNode's.
 */
class BaseSequence
{
public:
  //! Memory allocation
  DEFINE_STANDARD_ALLOC
  DEFINE_NCOLLECTION_ALLOC

public:
  class Iterator
  {
  public:
    //! Empty constructor
    Iterator(void)
        : myCurrent(NULL),
          myPrevious(NULL)
    {
    }

    //! Constructor with initialisation
    Iterator(const BaseSequence& theSeq, const Standard_Boolean isStart)
    {
      Init(theSeq, isStart);
    }

    //! Initialisation
    void Init(const BaseSequence& theSeq,
              const Standard_Boolean          isStart = Standard_True)
    {
      myCurrent  = (isStart ? theSeq.myFirstItem : NULL);
      myPrevious = (isStart ? NULL : theSeq.myLastItem);
    }

    //! Switch to previous element; note that it will reset
    void Previous()
    {
      myCurrent = myPrevious;
      if (myCurrent)
        myPrevious = myCurrent->Previous();
    }

  protected:
    SequenceNode* myCurrent;  //!< Pointer to the current node
    SequenceNode* myPrevious; //!< Pointer to the previous node
    friend class BaseSequence;
  };

public:
  // Methods PUBLIC
  //
  Standard_Boolean IsEmpty() const { return (mySize == 0); }

  Standard_Integer Length() const { return mySize; }

  //! Returns attached allocator
  const Handle(NCollection_BaseAllocator)& Allocator() const { return myAllocator; }

protected:
  // Methods PROTECTED
  //
  BaseSequence(const Handle(NCollection_BaseAllocator)& theAllocator)
      : myFirstItem(NULL),
        myLastItem(NULL),
        myCurrentItem(NULL),
        myCurrentIndex(0),
        mySize(0)
  {
    myAllocator =
      (theAllocator.IsNull() ? NCollection_BaseAllocator::CommonBaseAllocator() : theAllocator);
  }

  //! Destructor
  virtual ~BaseSequence() {}

  Standard_EXPORT void ClearSeq(NCollection_DelSeqNode fDel);
  Standard_EXPORT void PAppend(SequenceNode*);
  Standard_EXPORT void PAppend(BaseSequence& S);
  Standard_EXPORT void PPrepend(SequenceNode*);
  Standard_EXPORT void PPrepend(BaseSequence& S);
  Standard_EXPORT void PInsertAfter(Iterator& thePosition, SequenceNode*);
  Standard_EXPORT void PInsertAfter(const Standard_Integer Index, SequenceNode*);
  Standard_EXPORT void PInsertAfter(const Standard_Integer Index, BaseSequence& S);
  Standard_EXPORT void PSplit(const Standard_Integer Index, BaseSequence& Sub);
  Standard_EXPORT void RemoveSeq(Iterator& thePosition, NCollection_DelSeqNode fDel);
  Standard_EXPORT void RemoveSeq(const Standard_Integer Index, NCollection_DelSeqNode fDel);
  Standard_EXPORT void RemoveSeq(const Standard_Integer From,
                                 const Standard_Integer To,
                                 NCollection_DelSeqNode fDel);
  Standard_EXPORT void PReverse();
  Standard_EXPORT void PExchange(const Standard_Integer I, const Standard_Integer J);
  Standard_EXPORT SequenceNode* Find(const Standard_Integer) const;

protected:
  // Fields PROTECTED
  //
  Handle(NCollection_BaseAllocator) myAllocator;
  SequenceNode*              myFirstItem;
  SequenceNode*              myLastItem;
  SequenceNode*              myCurrentItem;
  Standard_Integer                  myCurrentIndex;
  Standard_Integer                  mySize;

private:
  // Methods PRIVATE
  //
  Standard_EXPORT BaseSequence(const BaseSequence& Other);

  void Nullify()
  {
    myFirstItem = myLastItem = myCurrentItem = NULL;
    myCurrentIndex = mySize = 0;
  }

  friend class Iterator;
};

#endif
