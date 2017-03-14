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

#include <tstring.h>

#include "dffproperties.h"
#include "dfffile.h"
#include <roon_taglib_utils.h>

using namespace TagLib;

class DFF::AudioProperties::PropertiesPrivate
{
public:
  PropertiesPrivate(File *f, ReadStyle s) :
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

  File *file;
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

DFF::AudioProperties::AudioProperties(File *file, ReadStyle style) :
  d(new PropertiesPrivate(file, style))
{
  if(file && file->isOpen()) 
    read();
}

DFF::AudioProperties::~AudioProperties()
{
  delete d;
}

int DFF::AudioProperties::length() const
{
  return lengthInSeconds();
}

int DFF::AudioProperties::lengthInSeconds() const
{
  return d->sampleCount / d->sampleRate;
}

int DFF::AudioProperties::lengthInMilliseconds() const
{
  return (int)(d->sampleCount  * 1000.0 / d->sampleRate);
}

int DFF::AudioProperties::bitrate() const
{
  return d->sampleRate * d->bitsPerSample * d->channels / 1024;
}

int DFF::AudioProperties::sampleRate() const
{
  return d->sampleRate;
}

int DFF::AudioProperties::channels() const
{
  return d->channels;
}

unsigned int DFF::AudioProperties::version() const
{
  return d->version;
}

unsigned short DFF::AudioProperties::channelType() const
{
  return d->channelType;
}

unsigned long long DFF::AudioProperties::fileSize() const
{
  return d->fileSize;
}

unsigned long long DFF::AudioProperties::sampleCount() const
{
  return d->sampleCount;
}

int DFF::AudioProperties::bitsPerSample() const
{
  return d->bitsPerSample;
}

ByteVector DFF::AudioProperties::signature() const
{
  return d->signature;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DFF::AudioProperties::read()
{
  // Go to the beginning of the file
  d->file->seek(0);
  DFFHeader h(d->file);

  if (!h.isValid()) {
    std::cerr << "DFF::AudioProperties::read(): file header is not valid" << std::endl;
    return;
  }

  d->sampleRate = h.sampleRate();
  d->sampleCount = h.sampleCount();
  d->bitsPerSample = h.bitsPerSample();
  d->channels = h.channelNum();
  d->version = h.version();
  d->fileSize = h.fileSize();
  d->channelType = h.channelType();

  unsigned long long data_start = h.dataStart();
  unsigned long long data_end   = h.dataEnd();

  d->signature = taglib_make_signature(d->file, data_start, data_end - data_start);
}
