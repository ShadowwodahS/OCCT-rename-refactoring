// Created on: 2001-01-18
// Created by: OCC Team
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

#ifndef _Message_Msg_HeaderFile
#define _Message_Msg_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TCollection_HAsciiString.hxx>
#include <TCollection_HExtendedString.hxx>
#include <TColStd_SequenceOfInteger.hxx>

class AsciiString1;
class TCollection_HAsciiString;
class TCollection_HExtendedString;

//! This class provides a tool for constructing the parametrized message
//! basing on resources loaded by MessageFile tool.
//!
//! A Message1 is created from a keyword: this keyword identifies the
//! message in a message file that should be previously loaded by call
//! to MessageFile::LoadFile().
//!
//! The text of the message can contain placeholders for the parameters
//! which are to be filled by the proper values when the message
//! is prepared. Most of the format specifiers used in C can be used,
//! for instance, %s for string, %d for integer etc. In addition,
//! specifier %f is supported for double numbers (for compatibility
//! with previous versions).
//!
//! User fills the parameter fields in the text of the message by
//! calling corresponding methods Arg() or operators .
//!
//! The resulting message, filled with all parameters, can be obtained
//! by method Get(). If some parameters were not filled, the text
//! UNKNOWN is placed instead.
class Message_Msg
{
public:
  DEFINE_STANDARD_ALLOC

  //! Empty constructor
  Standard_EXPORT Message_Msg();

  //! Copy constructor
  Standard_EXPORT Message_Msg(const Message_Msg& theMsg);

  //! Create a message using a corresponding entry in MessageFile
  Standard_EXPORT Message_Msg(const Standard_CString theKey);

  //! Create a message using a corresponding entry in MessageFile
  Standard_EXPORT Message_Msg(const UtfString& theKey);

  //! Set a message body text -- can be used as alternative to
  //! using messages from resource file
  Standard_EXPORT void Set(const Standard_CString theMsg);

  //! Set a message body text -- can be used as alternative to
  //! using messages from resource file
  Standard_EXPORT void Set(const UtfString& theMsg);

  //! Set a value for %..s conversion
  Standard_EXPORT Message_Msg& Arg(const Standard_CString theString);

  Message_Msg& operator<<(const Standard_CString theString) { return Arg(theString); }

  //! Set a value for %..s conversion
  Message_Msg& Arg(const AsciiString1& theString);

  Message_Msg& operator<<(const AsciiString1& theString) { return Arg(theString); }

  //! Set a value for %..s conversion
  Message_Msg& Arg(const Handle(TCollection_HAsciiString)& theString);

  Message_Msg& operator<<(const Handle(TCollection_HAsciiString)& theString)
  {
    return Arg(theString);
  }

  //! Set a value for %..s conversion
  Standard_EXPORT Message_Msg& Arg(const UtfString& theString);

  Message_Msg& operator<<(const UtfString& theString) { return Arg(theString); }

  //! Set a value for %..s conversion
  Message_Msg& Arg(const Handle(TCollection_HExtendedString)& theString);

  Message_Msg& operator<<(const Handle(TCollection_HExtendedString)& theString)
  {
    return Arg(theString);
  }

  //! Set a value for %..d, %..i, %..o, %..u, %..x or %..X conversion
  Standard_EXPORT Message_Msg& Arg(const Standard_Integer theInt);

  Message_Msg& operator<<(const Standard_Integer theInt) { return Arg(theInt); }

  //! Set a value for %..f, %..e, %..E, %..g or %..G conversion
  Standard_EXPORT Message_Msg& Arg(const Standard_Real theReal);

  Message_Msg& operator<<(const Standard_Real theReal) { return Arg(theReal); }

  //! Returns the original message text
  const UtfString& Original() const;

  //! Returns current state of the message text with
  //! parameters to the moment
  const UtfString& Value() const;

  //! Tells if Value differs from Original
  Standard_Boolean IsEdited() const;

  //! Return the resulting message string with all parameters
  //! filled. If some parameters were not yet filled by calls
  //! to methods Arg (or <<), these parameters are filled by
  //! the word UNKNOWN
  Standard_EXPORT const UtfString& Get();

  operator const UtfString&() { return Get(); }

protected:
private:
  Standard_EXPORT Standard_Integer getFormat(const Standard_Integer   theType,
                                             AsciiString1& theFormat);

  Standard_EXPORT void replaceText(const Standard_Integer            theFirst,
                                   const Standard_Integer            theNb,
                                   const UtfString& theStr);

  UtfString myOriginal;
  UtfString myMessageBody;
  TColStd_SequenceOfInteger  mySeqOfFormats;
};

#include <Message_Msg.lxx>

#endif // _Message_Msg_HeaderFile
