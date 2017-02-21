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

#include <bitset>

#include <taglib/toolkit/tfile.h>
#include <taglib/toolkit/tbytevector.h>
#include <taglib/toolkit/tstring.h>

#include "dffheader.h"
#include "dfffile.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static void LOG(const char *fmt, ...)
{
    /*
    va_list ap;
    va_start(ap, fmt); 
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    */
}

class DFFHeader::HeaderPrivate : public TagLib::RefCounter
{
public:
  HeaderPrivate() :
    isValid(false),
    version(4),
    sampleCount(0),
    channelType(0),
    channelNum(2),
    sampleRate(0),
    bitsPerSample(0),
    fileSize(0)
  {}

  bool               isValid;
  unsigned           version;           // DSDIFF version
  unsigned long long sampleCount;
  unsigned short     channelType;
  unsigned short     channelNum;
  unsigned int       sampleRate; 
  unsigned short     bitsPerSample;
  unsigned long long fileSize;
  unsigned long long dataStart;
  unsigned long long dataEnd;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DFFHeader::DFFHeader(DFFFile *file)
{
  d = new HeaderPrivate;
  parse(file);
}

DFFHeader::DFFHeader(const DFFHeader &h) : d(h.d)
{
  d->ref();
}

DFFHeader::~DFFHeader()
{
  if (d->deref())
    delete d;
}

bool DFFHeader::isValid() const
{
  return d->isValid;
}

unsigned int DFFHeader::version() const
{
  return d->version;
}

unsigned int DFFHeader::sampleRate() const
{
  return d->sampleRate;
}

unsigned short DFFHeader::channelType() const
{
  return d->channelType;
}

unsigned short DFFHeader::channelNum() const 
{
  return d->channelNum;
}

unsigned long long DFFHeader::dataStart() const
{
  return d->dataStart;
}

unsigned long long DFFHeader::dataEnd() const
{
  return d->dataEnd;
}

unsigned long long DFFHeader::sampleCount() const
{
  return d->sampleCount;
}

unsigned long long DFFHeader::fileSize() const
{
  return d->fileSize;
}

unsigned short DFFHeader::bitsPerSample() const
{
  return d->bitsPerSample;
}

DFFHeader &DFFHeader::operator=(const DFFHeader &h)
{
  if(&h == this)
    return *this;

  if(d->deref())
    delete d;

  d = h.d;
  d->ref();
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

static uint16_t decodebe16(const TagLib::ByteVector& bv, int off) { 
    uint8_t *b = (uint8_t*)bv.data();
    return ((uint16_t)b[off + 0] <<  8) | ((uint16_t)b[off + 1] <<  0); 
}

static uint32_t decodebe32(const TagLib::ByteVector& bv, int off) { 
    uint8_t *b = (uint8_t*)bv.data();
    return ((uint32_t)b[off + 0] << 24) | ((uint32_t)b[off + 1] << 16) | ((uint32_t)b[off + 2] <<  8) | ((uint32_t)b[off + 3] <<  0); 
}

static uint64_t decodebe64(const TagLib::ByteVector& bv, int off) { 
    uint8_t *b = (uint8_t*)bv.data();
    return ((uint64_t)b[off + 0] << 56) | ((uint64_t)b[off + 1] << 48) | ((uint64_t)b[off + 2] << 40) | ((uint64_t)b[off + 3] << 32) | ((uint64_t)b[off + 4] << 24) | ((uint64_t)b[off + 5] << 16) | ((uint64_t)b[off + 6] <<  8) | ((uint64_t)b[off + 7] <<  0); 
}

void DFFHeader::parse(DFFFile *file)
{
    LOG("parse");
    int64_t chunk_size, chunk_end;

    int          channel_num  = 0;
    int          sample_rate  = 0;
    int          version      = 0;
    short        channel_type = 0;

    int64_t sample_count = -1;
    int64_t data_start   = -1;
    int64_t data_end     = -1;

    TagLib::ByteVector frm8_chunk = file->readBlock(16);

    if (frm8_chunk[0] != 'F' || frm8_chunk[1] != 'R' || frm8_chunk[2] != 'M' || frm8_chunk[3] != '8')
        return;
    if (frm8_chunk[12] != 'D' || frm8_chunk[13] != 'S' || frm8_chunk[14] != 'D' || frm8_chunk[15] != ' ')
        return;

    chunk_size = decodebe64(frm8_chunk, 4);
    int64_t filesize = chunk_size + 12;
    int64_t fileposition = 16;

    // this loop reads FRM8 sub-chunks
    while (fileposition < filesize) {
        // read sub-chunk header
        TagLib::ByteVector chunk_header = file->readBlock(12);
        fileposition += 12;

        chunk_size = decodebe64(chunk_header, 4);
        chunk_end  = fileposition + chunk_size;

        if (!memcmp(chunk_header.data(), "FVER", 4)) {
            if (chunk_size != 4) { LOG("0"); return; }
            version = decodebe32(chunk_header, 4);
            LOG("[dff] got FVER 0x%x", version); 

        } else if (!memcmp(chunk_header.data(), "PROP", 4)) {
            if (chunk_size < 4) { LOG("1"); return; }

            // read properties_type
            TagLib::ByteVector properties_type = file->readBlock(4);
            fileposition += 4; chunk_size -= 4;

            LOG("[dff] got PROP %.*s", 4, properties_type.data());

            if (!memcmp(properties_type.data(), "SND ", 4)) {
                while (fileposition < chunk_end) {
                    TagLib::ByteVector property_header = file->readBlock(12);

                    fileposition += 12;
                    uint64_t property_size = decodebe64(property_header, 4);
                    uint64_t property_end  = fileposition + property_size;

                    if (!memcmp(property_header.data(), "FS  ", 4)) {
                        TagLib::ByteVector samplerate_data = file->readBlock(4);
                        fileposition += 4; 
                        sample_rate = decodebe32(samplerate_data, 0);
                        LOG("[dff] got SND prop FS=%d", sample_rate);

                    } else if (!memcmp(property_header.data(), "CHNL", 4)) {
                        if (property_size < 2 || property_size > 512) { LOG("2"); return; }
                        TagLib::ByteVector channels_data = file->readBlock(property_size);
                        fileposition += property_size;

                        channel_num = decodebe16(channels_data, 0);
                        property_size -= 2;
                        LOG("[dff] got CHNL prop: %d channels found", property_size/4);
                        int ch = 0;
                        uint8_t *ptr = (uint8_t*)channels_data.data()+2;
                        while (property_size > 0) {
                            LOG("[dff]     channel %d: %.*s", ch, 4, ptr);
                            ptr += 4; property_size -= 4; ch++;
                        }

                    } else if (!memcmp(property_header.data(), "CMPR", 4)) {
                        if (property_size < 4 || property_size > 512) { LOG("3"); return; }
                        TagLib::ByteVector compression_data = file->readBlock(property_size);
                        fileposition += property_size;
                        int count = compression_data[4];
                        LOG("[dff] got CMPR prop. Compression type '%.*s' (%.*s)", 4 , compression_data.data(), count, compression_data.data()+5);

                    } else if (!memcmp(property_header.data(), "ABSS", 4)) {
                        if (property_size < 4 || property_size > 512) { LOG("4"); return; }
                        TagLib::ByteVector abss_data = file->readBlock(property_size);
                        fileposition += property_size;

                        uint32_t hours   = decodebe16(abss_data, 0);
                        uint32_t minutes = abss_data[2];
                        uint32_t seconds = abss_data[3];
                        uint32_t samples = decodebe32(abss_data, 4);

                        LOG("[dff] got ABSS prop, %d:%02d:%02d:%d", hours, minutes, seconds, samples);

                    } else if (!memcmp(property_header.data(), "LSCO", 4)) {
                        if (property_size < 2 || property_size > 512) { LOG("5 %d", property_size); return; }
                        TagLib::ByteVector lsco_data = file->readBlock(property_size);
                        fileposition += property_size;
                        channel_type = decodebe16(lsco_data, 0);
                        LOG("[dff] got LSCO prop config=%d", channel_type);
                    }

                    file->seek(property_end);
                    fileposition = property_end;
                }
            } else {
                LOG("[dff] skipping PROP chunk of type '%.*s'", 4, properties_type.data());
            }
        } else if (!memcmp(chunk_header.data(), "DSD ", 4)) {
            // DSD DATA CHUNK
            data_start   = fileposition;
            data_end     = chunk_end;
            sample_count = (data_end - data_start) / channel_num * 8;
            LOG("[dff] got DSD chunk. data_start=%lld data_end=%lld sample_count=%lld", data_start, data_end, sample_count);
            break;

        } else if (!memcmp(chunk_header.data(), "DST ", 4)) {
            LOG("no support for DST compression");
            { LOG("6"); return; }
        } else {
            LOG("[dff] skipping '%.*s' chunk", 4, chunk_header.data());
        }
        file->seek(chunk_end);
        fileposition = chunk_end;
    }

    d->isValid            = true;
    d->version            = version;
    d->sampleCount        = sample_count;
    d->channelType        = channel_type;
    d->channelNum         = channel_num;
    d->sampleRate         = sample_rate;
    d->dataStart          = data_start;
    d->dataEnd            = data_end;
    d->bitsPerSample      = 1;
    d->fileSize           = file->length();

    LOG("parse out sample count %lld rate %lld", sample_count, sample_rate);
}
