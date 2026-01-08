// Copyright (c) 1998-1999 Matra Datavision
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

#include <OSD_Path.hxx>
#include <Standard_ConstructionError.hxx>
#include <TCollection_AsciiString.hxx>

static OSD_SysType whereAmI()
{
#if defined(__digital__) || defined(__FreeBSD__) || defined(SUNOS) || defined(__APPLE__)           \
  || defined(__QNX__) || defined(__FreeBSD_kernel__)
  return OSD_UnixBSD;
#elif defined(sgi) || defined(IRIX) || defined(__sun) || defined(SOLARIS) || defined(__sco__)      \
  || defined(__hpux) || defined(HPUX)
  return OSD_UnixSystemV;
#elif defined(__osf__) || defined(DECOSF1)
  return OSD_OSF;
#elif defined(OS2)
  return OSD_WindowsNT;
#elif defined(_WIN32) || defined(__WIN32__)
  return OSD_WindowsNT;
#elif defined(__CYGWIN32_) || defined(__MINGW32__)
  return OSD_WindowsNT;
#elif defined(vax) || defined(__vms)
  return OSD_VMS;
#elif defined(__linux__) || defined(__linux)
  return OSD_LinuxREDHAT;
#elif defined(__EMSCRIPTEN__)
  return OSD_LinuxREDHAT;
#elif defined(_AIX) || defined(AIX)
  return OSD_Aix;
#else
  struct utsname info;
  uname(&info);
  std::cout << info.sysname << std::endl;
  std::cout << info.nodename << std::endl;
  std::cout << info.release << std::endl;
  std::cout << info.version << std::endl;
  std::cout << info.machine << std::endl;
  return OSD_Default;
#endif
}

#if !(defined(_WIN32) || defined(__WIN32__))

  #include <Standard_NumericError.hxx>
  #include <Standard_NullObject.hxx>
  #include <Standard_ProgramError.hxx>
  #include <Standard_ConstructionError.hxx>
  #include <OSD_WhoAmI.hxx>

SystemPath::SystemPath()
{
  mySysDep = whereAmI();
}

static void VmsExtract(const AsciiString1& what,
                       AsciiString1&       node,
                       AsciiString1&       username,
                       AsciiString1&       password,
                       AsciiString1&       disk,
                       AsciiString1&       trek,
                       AsciiString1&       name,
                       AsciiString1&       ext)
{

  AsciiString1 buffer;
  Standard_Integer        pos;

  buffer = what;

  if (buffer.Search("\"") != -1)
  { // a username to extract

    if (buffer.Value(1) != '"')
    { // Begins with Node
      node = buffer.Token("\"");
      buffer.Remove(1, node.Length());
    }
    else
      node = "";

    username = buffer.Token("\" ");
    buffer.Remove(1, username.Length() + 2); // Removes <<"username ' ' or '"' >>

    if (buffer.Search("\"") != -1)
    { // a password to extract
      password = buffer.Token("\"");
      buffer.Remove(1, password.Length() + 1); // removes <<password">>
    }

    // If we found a node then we must find "::"
    if (buffer.Search("::") != -1)
      buffer.Remove(1, 2); // Removes <<::>>
  }
  else // No name or password
    if (buffer.Search("::") != -1)
    { // a node to extract
      node = buffer.Token(":");
      buffer.Remove(1, node.Length() + 2); // Removes <<node::>
    }

  if (buffer.Search(":") != -1)
  { // a disk to extract
    disk = buffer.Token(":");
    buffer.Remove(1, disk.Length() + 1); // Removes <<disk:>>
  }
  else
    disk = "";

  // Analyse trek

  if (buffer.Search("[") != -1)
  { // There is atrek to extract
    trek = buffer.Token("[]");

    if (trek.Value(1) == '.')
      trek.Remove(1, 1); // Removes first '.'
    else
      trek.Insert(1, '|'); // Add root

    trek.ChangeAll('.', '|'); // Translates to portable syntax
    trek.ChangeAll('-', '^');

    pos = trek.Search("000000");
    if (pos != -1)
    {
      trek.Remove(pos, 6); // on VMS [000000] is the root
      if (trek.Search("||") != -1)
        trek.Remove(1, 1); // When [000000.xxx] -> ||xxx
    }

    name = buffer.Token("]", 2);
  }
  else
    name = buffer;

  if (name.Search(".") != -1)
  {
    ext = name.Token(".", 2);
    ext.Insert(1, '.');
    name.Remove(name.Search("."), ext.Length());
  }
  else
    ext = "";
}

//=================================================================================================

static void UnixExtract(const AsciiString1& what,
                        AsciiString1&       node,
                        AsciiString1&       username,
                        AsciiString1&       password,
                        AsciiString1&       trek,
                        AsciiString1&       name,
                        AsciiString1&       ext)
{

  Standard_Integer        pos;
  AsciiString1 buffer; // To manipulate 'what' without modifying it

  Standard_PCharacter p;
  buffer = what;

  #ifdef TOTO // Username, password and node are no longer given in the string (LD)

  if (buffer.Search("@") != -1)
  { // There is a name to extract
    username = buffer.Token("\"@");
    buffer.Remove(1, username.Length() + 1); // Removes << user@ >>

    if (buffer.Search("\"") != -1)
    { // There is a password to extract
      password = buffer.Token("\"");
      buffer.Remove(1, password.Length() + 2); // Removes << "password" >>
    }
  }
  else
  {
    username = "";
    password = "";
  }

  #endif // node must be given (for DBT, DM) (ADN 29/8/96)

  if (buffer.Search(":/") != -1)
  { // There is a node to extract
    node = buffer.Token(":/");
    buffer.Remove(1, node.Length() + 1); // Removes << node: >>
  }
  else
    node = "";

  username = "";
  password = "";
  //  node = "";

  trek = buffer;

  trek.ChangeAll('/', '|'); // Translates to portable syntax

  pos = trek.SearchFromEnd("|"); // Extract name
  if (pos != -1)
  {
    p    = (Standard_PCharacter)trek.ToCString();
    name = &p[pos];
    if (name.Length())
      trek.Remove(pos + 1, name.Length());
  }
  else
  { // No '|' means no trek but a name
    name = buffer;
    trek = "";
  }

  pos = trek.Search("..");
  while (pos != -1)
  { // Changes every ".." by '^'
    trek.SetValue(pos, '^');
    trek.Remove(pos + 1, 1);
    pos = trek.Search("..");
  }

  pos = name.SearchFromEnd("."); // LD : debug
  if (pos != -1)                 // There is an extension to extract
    ext = name.Split(pos - 1);

  // if (name.Search(".") != -1){ // There is an extension to extract
  //   if ( name.Value(1) == '.' ) {
  //     ext = name;
  //     name.Clear();
  //   }
  //   else {
  //     ext = name.Token(".",2);
  //     ext.Insert(1,'.');            // Prepends 'dot'
  //     pos = name.Search(".");     // Removes extension from buffer
  //     if (pos != -1)
  //       name.Remove(pos,ext.Length());
  //   }
  // }
}

//=================================================================================================

static void DosExtract(const AsciiString1& what,
                       AsciiString1&       disk,
                       AsciiString1&       trek,
                       AsciiString1&       name,
                       AsciiString1&       ext)
{

  AsciiString1 buffer;
  Standard_Integer        pos;
  Standard_PCharacter     p;

  buffer = what;

  if (buffer.Search(":") != -1)
  { // There is a disk to extract
    disk = buffer.Token(":");
    disk += ":";
    buffer.Remove(1, disk.Length()); // Removes <<disk:>>
  }

  trek = buffer;

  trek.ChangeAll('\\', '|');

  pos = trek.Search("..");
  while (pos != -1)
  { // Changes every ".." by '^'
    trek.SetValue(pos, '^');
    trek.Remove(pos + 1, 1);
    pos = trek.Search("..");
  }

  pos = trek.SearchFromEnd("|"); // Extract name
  if (pos != -1)
  {
    p    = (Standard_PCharacter)trek.ToCString();
    name = &p[pos];
    if (name.Length())
      trek.Remove(pos + 1, name.Length());
  }
  else
  { // No '|' means no trek but a name
    name = buffer;
    trek = "";
  }

  pos = name.SearchFromEnd(".");
  if (pos != -1) // There is an extension to extract
    ext = name.Split(pos - 1);
}

//=================================================================================================

static void MacExtract(const AsciiString1& what,
                       AsciiString1&,
                       AsciiString1& trek,
                       AsciiString1& name,
                       AsciiString1&)
{

  Standard_Integer    pos;
  Standard_PCharacter p;

  // I don't know how to distinguish a disk from a trek !

  trek = what;

  pos = trek.Search("::");
  while (pos != -1)
  { // Changes every "::" by '^'
    trek.SetValue(pos, '^');
    trek.Remove(pos + 1, 1);
    pos = trek.Search("::");
  }

  trek.ChangeAll(':', '|'); // Translates to portable syntax

  pos = trek.SearchFromEnd("|"); // Extract name
  if (pos != -1)
  {
    p    = (Standard_PCharacter)trek.ToCString();
    name = &p[pos + 1];
    trek.Remove(trek.Search(name), name.Length());
  }
  else
  { // No '|' means no trek but a name
    name = what;
    trek = "";
  }
}

SystemPath::SystemPath(const AsciiString1& aDependentName, const OSD_SysType aSysType)
{

  mySysDep = whereAmI();

  OSD_SysType todo;
  //  Standard_Integer i,l;

  if (aSysType == OSD_Default)
  {
    todo = mySysDep;
  }
  else
  {
    todo = aSysType;
  }

  switch (todo)
  {
    case OSD_VMS:
      VmsExtract(aDependentName,
                 myNode,
                 myUserName,
                 myPassword,
                 myDisk,
                 myTrek,
                 myName,
                 myExtension);
      break;
    case OSD_LinuxREDHAT:
    case OSD_UnixBSD:
    case OSD_UnixSystemV:
    case OSD_Aix:
    case OSD_OSF:
      UnixExtract(aDependentName, myNode, myUserName, myPassword, myTrek, myName, myExtension);
      break;
    case OSD_OS2:
    case OSD_WindowsNT:
      DosExtract(aDependentName, myDisk, myTrek, myName, myExtension);
      break;
    case OSD_MacOs:
      MacExtract(aDependentName, myDisk, myTrek, myName, myExtension);
      break;
    default:
  #ifdef OCCT_DEBUG
      std::cout << " WARNING WARNING : OSD1 Path for an Unknown SYSTEM : " << (Standard_Integer)todo
                << std::endl;
  #endif
      break;
  }
}

SystemPath::SystemPath(const AsciiString1& Nod,
                   const AsciiString1& UsrNm,
                   const AsciiString1& Passwd,
                   const AsciiString1& Dsk,
                   const AsciiString1& Trk,
                   const AsciiString1& Nam,
                   const AsciiString1& ext)
{

  mySysDep = whereAmI();

  SetValues(Nod, UsrNm, Passwd, Dsk, Trk, Nam, ext);
}

void SystemPath::Values(AsciiString1& Nod,
                      AsciiString1& UsrNm,
                      AsciiString1& Passwd,
                      AsciiString1& Dsk,
                      AsciiString1& Trk,
                      AsciiString1& Nam,
                      AsciiString1& ext) const
{

  Nod    = myNode;
  UsrNm  = myUserName;
  Passwd = myPassword;
  Dsk    = myDisk;
  Trk    = myTrek;
  Nam    = myName;
  ext    = myExtension;
}

void SystemPath::SetValues(const AsciiString1& Nod,
                         const AsciiString1& UsrNm,
                         const AsciiString1& Passwd,
                         const AsciiString1& Dsk,
                         const AsciiString1& Trk,
                         const AsciiString1& Nam,
                         const AsciiString1& ext)
{
  myNode      = Nod;
  myUserName  = UsrNm;
  myPassword  = Passwd;
  myDisk      = Dsk;
  myTrek      = Trk;
  myName      = Nam;
  myExtension = ext;
}

void SystemPath::UpTrek()
{
  Standard_Integer length = TrekLength();

  if (length == 0)
    return;

  Standard_Integer        awhere, aHowmany;
  AsciiString1 tok;

  tok      = myTrek.Token("|", length);
  awhere   = myTrek.SearchFromEnd(tok);
  aHowmany = tok.Length();
  myTrek.Remove(awhere, aHowmany);

  awhere = myTrek.Search("||"); // Searches leaving "||"
  if (awhere != -1)
    myTrek.Remove(awhere);
}

void SystemPath::DownTrek(const AsciiString1& aName)
{
  myTrek += aName;
  // Pb signale par GG : pour ne pas avoir "||" ;
  if (aName.ToCString()[aName.Length() - 1] != '|')
    myTrek += "|";
}

Standard_Integer SystemPath::TrekLength() const
{
  Standard_Integer cpt = 0;

  while (myTrek.Token("|", cpt + 1) != "") // Counts token separated by '|'
    cpt++;

  return (cpt);
}

void SystemPath::RemoveATrek(const Standard_Integer thewhere)
{
  Standard_Integer length = TrekLength();

  if (length <= 0 || thewhere > length)
    throw Standard_NumericError("SystemPath::RemoveATrek : where has an invalid value");

  Standard_Integer        posit, aHowmany;
  AsciiString1 tok;

  tok      = myTrek.Token("|", thewhere);
  posit    = myTrek.Search(tok);
  aHowmany = tok.Length();
  myTrek.Remove(posit, aHowmany);

  posit = myTrek.Search("||"); // Searches leaving "||"
  if (posit != -1)
    myTrek.Remove(posit);
}

void SystemPath::RemoveATrek(const AsciiString1& aName)
{
  Standard_Integer length = TrekLength();

  if (length == 0)
    return;

  Standard_Integer awhere;

  awhere = myTrek.Search(aName);
  if (awhere != -1)
  {
    myTrek.Remove(awhere, aName.Length());

    awhere = myTrek.Search("||"); // Searches leaving "||"
    if (awhere != -1)
      myTrek.Remove(awhere);
  }
}

AsciiString1 SystemPath::TrekValue(const Standard_Integer thewhere) const
{
  AsciiString1 result = myTrek.Token("|", thewhere);

  if (result == "")
    throw Standard_NumericError("SystemPath::TrekValue : where is invalid");

  return (result);
}

void SystemPath::InsertATrek(const AsciiString1& aName, const Standard_Integer thewhere)
{
  Standard_Integer length = TrekLength();

  if (thewhere <= 0 || thewhere > length)
    throw Standard_NumericError("SystemPath::InsertATrek : where has an invalid value");

  AsciiString1 tok    = myTrek.Token("|", thewhere);
  Standard_Integer        wwhere = myTrek.Search(tok);
  AsciiString1 what   = aName;
  what += "|";

  myTrek.Insert(wwhere, what);
}

// The 4 following methods will be PUBLIC in the future

// Converts a VMS disk to other system syntax

static void VMSDisk2Other(AsciiString1& Disk)
{
  Disk.RemoveAll('$');
}

// Convert a Trek to VMS syntax

static void P2VMS(AsciiString1& Way)
{
  Standard_Integer length = Way.Length();

  if (length == 0)
    return;

  if (Way.Value(1) == '|') // If begin with '|' remove '|'
    if (Way.Value(1) != '\0')
      Way.Remove(1, 1);
    else
      Way = "000000"; // Si uniquement la racine -> [000000]
  else if (Way.Length() != 0)
    Way.Insert(1, '|'); // Else insert '|' at beginning if not empty;

  Way.ChangeAll('|', '.');
  Way.ChangeAll('^', '-');
}

// Convert a Trek to MAC syntax

static void P2MAC(AsciiString1& Way)
{
  int i, l;
  Way.ChangeAll('|', ':');

  l = (int)Way.Length();
  for (i = 1; i <= l; i++) // Replace '^' by "::"
    if (Way.Value(i) == '^')
    {
      Way.SetValue(i, ':');
      Way.Insert(i, ':');
      i++;
      l++;
    }
}

// Convert a Trek to UNIX syntax

static void P2UNIX(AsciiString1& Way)
{
  int              i, l;
  Standard_Integer length = Way.Length();

  if (length == 0)
    return;

  // if (Way.Value(length) == '|') // If Finishes with "|" removes it
  // Way.Trunc(length-1);

  Way.ChangeAll('|', '/');

  l = (int)Way.Length();
  for (i = 1; i <= l; i++) // Replace '^' by "../"
    if (Way.Value(i) == '^')
    {
      Way.SetValue(i, '.');
      Way.Insert(i + 1, '.');
      // Way.Insert(i+2,'/');
      i += 1;
      l += 1;
    }
}

// Convert a Trek to DOS like syntax

static void P2DOS(AsciiString1& Way)
{
  int              i, l;
  Standard_Integer len = Way.Length();

  if (len == 0)
    return;

  if (Way.Value(len) == '|') // If Finishes with "|" removes it
    Way.Trunc(len - 1);

  Way.ChangeAll('|', '\\');

  l = (int)Way.Length();
  for (i = 1; i <= l; i++) // Replace '^' by ".."
    if (Way.Value(i) == '^')
    {
      Way.SetValue(i, '.');
      Way.Insert(i, '.');
      i++;
      l++;
    }
}

// Convert a path to system dependent syntax

void SystemPath::SystemName(AsciiString1& FullName, const OSD_SysType aType) const
{
  AsciiString1 Way;
  AsciiString1 pNode;
  AsciiString1 pDisk;
  OSD_SysType             pType;

  if (aType == OSD_Default)
  {
    pType = mySysDep;
  }
  else
  {
    pType = aType;
  }

  Way = myTrek;
  FullName.Clear();

  switch (pType)
  {
    case OSD_VMS:
      pNode = myNode;

      P2VMS(Way); // Convert path

      if (myNode.Length() != 0)
        FullName += myNode; // Append Node

      if (myUserName.Length() != 0)
      { // Append User name

        if (pNode.Length() == 0)
        { // If a user name but no node, catenate "0"
          pNode = "0";
          FullName += pNode;
        }

        FullName += "\"";
        FullName += myUserName;

        if (myPassword.Length() != 0)
        { // Append password
          FullName += " ";
          FullName += myPassword;
        }

        FullName += "\"";
      }

      if (pNode.Length() != 0)
        FullName += "::";

      if (myDisk.Length() != 0)
      { // Append Disk
        FullName += myDisk;
        FullName += ":";
      }

      if (Way.Length() != 0) // Append VMS path
        FullName = FullName + "[" + Way + "]" + myName + myExtension;

      //   FullName.UpperCase();
      break;

    case OSD_OS2:
    case OSD_WindowsNT: // MSDOS-like syntax
    {
      int length = (int)myDisk.Length();

      P2DOS(Way);
      if (length != 1)

        if (myDisk.Length() != 0)
        {

          if ((length == 1 && IsAlphabetic(myDisk.Value(1))) || // 'A/a' to 'Z/z'
              (length == 2 && IsAlphabetic(myDisk.Value(1))
               && myDisk.Value(2) == ':') // 'A:' to 'Z:'
              )                           // This is a MSDOS disk syntax
          {
            FullName += myDisk;
            if (myDisk.Value(length) != ':')
              FullName += ":";
          }
          else // This is an assigned Disk
          {
            FullName += "\\";
            pDisk = myDisk;
            VMSDisk2Other(pDisk);
            FullName += pDisk;
            if (Way.Value(1) != '\\')
              FullName += "\\";
          }
        }

      if (Way.Length() != 0)
        FullName = FullName + Way + "\\";

      FullName += myName;
      FullName += myExtension;
      //    FullName.UpperCase();
      break;
    }

    case OSD_MacOs: // Mackintosh-like syntax
      if (myDisk.Length() != 0)
      {
        FullName += myDisk;
        FullName += ":";
      }
      P2MAC(Way);
      FullName += myName;
      FullName += myExtension;
      break;

    default: // UNIX-like syntax

      // Syntax :
      //             user"password"@host:/disk/xxx/xxx/filename
      P2UNIX(Way);

      if (myUserName.Length() != 0 && myNode.Length() != 0)
      {                         // If USER name
        FullName += myUserName; // appends user name

        if (myPassword.Length() != 0)
          FullName = FullName + "\"" + myPassword + "\""; // a password if not empty

        FullName += "@"; // and character '@'
      }

      if (myNode.Length() != 0)
      { // Appends HOST name
        FullName += myNode;
        FullName += ":";
      }

      if (myDisk.Length() != 0)
      { // Appends Disk name as path
        FullName += "/";
        pDisk = myDisk;
        VMSDisk2Other(pDisk);
        FullName += pDisk;
      }

      //    if (FullName.Length()) {                     // Adds a "/" if necessary
      //      FullName += "/";
      //    }

      if (Way.Length() != 0)
      { // Appends a path if not empty
        FullName += Way;
      }

      if (FullName.Length())
      {
        if (FullName.Value(FullName.Length()) != '/')
        {
          FullName += "/"; // Adds a / if necessary
        }
      }

      if (myName.Length())
      { // Adds the file name
        FullName += myName;
      }

      if (myExtension.Length())
      { // Adds the extension
        FullName += myExtension;
      }
      break;
  }
}

  #ifdef TOTO // A reactiver...

void SystemPath::SetSystemName(const AsciiString1& aDependentName,
                             const OSD_SysType              aSysType)
{
  UnixExtract(aDependentName, myNode, myUserName, myPassword, myTrek, myName, myExtension);
}
  #endif

AsciiString1 SystemPath::Node() const
{
  return (myNode);
}

AsciiString1 SystemPath::UserName() const
{
  return (myUserName);
}

AsciiString1 SystemPath::Password() const
{
  return (myPassword);
}

AsciiString1 SystemPath::Disk() const
{
  return (myDisk);
}

AsciiString1 SystemPath::Trek() const
{
  return (myTrek);
}

// Return extension (suffix) of file/directory name

AsciiString1 SystemPath::Extension() const
{
  return (myExtension);
}

AsciiString1 SystemPath::Name() const
{
  return (myName);
}

void SystemPath::SetNode(const AsciiString1& aName)
{
  myNode = aName;
}

void SystemPath::SetUserName(const AsciiString1& aName)
{
  myUserName = aName;
}

void SystemPath::SetPassword(const AsciiString1& aName)
{
  myPassword = aName;
}

void SystemPath::SetDisk(const AsciiString1& aName)
{
  myDisk = aName;
}

void SystemPath::SetTrek(const AsciiString1& aName)
{
  myTrek = aName;
}

void SystemPath::SetName(const AsciiString1& aName)
{
  myName = aName;
}

void SystemPath::SetExtension(const AsciiString1& aName)
{
  myExtension = aName;
}

#else

//------------------------------------------------------------------------
//-------------------  Windows sources for SystemPath -------------------
//------------------------------------------------------------------------

  #include <Standard_ProgramError.hxx>

  #include <windows.h>
  #include <stdlib.h>

  #define TEST_RAISE(type, arg) _test_raise((type), (arg))

static void __fastcall _test_raise(OSD_SysType, Standard_CString);
static void __fastcall _remove_dup(AsciiString1&);

SystemPath ::SystemPath()
    : myUNCFlag(Standard_False),
      mySysDep(OSD_WindowsNT)
{
} // end constructor ( 1 )

SystemPath ::SystemPath(const AsciiString1& aDependentName, const OSD_SysType aSysType)
    : myUNCFlag(Standard_False),
      mySysDep(OSD_WindowsNT)
{

  Standard_Integer i, j, len;
  char             __drive[_MAX_DRIVE];
  char             __dir[_MAX_DIR];
  char             __trek[_MAX_DIR];
  char             __fname[_MAX_FNAME];
  char             __ext[_MAX_EXT];

  memset(__drive, 0, _MAX_DRIVE);
  memset(__dir, 0, _MAX_DIR);
  memset(__trek, 0, _MAX_DIR);
  memset(__fname, 0, _MAX_FNAME);
  memset(__ext, 0, _MAX_EXT);
  Standard_Character chr;

  TEST_RAISE(aSysType, "SystemPath");

  _splitpath(aDependentName.ToCString(), __drive, __dir, __fname, __ext);

  myDisk      = __drive;
  myName      = __fname;
  myExtension = __ext;

  {
    AsciiString1 dir = __dir;
    len                         = dir.Length();
  }

  for (i = j = 0; i < len; ++i, ++j)
  {

    chr = __dir[i];

    if (chr == '\\' || chr == '/')

      __trek[j] = '|';

    else if (chr == '.' && (i + 1) < len && __dir[i + 1] == '.')
    {

      __trek[j] = '^';
      ++i;
    }
    else

      __trek[j] = chr;

  } // end for
  __trek[j]                    = '\0';
  AsciiString1 trek = __trek;
  _remove_dup(trek);
  myTrek = trek;

} // end constructor ( 2 )

SystemPath ::SystemPath(const AsciiString1& aNode,
                    const AsciiString1& aUsername,
                    const AsciiString1& aPassword,
                    const AsciiString1& aDisk,
                    const AsciiString1& aTrek,
                    const AsciiString1& aName,
                    const AsciiString1& anExtension)
    : myUNCFlag(Standard_False),
      mySysDep(OSD_WindowsNT)
{

  SetValues(aNode, aUsername, aPassword, aDisk, aTrek, aName, anExtension);

} // end constructor ( 3 )

void SystemPath ::Values(AsciiString1& aNode,
                       AsciiString1& aUsername,
                       AsciiString1& aPassword,
                       AsciiString1& aDisk,
                       AsciiString1& aTrek,
                       AsciiString1& aName,
                       AsciiString1& anExtension) const
{

  aNode     = myNode;
  aUsername = myUserName;
  aPassword = myPassword;
  aDisk     = myDisk;
  aTrek     = myTrek;
  if (!aTrek.IsEmpty() && aTrek.Value(aTrek.Length()) != '|')
    aTrek += "|"; // (LD)
  aName       = myName;
  anExtension = myExtension;

} // end SystemPath :: Values

void SystemPath ::SetValues(const AsciiString1& aNode,
                          const AsciiString1& aUsername,
                          const AsciiString1& aPassword,
                          const AsciiString1& aDisk,
                          const AsciiString1& aTrek,
                          const AsciiString1& aName,
                          const AsciiString1& anExtension)
{

  myNode      = aNode;
  myUserName  = aUsername;
  myPassword  = aPassword;
  myDisk      = aDisk;
  myTrek      = aTrek;
  myName      = aName;
  myExtension = anExtension;

  if (myExtension.Length() && myExtension.Value(1) != '.')

    myExtension.Insert(1, '.');

  _remove_dup(myTrek);

} // end SystemPath :: SetValues

void SystemPath ::SystemName(AsciiString1& FullName, const OSD_SysType aType) const
{

  Standard_Integer        i, j;
  AsciiString1 fullPath;
  Standard_Character      trek[_MAX_PATH];
  Standard_Character      chr;

  memset(trek, 0, _MAX_PATH);

  TEST_RAISE(aType, "SystemName");

  for (i = j = 1; i <= myTrek.Length() && j <= _MAX_PATH; ++i, ++j)
  {

    chr = myTrek.Value(i);

    if (chr == '|')
    {

      trek[j - 1] = '/';
    }
    else if (chr == '^' && j <= _MAX_PATH - 1)
    {

      strcpy(&(trek[(j++) - 1]), "..");
    }
    else

      trek[j - 1] = chr;

  } // end for

  fullPath = myDisk + AsciiString1(trek);

  if (trek[0])
    fullPath += "/";

  fullPath += (myName + myExtension);

  if (fullPath.Length() > 0)

    FullName = fullPath;

  else

    FullName.Clear();

} // end SystemPath :: SystemName

void SystemPath ::UpTrek()
{

  Standard_Integer pos = myTrek.SearchFromEnd("|");

  if (pos == -1)

    pos = 0;

  else if (pos > 1)
  {

    while (myTrek.Value(pos) == '|' && pos != 1)
      --pos;

  } // end if

  myTrek.Trunc(pos);

} // end SystemPath :: UpTrek

void SystemPath ::DownTrek(const AsciiString1& aName)
{

  Standard_Integer pos = myTrek.Length();

  if (!aName.IsEmpty() && aName.Value(1) != '|' && pos && myTrek.Value(pos) != '|')

    myTrek += "|";

  myTrek += aName;

  _remove_dup(myTrek);

} // end SystemPath :: DownTrek

Standard_Integer SystemPath ::TrekLength() const
{

  Standard_Integer i      = 1;
  Standard_Integer retVal = 0;

  if (myTrek.IsEmpty() || (myTrek.Length() == 1 && myTrek.Value(1) == '|'))

    return retVal;

  for (;;)
  {

    if (myTrek.Token("|", i++).IsEmpty())

      break;

    ++retVal;

  } // end while

  return retVal;

} // end TrekLength

void SystemPath ::RemoveATrek(const Standard_Integer thewhere)
{

  Standard_Integer i, j;
  Standard_Boolean flag = Standard_False;

  if (TrekLength() < thewhere)

    return;

  if (myTrek.Value(1) != '|')
  {

    flag = Standard_True;
    myTrek.Insert(1, '|');

  } // end if

  i = myTrek.Location(thewhere, '|', 1, myTrek.Length());

  if (i)
  {

    j = myTrek.Location(thewhere + 1, '|', 1, myTrek.Length());

    if (j == 0)

      j = myTrek.Length() + 1;

    myTrek.Remove(i, j - i);

  } // end if

  if (flag)

    myTrek.Remove(1);

} // end SystemPath :: RemoveATrek ( 1 )

void SystemPath ::RemoveATrek(const AsciiString1& aName)
{

  Standard_Integer        i;
  Standard_Boolean        flag = Standard_False;
  AsciiString1 tmp;

  if (myTrek.Value(1) != '|')
  {

    flag = Standard_True;
    myTrek.Insert(1, '|');

  } // end if

  myTrek += '|';

  tmp = aName;

  if (tmp.Value(1) != '|')

    tmp.Insert(1, '|');

  if (tmp.Value(tmp.Length()) != '|')

    tmp += '|';

  i = myTrek.Search(tmp);

  if (i != -1)

    myTrek.Remove(i + 1, tmp.Length() - 1);

  if (flag)

    myTrek.Remove(1);

  if (myTrek.Value(myTrek.Length()) == '|')

    myTrek.Trunc(myTrek.Length() - 1);

} // end SystemPath :: RemoveATrek ( 2 )

AsciiString1 SystemPath ::TrekValue(const Standard_Integer thewhere) const
{

  AsciiString1 retVal;
  AsciiString1 trek = myTrek;

  if (trek.Value(1) != '|')

    trek.Insert(1, '|');

  retVal = trek.Token("|", thewhere);

  return retVal;

} // end SystemPath :: TrekValue

void SystemPath ::InsertATrek(const AsciiString1& aName, const Standard_Integer thewhere)
{

  Standard_Integer        pos;
  AsciiString1 tmp  = aName;
  Standard_Boolean        flag = Standard_False;

  if (myTrek.Value(1) != '|')
  {

    flag = Standard_True;
    myTrek.Insert(1, '|');

  } // end if

  myTrek += '|';

  pos = myTrek.Location(thewhere, '|', 1, myTrek.Length());

  if (pos)
  {

    if (tmp.Value(tmp.Length()) != '|')

      tmp += '|';

    myTrek.Insert(pos + 1, tmp);

  } // end if

  if (flag)

    myTrek.Remove(1);

  if (myTrek.Value(myTrek.Length()) == '|')

    myTrek.Trunc(myTrek.Length() - 1);

  _remove_dup(myTrek);

} // end SystemPath :: InsertATrek

AsciiString1 SystemPath ::Node() const
{

  return myNode;

} // end SystemPath :: Node

AsciiString1 SystemPath ::UserName() const
{

  return myUserName;

} // end SystemPath :: UserName

AsciiString1 SystemPath ::Password() const
{

  return myPassword;

} // end SystemPath :: Password

AsciiString1 SystemPath ::Disk() const
{

  return myDisk;

} // end SystemPath :: Disk

AsciiString1 SystemPath ::Trek() const
{

  AsciiString1 retVal;
  retVal = myTrek;
  if (!retVal.IsEmpty() && retVal.Value(retVal.Length()) != '|')
    retVal += "|"; // (LD)
  return retVal;

} // end SystemPath :: Trek

AsciiString1 SystemPath ::Name() const
{

  return myName;

} // end SystemPath :: Name

AsciiString1 SystemPath ::Extension() const
{

  return myExtension;

} // end SystemPath :: Extension

void SystemPath ::SetNode(const AsciiString1& aName)
{

  myNode = aName;

} // end SystemPath :: SetNode

void SystemPath ::SetUserName(const AsciiString1& aName)
{

  myUserName = aName;

} // end SystemPath :: SetUserName

void SystemPath ::SetPassword(const AsciiString1& aName)
{

  myPassword = aName;

} // end SystemPath :: SetPassword

void SystemPath ::SetDisk(const AsciiString1& aName)
{

  myDisk = aName;

} // end SystemPath :: SetDisk

void SystemPath ::SetTrek(const AsciiString1& aName)
{

  myTrek = aName;

  _remove_dup(myTrek);

} // end SystemPath :: SetTrek

void SystemPath ::SetName(const AsciiString1& aName)
{

  myName = aName;

} // end SystemPath :: SetName

void SystemPath ::SetExtension(const AsciiString1& aName)
{

  myExtension = aName;

} // end SystemPath :: SetExtension

static void __fastcall _test_raise(OSD_SysType type, Standard_CString str)
{

  Standard_Character buff[64];

  if (type != OSD_Default && type != OSD_WindowsNT)
  {

    strcpy(buff, "SystemPath :: ");
    strcat(buff, str);
    strcat(buff, " (): unknown system type");

    throw Standard_ProgramError(buff);

  } // end if

} // end _test_raise

static void __fastcall _remove_dup(AsciiString1& str)
{

  Standard_Integer pos = 1, orgLen, len = str.Length();

  orgLen = len;

  while (pos <= len)
  {

    if (str.Value(pos) == '|' && pos != len && str.Value(pos + 1) == '|' && pos != 1)
    {

      ++pos;

      while (pos <= len && str.Value(pos) == '|')
        str.Remove(pos), --len;
    }
    else

      ++pos;

  } // end while

  if (orgLen > 1 && len > 0 && str.Value(len) == '|')
    str.Remove(len);

  pos    = 1;
  orgLen = len = str.Length();

  while (pos <= len)
  {

    if (str.Value(pos) == '^' && pos != len && str.Value(pos + 1) == '^')
    {

      ++pos;

      while (pos <= len && str.Value(pos) == '^')
        str.Remove(pos), --len;
    }
    else

      ++pos;

  } // end while

  // if (  orgLen > 1 && len > 0 && str.Value ( len ) == '^'  ) str.Remove ( len );

} // end _remove_dup

#endif // Windows sources for SystemPath

//=================================================================================================

static Standard_Boolean Analyse_VMS(const AsciiString1& theName)
{
  if (theName.Search("/") != -1 || theName.Search("@") != -1 || theName.Search("\\") != -1)
  {
    return Standard_False;
  }

  return Standard_True;
}

//=================================================================================================

static Standard_Boolean Analyse_DOS(const AsciiString1& theName)
{
  if (theName.Search("/") != -1 || theName.Search(":") != -1 || theName.Search("*") != -1
      || theName.Search("?") != -1 || theName.Search("\"") != -1 || theName.Search("<") != -1
      || theName.Search(">") != -1 || theName.Search("|") != -1)
  {
    return Standard_False;
  }

  return Standard_True;
}

//=================================================================================================

static Standard_Boolean Analyse_MACOS(const AsciiString1& theName)
{
  return theName.Search(":") == -1 ? theName.Length() <= 31 : Standard_True;
}

//=================================================================================================

Standard_Boolean SystemPath::IsValid(const AsciiString1& theDependentName,
                                   const OSD_SysType              theSysType)
{
  if (theDependentName.Length() == 0)
  {
    return Standard_True;
  }

  switch (theSysType == OSD_Default ? whereAmI() : theSysType)
  {
    case OSD_VMS:
      return Analyse_VMS(theDependentName);
    case OSD_OS2:
    case OSD_WindowsNT:
      return Analyse_DOS(theDependentName);
    case OSD_MacOs:
      return Analyse_MACOS(theDependentName);
    default:
      return Standard_True;
  }
}

// ---------------------------------------------------------------------------

// Elimine les separateurs inutiles

static Standard_Integer RemoveExtraSeparator(AsciiString1& aString)
{

  Standard_Integer i, j, len, start = 1;

  len = aString.Length();
#ifdef _WIN32
  if (len > 1 && aString.Value(1) == '/' && aString.Value(2) == '/')
    start = 2;
#endif
  for (i = j = start; j <= len; i++, j++)
  {
    Standard_Character c = aString.Value(j);
    aString.SetValue(i, c);
    if (c == '/')
      while (j < len && aString.Value(j + 1) == '/')
        j++;
  }
  len = i - 1;
  if (aString.Value(len) == '/')
    len--;
  aString.Trunc(len);
  return len;
}

// ---------------------------------------------------------------------------

AsciiString1 SystemPath::RelativePath(const AsciiString1& aDirPath,
                                               const AsciiString1& aAbsFilePath)
{
  AsciiString1 EmptyString = "";
  AsciiString1 FilePath;
  Standard_Integer        len;
  Standard_Boolean        Wnt = 0;

  FilePath = aAbsFilePath;

  if (aDirPath.Search(":") == 2)
  { // Cas WNT
    Wnt = 1;
    if (FilePath.Search(":") != 2 || UpperCase(aDirPath.Value(1)) != UpperCase(FilePath.Value(1)))
      return EmptyString;

    FilePath.ChangeAll('\\', '/');
    if (FilePath.Search("/") != 3)
      return EmptyString;
  }
  else
  { // Cas Unix
    if (aDirPath.Value(1) != '/' || FilePath.Value(1) != '/')
      return EmptyString;
  }

  // Eliminer les separateurs redondants

  len = RemoveExtraSeparator(FilePath);

  if (!Wnt)
  {
    if (len < 2)
      return EmptyString;
    FilePath = FilePath.SubString(2, len);
  }
  AsciiString1 DirToken, FileToken;
  Standard_Boolean        Sibling = 0;

  for (Standard_Integer n = 1;; n++)
  {
    DirToken = aDirPath.Token("/\\", n);
    if (DirToken.IsEmpty())
      return FilePath;

    if (!Sibling)
    {
      len                = FilePath.Length();
      Standard_Integer i = FilePath.Search("/");
      if (i > 0)
      {
        if (i == len)
          return EmptyString;

        FileToken = FilePath.SubString(1, i - 1);
        if (Wnt)
        {
          DirToken.UpperCase();
          FileToken.UpperCase();
        }
        if (DirToken == FileToken)
        {
          FilePath = FilePath.SubString(i + 1, len);
          continue;
        }
      }
      else if (DirToken == FilePath)
        return EmptyString;

      else
        Sibling = 1;
    }
    FilePath.Insert(1, "../");
  }
}

// ---------------------------------------------------------------------------

AsciiString1 SystemPath::AbsolutePath(const AsciiString1& aDirPath,
                                               const AsciiString1& aRelFilePath)
{
  AsciiString1 EmptyString = "";
  if (aRelFilePath.Search("/") == 1 || aRelFilePath.Search(":") == 2)
    return aRelFilePath;
  AsciiString1 DirPath = aDirPath, RelFilePath = aRelFilePath;
  Standard_Integer        i, len;

  if (DirPath.Search("/") != 1 && DirPath.Search(":") != 2)
    return EmptyString;

  if (DirPath.Search(":") == 2)
    DirPath.ChangeAll('\\', '/');
  RelFilePath.ChangeAll('\\', '/');
  RemoveExtraSeparator(DirPath);
  len = RemoveExtraSeparator(RelFilePath);

  while (RelFilePath.Search("../") == 1)
  {
    if (len == 3)
      return EmptyString;
    RelFilePath = RelFilePath.SubString(4, len);
    len -= 3;
    if (DirPath.IsEmpty())
      return EmptyString;
    i = DirPath.SearchFromEnd("/");
    if (i < 0)
      return EmptyString;
    DirPath.Trunc(i - 1);
  }
  DirPath += '/';
  DirPath += RelFilePath;
  return DirPath;
}

// void SystemPath::ExpandedName(AsciiString1& aName)
void SystemPath::ExpandedName(AsciiString1&) {}

// Standard_Boolean LocateExecFile(SystemPath& aPath)
Standard_Boolean LocateExecFile(SystemPath&)
{
  return Standard_False;
}

//=================================================================================================

void SystemPath::FolderAndFileFromPath(const AsciiString1& theFilePath,
                                     AsciiString1&       theFolder,
                                     AsciiString1&       theFileName)
{
  Standard_Integer aLastSplit = -1;
  Standard_CString aString    = theFilePath.ToCString();
  for (Standard_Integer anIter = 0; anIter < theFilePath.Length(); ++anIter)
  {
    if (aString[anIter] == '/' || aString[anIter] == '\\')
    {
      aLastSplit = anIter;
    }
  }

  if (aLastSplit == -1)
  {
    theFolder.Clear();
    theFileName = theFilePath;
    return;
  }

  theFolder = theFilePath.SubString(1, aLastSplit + 1);
  if (aLastSplit + 2 <= theFilePath.Length())
  {
    theFileName = theFilePath.SubString(aLastSplit + 2, theFilePath.Length());
  }
  else
  {
    theFileName.Clear();
  }
}

//=================================================================================================

void SystemPath::FileNameAndExtension(const AsciiString1& theFilePath,
                                    AsciiString1&       theName,
                                    AsciiString1&       theExtension)
{
  // clang-format off
  const Standard_Integer THE_EXT_MAX_LEN = 20; // this method is supposed to be used with normal extension
  // clang-format on
  const Standard_Integer aLen = theFilePath.Length();
  for (Standard_Integer anExtLen = 1; anExtLen < aLen && anExtLen < THE_EXT_MAX_LEN; ++anExtLen)
  {
    if (theFilePath.Value(aLen - anExtLen) == '.')
    {
      const Standard_Integer aNameUpper = aLen - anExtLen - 1;
      if (aNameUpper < 1)
      {
        break;
      }

      theName      = theFilePath.SubString(1, aNameUpper);
      theExtension = theFilePath.SubString(aLen - anExtLen + 1, aLen);
      theExtension.LowerCase();
      return;
    }
  }

  theName = theFilePath;
  theExtension.Clear();
}
