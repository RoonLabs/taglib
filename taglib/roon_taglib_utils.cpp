#include <roon_taglib_utils.h>
#include <SHA1.h>

#define CHUNK_SIZE 32 * 1024

// compute signature of file from offset to offset+length in f
ByteVector taglib_make_signature(File *f, unsigned long long offset, unsigned long long length)
{
    CSHA1 sha1;
    sha1.Reset();

    f->clear();

    if (length < 3 * CHUNK_SIZE)
    {
        f->seek(offset);
        ByteVector b = f->readBlock(length);
        //if (b.isNull()) return ByteVector::null;
        sha1.Update((const unsigned char *)b.data(), length);
    }
    else
    {
        f->seek(offset);
        ByteVector b = f->readBlock(CHUNK_SIZE);
        //if (b.isNull()) return ByteVector::null;
        sha1.Update((const unsigned char *)b.data(), CHUNK_SIZE);

        f->seek(offset + length / 2);
        b = f->readBlock(CHUNK_SIZE);
        //if (b.isNull()) return ByteVector::null;
        sha1.Update((const unsigned char *)b.data(), CHUNK_SIZE);

        f->seek(offset + length - CHUNK_SIZE);
        b = f->readBlock(CHUNK_SIZE);
        //if (b.isNull()) return ByteVector::null;
        sha1.Update((const unsigned char *)b.data(), CHUNK_SIZE);
    }

    // write data length as 64-bit little-endian bytes
    unsigned char b;
    b =  length & 0x00000000000000ffULL;        sha1.Update(&b, 1);
    b = (length & 0x000000000000ff00ULL) >> 8;  sha1.Update(&b, 1);
    b = (length & 0x0000000000ff0000ULL) >> 16; sha1.Update(&b, 1);
    b = (length & 0x00000000ff000000ULL) >> 24; sha1.Update(&b, 1);
    b = (length & 0x000000ff00000000ULL) >> 32; sha1.Update(&b, 1);
    b = (length & 0x0000ff0000000000ULL) >> 40; sha1.Update(&b, 1);
    b = (length & 0x00ff000000000000ULL) >> 48; sha1.Update(&b, 1);
    b = (length & 0xff00000000000000ULL) >> 56; sha1.Update(&b, 1);

    sha1.Final();

    unsigned char buf[20];
    //if (!sha1.GetHash(buf)) return ByteVector::null;
    ByteVector ret((const char *)buf, sizeof(buf));
    return ret;
}

// compute signature of bytevector
ByteVector taglib_make_signature(const ByteVector &bv)
{
    const char *data = bv.data();
    unsigned int data_len = bv.size();

    CSHA1 sha1;
    sha1.Reset();

    if (data_len < 3 * CHUNK_SIZE)
    {
        sha1.Update((const unsigned char *)data, data_len);
    }
    else
    {
        sha1.Update((const unsigned char *)data, CHUNK_SIZE);
        sha1.Update((const unsigned char *)(data + data_len / 2), CHUNK_SIZE);
        sha1.Update((const unsigned char *)(data + data_len - CHUNK_SIZE), CHUNK_SIZE);
    }

    // write data length as 64-bit little-endian bytes
    unsigned char b;
    b =  data_len & 0x000000ff;        sha1.Update(&b, 1);
    b = (data_len & 0x0000ff00) >> 8;  sha1.Update(&b, 1);
    b = (data_len & 0x00ff0000) >> 16; sha1.Update(&b, 1);
    b = (data_len & 0xff000000) >> 24; sha1.Update(&b, 1);
    b = 0; sha1.Update(&b, 1);
    b = 0; sha1.Update(&b, 1);
    b = 0; sha1.Update(&b, 1);
    b = 0; sha1.Update(&b, 1);

    sha1.Final();

    unsigned char buf[20];
    //if (!sha1.GetHash(buf)) return ByteVector::null;
    ByteVector ret((const char *)buf, sizeof(buf));
    return ret;
}
