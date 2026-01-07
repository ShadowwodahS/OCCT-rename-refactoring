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

//      	------------
// Version:	0.0
// Version	Date		Purpose
//		0.0	Mar 13 1997	Creation

#include <TCollection_AsciiString.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Data.hxx>
#include <TDF_DataSet.hxx>
#include <TDF_IDFilter.hxx>
#include <TDF_Label.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>
#include <TDF_MapIteratorOfAttributeMap.hxx>
#include <TDF_MapIteratorOfLabelMap.hxx>
#include <TDF_Tool.hxx>

#define TDF_TagSeparator ':'

static void TDF_Tool_ExtendedDeepDump(Standard_OStream&        anOS,
                                      const DataLabel&         aLabel,
                                      const IDFilter&      aFilter,
                                      TDF_AttributeIndexedMap& aMap);

static Standard_Boolean TDF_Tool_DescendantRef(const DataLabel&           aRefLabel,
                                               const DataLabel&           aLabel,
                                               const IDFilter&        aFilter,
                                               const Handle(TDF_DataSet)& ds);

static void TDF_Tool_OutReferers(const DataLabel&           aRefLabel,
                                 const DataLabel&           aLabel,
                                 TDF_AttributeMap&          atts,
                                 const IDFilter&        aFilterForReferers,
                                 const IDFilter&        aFilterForReferences,
                                 const Handle(TDF_DataSet)& ds);

static void TDF_Tool_OutReferences(const DataLabel&           aRefLabel,
                                   const DataLabel&           aLabel,
                                   TDF_AttributeMap&          atts,
                                   const IDFilter&        aFilterForReferers,
                                   const IDFilter&        aFilterForReferences,
                                   const Handle(TDF_DataSet)& ds);

//=======================================================================
// function : NbLabels
// purpose  : Returns the numbers of labels of the tree.
//=======================================================================

Standard_Integer Tool3::NbLabels(const DataLabel& aLabel)
{
  Standard_Integer n = 1;
  for (ChildIterator itr(aLabel, Standard_True); itr.More(); itr.Next())
    ++n;
  return n;
}

//=======================================================================
// function : NbAttributes
// purpose  : Returns the number of attributes of the tree.
//=======================================================================

Standard_Integer Tool3::NbAttributes(const DataLabel& aLabel)
{
  Standard_Integer n = aLabel.NbAttributes();
  for (ChildIterator itr(aLabel, Standard_True); itr.More(); itr.Next())
    n += itr.Value().NbAttributes();
  return n;
}

//=======================================================================
// function : NbAttributes
// purpose  : Returns the number of attributes of the tree,
//           selected by an IDFilter.
//=======================================================================

Standard_Integer Tool3::NbAttributes(const DataLabel& aLabel, const IDFilter& aFilter)
{
  Standard_Integer      n = 0;
  TDF_AttributeIterator it2;
  for (it2.Initialize(aLabel, Standard_True); it2.More(); it2.Next())
    if (aFilter.IsKept(it2.Value()))
      ++n;
  for (ChildIterator it1(aLabel, Standard_True); it1.More(); it1.Next())
    for (it2.Initialize(it1.Value(), Standard_True); it2.More(); it2.Next())
      if (aFilter.IsKept(it2.Value()))
        ++n;
  return n;
}

//=================================================================================================

Standard_Boolean Tool3::IsSelfContained(const DataLabel& aLabel)
{
  IDFilter filter(Standard_False); // Keep all.
  return IsSelfContained(aLabel, filter);
}

//=================================================================================================

Standard_Boolean Tool3::IsSelfContained(const DataLabel& aLabel, const IDFilter& aFilter)
{
  Handle(TDF_DataSet) ds = new TDF_DataSet();

  if (!TDF_Tool_DescendantRef(aLabel, aLabel, aFilter, ds))
    return Standard_False;

  for (ChildIterator itr(aLabel, Standard_True); itr.More(); itr.Next())
  {
    if (!TDF_Tool_DescendantRef(aLabel, itr.Value(), aFilter, ds))
      return Standard_False;
  }
  return Standard_True;
}

//=================================================================================================

static Standard_Boolean TDF_Tool_DescendantRef(const DataLabel&           aRefLabel,
                                               const DataLabel&           aLabel,
                                               const IDFilter&        aFilter,
                                               const Handle(TDF_DataSet)& ds)
{
  for (TDF_AttributeIterator itr(aLabel); itr.More(); itr.Next())
  {
    // CLE
    // const Handle(TDF_Attribute)& labAtt = itr.Value();
    Handle(TDF_Attribute) labAtt = itr.Value();
    // ENDCLE
    if (aFilter.IsKept(labAtt))
    {
      labAtt->References(ds);
      // First of all, the referenced labels.
      const TDF_LabelMap& labMap = ds->Labels();

      for (TDF_MapIteratorOfLabelMap labMItr(labMap); labMItr.More(); labMItr.Next())
      {
        if (!labMItr.Key().IsDescendant(aRefLabel))
          return Standard_False;
      }
      // Then the referenced attributes.
      const TDF_AttributeMap& attMap = ds->Attributes();
      for (TDF_MapIteratorOfAttributeMap attMItr(attMap); attMItr.More(); attMItr.Next())
      {
        // CLE
        // const Handle(TDF_Attribute)& att = attMItr.Key();
        const Handle(TDF_Attribute)& att = attMItr.Key();
        if (!att.IsNull() && !att->Label().IsNull())
        {
          // ENDCLE
          if (aFilter.IsKept(att) && !att->Label().IsDescendant(aRefLabel))
            return Standard_False;
        }
      }
      ds->Clear();
    }
  }
  return Standard_True;
}

//=================================================================================================

void Tool3::OutReferers(const DataLabel& aLabel, TDF_AttributeMap& atts)
{
  IDFilter filter(Standard_False); // Keep all.
  OutReferers(aLabel, filter, filter, atts);
}

//=================================================================================================

void Tool3::OutReferers(const DataLabel&    aLabel,
                           const IDFilter& aFilterForReferers,
                           const IDFilter& aFilterForReferences,
                           TDF_AttributeMap&   atts)
{
  Handle(TDF_DataSet) ds = new TDF_DataSet();
  TDF_Tool_OutReferers(aLabel, aLabel, atts, aFilterForReferers, aFilterForReferences, ds);
  for (ChildIterator itr(aLabel, Standard_True); itr.More(); itr.Next())
  {
    TDF_Tool_OutReferers(aLabel, itr.Value(), atts, aFilterForReferers, aFilterForReferences, ds);
  }
}

//=================================================================================================

static void TDF_Tool_OutReferers(const DataLabel&           aRefLabel,
                                 const DataLabel&           aLabel,
                                 TDF_AttributeMap&          atts,
                                 const IDFilter&        aFilterForReferers,
                                 const IDFilter&        aFilterForReferences,
                                 const Handle(TDF_DataSet)& ds)
{
  Standard_Boolean outRefFound = Standard_False;

  for (TDF_AttributeIterator itr(aLabel); itr.More(); itr.Next())
  {

    if (!aFilterForReferers.IsKept(itr.Value()))
      continue;
    itr.Value()->References(ds);

    const TDF_AttributeMap& attMap = ds->Attributes();
    for (TDF_MapIteratorOfAttributeMap attMItr(attMap); attMItr.More(); attMItr.Next())
    {
      // CLE
      // const Handle(TDF_Attribute)& att = attMItr.Key();
      const Handle(TDF_Attribute)& att = attMItr.Key();
      // ENDCLE
      if (aFilterForReferences.IsKept(att) && !att->Label().IsNull()
          && !att->Label().IsDescendant(aRefLabel))
      {
        atts.Add(itr.Value());
        outRefFound = Standard_True;
        break;
      }
    }

    if (!outRefFound)
    {
      const TDF_LabelMap& labMap = ds->Labels();
      for (TDF_MapIteratorOfLabelMap labMItr(labMap); labMItr.More(); labMItr.Next())
      {
        if (!labMItr.Key().IsDescendant(aRefLabel))
        {
          atts.Add(itr.Value());
          break;
        }
      }
    }

    outRefFound = Standard_False;
    ds->Clear();
  }
}

//=================================================================================================

void Tool3::OutReferences(const DataLabel& aLabel, TDF_AttributeMap& atts)
{
  IDFilter filter(Standard_False); // Keep all.
  OutReferences(aLabel, filter, filter, atts);
}

//=================================================================================================

void Tool3::OutReferences(const DataLabel&    aLabel,
                             const IDFilter& aFilterForReferers,
                             const IDFilter& aFilterForReferences,
                             TDF_AttributeMap&   atts)
{
  Handle(TDF_DataSet) ds = new TDF_DataSet();
  TDF_Tool_OutReferences(aLabel, aLabel, atts, aFilterForReferers, aFilterForReferences, ds);
  for (ChildIterator itr(aLabel, Standard_True); itr.More(); itr.Next())
  {
    TDF_Tool_OutReferences(aLabel, itr.Value(), atts, aFilterForReferers, aFilterForReferences, ds);
  }
}

//=================================================================================================

static void TDF_Tool_OutReferences(const DataLabel&           aRefLabel,
                                   const DataLabel&           aLabel,
                                   TDF_AttributeMap&          atts,
                                   const IDFilter&        aFilterForReferers,
                                   const IDFilter&        aFilterForReferences,
                                   const Handle(TDF_DataSet)& ds)
{
  for (TDF_AttributeIterator itr(aLabel); itr.More(); itr.Next())
  {
    if (!aFilterForReferers.IsKept(itr.Value()))
      continue;
    itr.Value()->References(ds);
    const TDF_AttributeMap& attMap = ds->Attributes();
    for (TDF_MapIteratorOfAttributeMap attMItr(attMap); attMItr.More(); attMItr.Next())
    {
      const Handle(TDF_Attribute)& att = attMItr.Key();
      if (aFilterForReferences.IsKept(att) && !att->Label().IsNull()
          && !att->Label().IsDescendant(aRefLabel))
      {
        atts.Add(att);
      }
    }
    const TDF_LabelMap& labMap = ds->Labels();
    for (TDF_MapIteratorOfLabelMap labMItr(labMap); labMItr.More(); labMItr.Next())
    {
      if (!labMItr.Key().IsDescendant(aRefLabel))
      {
        TDF_AttributeIterator itra(labMItr.Key());
        for (; itra.More(); itra.Next())
        {
          if (aFilterForReferences.IsKept(itra.Value()))
          {
            atts.Add(itra.Value());
          }
        }
      }
    }
  }
  ds->Clear();
}

//=================================================================================================

void Tool3::RelocateLabel(const DataLabel&       aSourceLabel,
                             const DataLabel&       fromRoot,
                             const DataLabel&       toRoot,
                             DataLabel&             aTargetLabel,
                             const Standard_Boolean create)
{
  if (!aSourceLabel.IsDescendant(fromRoot))
    return;
  aTargetLabel.Nullify();
  TColStd_ListOfInteger labelTags;
  Tool3::TagList(aSourceLabel, labelTags);
  TColStd_ListOfInteger toTags;
  Tool3::TagList(toRoot, toTags);
  for (Standard_Integer i = fromRoot.Depth(); i >= 0; --i)
    labelTags.RemoveFirst();
  labelTags.Prepend(toTags);
  Tool3::Label(toRoot.Data(), labelTags, aTargetLabel, create);
}

//=======================================================================
// function : Entry
// purpose  : Returns the entry as an ascii string.
//=======================================================================

void Tool3::Entry(const DataLabel& aLabel, AsciiString1& anEntry)
{
  if (!aLabel.IsNull())
  {
    int       aStrLen = 1; // initial "0" of a root label
    DataLabel aLab    = aLabel;
    for (; !aLab.IsRoot(); aLab = aLab.Father())
    {
      for (int aTag = aLab.Tag(); aTag > 9; aTag /= 10)
        ++aStrLen;
      aStrLen += 2; // one digit and separator
    }

    if (aStrLen == 1)
    {
      // an exceptional case for the root label, it ends with separator
      static const AsciiString1 THE_ROOT_ENTRY =
        AsciiString1('0') + TDF_TagSeparator;
      anEntry = THE_ROOT_ENTRY;
    }
    else
    {
      anEntry                  = AsciiString1(aStrLen, TDF_TagSeparator);
      Standard_Character* aPtr = const_cast<Standard_Character*>(anEntry.ToCString() + aStrLen - 1);
      for (aLab = aLabel; !aLab.IsRoot(); aLab = aLab.Father())
      {
        int aTag = aLab.Tag();
        for (; aTag > 9; --aPtr, aTag /= 10)
          *aPtr = Standard_Character(aTag % 10) + '0';
        *aPtr = Standard_Character(aTag) + '0';
        aPtr -= 2;
      }
      *aPtr = '0';
    }
  }
  else
    anEntry.Clear();
}

//=======================================================================
// function : TagList
// purpose  : Returns the entry of a label as a list of integers.
//=======================================================================

void Tool3::TagList(const DataLabel& aLabel, TColStd_ListOfInteger& aTagList)
{
  aTagList.Clear();
  if (!aLabel.IsNull())
  {
    DataLabel Label = aLabel;
    for (;;)
    {
      aTagList.Prepend(Label.Tag());
      if (Label.IsRoot())
        break;
      Label = Label.Father();
    }
  }
}

//=======================================================================
// function : TagList
// purpose  : Returns the entry expressed as a string as a list of integers.
//=======================================================================

void Tool3::TagList(const AsciiString1& anEntry, TColStd_ListOfInteger& aTagList)
{
  char*            cc = (char*)anEntry.ToCString();
  Standard_Integer n  = 0;
  aTagList.Clear();
  while (*cc != '\0')
  {
    while (*cc >= '0' && *cc <= '9')
    {
      n = 10 * n + (*cc - '0');
      ++cc;
    }
    if (*cc == TDF_TagSeparator || *cc == '\0')
    {
      aTagList.Append(n);
      n = 0;
      if (*cc != '\0')
        ++cc;
    }
    else
    { // Not an entry!
      aTagList.Clear();
      break;
    }
  }
}

//=======================================================================
// function : Label
// purpose  : Returns the label expressed by <anEntry>.
//=======================================================================

void Tool3::Label(const Handle(TDF_Data)&        aDF,
                     const AsciiString1& anEntry,
                     DataLabel&                     aLabel,
                     const Standard_Boolean         create)
{
  Standard_Boolean isFound = Standard_False;
  if (aDF->IsAccessByEntries())
    isFound = aDF->GetLabel(anEntry, aLabel);

  if (!isFound)
    Tool3::Label(aDF, anEntry.ToCString(), aLabel, create);
}

//=======================================================================
// function : Label
// purpose  : Returns the label expressed by <anEntry>,
//           and creates it if <create> is true.
//=======================================================================

void Tool3::Label(const Handle(TDF_Data)& aDF,
                     const Standard_CString  anEntry,
                     DataLabel&              aLabel,
                     const Standard_Boolean  create)
{
  Standard_Boolean isFound = Standard_False;
  if (aDF->IsAccessByEntries())
    isFound = aDF->GetLabel(anEntry, aLabel);

  if (!isFound)
  {
    TColStd_ListOfInteger tagList;
    Tool3::TagList(anEntry, tagList);
    Tool3::Label(aDF, tagList, aLabel, create);
  }
}

//=======================================================================
// function : Label
// purpose  : Returns the label expressed by <anEntry>,
//           and creates it if <create> is true.
//=======================================================================

void Tool3::Label(const Handle(TDF_Data)&      aDF,
                     const TColStd_ListOfInteger& aTagList,
                     DataLabel&                   aLabel,
                     const Standard_Boolean       create)
{
  if (aTagList.Extent() == 0)
  {
    aLabel.Nullify();
  }
  else
  {
    aLabel = aDF->Root();
    if (aTagList.Extent() == 1 && aTagList.First() == 0)
      return;
    else
    {
      TColStd_ListIteratorOfListOfInteger tagItr(aTagList);
      tagItr.Next(); // Suppresses root tag.
      for (; !aLabel.IsNull() && tagItr.More(); tagItr.Next())
      {
        aLabel = aLabel.FindChild(tagItr.Value(), create);
      }
    }
  }
}

//=================================================================================================

void Tool3::CountLabels(TDF_LabelList& aLabelList, TDF_LabelIntegerMap& aLabelMap)
{
  if (aLabelList.IsEmpty())
    return;
  Standard_Boolean            next = Standard_True;
  TDF_ListIteratorOfLabelList itr(aLabelList);
  while (itr.More())
  {
    const DataLabel& lab = itr.Value();
    if (aLabelMap.IsBound(lab))
    {
      aLabelMap(lab) += 1;
      aLabelList.Remove(itr);
      next = Standard_False;
    }
    else
    {
      aLabelMap.Bind(lab, 1);
      next = itr.More();
    }
    if (next && !aLabelList.IsEmpty())
      itr.Next();
  }
}

//=================================================================================================

void Tool3::DeductLabels(TDF_LabelList& aLabelList, TDF_LabelIntegerMap& aLabelMap)
{
  if (aLabelList.IsEmpty())
    return;
  Standard_Boolean            next = Standard_True;
  TDF_ListIteratorOfLabelList itr(aLabelList);
  while (itr.More())
  {
    const DataLabel& lab = itr.Value();
    if (aLabelMap.IsBound(lab))
    {
      aLabelMap(lab) -= 1;
      if (aLabelMap(lab) == 0)
      {
        aLabelMap.UnBind(lab);
        aLabelList.Remove(itr);
        next = Standard_False;
      }
    }
    else
      next = itr.More();
    if (next && !aLabelList.IsEmpty())
      itr.Next();
  }
}

//=======================================================================
// function : DeepDump
// purpose  : Deep dump of a DF.
//=======================================================================

void Tool3::DeepDump(Standard_OStream& anOS, const Handle(TDF_Data)& aDF)
{
  anOS << aDF;
  Tool3::DeepDump(anOS, aDF->Root());
}

//=======================================================================
// function : ExtendedDeepDump
// purpose  : Extended deep dump of a DF.
//=======================================================================

void Tool3::ExtendedDeepDump(Standard_OStream&       anOS,
                                const Handle(TDF_Data)& aDF,
                                const IDFilter&     aFilter)
{
  anOS << aDF;
  Tool3::ExtendedDeepDump(anOS, aDF->Root(), aFilter);
}

//=======================================================================
// function : DeepDump
// purpose  : Deep dump of a label.
//=======================================================================

void Tool3::DeepDump(Standard_OStream& anOS, const DataLabel& aLabel)
{
  // Dumps the label.
  anOS << aLabel;
  // Its children
  for (ChildIterator ChildIt(aLabel); ChildIt.More(); ChildIt.Next())
  {
    Tool3::DeepDump(anOS, ChildIt.Value());
  }
}

//=======================================================================
// function : ExtendedDeepDump
// purpose  : Extended deep dump of a label.
//=======================================================================

void Tool3::ExtendedDeepDump(Standard_OStream&   anOS,
                                const DataLabel&    aLabel,
                                const IDFilter& aFilter)
{
  TDF_AttributeIndexedMap map;
  TDF_Tool_ExtendedDeepDump(anOS, aLabel, aFilter, map);

  anOS << map.Extent() << " attribute";
  if (map.Extent() > 1)
    anOS << "s";
  anOS << " referenced by the label structure." << std::endl;

  anOS << std::endl << "Extended dump of filtered attribute(s):" << std::endl;
  Standard_Integer        nba = 0;
  AsciiString1 entry;
  Standard_Integer        i;
  for (i = 1; i <= map.Extent(); ++i)
  {
    const Handle(TDF_Attribute)& att = map.FindKey(i);
    if (aFilter.IsKept(att))
    {
      ++nba;
      anOS << "# " << i;
      if (att->Label().IsNull())
      {
        anOS << " (no label)" << std::endl;
      }
      else
      {
        Tool3::Entry(att->Label(), entry);
        anOS << " (label: " << entry << ")" << std::endl;
      }
      att->ExtendedDump(anOS, aFilter, map);
      anOS << std::endl;
    }
  }
  anOS << std::endl << nba << " attribute";
  if (nba > 1)
    anOS << "s";
  anOS << " dumped between " << --i << std::endl;
}

//=======================================================================
// function : ExtendedDeepDump
// purpose  : Internal method.
//=======================================================================

static void TDF_Tool_ExtendedDeepDump(Standard_OStream&        anOS,
                                      const DataLabel&         aLabel,
                                      const IDFilter&      aFilter,
                                      TDF_AttributeIndexedMap& aMap)
{
  // Dumps the label.
  aLabel.ExtendedDump(anOS, aFilter, aMap);
  // Its children
  for (ChildIterator ChildIt(aLabel); ChildIt.More(); ChildIt.Next())
  {
    TDF_Tool_ExtendedDeepDump(anOS, ChildIt.Value(), aFilter, aMap);
  }
}
