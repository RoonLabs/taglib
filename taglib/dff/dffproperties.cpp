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

#include <taglib/toolkit/tstring.h>

#include "dffproperties.h"
#include "dfffile.h"
#include <roon_taglib_utils.h>

class DFFProperties::PropertiesPrivate
{
public:
  PropertiesPrivate(DFFFile *f, ReadStyle s) :
    file(f),
    style(s),
    length(0),
    bitrate(0),
    sampleRate(0),
    channels(0),
    sampleCount(0),
    fileSize(0),
    bitsPerSample(1),
    version(0),
    channelType(0)
   {}

  DFFFile *file;
  TagLib::AudioProperties::ReadStyle style;
  int length;
  int bitrate;
  int sampleRate;
  int channels;
  unsigned long long sampleCount;
  unsigned long long fileSize;
  int bitsPerSample;
  unsigned int version;
  unsigned short channelType;
  ByteVector signature;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DFFProperties::DFFProperties(DFFFile *file, 
			     TagLib::AudioProperties::ReadStyle style) 
  : TagLib::AudioProperties(style)
{
  d = new PropertiesPrivate(file, style);

  if(file && file->isOpen()) 
    read();
}

DFFProperties::~DFFProperties()
{
  delete d;
}

int DFFProperties::length() const
{
  return d->sampleCount / d->sampleRate;
}

int DFFProperties::lengthMs() const
{
  return (int)(d->sampleCount  * 1000.0 / d->sampleRate);
}

int DFFProperties::bitrate() const
{
  return d->sampleRate * d->bitsPerSample * d->channels / 1024;
}

int DFFProperties::sampleRate() const
{
  return d->sampleRate;
}

int DFFProperties::channels() const
{
  return d->channels;
}

unsigned int DFFProperties::version() const
{
  return d->version;
}

unsigned short DFFProperties::channelType() const
{
  return d->channelType;
}

unsigned long long DFFProperties::fileSize() const
{
  return d->fileSize;
}

unsigned long long DFFProperties::sampleCount() const
{
  return d->sampleCount;
}

int DFFProperties::bitsPerSample() const
{
  return d->bitsPerSample;
}

ByteVector DFFProperties::signature() const
{
  return d->signature;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DFFProperties::read()
{
  // Go to the beginning of the file
  d->file->seek(0);
  DFFHeader h(d->file);

  if (!h.isValid()) {
    std::cerr << "DFFProperties::read(): file header is not valid" << std::endl;
    return;
  }

  d->sampleRate = h.sampleRate();
  d->sampleCount = h.sampleCount();
  d->bitsPerSample = h.bitsPerSample();
  d->channels = h.channelNum();
  d->version = h.version();
  d->fileSize = h.fileSize();
  d->channelType = h.channelType();

  ulonglong data_start = h.dataStart();
  ulonglong data_end   = h.dataEnd();

  d->signature = taglib_make_signature(d->file, data_start, data_end - data_start);
}
