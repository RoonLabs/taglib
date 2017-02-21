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


class DFFFile;

//! An implementation of audio property reading for DFF

/*!
 * This reads the data from a DFF stream found in the
 * AudioProperties API.
 */

class DFFProperties : public TagLib::AudioProperties
{
 public:
  /*!
   * Create an instance of DFF::Properties with the data read from the
   * DFF::File \a file.
   */
  DFFProperties(DFFFile *file, 
		TagLib::AudioProperties::ReadStyle style = Average);

  /*!
   * Destroys this DFF Properties instance.
   */
  virtual ~DFFProperties();
  
  /*!
   * Returns the length of the file in seconds.  The length is rounded down to
   * the nearest whole second.
   *
   * \note This method is just an alias of lengthInSeconds().
   *
   * \deprecated
   */
  virtual int length() const;

  /*!
   * Returns the length of the file in seconds.  The length is rounded down to
   * the nearest whole second.
   *
   * \see lengthInMilliseconds()
   */
  // BIC: make virtual
  int lengthInSeconds() const;

  /*!
   * Returns the length of the file in milliseconds.
   *
   * \see lengthInSeconds()
   */
  // BIC: make virtual
  int lengthInMilliseconds() const;

  // Reimplementations.
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
  DFFProperties(const DFFProperties &);
  DFFProperties &operator=(const DFFProperties &);

  void read();

  class PropertiesPrivate;
  PropertiesPrivate *d;
};

#endif
