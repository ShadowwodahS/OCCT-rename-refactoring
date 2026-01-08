// Copyright (c) 2021 OPEN CASCADE SAS
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

#include <RWMesh.hxx>

#include <TDataStd_Name.hxx>
#include <TDF_Tool.hxx>

//=================================================================================================

AsciiString1 RWMesh1::ReadNameAttribute(const DataLabel& theLabel)
{
  Handle(NameAttribute) aNodeName;
  return theLabel.FindAttribute(NameAttribute::GetID(), aNodeName)
           ? AsciiString1(aNodeName->Get())
           : AsciiString1();
}

//=================================================================================================

AsciiString1 RWMesh1::FormatName(RWMesh_NameFormat theFormat,
                                           const DataLabel&  theLabel,
                                           const DataLabel&  theRefLabel)
{
  switch (theFormat)
  {
    case RWMesh_NameFormat_Empty: {
      return AsciiString1();
    }
    case RWMesh_NameFormat_Product: {
      Handle(NameAttribute) aRefNodeName;
      return theRefLabel.FindAttribute(NameAttribute::GetID(), aRefNodeName)
               ? AsciiString1(aRefNodeName->Get())
               : AsciiString1();
    }
    case RWMesh_NameFormat_Instance: {
      Handle(NameAttribute) aNodeName;
      return theLabel.FindAttribute(NameAttribute::GetID(), aNodeName)
               ? AsciiString1(aNodeName->Get())
               : AsciiString1();
    }
    case RWMesh_NameFormat_InstanceOrProduct: {
      Handle(NameAttribute) aNodeName;
      if (theLabel.FindAttribute(NameAttribute::GetID(), aNodeName) && !aNodeName->Get().IsEmpty())
      {
        return AsciiString1(aNodeName->Get());
      }

      Handle(NameAttribute) aRefNodeName;
      return theRefLabel.FindAttribute(NameAttribute::GetID(), aRefNodeName)
               ? AsciiString1(aRefNodeName->Get())
               : AsciiString1();
    }
    case RWMesh_NameFormat_ProductOrInstance: {
      Handle(NameAttribute) aRefNodeName;
      if (theRefLabel.FindAttribute(NameAttribute::GetID(), aRefNodeName)
          && !aRefNodeName->Get().IsEmpty())
      {
        return AsciiString1(aRefNodeName->Get());
      }

      Handle(NameAttribute) aNodeName;
      return theLabel.FindAttribute(NameAttribute::GetID(), aNodeName)
               ? AsciiString1(aNodeName->Get())
               : AsciiString1();
    }
    case RWMesh_NameFormat_ProductAndInstance: {
      const AsciiString1 anInstName = ReadNameAttribute(theLabel);
      const AsciiString1 aProdName  = ReadNameAttribute(theRefLabel);
      return !anInstName.IsEmpty() && aProdName != anInstName
               ? aProdName + " [" + anInstName + "]"
               : (!aProdName.IsEmpty() ? aProdName : AsciiString1(""));
    }
    case RWMesh_NameFormat_ProductAndInstanceAndOcaf: {
      const AsciiString1 anInstName = ReadNameAttribute(theLabel);
      const AsciiString1 aProdName  = ReadNameAttribute(theRefLabel);
      AsciiString1       anEntryId;
      Tool3::Entry(theLabel, anEntryId);
      return !anInstName.IsEmpty() && aProdName != anInstName
               ? aProdName + " [" + anInstName + "]" + " [" + anEntryId + "]"
               : aProdName + " [" + anEntryId + "]";
    }
  }

  return AsciiString1();
}
