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

#include <tpropertymap.h>

#include <bitset>

#include "dfffile.h"
#include "dffheader.h"

using namespace TagLib;

class DFF::File::FilePrivate
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
  AudioProperties *properties;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DFF::File::File(FileName file, bool readProperties,
		AudioProperties::ReadStyle propertiesStyle) 
  : TagLib::File(file)
{
  d = new FilePrivate;

  if(isOpen())
    read(readProperties, propertiesStyle);
}

DFF::File::File(TagLib::IOStream *stream, 
		 bool readProperties, 
		 TagLib::AudioProperties::ReadStyle propertiesStyle) :
  TagLib::File(stream)
{
  d = new FilePrivate;

  if(isOpen())
    read(readProperties, propertiesStyle);
}


DFF::File::~File()
{
  delete d;
}

DFF::AudioProperties *DFF::File::audioProperties() const
{
  return d->properties;
}

bool DFF::File::save() 
{
    std::cerr << "DFFFile::save() -- Not Supported (no tagging support in DSDIFF)" << std::endl;
    return false;
}

/*
TagLib::Tag *DFFFile::tag() const
{
  return NULL;
}
*/

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DFF::File::read(bool readProperties, 
		   TagLib::AudioProperties::ReadStyle propertiesStyle)
{
    if(readProperties) {
        d->properties = new AudioProperties(this);
        d->fileSize = d->properties->fileSize();
    }
}
