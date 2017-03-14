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

#ifndef TAGLIB_DFFHEADER_H
#define TAGLIB_DFFHEADER_H

#include <taglib/toolkit/tfile.h>

class DFFFile;

class DFFHeader
{
 public:
  /*!
   * Parses an DFF header based on \a data.
   */
  DFFHeader(DFFFile *file);

  /*!
   * Does a shallow copy of \a h.
   */
  DFFHeader(const DFFHeader &h);

  /*!
   * Destroys this Header instance.
   */
  virtual ~DFFHeader();

  /*!
   * Returns true if header has legal values.
   */
  bool isValid() const;

  /*!
   * Returns the DSD Version of the header.
   */
  unsigned version() const;

  /*!
   * Returns the Channel Type of the header
   */
  unsigned short channelType() const;

  /*!
   * Returns the Channel Num of the header
   */
  unsigned short channelNum() const;

  /*!
   * Returns the sample rate in Hz.
   */
  unsigned int sampleRate() const;

  /*!
   * Returns the sample count
   */
  unsigned long long sampleCount() const;

  /*!
   * Returns the start of the sample data
   */
  unsigned long long dataStart() const;

  /*!
   * Returns the end of the sample data
   */
  unsigned long long dataEnd() const;

  /*!
   * Returns the bits per sample
   */
  unsigned short bitsPerSample() const;

  /*!
   * Returns the file size
   */
  unsigned long long fileSize() const;

  /*!
   * Makes a shallow copy of the header.
   */
  DFFHeader &operator=(const DFFHeader &h);

 private:
  void parse(DFFFile *file);

  class HeaderPrivate;
  HeaderPrivate *d;
};


#endif
