// Created on: 2015-03-15
// Created by: Danila ULYANOV
// Copyright (c) 2014 OPEN CASCADE SAS
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

#include <ViewerTest_CmdParser.hxx>

#include <Draw.hxx>
#include <Message.hxx>
#include <ViewerTest.hxx>

#include <algorithm>
#include <iostream>
#include <limits>

namespace
{

//! Converts the given string to lowercase
//! @param theString the string to be converted
//! @return a converted string (a string in lowercase)
static std::string toLowerCase(std::string theString)
{
  std::transform(theString.begin(), theString.end(), theString.begin(), ::LowerCase);
  return theString;
}

//! Converts the vector of std::strings to a vector of pointers to its data
//! @param theStringList the vector of strings to be converted
//! @return a vector of pointers to the data of given strings
static std::vector<const char*> convertToRawStringList(
  const std::vector<std::string>& theStringList)
{
  const std::size_t        aListSize = theStringList.size();
  std::vector<const char*> aRawStringList(aListSize);
  for (std::size_t anIndex = 0; anIndex < aListSize; ++anIndex)
  {
    aRawStringList[anIndex] = theStringList[anIndex].c_str();
  }
  return aRawStringList;
}

} // namespace

const std::size_t CommandParser::THE_UNNAMED_COMMAND_OPTION_KEY =
  (std::numeric_limits<std::size_t>::max)();

const std::size_t CommandParser::THE_HELP_COMMAND_OPTION_KEY = 0;

//=================================================================================================

CommandParser::CommandParser(const std::string& theDescription)
    : myDescription(theDescription)
{
  AddOption("help|h", "Prints a short description of the command and its options.");
}

//=================================================================================================

ViewerTest_CommandOptionKey CommandParser::AddOption(const std::string& theOptionNames,
                                                            const std::string& theOptionDescription)
{
  CommandOption1 aNewOption;

  // extract option names
  std::vector<std::string> aNames;
  std::stringstream        aStream(theOptionNames);
  std::string              anItem;
  while (std::getline(aStream, anItem, '|'))
  {
    if (!anItem.empty())
    {
      aNames.push_back(anItem);
    }
  }

  aNewOption.Name = aNames.front();
  if (aNames.size() > 1)
  {
    const std::size_t aNumberOfAliases = aNames.size() - 1;

    aNewOption.Aliases.reserve(aNumberOfAliases);
    std::copy(aNames.begin() + 1, aNames.end(), std::back_inserter(aNewOption.Aliases));
  }
  aNewOption.Description = theOptionDescription;

  const ViewerTest_CommandOptionKey aNewOptionKey = myOptionStorage.size();

  myOptionStorage.push_back(aNewOption);

  std::vector<std::string>::const_iterator anIt = aNames.begin();
  for (; anIt != aNames.end(); ++anIt)
  {
    const std::string aNameInLowerCase = toLowerCase(*anIt);

    myOptionMap[aNameInLowerCase] = aNewOptionKey;
  }

  return aNewOptionKey;
}

//=================================================================================================

void CommandParser::PrintHelp() const
{
  std::cout << myDescription << std::endl;
  std::vector<CommandOption1>::const_iterator anIt = myOptionStorage.begin();
  for (++anIt; anIt != myOptionStorage.end(); ++anIt)
  {
    const CommandOption1& aCommandOption = *anIt;
    std::cout << "\n\t-" << aCommandOption.Name;
    const OptionAliases& anAliases = aCommandOption.Aliases;
    if (!anAliases.empty())
    {
      std::cout << " (-" << anAliases.front();
      for (OptionAliases::const_iterator anAliasIterator = anAliases.begin() + 1;
           anAliasIterator != anAliases.end();
           ++anAliasIterator)
      {
        std::cout << ", -" << *anAliasIterator;
      }
      std::cout << ")";
    }
    std::cout << " : " << aCommandOption.Description;
  }
  std::cout << std::endl;
}

//=================================================================================================

void CommandParser::Parse(const Standard_Integer   theArgsNb,
                                 const char* const* const theArgVec)
{
  std::size_t aCurrentUsedOptionIndex = 0;
  for (Standard_Integer anIter = 1; anIter < theArgsNb; ++anIter)
  {
    const char* const anArgument = theArgVec[anIter];
    if (anArgument[0] == '-' && !std::isdigit(anArgument[1]))
    {
      const std::string   anOptionName = toLowerCase(anArgument + 1);
      OptionMap::iterator aMapIter     = myOptionMap.find(anOptionName);
      if (aMapIter != myOptionMap.end())
      {
        const ViewerTest_CommandOptionKey aCurrentUsedOptionKey = aMapIter->second;
        aCurrentUsedOptionIndex = addUsedOption(aCurrentUsedOptionKey);
      }
      else
      {
        Message1::SendFail() << "Error: unknown argument '" << anOptionName << "'";
        return;
      }
    }
    else
    {
      if (anIter == 1)
      {
        aCurrentUsedOptionIndex = addUsedOption(THE_UNNAMED_COMMAND_OPTION_KEY);
      }
      myOptionArgumentStorage[aCurrentUsedOptionIndex].push_back(anArgument);
    }
  }
}

//=================================================================================================

std::string CommandParser::GetOptionNameByKey(
  const ViewerTest_CommandOptionKey theOptionKey) const
{
  if (theOptionKey == THE_UNNAMED_COMMAND_OPTION_KEY)
  {
    return "Unnamed";
  }
  return myOptionStorage[theOptionKey].Name;
}

//=================================================================================================

ViewerTest_CommandOptionKeySet CommandParser::GetUsedOptions() const
{
  ViewerTest_CommandOptionKeySet aUsedOptions;
  for (UsedOptionMap::const_iterator aUsedOptionMapIterator = myUsedOptionMap.begin();
       aUsedOptionMapIterator != myUsedOptionMap.end();
       ++aUsedOptionMapIterator)
  {
    aUsedOptions.insert(aUsedOptionMapIterator->first);
  }
  return aUsedOptions;
}

//=================================================================================================

bool CommandParser::HasNoOption() const
{
  return myUsedOptionMap.empty();
}

//=================================================================================================

bool CommandParser::HasUnnamedOption() const
{
  return myUsedOptionMap.find(THE_UNNAMED_COMMAND_OPTION_KEY) != myUsedOptionMap.end();
}

//=================================================================================================

bool CommandParser::HasOnlyUnnamedOption() const
{
  return HasUnnamedOption() && (myUsedOptionMap.size() == 1);
}

//=================================================================================================

bool CommandParser::HasOption(const std::string& theOptionName,
                                     const std::size_t  theMandatoryArgsNb /* = 0 */,
                                     const bool         isFatal /* = false */) const
{
  ViewerTest_CommandOptionKey anOptionKey;
  if (!findOptionKey(theOptionName, anOptionKey))
  {
    return false;
  }
  return HasOption(anOptionKey, theMandatoryArgsNb, isFatal);
}

//=================================================================================================

bool CommandParser::HasOption(const ViewerTest_CommandOptionKey theOptionKey,
                                     const std::size_t                 theMandatoryArgsNb /* = 0 */,
                                     const bool                        isFatal /* = false */) const
{
  std::size_t aUsedOptionIndex = 0;
  if (!findUsedOptionIndex(theOptionKey, aUsedOptionIndex))
  {
    return false;
  }
  const OptionArguments& anOptionArguments = myOptionArgumentStorage[aUsedOptionIndex];
  const bool             aResult           = (anOptionArguments.size() >= theMandatoryArgsNb);
  if (isFatal && !aResult)
  {
    Message1::SendFail() << "Error: wrong syntax at option '" << myOptionStorage[theOptionKey].Name
                        << "'\n"
                        << "At least " << theMandatoryArgsNb << "expected, but only "
                        << anOptionArguments.size() << "provided.";
  }
  return aResult;
}

//=================================================================================================

Standard_Integer CommandParser::GetNumberOfOptionArguments(
  const std::string& theOptionName) const
{
  ViewerTest_CommandOptionKey anOptionKey = THE_UNNAMED_COMMAND_OPTION_KEY;
  if (!findOptionKey(theOptionName, anOptionKey))
  {
    return 0;
  }
  return GetNumberOfOptionArguments(anOptionKey);
}

//=================================================================================================

Standard_Integer CommandParser::GetNumberOfOptionArguments(
  const ViewerTest_CommandOptionKey theOptionKey) const
{
  std::size_t aUsedOptionIndex = 0;
  if (!findUsedOptionIndex(theOptionKey, aUsedOptionIndex))
  {
    return false;
  }
  return static_cast<Standard_Integer>(myOptionArgumentStorage[aUsedOptionIndex].size());
}

//=================================================================================================

bool CommandParser::Arg(const std::string&     theOptionName,
                               const Standard_Integer theArgumentIndex,
                               std::string&           theOptionArgument) const
{
  Standard_ASSERT_RETURN(theArgumentIndex >= 0,
                         "'theArgumentIndex' must be greater than or equal to zero.",
                         false);
  ViewerTest_CommandOptionKey anOptionKey = THE_UNNAMED_COMMAND_OPTION_KEY;
  if (!theOptionName.empty() && !findOptionKey(theOptionName, anOptionKey))
  {
    return false;
  }
  return Arg(anOptionKey, theArgumentIndex, theOptionArgument);
}

//=================================================================================================

bool CommandParser::Arg(const ViewerTest_CommandOptionKey theOptionKey,
                               const Standard_Integer            theArgumentIndex,
                               std::string&                      theOptionArgument) const
{
  Standard_ASSERT_RETURN(theArgumentIndex >= 0,
                         "'theArgumentIndex' must be greater than or equal to zero.",
                         false);
  std::size_t aUsedOptionIndex = 0;
  if (!findUsedOptionIndex(theOptionKey, aUsedOptionIndex))
  {
    return false;
  }
  const OptionArguments& anOptionArguments = myOptionArgumentStorage[aUsedOptionIndex];
  if (static_cast<std::size_t>(theArgumentIndex) >= anOptionArguments.size())
  {
    return false;
  }
  theOptionArgument = anOptionArguments[theArgumentIndex];
  return true;
}

//=================================================================================================

std::string CommandParser::Arg(const std::string&     theOptionName,
                                      const Standard_Integer theArgumentIndex) const
{
  Standard_ASSERT_RETURN(theArgumentIndex >= 0,
                         "'theArgumentIndex' must be greater than or equal to zero.",
                         std::string());
  std::string anOptionArgument;
  if (!Arg(theOptionName, theArgumentIndex, anOptionArgument))
  {
    return std::string();
  }
  return anOptionArgument;
}

//=================================================================================================

std::string CommandParser::Arg(const ViewerTest_CommandOptionKey theOptionKey,
                                      const Standard_Integer            theArgumentIndex) const
{
  std::string anOptionArgument;
  if (!Arg(theOptionKey, theArgumentIndex, anOptionArgument))
  {
    return std::string();
  }
  return anOptionArgument;
}

//=================================================================================================

Graphic3d_Vec3 CommandParser::ArgVec3f(const std::string& theOptionName,
                                              Standard_Integer   theArgumentIndex) const
{
  return Graphic3d_Vec3(
    static_cast<Standard_ShortReal>(Draw1::Atof(Arg(theOptionName, theArgumentIndex).c_str())),
    static_cast<Standard_ShortReal>(Draw1::Atof(Arg(theOptionName, theArgumentIndex + 1).c_str())),
    static_cast<Standard_ShortReal>(Draw1::Atof(Arg(theOptionName, theArgumentIndex + 2).c_str())));
}

//=================================================================================================

Graphic3d_Vec3d CommandParser::ArgVec3d(const std::string& theOptionName,
                                               Standard_Integer   theArgumentIndex) const
{
  return Graphic3d_Vec3d(Draw1::Atof(Arg(theOptionName, theArgumentIndex).c_str()),
                         Draw1::Atof(Arg(theOptionName, theArgumentIndex + 1).c_str()),
                         Draw1::Atof(Arg(theOptionName, theArgumentIndex + 2).c_str()));
}

//=================================================================================================

Vector3d CommandParser::ArgVec(const std::string& theOptionName,
                                    Standard_Integer   theArgumentIndex) const
{
  return Vector3d(Draw1::Atof(Arg(theOptionName, theArgumentIndex).c_str()),
                Draw1::Atof(Arg(theOptionName, theArgumentIndex + 1).c_str()),
                Draw1::Atof(Arg(theOptionName, theArgumentIndex + 2).c_str()));
}

//=================================================================================================

Point3d CommandParser::ArgPnt(const std::string& theOptionName,
                                    Standard_Integer   theArgumentIndex) const
{
  return Point3d(Draw1::Atof(Arg(theOptionName, theArgumentIndex).c_str()),
                Draw1::Atof(Arg(theOptionName, theArgumentIndex + 1).c_str()),
                Draw1::Atof(Arg(theOptionName, theArgumentIndex + 2).c_str()));
}

//=================================================================================================

Standard_Real CommandParser::ArgDouble(const std::string& theOptionName,
                                              Standard_Integer   theArgumentIndex) const
{
  return Draw1::Atof(Arg(theOptionName, theArgumentIndex).c_str());
}

//=================================================================================================

Standard_ShortReal CommandParser::ArgFloat(const std::string& theOptionName,
                                                  Standard_Integer   theArgumentIndex) const
{
  return static_cast<Standard_ShortReal>(Draw1::Atof(Arg(theOptionName, theArgumentIndex).c_str()));
}

//=================================================================================================

Standard_Integer CommandParser::ArgInt(const std::string&     theOptionName,
                                              const Standard_Integer theArgumentIndex) const
{
  return static_cast<Standard_Integer>(Draw1::Atoi(Arg(theOptionName, theArgumentIndex).c_str()));
}

//=================================================================================================

bool CommandParser::ArgBool(const std::string&     theOptionName,
                                   const Standard_Integer theArgumentIndex) const
{
  return Draw1::Atoi(Arg(theOptionName, theArgumentIndex).c_str()) != 0;
}

//=================================================================================================

template <typename TheColor>
bool CommandParser::ArgColor(const std::string& theOptionName,
                                    Standard_Integer&  theArgumentIndex,
                                    TheColor&          theColor) const
{
  ViewerTest_CommandOptionKey anOptionKey;
  if (!findOptionKey(theOptionName, anOptionKey))
  {
    return false;
  }
  return ArgColor(anOptionKey, theArgumentIndex, theColor);
}

//! CommandParser::ArgColor() explicit template instantiation definitions
template bool CommandParser::ArgColor(const std::string& theOptionName,
                                             Standard_Integer&  theArgumentIndex,
                                             Quantity_Color&    theColor) const;

template bool CommandParser::ArgColor(const std::string&  theOptionName,
                                             Standard_Integer&   theArgumentIndex,
                                             Quantity_ColorRGBA& theColor) const;

//=================================================================================================

template <typename TheColor>
bool CommandParser::ArgColor(const ViewerTest_CommandOptionKey theOptionKey,
                                    Standard_Integer&                 theArgumentIndex,
                                    TheColor&                         theColor) const
{
  std::size_t aUsedOptionIndex = 0;
  if (!findUsedOptionIndex(theOptionKey, aUsedOptionIndex))
  {
    return false;
  }
  const RawStringArguments aRawStringArguments = getRawStringArguments(aUsedOptionIndex);
  const Standard_Integer   aNumberOfArguments =
    static_cast<Standard_Integer>(aRawStringArguments.size());
  Standard_ASSERT_RETURN(
    theArgumentIndex < aNumberOfArguments,
    "'theArgumentIndex' must be less than the number of command-line arguments "
    "passed with the option which access key is 'theOptionKey'.",
    false);
  const Standard_Integer aNumberOfAvailableArguments = aNumberOfArguments - theArgumentIndex;
  TheColor               aColor;
  const Standard_Integer aNumberOfParsedArguments =
    Draw1::ParseColor(aNumberOfAvailableArguments, &aRawStringArguments[theArgumentIndex], aColor);
  if (aNumberOfParsedArguments == 0)
  {
    return false;
  }
  theArgumentIndex += aNumberOfParsedArguments;
  theColor = aColor;
  return true;
}

//! CommandParser::ArgColor() explicit template instantiation definitions
template bool CommandParser::ArgColor(ViewerTest_CommandOptionKey theOptionKey,
                                             Standard_Integer&           theArgumentIndex,
                                             Quantity_Color&             theColor) const;

template bool CommandParser::ArgColor(ViewerTest_CommandOptionKey theOptionKey,
                                             Standard_Integer&           theArgumentIndex,
                                             Quantity_ColorRGBA&         theColor) const;

//=================================================================================================

bool CommandParser::findOptionKey(const std::string&           theOptionName,
                                         ViewerTest_CommandOptionKey& theOptionKey) const
{
  const std::string               anOptionNameInLowercase = toLowerCase(theOptionName);
  const OptionMap::const_iterator aMapIter = myOptionMap.find(anOptionNameInLowercase);
  if (aMapIter == myOptionMap.end())
  {
    return false;
  }
  theOptionKey = aMapIter->second;
  return true;
}

//=================================================================================================

bool CommandParser::findUsedOptionIndex(const ViewerTest_CommandOptionKey theOptionKey,
                                               std::size_t& theUsedOptionIndex) const
{
  const UsedOptionMap::const_iterator aUsedOptionIterator = myUsedOptionMap.find(theOptionKey);
  if (aUsedOptionIterator == myUsedOptionMap.end())
  {
    return false;
  }
  theUsedOptionIndex = aUsedOptionIterator->second;
  return true;
}

//=================================================================================================

bool CommandParser::findUsedOptionIndex(const std::string& theOptionName,
                                               std::size_t&       theUsedOptionIndex) const
{
  ViewerTest_CommandOptionKey anOptionKey = THE_UNNAMED_COMMAND_OPTION_KEY;
  if (!findOptionKey(theOptionName, anOptionKey))
  {
    return false;
  }
  std::size_t aUsedOptionIndex = 0;
  if (!findUsedOptionIndex(anOptionKey, aUsedOptionIndex))
  {
    return false;
  }
  theUsedOptionIndex = aUsedOptionIndex;
  return true;
}

//=================================================================================================

std::size_t CommandParser::addUsedOption(
  const ViewerTest_CommandOptionKey theNewUsedOptionKey)
{
  const std::size_t aNewUsedOptionIndex = myOptionArgumentStorage.size();
  myOptionArgumentStorage.push_back(OptionArguments());
  myUsedOptionMap[theNewUsedOptionKey] = aNewUsedOptionIndex;
  return aNewUsedOptionIndex;
}

//=================================================================================================

CommandParser::RawStringArguments CommandParser::getRawStringArguments(
  const std::size_t theUsedOptionIndex) const
{
  Standard_ASSERT_RETURN(
    theUsedOptionIndex < myOptionArgumentStorage.size(),
    "'theUsedOptionIndex' must be less than the size of 'myOptionArgumentStorage'.",
    RawStringArguments());
  const OptionArguments& anOptionArguments = myOptionArgumentStorage[theUsedOptionIndex];
  return convertToRawStringList(anOptionArguments);
}
