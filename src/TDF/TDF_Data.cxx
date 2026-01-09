// Created by: DAUTRY Philippe
// Copyright (c) 1997-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#include <NCollection_IncAllocator.hxx>
#include <Standard_Dump.hxx>
#include <Standard_Type.hxx>
#include <Standard_GUID.hxx>
#include <NCollection_Array1.hxx>
#include <TCollection_AsciiString.hxx>
#include <TDF_AttributeDelta.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Data.hxx>
#include <TDF_Delta.hxx>
#include <TDF_DeltaOnAddition.hxx>
#include <TDF_DeltaOnForget.hxx>
#include <TDF_DeltaOnModification.hxx>
#include <TDF_DeltaOnRemoval.hxx>
#include <TDF_DeltaOnResume.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelNode.hxx>
#include <TDF_Tool.hxx>

typedef NCollection_Array1<Handle(TDF_AttributeDelta)> TDF_Array1OfAttributeIDelta;

IMPLEMENT_STANDARD_RTTIEXT(Data2, RefObject)

#undef DEB_DELTA_CREATION
#define TDF_DATA_COMMIT_OPTIMIZED

#ifdef OCCT_DEBUG_DELTA
  #define TDF_Data_DebugModified(ACTION)                                                           \
    std::cout << "After " << ACTION << " #" << myTransaction + 1 << ", DF " << this << " had "     \
              << myNbTouchedAtt << " attribute(s) touched. Time = " << myTime << std::endl;        \
    if (!myTransaction)                                                                            \
    {                                                                                              \
      AsciiString1 entry;                                                               \
      for (ChildIterator itr(Root(), Standard_True); itr.More(); itr.Next())                   \
      {                                                                                            \
        const TDF_LabelNode* lnp = itr.Value().myLabelNode;                                        \
        if (lnp->AttributesModified() || lnp->MayBeModified())                                     \
        {                                                                                          \
          Tool3::Entry(itr.Value(), entry);                                                     \
          std::cout << ACTION << " on " << entry << " : flag(s) ";                                 \
          if (lnp->AttributesModified())                                                           \
            std::cout << "AttributesModified ";                                                    \
          if (lnp->MayBeModified())                                                                \
            std::cout                                                                              \
              << "MayBeModified already set in transaction 0! Please contact TDF1 developer.";      \
          std::cout << std::endl;                                                                  \
          std::cout << itr.Value() << std::endl;                                                   \
          entry.Clear();                                                                           \
        }                                                                                          \
      }                                                                                            \
    }
#else
  #define TDF_Data_DebugModified(ACTION)
#endif

#ifdef OCCT_DEBUG_DELTA_CREATION
  #define TDF_DataDebugDeltaCreation(DELTATYPE)                                                    \
    {                                                                                              \
      AsciiString1 entry;                                                               \
      Tool3::Entry(currentAtt->Label(), entry);                                                 \
      std::cout << "Creation of a DeltaOn" << DELTATYPE << " \tat " << entry << " \ton "           \
                << currentAtt->DynamicType() << std::endl;                                         \
    }
#else
  #define TDF_DataDebugDeltaCreation(DELTATYPE)
#endif

#define TDF_Data_DeltaCreation(DELTACOMMENT, DELTACREATION)                                        \
  if (withDelta)                                                                                   \
  {                                                                                                \
    TDF_DataDebugDeltaCreation(DELTACOMMENT);                                                      \
    aDelta->AddAttributeDelta(DELTACREATION);                                                      \
  }

//=======================================================================
// function : Data2
// purpose  : empty constructor
//=======================================================================

Data2::Data2()
    : myTransaction(0),
      myNbTouchedAtt(0),
      myNotUndoMode(Standard_True),
      myTime(0),
      myAllowModification(Standard_True),
      myAccessByEntries(Standard_False)
{
  const Handle(NCollection_IncAllocator) anIncAllocator = new NCollection_IncAllocator(16000);
  myLabelNodeAllocator                                  = anIncAllocator;
  myRoot                                                = new (anIncAllocator) TDF_LabelNode(this);
}

//=======================================================================
// function : Destroy
// purpose  : Used to implement the destructor ~.
//=======================================================================

void Data2::Destroy()
{
  AbortUntilTransaction(1);
  // Forget the Owner attribute from the root label to avoid referencing document before
  // desctuction of the framework (on custom attributes forget). Don't call ForgetAll because
  // it may call backup.
  while (!myRoot->FirstAttribute().IsNull())
  {
    static Handle(TDF_Attribute) anEmpty;
    Handle(TDF_Attribute)        aFirst = myRoot->FirstAttribute();
    myRoot->RemoveAttribute(anEmpty, aFirst);
  }
  myAccessByEntriesTable.Clear();
  myRoot->Destroy(myLabelNodeAllocator);
  myRoot = NULL;
}

//=================================================================================================

Standard_Integer Data2::OpenTransaction()
{
  myTimes.Prepend(myTime);
  return ++myTransaction;
}

//=======================================================================
// function : CommitTransaction
// purpose  : Commits the current transaction.
//=======================================================================

Handle(Delta) Data2::CommitTransaction(const Standard_Boolean withDelta)
{
  Handle(Delta) delta;
  if (myTransaction > 0)
  {
    if (withDelta)
      delta = new Delta();
#ifdef OCCT_DEBUG_DELTA
    std::cout << "Data2::Begin Commit #" << myTransaction << std::endl;
#endif
#ifdef TDF_DATA_COMMIT_OPTIMIZED
    myNbTouchedAtt = 0;
    if (Root().myLabelNode->MayBeModified())
#endif
      myNbTouchedAtt = Data2::CommitTransaction(Root(), delta, withDelta);

    if (myNbTouchedAtt && !(withDelta && delta->IsEmpty()))
      ++myTime;
    --myTransaction;
    if (withDelta)
    {
      if (!delta->IsEmpty())
      {
        delta->Validity(myTimes.First(), myTime);
#ifdef OCCT_DEBUG_DELTA
        if (myTransaction == 0)
        {
          std::cout << "Data2::Commit generated this delta in t=0:" << std::endl;
          delta->Dump(std::cout);
        }
#endif
      }
#ifdef OCCT_DEBUG_DELTA
      else
      {
        if (myTransaction == 0)
          std::cout << "Data2::Commit generated NO delta." << std::endl;
      }
#endif
    }
    myTimes.RemoveFirst();
  }
  TDF_Data_DebugModified("COMMIT");
  return delta;
}

//=======================================================================
// function : CommitUntilTransaction
// purpose  : Commits the transactions until AND including
//           the given transaction index.
//=======================================================================

Handle(Delta) Data2::CommitUntilTransaction(const Standard_Integer untilTransaction,
                                                   const Standard_Boolean withDelta)
{
  Handle(Delta) delta;
  if ((untilTransaction > 0) && (myTransaction >= untilTransaction))
  {
    while (myTransaction > untilTransaction)
    {
      delta = Data2::CommitTransaction(Standard_False);
    }
    delta = Data2::CommitTransaction(withDelta);
  }
  return delta;
}

//=======================================================================
// function : CommitTransaction
// purpose  : Recursive method used to implement the commit action.
//=======================================================================

Standard_Integer Data2::CommitTransaction(const DataLabel&         aLabel,
                                             const Handle(Delta)& aDelta,
                                             const Standard_Boolean   withDelta)
{
  aLabel.myLabelNode->MayBeModified(Standard_False);
  Standard_Integer nbTouchedAtt = 0;
#ifdef TDF_DATA_COMMIT_OPTIMIZED
  Standard_Boolean attMod = aLabel.myLabelNode->AttributesModified();
#else
  Standard_Boolean attMod = Standard_True;
#endif

  if (attMod)
  {
    Handle(TDF_Attribute) lastAtt;
    Handle(TDF_Attribute) backupAtt;
    Standard_Boolean      currentIsRemoved = Standard_False;
    attMod                                 = Standard_False;

    TDF_AttributeIterator itr1(aLabel, Standard_False);
    while (itr1.More())
    {
      Handle(TDF_Attribute) aPtrCurrentAtt = itr1.Value();
      itr1.Next();
      //      currentAtt = itr1.Value();

      // A callback:
      aPtrCurrentAtt->BeforeCommitTransaction();

      backupAtt = aPtrCurrentAtt->myBackup;

      if (aPtrCurrentAtt->myTransaction == myTransaction)
      {
        ++nbTouchedAtt;
        --(aPtrCurrentAtt->myTransaction);

        // ------------------------------------------------------- Forgotten
        if (aPtrCurrentAtt->IsForgotten())
        {
          if (aPtrCurrentAtt->mySavedTransaction >= aPtrCurrentAtt->myTransaction)
          {
            const Handle(TDF_Attribute)& currentAtt = aPtrCurrentAtt;
            // Collision with a not forgotten version.
            if (backupAtt.IsNull())
            {
              TDF_Data_DeltaCreation("Removal(1)", currentAtt->DeltaOnRemoval());
              if (myNotUndoMode)
                currentAtt->BeforeRemoval();
              aLabel.myLabelNode->RemoveAttribute(lastAtt, currentAtt);
              currentIsRemoved = Standard_True;
              attMod           = Standard_True;
            }
            else
            {
              // Modified then Forgotten...
              // Forgotten flag spreading?
              currentAtt->Resume();
              currentAtt->Restore(backupAtt);
              currentAtt->myTransaction = backupAtt->myTransaction;
              currentAtt->RemoveBackup();
              backupAtt = currentAtt->myBackup;
              if (myTransaction == 1)
              {
                TDF_Data_DeltaCreation("Removal(2)", currentAtt->DeltaOnRemoval());
                if (myNotUndoMode)
                  currentAtt->BeforeRemoval();
                aLabel.myLabelNode->RemoveAttribute(lastAtt, currentAtt);
                currentIsRemoved = Standard_True;
              }
              else
              {
                // BeforeForget has already been called once.
                // if (myNotUndoMode) currentAtt->BeforeForget();
                currentAtt->Forget(myTransaction - 1);
                TDF_Data_DeltaCreation("Forget(1)", currentAtt->DeltaOnForget());
                attMod = Standard_True;
              }
            }
          }
          else
          {
            // Forgotten in lower transaction than the current one.
            TDF_Data_DeltaCreation("Forget(2)", aPtrCurrentAtt->DeltaOnForget());
          }
        }
        // ---------------------------------------------------------- Resumed.
        else if (aPtrCurrentAtt->mySavedTransaction < 0)
        {
          TDF_Data_DeltaCreation("Resume", aPtrCurrentAtt->DeltaOnResume());
          aPtrCurrentAtt->mySavedTransaction = 0;
          attMod                             = attMod || (aPtrCurrentAtt->myTransaction > 0);
        }

        // ------------------------------------------------------------ Added.
        else if (backupAtt.IsNull())
        {
          TDF_Data_DeltaCreation("Addition", aPtrCurrentAtt->DeltaOnAddition());
          attMod = attMod || (aPtrCurrentAtt->myTransaction > 0);
        }
        // --------------------------------------------------------- Modified.
        else
        {
          const TDF_Attribute* anAttrPtr = aPtrCurrentAtt.operator->(); // to avoid ambiguity
          TDF_Data_DeltaCreation("Modification", anAttrPtr->DeltaOnModification(backupAtt));
          if (aPtrCurrentAtt->myTransaction == backupAtt->myTransaction)
            aPtrCurrentAtt->RemoveBackup();
          attMod = attMod || (aPtrCurrentAtt->myTransaction > 0);
        }
      }
      else
        attMod = attMod || (aPtrCurrentAtt->myTransaction > 0);

      if (currentIsRemoved)
        currentIsRemoved = Standard_False;
      else
        lastAtt = aPtrCurrentAtt;
    }
    aLabel.myLabelNode->AttributesModified(attMod);
  }

  // Iteration on the children to do the same!
  //------------------------------------------
  for (ChildIterator itr2(aLabel); itr2.More(); itr2.Next())
  {
#ifdef TDF_DATA_COMMIT_OPTIMIZED
    if (itr2.Value().myLabelNode->MayBeModified())
#endif
      nbTouchedAtt += Data2::CommitTransaction(itr2.Value(), aDelta, withDelta);
  }

  return nbTouchedAtt;
}

//=======================================================================
// function : AbortTransaction
// purpose  : Aborts the current transaction.
//=======================================================================

void Data2::AbortTransaction()
{
  if (myTransaction > 0)
    Undo(Data2::CommitTransaction(Standard_True), Standard_False);
  TDF_Data_DebugModified("New ABORT");
}

//=======================================================================
// function : AbortUntilTransaction
// purpose  : Aborts the transactions until AND including the given index.
//=======================================================================

void Data2::AbortUntilTransaction(const Standard_Integer untilTransaction)
{
  if (untilTransaction > 0)
    Undo(Data2::CommitUntilTransaction(untilTransaction, Standard_True), Standard_False);
}

//=================================================================================================

Standard_Boolean Data2::IsApplicable(const Handle(Delta)& aDelta) const
{
  return !aDelta.IsNull() && aDelta->IsApplicable(myTime);
}

//=================================================================================================

void Data2::FixOrder(const Handle(Delta)& theDelta)
{
  // make all OnRemoval (which will cause addition of the attribute) are in the end
  // to do not put two attributes with the same GUID at one label during undo/redo
  TDF_AttributeDeltaList anOrderedList;

  const TDF_AttributeDeltaList&        attList = theDelta->AttributeDeltas();
  TDF_ListIteratorOfAttributeDeltaList anIt(attList);
  for (; anIt.More(); anIt.Next())
  { // append not-removal
    Handle(TDF_AttributeDelta) attDelta = anIt.Value();
    if (!attDelta->IsKind(STANDARD_TYPE(TDF_DeltaOnRemoval)))
    {
      anOrderedList.Append(attDelta);
    }
  }
  for (anIt.Initialize(attList); anIt.More(); anIt.Next())
  { // append removal
    Handle(TDF_AttributeDelta) attDelta = anIt.Value();
    if (attDelta->IsKind(STANDARD_TYPE(TDF_DeltaOnRemoval)))
    {
      anOrderedList.Append(attDelta);
    }
  }
  theDelta->ReplaceDeltaList(anOrderedList);
}

//=======================================================================
// function : Undo
// purpose  : Applies a delta to undo  actions.
//=======================================================================

Handle(Delta) Data2::Undo(const Handle(Delta)& aDelta, const Standard_Boolean withDelta)
{
  Handle(Delta) newDelta;
  if (!aDelta.IsNull())
  {
    if (aDelta->IsApplicable(myTime))
    {
      if (withDelta)
        OpenTransaction();
#ifdef OCCT_DEBUG_DELTA
      std::cout << "Data2::Undo applies this delta:" << std::endl;
      aDelta->Dump(std::cout);
#endif
      aDelta->BeforeOrAfterApply(Standard_True);
      myNotUndoMode = Standard_False;
      FixOrder(aDelta);
      aDelta->Apply();
      myNotUndoMode = Standard_True;
      if (withDelta)
      {
        newDelta = CommitTransaction(Standard_True);
        newDelta->Validity(aDelta->EndTime(), aDelta->BeginTime());
#ifdef OCCT_DEBUG_DELTA
        std::cout
          << "Data2::Undo, after validity correction, Delta is now available from time \t#"
          << newDelta->BeginTime() << " to time \t#" << newDelta->EndTime() << std::endl;
#endif
      }
      myTime = aDelta->BeginTime();
      aDelta->BeforeOrAfterApply(Standard_False);
    }
  }
  return newDelta;
}

//=================================================================================================

void Data2::SetAccessByEntries(const Standard_Boolean aSet)
{
  myAccessByEntries = aSet;

  myAccessByEntriesTable.Clear();
  if (myAccessByEntries)
  {
    // Add root label.
    AsciiString1 anEntry;
    Tool3::Entry(myRoot, anEntry);
    myAccessByEntriesTable.Bind(anEntry, myRoot);

    // Add all other labels.
    ChildIterator itr(myRoot, Standard_True);
    for (; itr.More(); itr.Next())
    {
      const DataLabel aLabel = itr.Value();
      Tool3::Entry(aLabel, anEntry);
      myAccessByEntriesTable.Bind(anEntry, aLabel);
    }
  }
}

//=================================================================================================

void Data2::RegisterLabel(const DataLabel& aLabel)
{
  AsciiString1 anEntry;
  Tool3::Entry(aLabel, anEntry);
  myAccessByEntriesTable.Bind(anEntry, aLabel);
}

//=================================================================================================

Standard_OStream& Data2::Dump(Standard_OStream& anOS) const
{
  anOS << "Dump of a Data2." << std::endl;
  anOS << "Current transaction: " << myTransaction;
  anOS << "; Current tick: " << myTime << ";" << std::endl;
  return anOS;
}

//=================================================================================================

void Data2::DumpJson(Standard_OStream& theOStream, Standard_Integer /*theDepth*/) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  AsciiString1 aStrForTDF_Label;
  Tool3::Entry(myRoot, aStrForTDF_Label);
  OCCT_DUMP_FIELD_VALUE_STRING(theOStream, aStrForTDF_Label)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myTransaction)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myNbTouchedAtt)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myNotUndoMode)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myTime)
  for (TColStd_ListOfInteger::Iterator aTimeIt(myTimes); aTimeIt.More(); aTimeIt.Next())
  {
    const Standard_Integer aTime = aTimeIt.Value();
    OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, aTime)
  }
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myAllowModification)
}
