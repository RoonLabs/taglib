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

#ifndef TAGLIB_DFFPROPERTIES_H
#define TAGLIB_DFFPROPERTIES_H

#include <taglib/audioproperties.h>
#include <taglib/toolkit/tbytevector.h>

#include "dffheader.h"

namespace TagLib {
  namespace DFF {
    class File;

    //! An implementation of audio property reading for DFF

    /*!
     * This reads the data from a DFF stream found in the
     * AudioProperties API.
     */

    class TAGLIB_EXPORT AudioProperties : public TagLib::AudioProperties
    {
    public:
      /*!
       * Create an instance of DFF::Properties with the data read from the
       * DFF::File \a file.
       */
      AudioProperties(File *file, ReadStyle style = Average);

      /*!
       * Destroys this DFF Properties instance.
       */
      virtual ~AudioProperties();

      // Reimplementations.

      virtual int length() const;
      virtual int lengthInSeconds() const;
      virtual int lengthInMilliseconds() const;
      virtual int bitrate() const;
      virtual int sampleRate() const;
      virtual int channels() const;
      virtual TagLib::ByteVector signature() const;

      /*!
       * Returns the DFF Version of the file.
       */
      unsigned version() const;

      /*!
       * Returns the channel type
       */
      unsigned short channelType() const;

      /*!
       * Returns the ID3v2 offset in the file
       */
      unsigned long long ID3v2Offset() const;

      /*!
       * Returns the file size
       */
      unsigned long long fileSize() const;

      /*!
       * Returns the sample count
       */
      unsigned long long sampleCount() const;

      /*!
       * Returns the bits per sample
       */
      int bitsPerSample() const;

    private:
      AudioProperties(const AudioProperties &);
      AudioProperties &operator=(const AudioProperties &);

      void read();

      class PropertiesPrivate;
      PropertiesPrivate *d;
    };
  }
}
#endif
