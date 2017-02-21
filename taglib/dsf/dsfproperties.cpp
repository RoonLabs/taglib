/***************************************************************************
    copyright            : (C) 2014 by Peking Duck Labs
    email                : pekingducklabs@gmail.com
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

#include "dsfproperties.h"
#include "dsffile.h"
#include <roon_taglib_utils.h>

class DSFProperties::PropertiesPrivate
{
public:
  PropertiesPrivate(DSFFile *f, ReadStyle s) :
    file(f),
    style(s),
    length(0),
    bitrate(0),
    sampleRate(0),
    channels(0),
    ID3v2Offset(0),
    sampleCount(0),
    fileSize(0),
    bitsPerSample(1),
    version(DSFHeader::Version1),
    channelType(DSFHeader::Stereo)
   {}

  DSFFile *file;
  TagLib::AudioProperties::ReadStyle style;
  int length;
  int bitrate;
  int sampleRate;
  int channels;
  unsigned long long ID3v2Offset;
  unsigned long long sampleCount;
  unsigned long long fileSize;
  int bitsPerSample;
  DSFHeader::Version version;
  DSFHeader::ChannelType channelType;
  ByteVector signature;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DSFProperties::DSFProperties(DSFFile *file, 
			     TagLib::AudioProperties::ReadStyle style) 
  : TagLib::AudioProperties(style)
{
  d = new PropertiesPrivate(file, style);

  if(file && file->isOpen()) 
    read();
}

DSFProperties::~DSFProperties()
{
  delete d;
}

int DSFProperties::length() const
{
  return d->sampleCount / d->sampleRate;
}

int DSFProperties::lengthMs() const
{
  return (int)(d->sampleCount  * 1000.0 / d->sampleRate);
}

int DSFProperties::bitrate() const
{
  return d->sampleRate * d->bitsPerSample / 1024;
}

int DSFProperties::sampleRate() const
{
  return d->sampleRate;
}

int DSFProperties::channels() const
{
  return d->channels;
}

DSFHeader::Version DSFProperties::version() const
{
  return d->version;
}

DSFHeader::ChannelType DSFProperties::channelType() const
{
  return d->channelType;
}

unsigned long long DSFProperties::ID3v2Offset() const 
{
  return d->ID3v2Offset;
}

unsigned long long DSFProperties::fileSize() const
{
  return d->fileSize;
}

unsigned long long DSFProperties::sampleCount() const
{
  return d->sampleCount;
}

int DSFProperties::bitsPerSample() const
{
  return d->bitsPerSample;
}

ByteVector DSFProperties::signature() const
{
  return d->signature;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DSFProperties::read()
{
  // Go to the beginning of the file
  d->file->seek(0);

  DSFHeader h(d->file->readBlock(DSFHeader::DSD_HEADER_SIZE + 
				 DSFHeader::FMT_HEADER_SIZE));

  if (!h.isValid()) {
    std::cerr << "DSFProperties::read(): file header is not valid" << std::endl;
    return;
  }

  d->sampleRate = h.sampleRate();
  d->sampleCount = h.sampleCount();
  d->bitsPerSample = h.bitsPerSample();
  d->channels = h.channelNum();
  d->version = h.version();
  d->fileSize = h.fileSize();
  d->channelType = h.channelType();
  d->ID3v2Offset = h.ID3v2Offset();

  ulonglong data_start = DSFHeader::DSD_HEADER_SIZE + DSFHeader::FMT_HEADER_SIZE;
  ulonglong data_end   = (ulonglong)d->file->length();
  // if there is a valid id3v2 offset at end of file, exclude it from audio signature
  if (d->ID3v2Offset > 0 && d->ID3v2Offset < d->file->length()) {
      data_end = d->ID3v2Offset;
  }
  d->signature = taglib_make_signature(d->file, data_start, data_end - data_start);
}
