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

#ifndef TAGLIB_DFFFILE_H
#define TAGLIB_DFFFILE_H

#include "tfile.h"
#include "tag.h"

#include "dffproperties.h"

namespace TagLib {
//! An implementation of TagLib::File with DFF (DSD) specific methods
//
//! An DFF file class with some useful methods specific to DFF

  namespace DFF {
  
    class TAGLIB_EXPORT File : public TagLib::File
    {
    public:
      /*!
       * Constructs an DFF file from \a file.  If \a readProperties is true the
       * file's audio properties will also be read.
       *
       * \note In the current implementation, \a propertiesStyle is ignored.
       *
       * \deprecated This constructor will be dropped in favor of the one below
       * in a future version.
       */
      File(FileName file, bool readProperties = true,
           AudioProperties::ReadStyle propertiesStyle 
           = AudioProperties::Average);


      /*!
       * Constructs an DFF file from \a stream.  If \a readProperties is true the
       * file's audio properties will also be read.
       *
       * \note TagLib will *not* take ownership of the stream, the caller is
       * responsible for deleting it after the File object.
       *
       * \note In the current implementation, \a propertiesStyle is ignored.
       */
      File(IOStream *stream, 
           bool readProperties = true,
           AudioProperties::ReadStyle propertiesStyle = 
           AudioProperties::Average);

      /*!
       * Destroys this instance of the File.
       */
      virtual ~File();

      /*!
       * Returns the DFF::Properties for this file.  If no audio properties
       * were read then this will return a null pointer.
       */
      virtual AudioProperties *audioProperties() const;

      /*!
       * Returns a pointer to a tag
       */
      virtual Tag *tag() const;

      /*!
       * This does nothing and returns false because DFF files do not support tags.
       *
       * \see save(int tags)
       */
      virtual bool save();

    private:
      File(const File &);
      File &operator=(const File &);

      // Read the actual audio file for tags
      void read(bool readProperties, AudioProperties::ReadStyle propertiesStyle);

      class FilePrivate;
      FilePrivate *d;
    };
  }
}
#endif
