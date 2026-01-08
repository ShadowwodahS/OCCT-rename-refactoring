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

#include <Interface_Static.hxx>

#include <OSD_Path.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>
#include <TCollection_HAsciiString.hxx>

#include <stdio.h>
IMPLEMENT_STANDARD_RTTIEXT(ExchangeConfig, Interface_TypedValue)

static char defmess[31];

//  Fonctions Satisfies offertes en standard ...

// svv #2
// static Standard_Boolean StaticPath(const Handle(TCollection_HAsciiString)& val)
//{
//   SystemPath apath;
//   return apath.IsValid (AsciiString1(val->ToCString()));
// }

ExchangeConfig::ExchangeConfig(const Standard_CString    family,
                                   const Standard_CString    name,
                                   const Interface_ParamType type,
                                   const Standard_CString    init)
    : Interface_TypedValue(name, type, init),
      thefamily(family),
      theupdate(Standard_True)
{
}

ExchangeConfig::ExchangeConfig(const Standard_CString          family,
                                   const Standard_CString          name,
                                   const Handle(ExchangeConfig)& other)
    : Interface_TypedValue(name, other->Type(), ""),
      thefamily(family),
      theupdate(Standard_True)
{
  switch (Type())
  {
    case Interface_ParamInteger: {
      Standard_Integer lim;
      if (other->IntegerLimit(Standard_True, lim))
        SetIntegerLimit(Standard_True, lim);
      if (other->IntegerLimit(Standard_False, lim))
        SetIntegerLimit(Standard_False, lim);
    }
    break;
    case Interface_ParamReal: {
      Standard_Real lim;
      if (other->RealLimit(Standard_True, lim))
        SetRealLimit(Standard_True, lim);
      if (other->RealLimit(Standard_False, lim))
        SetRealLimit(Standard_False, lim);
      SetUnitDef(other->UnitDef());
    }
    break;
    case Interface_ParamEnum: {
      Standard_Boolean match;
      Standard_Integer e0, e1, i;
      other->EnumDef(e0, e1, match);
      StartEnum(e0, match);
      //      if (e1 >= e0) theenums = new TColStd_HArray1OfAsciiString(e0,e1);
      for (i = e0; i <= e1; i++)
        AddEnum(other->EnumVal(i));
    }
    break;
    case Interface_ParamIdent:
      SetObjectType(other->ObjectType());
      break;
    default:
      break;
  }

  if (other->IsSetValue())
    SetCStringValue(other->CStringValue());
}

//  ##   Print   ##

void ExchangeConfig::PrintStatic(Standard_OStream& S) const
{
  S << "--- Static Value : " << Name() << "  Family:" << Family();
  Print(S);
  if (!thewild.IsNull())
    S << " -- Attached to wild-card : " << thewild->Name() << std::endl;
  S << "--- Actual status : " << (theupdate ? "" : "original") << "  Value : ";

  if (thesatisf)
    S << " -- Specific Function for Satisfies : " << thesatisn.ToCString() << std::endl;
}

//  #########    COMPLEMENTS    ##########

Standard_CString ExchangeConfig::Family() const
{
  return thefamily.ToCString();
}

Handle(ExchangeConfig) ExchangeConfig::Wild() const
{
  return thewild;
}

void ExchangeConfig::SetWild(const Handle(ExchangeConfig)& wild)
{
  thewild = wild;
}

//  #########   UPDATE    ##########

void ExchangeConfig::SetUptodate()
{
  theupdate = Standard_True;
}

Standard_Boolean ExchangeConfig::UpdatedStatus() const
{
  return theupdate;
}

//  #######################################################################
//  #########    DICTIONNAIRE DES STATICS (static sur Static)    ##########

Standard_Boolean ExchangeConfig::Init(const Standard_CString    family,
                                        const Standard_CString    name,
                                        const Interface_ParamType type,
                                        const Standard_CString    init)
{
  if (name[0] == '\0')
    return Standard_False;

  if (MoniTool_TypedValue::Stats().IsBound(name))
    return Standard_False;
  Handle(ExchangeConfig) item;
  if (type == Interface_ParamMisc)
  {
    Handle(ExchangeConfig) other = ExchangeConfig::Static(init);
    if (other.IsNull())
      return Standard_False;
    item = new ExchangeConfig(family, name, other);
  }
  else
    item = new ExchangeConfig(family, name, type, init);

  MoniTool_TypedValue::Stats().Bind(name, item);
  return Standard_True;
}

Standard_Boolean ExchangeConfig::Init(const Standard_CString   family,
                                        const Standard_CString   name,
                                        const Standard_Character type,
                                        const Standard_CString   init)
{
  Interface_ParamType epyt;
  switch (type)
  {
    case 'e':
      epyt = Interface_ParamEnum;
      break;
    case 'i':
      epyt = Interface_ParamInteger;
      break;
    case 'o':
      epyt = Interface_ParamIdent;
      break;
    case 'p':
      epyt = Interface_ParamText;
      break;
    case 'r':
      epyt = Interface_ParamReal;
      break;
    case 't':
      epyt = Interface_ParamText;
      break;
    case '=':
      epyt = Interface_ParamMisc;
      break;
    case '&': {
      Handle(ExchangeConfig) unstat = ExchangeConfig::Static(name);
      if (unstat.IsNull())
        return Standard_False;
      //    Editions : init donne un petit texte d edition, en 2 termes "cmd var" :
      //  imin <ival>  imax <ival>  rmin <rval>  rmax <rval>  unit <def>
      //  enum <from>  ematch <from>  eval <cval>
      Standard_Integer i, iblc = 0;
      for (i = 0; init[i] != '\0'; i++)
        if (init[i] == ' ')
          iblc = i + 1;
      //  Reconnaissance du sous-cas et aiguillage
      if (init[0] == 'i' && init[2] == 'i')
        unstat->SetIntegerLimit(Standard_False, atoi(&init[iblc]));
      else if (init[0] == 'i' && init[2] == 'a')
        unstat->SetIntegerLimit(Standard_True, atoi(&init[iblc]));
      else if (init[0] == 'r' && init[2] == 'i')
        unstat->SetRealLimit(Standard_False, Atof(&init[iblc]));
      else if (init[0] == 'r' && init[2] == 'a')
        unstat->SetRealLimit(Standard_True, Atof(&init[iblc]));
      else if (init[0] == 'u')
        unstat->SetUnitDef(&init[iblc]);
      else if (init[0] == 'e' && init[1] == 'm')
        unstat->StartEnum(atoi(&init[iblc]), Standard_True);
      else if (init[0] == 'e' && init[1] == 'n')
        unstat->StartEnum(atoi(&init[iblc]), Standard_False);
      else if (init[0] == 'e' && init[1] == 'v')
        unstat->AddEnum(&init[iblc]);
      else
        return Standard_False;
      return Standard_True;
    }
    default:
      return Standard_False;
  }
  if (!ExchangeConfig::Init(family, name, epyt, init))
    return Standard_False;
  if (type != 'p')
    return Standard_True;
  Handle(ExchangeConfig) stat = ExchangeConfig::Static(name);
  // NT  stat->SetSatisfies (StaticPath,"Path");
  if (!stat->Satisfies(stat->HStringValue()))
    stat->SetCStringValue("");
  return Standard_True;
}

Handle(ExchangeConfig) ExchangeConfig::Static(const Standard_CString name)
{
  Handle(RefObject) result;
  MoniTool_TypedValue::Stats().Find(name, result);
  return Handle(ExchangeConfig)::DownCast(result);
}

Standard_Boolean ExchangeConfig::IsPresent(const Standard_CString name)
{
  return MoniTool_TypedValue::Stats().IsBound(name);
}

Standard_CString ExchangeConfig::CDef(const Standard_CString name, const Standard_CString part)
{
  if (!part || part[0] == '\0')
    return "";
  Handle(ExchangeConfig) stat = ExchangeConfig::Static(name);
  if (stat.IsNull())
    return "";
  if (part[0] == 'f' && part[1] == 'a')
    return stat->Family();
  if (part[0] == 'l' && part[1] == 'a')
    return stat->Label();
  if (part[0] == 's' && part[1] == 'a')
    return stat->SatisfiesName();
  if (part[0] == 't' && part[1] == 'y')
  {
    Interface_ParamType typ = stat->Type();
    if (typ == Interface_ParamInteger)
      return "integer";
    if (typ == Interface_ParamReal)
      return "real";
    if (typ == Interface_ParamText)
      return "text";
    if (typ == Interface_ParamEnum)
      return "enum";
    return "?type?";
  }
  if (part[0] == 'e')
  {
    Standard_Integer nume = 0;
    sscanf(part, "%30s %d", defmess, &nume);
    return stat->EnumVal(nume);
  }
  if (part[0] == 'i')
  {
    Standard_Integer ilim;
    if (!stat->IntegerLimit((part[2] == 'a'), ilim))
      return "";
    Sprintf(defmess, "%d", ilim);
    return defmess;
  }
  if (part[0] == 'r')
  {
    Standard_Real rlim;
    if (!stat->RealLimit((part[2] == 'a'), rlim))
      return "";
    Sprintf(defmess, "%f", rlim);
    return defmess;
  }
  if (part[0] == 'u')
    return stat->UnitDef();
  return "";
}

Standard_Integer ExchangeConfig::IDef(const Standard_CString name, const Standard_CString part)
{
  if (!part || part[0] == '\0')
    return 0;
  Handle(ExchangeConfig) stat = ExchangeConfig::Static(name);
  if (stat.IsNull())
    return 0;
  if (part[0] == 'i')
  {
    Standard_Integer ilim;
    if (!stat->IntegerLimit((part[2] == 'a'), ilim))
      return 0;
    return ilim;
  }
  if (part[0] == 'e')
  {
    Standard_Integer startcase, endcase;
    Standard_Boolean match;
    stat->EnumDef(startcase, endcase, match);
    if (part[1] == 's')
      return startcase;
    if (part[1] == 'c')
      return (endcase - startcase + 1);
    if (part[1] == 'm')
      return (match ? 1 : 0);
    if (part[1] == 'v')
    {
      char vale[51];
      sscanf(part, "%30s %50s", defmess, vale);
      return stat->EnumCase(vale);
    }
  }
  return 0;
}

//  ##########  VALEUR COURANTE  ###########

Standard_Boolean ExchangeConfig::IsSet(const Standard_CString name, const Standard_Boolean proper)
{
  Handle(ExchangeConfig) item = ExchangeConfig::Static(name);
  if (item.IsNull())
    return Standard_False;
  if (item->IsSetValue())
    return Standard_True;
  if (proper)
    return Standard_False;
  item = item->Wild();
  return item->IsSetValue();
}

Standard_CString ExchangeConfig::CVal(const Standard_CString name)
{
  Handle(ExchangeConfig) item = ExchangeConfig::Static(name);
  if (item.IsNull())
  {
#ifdef OCCT_DEBUG
    std::cout << "Warning: ExchangeConfig::CVal: incorrect parameter " << name << std::endl;
#endif
    return "";
  }
  return item->CStringValue();
}

Standard_Integer ExchangeConfig::IVal(const Standard_CString name)
{
  Handle(ExchangeConfig) item = ExchangeConfig::Static(name);
  if (item.IsNull())
  {
#ifdef OCCT_DEBUG
    std::cout << "Warning: ExchangeConfig::IVal: incorrect parameter " << name << std::endl;
#endif
    return 0;
  }
  return item->IntegerValue();
}

Standard_Real ExchangeConfig::RVal(const Standard_CString name)
{
  Handle(ExchangeConfig) item = ExchangeConfig::Static(name);
  if (item.IsNull())
  {
#ifdef OCCT_DEBUG
    std::cout << "Warning: ExchangeConfig::RVal: incorrect parameter " << name << std::endl;
#endif
    return 0.0;
  }
  return item->RealValue();
}

Standard_Boolean ExchangeConfig::SetCVal(const Standard_CString name, const Standard_CString val)
{
  Handle(ExchangeConfig) item = ExchangeConfig::Static(name);
  if (item.IsNull())
    return Standard_False;
  return item->SetCStringValue(val);
}

Standard_Boolean ExchangeConfig::SetIVal(const Standard_CString name, const Standard_Integer val)
{
  Handle(ExchangeConfig) item = ExchangeConfig::Static(name);
  if (item.IsNull())
    return Standard_False;
  if (!item->SetIntegerValue(val))
    return Standard_False;
  return Standard_True;
}

Standard_Boolean ExchangeConfig::SetRVal(const Standard_CString name, const Standard_Real val)
{
  Handle(ExchangeConfig) item = ExchangeConfig::Static(name);
  if (item.IsNull())
    return Standard_False;
  return item->SetRealValue(val);
}

//    UPDATE

Standard_Boolean ExchangeConfig::Update(const Standard_CString name)
{
  Handle(ExchangeConfig) item = ExchangeConfig::Static(name);
  if (item.IsNull())
    return Standard_False;
  item->SetUptodate();
  return Standard_True;
}

Standard_Boolean ExchangeConfig::IsUpdated(const Standard_CString name)
{
  Handle(ExchangeConfig) item = ExchangeConfig::Static(name);
  if (item.IsNull())
    return Standard_False;
  return item->UpdatedStatus();
}

Handle(TColStd_HSequenceOfHAsciiString) ExchangeConfig::Items(const Standard_Integer mode,
                                                                const Standard_CString criter)
{
  Standard_Integer                        modup = (mode / 100); // 0 any, 1 non-update, 2 update
  Handle(TColStd_HSequenceOfHAsciiString) list  = new TColStd_HSequenceOfHAsciiString();
  NCollection_DataMap<AsciiString1, Handle(RefObject)>::Iterator iter(
    MoniTool_TypedValue::Stats());
  for (; iter.More(); iter.Next())
  {
    Handle(ExchangeConfig) item = Handle(ExchangeConfig)::DownCast(iter.Value());
    if (item.IsNull())
      continue;
    Standard_Boolean ok = Standard_True;
    if (criter[0] == '$' && criter[1] == '\0')
    {
      if ((item->Family())[0] != '$')
        ok = Standard_False;
    }
    else if (criter[0] != '\0')
    {
      if (strcmp(criter, item->Family()))
        continue;
      ok = Standard_True;
    }
    else
    { // tous ... sauf famille a $
      if (item->Family()[0] == '$')
        continue;
    }
    if (ok && (modup == 1))
      ok = !item->UpdatedStatus();
    if (ok && (modup == 2))
      ok = item->UpdatedStatus();

    if (ok)
      list->Append(new TCollection_HAsciiString(iter.Key1()));
  }
  return list;
}

//=======================================================================
// function : FillMap
// purpose  : Fills given string-to-string map with all static data
//=======================================================================
void ExchangeConfig::FillMap(
  NCollection_DataMap<AsciiString1, AsciiString1>& theMap)
{
  theMap.Clear();

  NCollection_DataMap<AsciiString1, Handle(RefObject)>& aMap =
    MoniTool_TypedValue::Stats();

  for (NCollection_DataMap<AsciiString1, Handle(RefObject)>::Iterator anIt(
         aMap);
       anIt.More();
       anIt.Next())
  {
    Handle(ExchangeConfig) aValue = Handle(ExchangeConfig)::DownCast(anIt.Value());
    if (aValue.IsNull())
    {
      continue;
    }
    if (aValue->HStringValue().IsNull())
    {
      continue;
    }

    theMap.Bind(anIt.Key1(), aValue->HStringValue()->String());
  }
}
