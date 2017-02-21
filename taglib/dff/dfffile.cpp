/***************************************************************************
    copyright            : (C) 2014 Roon Labs LLC
    email                : brian@roonlabs.com
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 ***************************************************************************/

#include <stdint.h>

#include <taglib/toolkit/tpropertymap.h>

#include <bitset>

#include "dfffile.h"
#include "dffheader.h"

//using namespace TagLib;

class DFFFile::FilePrivate
{
public:
  FilePrivate() :
    fileSize(0),
    properties(0)
  {}

  ~FilePrivate()
  {
    if (properties) delete properties;
  }
  unsigned long long fileSize;
  DFFProperties *properties;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DFFFile::DFFFile(TagLib::FileName file, bool readProperties,
		 TagLib::AudioProperties::ReadStyle propertiesStyle) 
  : TagLib::File(file)
{
  d = new FilePrivate;

  if(isOpen())
    read(readProperties, propertiesStyle);
}

DFFFile::DFFFile(TagLib::IOStream *stream, 
		 bool readProperties, 
		 TagLib::AudioProperties::ReadStyle propertiesStyle) :
  TagLib::File(stream)
{
  d = new FilePrivate;

  if(isOpen())
    read(readProperties, propertiesStyle);
}


DFFFile::~DFFFile()
{
  delete d;
}

TagLib::PropertyMap DFFFile::properties() const
{
  return TagLib::PropertyMap();
}

void DFFFile::removeUnsupportedProperties(const TagLib::StringList &properties)
{
}

TagLib::AudioProperties *DFFFile::audioProperties() const
{
  return d->properties;
}

bool DFFFile::save() 
{
    std::cerr << "DFFFile::save() -- Not Supported (no tagging support in DSDIFF)" << std::endl;
    return false;
}

TagLib::Tag *DFFFile::tag() const
{
  return NULL;
}


////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DFFFile::read(bool readProperties, 
		   TagLib::AudioProperties::ReadStyle propertiesStyle)
{
    if(readProperties) {
        d->properties = new DFFProperties(this, propertiesStyle);
        d->fileSize = d->properties->fileSize();
    }
}
