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

//      	-----------------------
// Version:	0.0
// Version	Date		Purpose
//		0.0	Mar  7 1997	Creation

#include <Standard_Transient.hxx>
#include <TDF_RelocationTable.hxx>

IMPLEMENT_STANDARD_RTTIEXT(RelocationTable1, RefObject)

//=================================================================================================

RelocationTable1::RelocationTable1(const Standard_Boolean selfRelocate)
    : mySelfRelocate(selfRelocate),
      myAfterRelocate(Standard_False)
{
}

//=================================================================================================

void RelocationTable1::SelfRelocate(const Standard_Boolean selfRelocate)
{
  mySelfRelocate = selfRelocate;
}

//=================================================================================================

Standard_Boolean RelocationTable1::SelfRelocate() const
{
  return mySelfRelocate;
}

//=================================================================================================

void RelocationTable1::AfterRelocate(const Standard_Boolean afterRelocate)
{
  myAfterRelocate = afterRelocate;
}

//=================================================================================================

Standard_Boolean RelocationTable1::AfterRelocate() const
{
  return myAfterRelocate;
}

//=======================================================================
// function : SetRelocation
// purpose  : Sets the relocation value of <aSourceLabel>
//           to <aTargetLabel>.
//=======================================================================

void RelocationTable1::SetRelocation(const DataLabel& aSourceLabel,
                                        const DataLabel& aTargetLabel)
{
  if (!myLabelTable.IsBound(aSourceLabel))
    myLabelTable.Bind(aSourceLabel, aTargetLabel);
}

//=======================================================================
// function : HasRelocation
// purpose  : Finds the relocation value of <aSourceLabel>
//           and returns it into <aTargetLabel>.
//=======================================================================

Standard_Boolean RelocationTable1::HasRelocation(const DataLabel& aSourceLabel,
                                                    DataLabel&       aTargetLabel) const
{
  aTargetLabel.Nullify();
  if (myLabelTable.IsBound(aSourceLabel))
  {
    aTargetLabel = myLabelTable.Find(aSourceLabel);
    return Standard_True;
  }
  if (mySelfRelocate)
  {
    aTargetLabel = aSourceLabel;
    return !myAfterRelocate;
  }
  return Standard_False;
}

//=======================================================================
// function : SetRelocation
// purpose  : Sets the relocation value of <aSourceAttribute>
//           to <aTargetAttribute>.
//=======================================================================

void RelocationTable1::SetRelocation(const Handle(TDF_Attribute)& aSourceAttribute,
                                        const Handle(TDF_Attribute)& aTargetAttribute)
{
  if (!myAttributeTable.IsBound(aSourceAttribute))
    myAttributeTable.Bind(aSourceAttribute, aTargetAttribute);
}

//=======================================================================
// function : HasRelocation
// purpose  : Finds the relocation value of <aSourceAttribute>
//           and returns it into <aTargetAttribute>.
//=======================================================================

Standard_Boolean RelocationTable1::HasRelocation(const Handle(TDF_Attribute)& aSourceAttribute,
                                                    Handle(TDF_Attribute)& aTargetAttribute) const
{
  aTargetAttribute.Nullify();
  if (myAttributeTable.IsBound(aSourceAttribute))
  {
    aTargetAttribute = myAttributeTable.Find(aSourceAttribute);
    return Standard_True;
  }
  if (mySelfRelocate)
  {
    aTargetAttribute = aSourceAttribute;
    return !myAfterRelocate;
  }
  return Standard_False;
}

//=======================================================================
// function : SetTransientRelocation
// purpose  : Sets the relocation value of <aSourceTransient>
//           to <aTargetTransient>.
//=======================================================================

void RelocationTable1::SetTransientRelocation(const Handle(RefObject)& aSourceTransient,
                                                 const Handle(RefObject)& aTargetTransient)
{
  if (!myTransientTable.Contains(aSourceTransient))
    myTransientTable.Add(aSourceTransient, aTargetTransient);
}

//=======================================================================
// function : HasTransientRelocation
// purpose  : Finds the relocation value of <aSourceTransient>
//           and returns it into <aTargetTransient>.
//=======================================================================

Standard_Boolean RelocationTable1::HasTransientRelocation(
  const Handle(RefObject)& aSourceTransient,
  Handle(RefObject)&       aTargetTransient) const
{
  aTargetTransient.Nullify();
  if (myTransientTable.Contains(aSourceTransient))
  {
    aTargetTransient = myTransientTable.FindFromKey(aSourceTransient);
    return Standard_True;
  }
  if (mySelfRelocate)
  {
    aTargetTransient = aSourceTransient;
    return !myAfterRelocate;
  }
  return Standard_False;
}

//=======================================================================
// function : Clear
// purpose  : Clears the relocation dictionary.
//=======================================================================

void RelocationTable1::Clear()
{
  myLabelTable.Clear();
  myAttributeTable.Clear();
  myTransientTable.Clear();
}

//=================================================================================================

void RelocationTable1::TargetLabelMap(TDF_LabelMap& aLabelMap) const
{
  for (TDF_DataMapIteratorOfLabelDataMap itr(myLabelTable); itr.More(); itr.Next())
    aLabelMap.Add(itr.Value());
}

//=================================================================================================

void RelocationTable1::TargetAttributeMap(TDF_AttributeMap& anAttributeMap) const
{
  for (TDF_DataMapIteratorOfAttributeDataMap itr(myAttributeTable); itr.More(); itr.Next())
    anAttributeMap.Add(itr.Value());
}

//=================================================================================================

TDF_LabelDataMap& RelocationTable1::LabelTable()
{
  return myLabelTable;
}

//=================================================================================================

TDF_AttributeDataMap& RelocationTable1::AttributeTable()
{
  return myAttributeTable;
}

//=================================================================================================

TColStd_IndexedDataMapOfTransientTransient& RelocationTable1::TransientTable()
{
  return myTransientTable;
}

//=================================================================================================

Standard_OStream& RelocationTable1::Dump(const Standard_Boolean dumpLabels,
                                            const Standard_Boolean dumpAttributes,
                                            const Standard_Boolean dumpTransients,
                                            Standard_OStream&      anOS) const
{
  anOS << "Relocation Table  ";
  if (mySelfRelocate)
    anOS << "IS";
  else
    anOS << "NOT";
  anOS << " self relocate ";
  if (myAfterRelocate)
    anOS << "WITH";
  else
    anOS << "WITHOUT";
  anOS << " after relocate" << std::endl;
  anOS << "Nb labels=" << myLabelTable.Extent();
  anOS << "  Nb attributes=" << myAttributeTable.Extent();
  anOS << "  Nb transients=" << myTransientTable.Extent() << std::endl;

  Standard_Integer nb = 0;
  if (dumpLabels)
  {
    anOS << "Label Table:" << std::endl;
    for (TDF_DataMapIteratorOfLabelDataMap itr(myLabelTable); itr.More(); itr.Next())
    {
      ++nb;
      anOS << nb << " ";
      itr.Key1().EntryDump(anOS);
      anOS << "<=>";
      itr.Value().EntryDump(anOS);
      anOS << "| ";
    }
    std::cout << std::endl;
  }

  nb = 0;
  if (dumpAttributes)
  {
    anOS << "Attribute Table:" << std::endl;
    for (TDF_DataMapIteratorOfAttributeDataMap itr(myAttributeTable); itr.More(); itr.Next())
    {
      ++nb;
      anOS << nb << " ";
      itr.Key1()->Dump(anOS);
      anOS << "<=>";
      itr.Value()->Dump(anOS);
      anOS << "| ";
      anOS << std::endl;
    }
  }

  if (dumpTransients)
  {
    anOS << "Transient Table:" << myTransientTable.Extent() << " transient(s) in table."
         << std::endl;
  }

  return anOS;
}
