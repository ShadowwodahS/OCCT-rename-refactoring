// Copyright (c) 2017-2018 OPEN CASCADE SAS
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

#include <XCAFDoc_NotesTool.hxx>

#include <TColStd_HArray1OfByte.hxx>
#include <TDF_Label.hxx>
#include <TDF_ChildIDIterator.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDF_Tool.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_GraphNode.hxx>
#include <XCAFDoc_NoteBalloon.hxx>
#include <XCAFDoc_NoteComment.hxx>
#include <XCAFDoc_NoteBinData.hxx>
#include <XCAFDoc_AssemblyItemRef.hxx>

namespace
{

AssemblyItemId labeledItem(const DataLabel& theLabel)
{
  AsciiString1 anEntry;
  Tool3::Entry(theLabel, anEntry);
  return AssemblyItemId(anEntry);
}

} // namespace

IMPLEMENT_DERIVED_ATTRIBUTE(XCAFDoc_NotesTool, TDataStd_GenericEmpty)

enum NotesTool_RootLabels
{
  NotesTool_NotesRoot = 1,
  NotesTool_AnnotatedItemsRoot
};

//=================================================================================================

const Standard_GUID& XCAFDoc_NotesTool::GetID()
{
  static Standard_GUID s_ID("8F8174B1-6125-47a0-B357-61BD2D89380C");
  return s_ID;
}

//=================================================================================================

Handle(XCAFDoc_NotesTool) XCAFDoc_NotesTool::Set(const DataLabel& theLabel)
{
  Handle(XCAFDoc_NotesTool) aTool;
  if (!theLabel.IsNull() && !theLabel.FindAttribute(XCAFDoc_NotesTool::GetID(), aTool))
  {
    aTool = new XCAFDoc_NotesTool();
    theLabel.AddAttribute(aTool);
  }
  return aTool;
}

//=================================================================================================

XCAFDoc_NotesTool::XCAFDoc_NotesTool() {}

//=================================================================================================

DataLabel XCAFDoc_NotesTool::GetNotesLabel() const
{
  return Label().FindChild(NotesTool_NotesRoot);
}

//=================================================================================================

DataLabel XCAFDoc_NotesTool::GetAnnotatedItemsLabel() const
{
  return Label().FindChild(NotesTool_AnnotatedItemsRoot);
}

//=================================================================================================

Standard_Integer XCAFDoc_NotesTool::NbNotes() const
{
  Standard_Integer nbNotes = 0;
  for (ChildIterator anIter(GetNotesLabel()); anIter.More(); anIter.Next())
  {
    const DataLabel aLabel = anIter.Value();
    if (!XCAFDoc_Note::Get(aLabel).IsNull())
      ++nbNotes;
  }
  return nbNotes;
}

//=================================================================================================

Standard_Integer XCAFDoc_NotesTool::NbAnnotatedItems() const
{
  Standard_Integer nbItems = 0;
  for (TDF_ChildIDIterator anIter(GetAnnotatedItemsLabel(), XCAFDoc_AssemblyItemRef::GetID());
       anIter.More();
       anIter.Next())
  {
    ++nbItems;
  }
  return nbItems;
}

//=================================================================================================

void XCAFDoc_NotesTool::GetNotes(TDF_LabelSequence& theNoteLabels) const
{
  for (ChildIterator anIter(GetNotesLabel()); anIter.More(); anIter.Next())
  {
    const DataLabel aLabel = anIter.Value();
    if (!XCAFDoc_Note::Get(aLabel).IsNull())
      theNoteLabels.Append(aLabel);
  }
}

//=================================================================================================

void XCAFDoc_NotesTool::GetAnnotatedItems(TDF_LabelSequence& theItemLabels) const
{
  for (TDF_ChildIDIterator anIter(GetAnnotatedItemsLabel(), XCAFDoc_AssemblyItemRef::GetID());
       anIter.More();
       anIter.Next())
  {
    theItemLabels.Append(anIter.Value()->Label());
  }
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::IsAnnotatedItem(const AssemblyItemId& theItemId) const
{
  return !FindAnnotatedItem(theItemId).IsNull();
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::IsAnnotatedItem(const DataLabel& theItemLabel) const
{
  return IsAnnotatedItem(labeledItem(theItemLabel));
}

//=================================================================================================

DataLabel XCAFDoc_NotesTool::FindAnnotatedItem(const AssemblyItemId& theItemId) const
{
  for (TDF_ChildIDIterator anIter(GetAnnotatedItemsLabel(), XCAFDoc_AssemblyItemRef::GetID());
       anIter.More();
       anIter.Next())
  {
    Handle(XCAFDoc_AssemblyItemRef) anItemRef =
      Handle(XCAFDoc_AssemblyItemRef)::DownCast(anIter.Value());
    if (!anItemRef.IsNull() && anItemRef->GetItem().IsEqual(theItemId) && !anItemRef->HasExtraRef())
      return anItemRef->Label();
  }
  return DataLabel();
}

//=================================================================================================

DataLabel XCAFDoc_NotesTool::FindAnnotatedItem(const DataLabel& theItemLabel) const
{
  return FindAnnotatedItem(labeledItem(theItemLabel));
}

//=================================================================================================

DataLabel XCAFDoc_NotesTool::FindAnnotatedItemAttr(const AssemblyItemId& theItemId,
                                                   const Standard_GUID&          theGUID) const
{
  for (TDF_ChildIDIterator anIter(GetAnnotatedItemsLabel(), XCAFDoc_AssemblyItemRef::GetID());
       anIter.More();
       anIter.Next())
  {
    Handle(XCAFDoc_AssemblyItemRef) anItemRef =
      Handle(XCAFDoc_AssemblyItemRef)::DownCast(anIter.Value());
    if (!anItemRef.IsNull() && anItemRef->GetItem().IsEqual(theItemId) && anItemRef->HasExtraRef()
        && anItemRef->GetGUID() == theGUID)
      return anItemRef->Label();
  }
  return DataLabel();
}

//=================================================================================================

DataLabel XCAFDoc_NotesTool::FindAnnotatedItemAttr(const DataLabel&     theItemLabel,
                                                   const Standard_GUID& theGUID) const
{
  return FindAnnotatedItemAttr(labeledItem(theItemLabel), theGUID);
}

//=================================================================================================

DataLabel XCAFDoc_NotesTool::FindAnnotatedItemSubshape(const AssemblyItemId& theItemId,
                                                       Standard_Integer theSubshapeIndex) const
{
  for (TDF_ChildIDIterator anIter(GetAnnotatedItemsLabel(), XCAFDoc_AssemblyItemRef::GetID());
       anIter.More();
       anIter.Next())
  {
    Handle(XCAFDoc_AssemblyItemRef) anItemRef =
      Handle(XCAFDoc_AssemblyItemRef)::DownCast(anIter.Value());
    if (!anItemRef.IsNull() && anItemRef->GetItem().IsEqual(theItemId) && anItemRef->HasExtraRef()
        && anItemRef->GetSubshapeIndex() == theSubshapeIndex)
      return anItemRef->Label();
  }
  return DataLabel();
}

//=================================================================================================

DataLabel XCAFDoc_NotesTool::FindAnnotatedItemSubshape(const DataLabel& theItemLabel,
                                                       Standard_Integer theSubshapeIndex) const
{
  return FindAnnotatedItemSubshape(labeledItem(theItemLabel), theSubshapeIndex);
}

//=================================================================================================

Handle(XCAFDoc_Note) XCAFDoc_NotesTool::CreateComment(
  const UtfString& theUserName,
  const UtfString& theTimeStamp,
  const UtfString& theComment)
{
  DataLabel     aNoteLabel;
  TDF_TagSource aTag;
  aNoteLabel = aTag.NewChild(GetNotesLabel());
  return XCAFDoc_NoteComment::Set(aNoteLabel, theUserName, theTimeStamp, theComment);
}

//=================================================================================================

Handle(XCAFDoc_Note) XCAFDoc_NotesTool::CreateBalloon(
  const UtfString& theUserName,
  const UtfString& theTimeStamp,
  const UtfString& theComment)
{
  DataLabel     aNoteLabel;
  TDF_TagSource aTag;
  aNoteLabel = aTag.NewChild(GetNotesLabel());
  return XCAFDoc_NoteBalloon::Set(aNoteLabel, theUserName, theTimeStamp, theComment);
}

//=================================================================================================

Handle(XCAFDoc_Note) XCAFDoc_NotesTool::CreateBinData(
  const UtfString& theUserName,
  const UtfString& theTimeStamp,
  const UtfString& theTitle,
  const AsciiString1&    theMIMEtype,
  SystemFile&                         theFile)
{
  DataLabel     aNoteLabel;
  TDF_TagSource aTag;
  aNoteLabel = aTag.NewChild(GetNotesLabel());
  return XCAFDoc_NoteBinData::Set(aNoteLabel,
                                  theUserName,
                                  theTimeStamp,
                                  theTitle,
                                  theMIMEtype,
                                  theFile);
}

//=================================================================================================

Handle(XCAFDoc_Note) XCAFDoc_NotesTool::CreateBinData(
  const UtfString&    theUserName,
  const UtfString&    theTimeStamp,
  const UtfString&    theTitle,
  const AsciiString1&       theMIMEtype,
  const Handle(TColStd_HArray1OfByte)& theData)
{
  DataLabel     aNoteLabel;
  TDF_TagSource aTag;
  aNoteLabel = aTag.NewChild(GetNotesLabel());
  return XCAFDoc_NoteBinData::Set(aNoteLabel,
                                  theUserName,
                                  theTimeStamp,
                                  theTitle,
                                  theMIMEtype,
                                  theData);
}

//=================================================================================================

Standard_Integer XCAFDoc_NotesTool::GetNotes(const AssemblyItemId& theItemId,
                                             TDF_LabelSequence&            theNoteLabels) const
{
  DataLabel anAnnotatedItem = FindAnnotatedItem(theItemId);
  if (anAnnotatedItem.IsNull())
    return 0;

  Handle(XCAFDoc_GraphNode) aChild;
  if (!anAnnotatedItem.FindAttribute(XCAFDoc1::NoteRefGUID(), aChild))
    return 0;

  Standard_Integer nbFathers = aChild->NbFathers();
  for (Standard_Integer iFather = 1; iFather <= nbFathers; ++iFather)
  {
    Handle(XCAFDoc_GraphNode) aFather = aChild->GetFather(iFather);
    theNoteLabels.Append(aFather->Label());
  }

  return theNoteLabels.Length();
}

//=================================================================================================

Standard_Integer XCAFDoc_NotesTool::GetNotes(const DataLabel&   theItemLabel,
                                             TDF_LabelSequence& theNoteLabels) const
{
  return GetNotes(labeledItem(theItemLabel), theNoteLabels);
}

//=================================================================================================

Standard_Integer XCAFDoc_NotesTool::GetAttrNotes(const AssemblyItemId& theItemId,
                                                 const Standard_GUID&          theGUID,
                                                 TDF_LabelSequence&            theNoteLabels) const
{
  DataLabel anAnnotatedItem = FindAnnotatedItemAttr(theItemId, theGUID);
  if (anAnnotatedItem.IsNull())
    return 0;

  Handle(XCAFDoc_GraphNode) aChild;
  if (!anAnnotatedItem.FindAttribute(XCAFDoc1::NoteRefGUID(), aChild))
    return 0;

  Standard_Integer nbFathers = aChild->NbFathers();
  for (Standard_Integer iFather = 1; iFather <= nbFathers; ++iFather)
  {
    Handle(XCAFDoc_GraphNode) aFather = aChild->GetFather(iFather);
    theNoteLabels.Append(aFather->Label());
  }

  return theNoteLabels.Length();
}

//=================================================================================================

Standard_Integer XCAFDoc_NotesTool::GetAttrNotes(const DataLabel&     theItemLabel,
                                                 const Standard_GUID& theGUID,
                                                 TDF_LabelSequence&   theNoteLabels) const
{
  return GetAttrNotes(labeledItem(theItemLabel), theGUID, theNoteLabels);
}

//=================================================================================================

Standard_Integer XCAFDoc_NotesTool::GetSubshapeNotes(const AssemblyItemId& theItemId,
                                                     Standard_Integer              theSubshapeIndex,
                                                     TDF_LabelSequence& theNoteLabels) const
{
  DataLabel anAnnotatedItem = FindAnnotatedItemSubshape(theItemId, theSubshapeIndex);
  if (anAnnotatedItem.IsNull())
    return 0;

  Handle(XCAFDoc_GraphNode) aChild;
  if (!anAnnotatedItem.FindAttribute(XCAFDoc1::NoteRefGUID(), aChild))
    return 0;

  Standard_Integer nbFathers = aChild->NbFathers();
  for (Standard_Integer iFather = 1; iFather <= nbFathers; ++iFather)
  {
    Handle(XCAFDoc_GraphNode) aFather = aChild->GetFather(iFather);
    theNoteLabels.Append(aFather->Label());
  }

  return theNoteLabels.Length();
}

//=================================================================================================

Handle(XCAFDoc_AssemblyItemRef) XCAFDoc_NotesTool::AddNote(const DataLabel& theNoteLabel,
                                                           const AssemblyItemId& theItemId)
{
  Handle(XCAFDoc_AssemblyItemRef) anItemRef;

  if (!XCAFDoc_Note::IsMine(theNoteLabel))
    return anItemRef;

  Handle(XCAFDoc_GraphNode) aChild;
  DataLabel                 anAnnotatedItem = FindAnnotatedItem(theItemId);
  if (anAnnotatedItem.IsNull())
  {
    TDF_TagSource aTag;
    anAnnotatedItem = aTag.NewChild(GetAnnotatedItemsLabel());
    if (anAnnotatedItem.IsNull())
      return anItemRef;
  }

  if (!anAnnotatedItem.FindAttribute(XCAFDoc1::NoteRefGUID(), aChild))
  {
    aChild = XCAFDoc_GraphNode::Set(anAnnotatedItem, XCAFDoc1::NoteRefGUID());
    if (aChild.IsNull())
      return anItemRef;
  }

  if (!anAnnotatedItem.FindAttribute(XCAFDoc_AssemblyItemRef::GetID(), anItemRef))
  {
    anItemRef = XCAFDoc_AssemblyItemRef::Set(anAnnotatedItem, theItemId);
    if (anItemRef.IsNull())
      return anItemRef;
  }

  Handle(XCAFDoc_GraphNode) aFather;
  if (!theNoteLabel.FindAttribute(XCAFDoc1::NoteRefGUID(), aFather))
  {
    aFather = XCAFDoc_GraphNode::Set(theNoteLabel, XCAFDoc1::NoteRefGUID());
    if (aFather.IsNull())
      return anItemRef;
  }

  aChild->SetFather(aFather);
  aFather->SetChild(aChild);

  return anItemRef;
}

//=================================================================================================

Handle(XCAFDoc_AssemblyItemRef) XCAFDoc_NotesTool::AddNote(const DataLabel& theNoteLabel,
                                                           const DataLabel& theItemLabel)
{
  return AddNote(theNoteLabel, labeledItem(theItemLabel));
}

//=================================================================================================

Handle(XCAFDoc_AssemblyItemRef) XCAFDoc_NotesTool::AddNoteToAttr(
  const DataLabel&              theNoteLabel,
  const AssemblyItemId& theItemId,
  const Standard_GUID&          theGUID)
{
  Handle(XCAFDoc_AssemblyItemRef) anItemRef;

  if (!XCAFDoc_Note::IsMine(theNoteLabel))
    return anItemRef;

  Handle(XCAFDoc_GraphNode) aChild;
  DataLabel                 anAnnotatedItem = FindAnnotatedItemAttr(theItemId, theGUID);
  if (anAnnotatedItem.IsNull())
  {
    TDF_TagSource aTag;
    anAnnotatedItem = aTag.NewChild(GetAnnotatedItemsLabel());
    if (anAnnotatedItem.IsNull())
      return anItemRef;
  }

  if (!anAnnotatedItem.FindAttribute(XCAFDoc1::NoteRefGUID(), aChild))
  {
    aChild = XCAFDoc_GraphNode::Set(anAnnotatedItem, XCAFDoc1::NoteRefGUID());
    if (aChild.IsNull())
      return anItemRef;
  }

  if (!anAnnotatedItem.FindAttribute(XCAFDoc_AssemblyItemRef::GetID(), anItemRef))
  {
    anItemRef = XCAFDoc_AssemblyItemRef::Set(anAnnotatedItem, theItemId);
    if (anItemRef.IsNull())
      return anItemRef;
  }

  Handle(XCAFDoc_GraphNode) aFather;
  if (!theNoteLabel.FindAttribute(XCAFDoc1::NoteRefGUID(), aFather))
  {
    aFather = XCAFDoc_GraphNode::Set(theNoteLabel, XCAFDoc1::NoteRefGUID());
    if (aFather.IsNull())
      return anItemRef;
  }

  aChild->SetFather(aFather);
  aFather->SetChild(aChild);

  anItemRef->SetGUID(theGUID);

  return anItemRef;
}

//=================================================================================================

Handle(XCAFDoc_AssemblyItemRef) XCAFDoc_NotesTool::AddNoteToAttr(const DataLabel&     theNoteLabel,
                                                                 const DataLabel&     theItemLabel,
                                                                 const Standard_GUID& theGUID)
{
  return AddNoteToAttr(theNoteLabel, labeledItem(theItemLabel), theGUID);
}

//=================================================================================================

Handle(XCAFDoc_AssemblyItemRef) XCAFDoc_NotesTool::AddNoteToSubshape(
  const DataLabel&              theNoteLabel,
  const AssemblyItemId& theItemId,
  Standard_Integer              theSubshapeIndex)
{
  Handle(XCAFDoc_AssemblyItemRef) anItemRef;

  if (!XCAFDoc_Note::IsMine(theNoteLabel))
    return anItemRef;

  Handle(XCAFDoc_GraphNode) aChild;
  DataLabel anAnnotatedItem = FindAnnotatedItemSubshape(theItemId, theSubshapeIndex);
  if (anAnnotatedItem.IsNull())
  {
    TDF_TagSource aTag;
    anAnnotatedItem = aTag.NewChild(GetAnnotatedItemsLabel());
    if (anAnnotatedItem.IsNull())
      return anItemRef;
  }

  if (!anAnnotatedItem.FindAttribute(XCAFDoc1::NoteRefGUID(), aChild))
  {
    aChild = XCAFDoc_GraphNode::Set(anAnnotatedItem, XCAFDoc1::NoteRefGUID());
    if (aChild.IsNull())
      return anItemRef;
  }

  if (!anAnnotatedItem.FindAttribute(XCAFDoc_AssemblyItemRef::GetID(), anItemRef))
  {
    anItemRef = XCAFDoc_AssemblyItemRef::Set(anAnnotatedItem, theItemId);
    if (anItemRef.IsNull())
      return anItemRef;
  }

  Handle(XCAFDoc_GraphNode) aFather;
  if (!theNoteLabel.FindAttribute(XCAFDoc1::NoteRefGUID(), aFather))
  {
    aFather = XCAFDoc_GraphNode::Set(theNoteLabel, XCAFDoc1::NoteRefGUID());
    if (aFather.IsNull())
      return anItemRef;
  }

  aChild->SetFather(aFather);
  aFather->SetChild(aChild);

  anItemRef->SetSubshapeIndex(theSubshapeIndex);

  return anItemRef;
}

//=================================================================================================

Handle(XCAFDoc_AssemblyItemRef) XCAFDoc_NotesTool::AddNoteToSubshape(
  const DataLabel& theNoteLabel,
  const DataLabel& theItemLabel,
  Standard_Integer theSubshapeIndex)
{
  return AddNoteToSubshape(theNoteLabel, labeledItem(theItemLabel), theSubshapeIndex);
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::RemoveNote(const DataLabel&              theNoteLabel,
                                               const AssemblyItemId& theItemId,
                                               Standard_Boolean              theDelIfOrphan)
{
  Handle(XCAFDoc_Note) aNote = XCAFDoc_Note::Get(theNoteLabel);

  if (aNote.IsNull())
    return Standard_False;

  Handle(XCAFDoc_GraphNode) aFather;
  if (!theNoteLabel.FindAttribute(XCAFDoc1::NoteRefGUID(), aFather))
    return Standard_False;

  DataLabel anAnnotatedItem = FindAnnotatedItem(theItemId);
  if (anAnnotatedItem.IsNull())
    return Standard_False;

  Handle(XCAFDoc_GraphNode) aChild;
  if (!anAnnotatedItem.FindAttribute(XCAFDoc1::NoteRefGUID(), aChild))
    return Standard_False;

  aChild->UnSetFather(aFather);
  if (aChild->NbFathers() == 0)
    anAnnotatedItem.ForgetAllAttributes();

  if (theDelIfOrphan && aNote->IsOrphan())
    DeleteNote(theNoteLabel);

  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::RemoveNote(const DataLabel& theNoteLabel,
                                               const DataLabel& theItemLabel,
                                               Standard_Boolean theDelIfOrphan)
{
  return RemoveNote(theNoteLabel, labeledItem(theItemLabel), theDelIfOrphan);
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::RemoveSubshapeNote(const DataLabel&              theNoteLabel,
                                                       const AssemblyItemId& theItemId,
                                                       Standard_Integer theSubshapeIndex,
                                                       Standard_Boolean theDelIfOrphan)
{
  Handle(XCAFDoc_Note) aNote = XCAFDoc_Note::Get(theNoteLabel);

  if (aNote.IsNull())
    return Standard_False;

  Handle(XCAFDoc_GraphNode) aFather;
  if (!theNoteLabel.FindAttribute(XCAFDoc1::NoteRefGUID(), aFather))
    return Standard_False;

  DataLabel anAnnotatedItem = FindAnnotatedItemSubshape(theItemId, theSubshapeIndex);
  if (anAnnotatedItem.IsNull())
    return Standard_False;

  Handle(XCAFDoc_GraphNode) aChild;
  if (!anAnnotatedItem.FindAttribute(XCAFDoc1::NoteRefGUID(), aChild))
    return Standard_False;

  aChild->UnSetFather(aFather);
  if (aChild->NbFathers() == 0)
    anAnnotatedItem.ForgetAllAttributes();

  if (theDelIfOrphan && aNote->IsOrphan())
    DeleteNote(theNoteLabel);

  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::RemoveSubshapeNote(const DataLabel& theNoteLabel,
                                                       const DataLabel& theItemLabel,
                                                       Standard_Integer theSubshapeIndex,
                                                       Standard_Boolean theDelIfOrphan)
{
  return RemoveSubshapeNote(theNoteLabel,
                            labeledItem(theItemLabel),
                            theSubshapeIndex,
                            theDelIfOrphan);
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::RemoveAttrNote(const DataLabel&              theNoteLabel,
                                                   const AssemblyItemId& theItemId,
                                                   const Standard_GUID&          theGUID,
                                                   Standard_Boolean              theDelIfOrphan)
{
  Handle(XCAFDoc_Note) aNote = XCAFDoc_Note::Get(theNoteLabel);

  if (aNote.IsNull())
    return Standard_False;

  Handle(XCAFDoc_GraphNode) aFather;
  if (!theNoteLabel.FindAttribute(XCAFDoc1::NoteRefGUID(), aFather))
    return Standard_False;

  DataLabel anAnnotatedItem = FindAnnotatedItemAttr(theItemId, theGUID);
  if (anAnnotatedItem.IsNull())
    return Standard_False;

  Handle(XCAFDoc_GraphNode) aChild;
  if (!anAnnotatedItem.FindAttribute(XCAFDoc1::NoteRefGUID(), aChild))
    return Standard_False;

  aChild->UnSetFather(aFather);
  if (aChild->NbFathers() == 0)
    anAnnotatedItem.ForgetAllAttributes();

  if (theDelIfOrphan && aNote->IsOrphan())
    DeleteNote(theNoteLabel);

  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::RemoveAttrNote(const DataLabel&     theNoteLabel,
                                                   const DataLabel&     theItemLabel,
                                                   const Standard_GUID& theGUID,
                                                   Standard_Boolean     theDelIfOrphan)
{
  return RemoveAttrNote(theNoteLabel, labeledItem(theItemLabel), theGUID, theDelIfOrphan);
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::RemoveAllNotes(const AssemblyItemId& theItemId,
                                                   Standard_Boolean              theDelIfOrphan)
{
  DataLabel anAnnotatedItem = FindAnnotatedItem(theItemId);
  if (anAnnotatedItem.IsNull())
    return Standard_False;

  Handle(XCAFDoc_GraphNode) aChild;
  if (!anAnnotatedItem.FindAttribute(XCAFDoc1::NoteRefGUID(), aChild))
    return Standard_False;

  while (aChild->NbFathers() > 0)
  {
    Handle(XCAFDoc_GraphNode) aFather = aChild->GetFather(1);
    Handle(XCAFDoc_Note)      aNote   = XCAFDoc_Note::Get(aFather->Label());
    if (!aNote.IsNull())
    {
      aFather->UnSetChild(aChild);
      if (theDelIfOrphan && aNote->IsOrphan())
        DeleteNote(aFather->Label());
    }
  }

  anAnnotatedItem.ForgetAllAttributes();

  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::RemoveAllNotes(const DataLabel& theItemLabel,
                                                   Standard_Boolean theDelIfOrphan)
{
  return RemoveAllNotes(labeledItem(theItemLabel), theDelIfOrphan);
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::RemoveAllSubshapeNotes(const AssemblyItemId& theItemId,
                                                           Standard_Integer theSubshapeIndex,
                                                           Standard_Boolean theDelIfOrphan)
{
  DataLabel anAnnotatedItem = FindAnnotatedItemSubshape(theItemId, theSubshapeIndex);
  if (anAnnotatedItem.IsNull())
    return Standard_False;

  Handle(XCAFDoc_GraphNode) aChild;
  if (!anAnnotatedItem.FindAttribute(XCAFDoc1::NoteRefGUID(), aChild))
    return Standard_False;

  while (aChild->NbFathers() > 0)
  {
    Handle(XCAFDoc_GraphNode) aFather = aChild->GetFather(1);
    Handle(XCAFDoc_Note)      aNote   = XCAFDoc_Note::Get(aFather->Label());
    if (!aNote.IsNull())
    {
      aFather->UnSetChild(aChild);
      if (theDelIfOrphan && aNote->IsOrphan())
        DeleteNote(aFather->Label());
    }
  }

  anAnnotatedItem.ForgetAllAttributes();

  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::RemoveAllAttrNotes(const AssemblyItemId& theItemId,
                                                       const Standard_GUID&          theGUID,
                                                       Standard_Boolean              theDelIfOrphan)
{
  DataLabel anAnnotatedItem = FindAnnotatedItemAttr(theItemId, theGUID);
  if (anAnnotatedItem.IsNull())
    return Standard_False;

  Handle(XCAFDoc_GraphNode) aChild;
  if (!anAnnotatedItem.FindAttribute(XCAFDoc1::NoteRefGUID(), aChild))
    return Standard_False;

  while (aChild->NbFathers() > 0)
  {
    Handle(XCAFDoc_GraphNode) aFather = aChild->GetFather(1);
    Handle(XCAFDoc_Note)      aNote   = XCAFDoc_Note::Get(aFather->Label());
    if (!aNote.IsNull())
    {
      aFather->UnSetChild(aChild);
      if (theDelIfOrphan && aNote->IsOrphan())
        DeleteNote(aFather->Label());
    }
  }

  anAnnotatedItem.ForgetAllAttributes();

  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::RemoveAllAttrNotes(const DataLabel&     theItemLabel,
                                                       const Standard_GUID& theGUID,
                                                       Standard_Boolean     theDelIfOrphan)
{
  return RemoveAllAttrNotes(labeledItem(theItemLabel), theGUID, theDelIfOrphan);
}

//=================================================================================================

Standard_Boolean XCAFDoc_NotesTool::DeleteNote(const DataLabel& theNoteLabel)
{
  Handle(XCAFDoc_Note) aNote = XCAFDoc_Note::Get(theNoteLabel);
  if (!aNote.IsNull())
  {
    Handle(XCAFDoc_GraphNode) aFather;
    if (theNoteLabel.FindAttribute(XCAFDoc1::NoteRefGUID(), aFather) && !aFather.IsNull())
    {
      while (aFather->NbChildren() > 0)
      {
        Handle(XCAFDoc_GraphNode) aChild = aFather->GetChild(1);
        aFather->UnSetChild(aChild);
        if (aChild->NbFathers() == 0)
          aChild->Label().ForgetAllAttributes(Standard_True);
      }
    }
    theNoteLabel.ForgetAllAttributes(Standard_True);
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Integer XCAFDoc_NotesTool::DeleteNotes(TDF_LabelSequence& theNoteLabels)
{
  Standard_Integer nbNotes = 0;
  for (TDF_LabelSequence::Iterator anIter(theNoteLabels); anIter.More(); anIter.Next())
  {
    if (DeleteNote(anIter.Value()))
      ++nbNotes;
  }
  return nbNotes;
}

//=================================================================================================

Standard_Integer XCAFDoc_NotesTool::DeleteAllNotes()
{
  Standard_Integer nbNotes = 0;
  for (ChildIterator anIter(GetNotesLabel()); anIter.More(); anIter.Next())
  {
    if (DeleteNote(anIter.Value()))
      ++nbNotes;
  }
  return nbNotes;
}

//=================================================================================================

Standard_Integer XCAFDoc_NotesTool::NbOrphanNotes() const
{
  Standard_Integer nbNotes = 0;
  for (ChildIterator anIter(GetNotesLabel()); anIter.More(); anIter.Next())
  {
    const DataLabel      aLabel = anIter.Value();
    Handle(XCAFDoc_Note) aNote  = XCAFDoc_Note::Get(aLabel);
    if (!aNote.IsNull() && aNote->IsOrphan())
      ++nbNotes;
  }
  return nbNotes;
}

//=================================================================================================

void XCAFDoc_NotesTool::GetOrphanNotes(TDF_LabelSequence& theNoteLabels) const
{
  for (ChildIterator anIter(GetNotesLabel()); anIter.More(); anIter.Next())
  {
    const DataLabel      aLabel = anIter.Value();
    Handle(XCAFDoc_Note) aNote  = XCAFDoc_Note::Get(aLabel);
    if (!aNote.IsNull() && aNote->IsOrphan())
      theNoteLabels.Append(aLabel);
  }
}

//=================================================================================================

Standard_Integer XCAFDoc_NotesTool::DeleteOrphanNotes()
{
  Standard_Integer nbNotes = 0;
  for (ChildIterator anIter(GetNotesLabel()); anIter.More(); anIter.Next())
  {
    const DataLabel      aLabel = anIter.Value();
    Handle(XCAFDoc_Note) aNote  = XCAFDoc_Note::Get(aLabel);
    if (!aNote.IsNull() && aNote->IsOrphan() && DeleteNote(aLabel))
      ++nbNotes;
  }
  return nbNotes;
}

//=================================================================================================

const Standard_GUID& XCAFDoc_NotesTool::ID() const
{
  return GetID();
}

//=================================================================================================

Standard_OStream& XCAFDoc_NotesTool::Dump(Standard_OStream& theOS) const
{
  theOS << "Notes           : " << NbNotes() << "\n"
        << "Annotated items : " << NbAnnotatedItems() << "\n";
  return theOS;
}
