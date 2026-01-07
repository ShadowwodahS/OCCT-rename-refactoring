// Created on: 1999-06-24
// Created by: Sergey ZARITCHNY
// Copyright (c) 1999-1999 Matra Datavision
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

#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_ClosureMode.hxx>
#include <TDF_ClosureTool.hxx>
#include <TDF_CopyLabel.hxx>
#include <TDF_CopyTool.hxx>
#include <TDF_Data.hxx>
#include <TDF_DataSet.hxx>
#include <TDF_IDFilter.hxx>
#include <TDF_Label.hxx>
#include <TDF_RelocationTable.hxx>
#include <TDF_Tool.hxx>

// The bug concerns the COPY operation of some attributes of a non-self-contained label.
// The attributes making the label non-self-contained are not involved by the operation.
// Therefore, these attributes shouldn't be considered by the COPY mechanism and
// the label should be considered as a self-contained.
// Correction of the bug consists of ignoring the attributes not involved by the COPY operation.
//=================================================================================================

TDF_CopyLabel::TDF_CopyLabel()
    : myFilter(Standard_False),
      myIsDone(Standard_False)
{
  mySL.Nullify();
  myTL.Nullify();
}

//=================================================================================================

TDF_CopyLabel::TDF_CopyLabel(const DataLabel& aSource, const DataLabel& aTarget)
    : myFilter(Standard_False),
      myIsDone(Standard_False)
{
  mySL = aSource;
  myTL = aTarget;
}

//=================================================================================================

void TDF_CopyLabel::Load(const DataLabel& aSource, const DataLabel& aTarget)
{
  mySL = aSource;
  myTL = aTarget;
}

//=================================================================================================

void TDF_CopyLabel::ExternalReferences(const DataLabel&     aRefLabel,
                                       const DataLabel&     aLabel,
                                       TDF_AttributeMap&    aExternals,
                                       const IDFilter&  aFilter,
                                       Handle(TDF_DataSet)& ds)
{
  //  AsciiString1 entr1,entr2; //d
  for (TDF_AttributeIterator itr(aLabel); itr.More(); itr.Next())
  {
    itr.Value()->References(ds);
    const TDF_AttributeMap& attMap = ds->Attributes(); // attMap
    //     Tool3::Entry(itr.Value()->Label(), entr1);  //d
    //     std::cout<<"\tSource Attribute dynamic type = "<<itr.Value()->DynamicType()<<" Label =
    //     "<<entr1 <<std::endl;
    for (TDF_MapIteratorOfAttributeMap attMItr(attMap); attMItr.More(); attMItr.Next())
    {
      const Handle(TDF_Attribute)& att = attMItr.Key();

      //       Tool3::Entry(att->Label(), entr1);
      //       std::cout<<"\t\tReferences attribute dynamic type = "<<att->DynamicType()<<" Label =
      //       "<<entr1 <<std::endl;
      if (!att.IsNull() && !att->Label().IsNull())
      {
        if (aFilter.IsKept(att) && att->Label().IsDifferent(aRefLabel)
            && !att->Label().IsDescendant(aRefLabel))
        {
          aExternals.Add(att);
        }
      }
    }

    //     const TDF_LabelMap& labMap = ds->Labels();
    //     for (TDF_MapIteratorOfLabelMap labMItr(labMap);labMItr.More(); labMItr.Next()) {
    //       Tool3::Entry(labMItr.Key(), entr1);
    // 	std::cout<<"\t\tLABELS from DS of Attr:: Lab = "<<entr1<<std::endl;
    //       if (!labMItr.Key().IsDescendant(aRefLabel) && labMItr.Key().IsDifferent(aRefLabel)) {
    // //	aExternals.Add(itr.Value()); // ??? LabelMap of Attribute has label which don't
    // 	// belongs to source hierarchy. So, what we should do ?
    // 	// Add this Attribute to the aExternals or add all attributes
    // 	// from this label ?
    // 	AsciiString1 entr1, entr2;
    // 	Tool3::Entry(labMItr.Key(), entr1);
    // 	Tool3::Entry(aRefLabel, entr2);
    // 	std::cout<<"\t\t\tNot descendant label:: Lab1 = "<<entr1<<" and RefLab =
    // "<<entr2<<std::endl;
    //       }
    //     }

    ds->Clear();
  }
}

//=================================================================================================

Standard_Boolean TDF_CopyLabel::ExternalReferences(const DataLabel&    L,
                                                   TDF_AttributeMap&   aExternals,
                                                   const IDFilter& aFilter)
{
  Handle(TDF_DataSet) ds = new TDF_DataSet();
  ExternalReferences(L, L, aExternals, aFilter, ds);
  for (ChildIterator itr(L, Standard_True); itr.More(); itr.Next())
  {
    ExternalReferences(L, itr.Value(), aExternals, aFilter, ds);
  }
  if (!aExternals.Extent())
    return Standard_False;
  else
    return Standard_True;
}

//=======================================================================
#ifdef OCCT_DEBUG
static void PrintEntry(const DataLabel& label, const Standard_Boolean allLevels)
{
  AsciiString1 entry;
  Tool3::Entry(label, entry);
  std::cout << "\tShareable attribute on the label = " << entry << std::endl;
  ChildIterator it(label, allLevels);
  for (; it.More(); it.Next())
  {
    Tool3::Entry(it.Value(), entry);
    std::cout << "\tChildLabelEntry = " << entry << std::endl;
  }
}
#endif
//=================================================================================================

void TDF_CopyLabel::Perform()
{
  myIsDone = Standard_False;
  if (mySL.Data()->Root().IsDifferent(myTL.Data()->Root()) && // TDF_Data is not the same
                                                              // clang-format off
     !Tool3::IsSelfContained(mySL, myFilter)) return;               //source label isn't self-contained
  // clang-format on

  Standard_Boolean extReferers = ExternalReferences(mySL, myMapOfExt, myFilter);

  myRT                   = new TDF_RelocationTable(Standard_True);
  Handle(TDF_DataSet) ds = new TDF_DataSet();
  ClosureMode     mode(Standard_True); // descendant plus reference
  ds->AddLabel(mySL);
  myRT->SetRelocation(mySL, myTL);
  ClosureTool::Closure(ds, myFilter, mode);
  if (extReferers)
  {
    for (TDF_MapIteratorOfAttributeMap attMItr(myMapOfExt); attMItr.More(); attMItr.Next())
    {
      const Handle(TDF_Attribute)& att = attMItr.Key();
      myRT->SetRelocation(att, att);
#ifdef OCCT_DEBUG
      PrintEntry(att->Label(), Standard_True);
#endif
    }
  }

  CopyTool::Copy(ds, myRT);
  myIsDone = Standard_True;
}

//=================================================================================================

const Handle(TDF_RelocationTable)& TDF_CopyLabel::RelocationTable() const
{
  return myRT;
}

//=================================================================================================

void TDF_CopyLabel::UseFilter(const IDFilter& aFilter)
{
  myFilter.Assign(aFilter);
}
