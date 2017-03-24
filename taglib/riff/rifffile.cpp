/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
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
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include <algorithm>
#include <vector>

#include <tbytevector.h>
#include <tdebug.h>
#include <tstring.h>

#include "rifffile.h"
#include "riffutils.h"

using namespace TagLib;

namespace
{
  struct Chunk
  {
    ByteVector         name;
    long long          offset;
    unsigned long long size;
    unsigned int       padding;
  };

  unsigned int toUInt32(const ByteVector &v, size_t offset, ByteOrder endian)
  {
    if(endian == BigEndian)
      return v.toUInt32BE(offset);
    else
      return v.toUInt32LE(offset);
  }

  ByteVector fromUInt32(size_t value, ByteOrder endian)
  {
    if(endian == BigEndian)
      return ByteVector::fromUInt32BE(value);
    else
      return ByteVector::fromUInt32LE(value);
  }

  unsigned long long toUInt64(const ByteVector &v, size_t offset, ByteOrder endian)
  {
    if(endian == BigEndian)
      return v.toUInt64BE(offset);
    else
      return v.toUInt64LE(offset);
  }

}

class RIFF::File::FilePrivate
{
public:
  explicit FilePrivate(ByteOrder endianness) :
    endianness(endianness),
    size(0),
    sizeOffset(0) {}

  const ByteOrder endianness;
  ByteVector type;
  unsigned long long size;
  long long sizeOffset;

  std::vector<Chunk> chunks;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::File::~File()
{
  delete d;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

RIFF::File::File(FileName file, ByteOrder endianness) :
  TagLib::File(file),
  d(new FilePrivate(endianness))
{
  if(isOpen())
    read();
}

RIFF::File::File(IOStream *stream, ByteOrder endianness) :
  TagLib::File(stream),
  d(new FilePrivate(endianness))
{
  if(isOpen())
    read();
}

unsigned long long RIFF::File::riffSize() const
{
  return d->size;
}

size_t RIFF::File::chunkCount() const
{
  return d->chunks.size();
}

unsigned long long RIFF::File::chunkDataSize(unsigned int i) const
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::chunkPadding() - Index out of range. Returning 0.");
    return 0;
  }

  return d->chunks[i].size;
}

long long RIFF::File::chunkOffset(unsigned int i) const
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::chunkPadding() - Index out of range. Returning 0.");
    return 0;
  }

  return d->chunks[i].offset;
}

unsigned int RIFF::File::chunkPadding(unsigned int i) const
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::chunkPadding() - Index out of range. Returning 0.");
    return 0;
  }

  return d->chunks[i].padding;
}

ByteVector RIFF::File::chunkName(unsigned int i) const
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::chunkName() - Index out of range. Returning an empty vector.");
    return ByteVector();
  }

  return d->chunks[i].name;
}

ByteVector RIFF::File::chunkData(unsigned int i)
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::chunkData() - Index out of range. Returning an empty vector.");
    return ByteVector();
  }

  seek(d->chunks[i].offset);
  return readBlock(d->chunks[i].size);
}

void RIFF::File::setChunkData(unsigned int i, const ByteVector &data)
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::setChunkData() - Index out of range.");
    return;
  }

  // Now update the specific chunk

  std::vector<Chunk>::iterator it = d->chunks.begin();
  std::advance(it, i);

  const long long originalSize = static_cast<long long>(it->size) + it->padding;

  writeChunk(it->name, data, it->offset - 8, it->size + it->padding + 8);

  it->size    = static_cast<unsigned int>(data.size());
  it->padding = data.size() % 2;

  const long long diff = static_cast<long long>(it->size) + it->padding - originalSize;

  // Now update the internal offsets

  for(++it; it != d->chunks.end(); ++it)
    it->offset += diff;

  // Update the global size.

  updateGlobalSize();
}

void RIFF::File::setChunkData(const ByteVector &name, const ByteVector &data)
{
  setChunkData(name, data, false);
}

void RIFF::File::setChunkData(const ByteVector &name, const ByteVector &data, bool alwaysCreate)
{
  if(d->chunks.size() == 0) {
    debug("RIFF::File::setChunkData - No valid chunks found.");
    return;
  }

  if(alwaysCreate && name != "LIST") {
    debug("RIFF::File::setChunkData - alwaysCreate should be used for only \"LIST\" chunks.");
    return;
  }

  if(!alwaysCreate) {
    for(unsigned int i = 0; i < d->chunks.size(); i++) {
      if(d->chunks[i].name == name) {
        setChunkData(i, data);
        return;
      }
    }
  }

  // Couldn't find an existing chunk, so let's create a new one.

  // Adjust the padding of the last chunk to place the new chunk at even position.

  Chunk &last = d->chunks.back();

  long long offset = last.offset + last.size + last.padding;
  if(offset & 1) {
    if(last.padding == 1) {
      last.padding = 0; // This should not happen unless the file is corrupted.
      offset--;
      removeBlock(offset, 1);
    }
    else {
      insert(ByteVector("\0", 1), offset, 0);
      last.padding = 1;
      offset++;
    }
  }

  // Now add the chunk to the file.

  writeChunk(name, data, offset, 0);

  // And update our internal structure

  Chunk chunk;
  chunk.name    = name;
  chunk.size    = static_cast<unsigned int>(data.size());
  chunk.offset  = offset + 8;
  chunk.padding = data.size() % 2;

  d->chunks.push_back(chunk);

  // Update the global size.

  updateGlobalSize();
}

void RIFF::File::removeChunk(unsigned int i)
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::removeChunk() - Index out of range.");
    return;
  }

  std::vector<Chunk>::iterator it = d->chunks.begin();
  std::advance(it, i);

  const size_t removeSize = it->size + it->padding + 8;
  removeBlock(it->offset - 8, removeSize);
  it = d->chunks.erase(it);

  for(; it != d->chunks.end(); ++it)
    it->offset -= removeSize;

  // Update the global size.

  updateGlobalSize();
}

void RIFF::File::removeChunk(const ByteVector &name)
{
  for(int i = static_cast<int>(d->chunks.size()) - 1; i >= 0; --i) {
    if(d->chunks[i].name == name)
      removeChunk(i);
  }
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RIFF::File::read()
{
  // note: if at any point we decide file is unreadable, we want to just bail without parsing any chunks.

  d->type = readBlock(4);

  if (d->type == fromUInt32(0x72696666, BigEndian) && length() >= 40) { // sony wav64

    const char _riff_guid[] = { 0x72, 0x69, 0x66, 0x66, 0x2e, 0x91, 0xcf, 0x11, 0xa5, 0xd6, 0x28, 0xdb, 0x04, 0xc1, 0x00, 0x00 };
    const char _wave_guid[] = { 0x77, 0x61, 0x76, 0x65, 0xf3, 0xac, 0xd3, 0x11, 0x8c, 0xd1, 0x00, 0xc0, 0x4f, 0x8e, 0xdb, 0x8a };
    const char _fmt_guid[]  = { 0x66, 0x6d, 0x74, 0x20, 0xf3, 0xac, 0xd3, 0x11, 0x8c, 0xd1, 0x00, 0xc0, 0x4f, 0x8e, 0xdb, 0x8a };
    const char _data_guid[] = { 0x64, 0x61, 0x74, 0x61, 0xf3, 0xac, 0xd3, 0x11, 0x8c, 0xd1, 0x00, 0xc0, 0x4f, 0x8e, 0xdb, 0x8a };

    ByteVector riff_guid(_riff_guid, 16);
    ByteVector wave_guid(_wave_guid, 16);
    ByteVector fmt_guid(_fmt_guid, 16);
    ByteVector data_guid(_data_guid, 16);

    // ensure the remainder of the riff guid is there
    if (readBlock(12) != riff_guid.mid(4, 12)) return;
    // grab size
    ulong size = readBlock(8).toUInt64LE(0);
    // ensure the the wave guid is there
    if (readBlock(16) != wave_guid) return;

    d->size = size - 40; // size in wav64 includes header. remove to be compatible with standard wav.

    // + 8: chunk header at least, fix for additional junk bytes
    while (tell() + 24 <= length()) {
      ByteVector chunkGuid = readBlock(16);
      ByteVector chunkName = chunkGuid.mid(0, 4);
      ulong chunkSize = readBlock(8).toUInt64LE(0) - 24;

      if(!isValidChunkName(chunkName)) {
        debug("RIFF::File::read() -- Chunk '" + chunkName + "' has invalid ID");
        setValid(false);
        break;
      }
      if(tell() + chunkSize > ulong(length())) {
        debug("RIFF::File::read() -- Chunk '" + chunkName + "' has invalid size (larger than the file size)");
        setValid(false);
        break;
      }

      Chunk chunk;
      chunk.name = chunkName;
      chunk.size = chunkSize;
      chunk.offset = tell();

      seek(chunk.size, Current);

      // check padding
      chunk.padding = 0;
      ulong uPosNotPadded = tell();
      if((uPosNotPadded & 0x01) != 0) {
        ByteVector iByte = readBlock(1);
        if((iByte.size() != 1) || (iByte[0] != 0)) {
          // not well formed, re-seek
          seek(uPosNotPadded, Beginning);
        }
        else {
          chunk.padding = 1;
        }
      }
      d->chunks.push_back(chunk);
    }
  }
  else { // standard wav or aiff
    long long offset = tell();

    d->sizeOffset = offset;

    seek(offset);
    d->size = toUInt32(readBlock(4), 0, d->endianness);

    offset += 8;

    // + 8: chunk header at least, fix for additional junk bytes
    while(offset + 8 <= length()) {

      seek(offset);
      const ByteVector   chunkName = readBlock(4);
      const unsigned int chunkSize = toUInt32(readBlock(4), 0, d->endianness);

      if(!isValidChunkName(chunkName)) {
        debug("RIFF::File::read() -- Chunk '" + chunkName + "' has invalid ID");
        setValid(false);
        break;
      }

      if(offset + 8 + chunkSize > length()) {
        debug("RIFF::File::read() -- Chunk '" + chunkName + "' has invalid size (larger than the file size)");
        setValid(false);
        break;
      }

      Chunk chunk;
      chunk.name    = chunkName;
      chunk.size    = chunkSize;
      chunk.offset  = offset + 8;
      chunk.padding = 0;

      offset = chunk.offset + chunk.size;

      // Check padding

      if(offset & 1) {
        seek(offset);
        const ByteVector iByte = readBlock(1);
        if(iByte.size() == 1 && iByte[0] == '\0') {
          chunk.padding = 1;
          offset++;
        }
      }

      d->chunks.push_back(chunk);
    }
  }
}

void RIFF::File::writeChunk(const ByteVector &name, const ByteVector &data,
                            long long offset, size_t replace)
{
  ByteVector combined;

  combined.append(name);
  combined.append(fromUInt32(data.size(), d->endianness));
  combined.append(data);

  if(data.size() & 1)
    combined.resize(combined.size() + 1, '\0');

  insert(combined, offset, replace);
}

void RIFF::File::updateGlobalSize()
{
  const Chunk first = d->chunks.front();
  const Chunk last  = d->chunks.back();
  d->size = static_cast<unsigned int>(last.offset + last.size + last.padding - first.offset + 12);

  const ByteVector data = fromUInt32(d->size, d->endianness);
  insert(data, d->sizeOffset, 4);
}
