# -*- coding: utf-8 -*-

# ******************************************************************************
#
# Copyright (C) 2006-2011 Olivier Tilloy <olivier@tilloy.net>
# Copyright (C) 2015-2023 Vincent Vande Vyvre <vincent.vandevyvre@oqapy.eu>
# Copyright (C) 2024 fdenivac <fdenivac@gmail.com>
#
# This file is part of the py3exiv2 distribution.
#
# py3exiv2 is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 3 as published by the Free Software Foundation.
#
# py3exiv2 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with py3exiv2; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
#
# Maintainer: Vincent Vande Vyvre <vincent.vandevyvre@oqapy.eu>
#
# ******************************************************************************

"""
EXIF specific code.
"""

from . import libexiv2python

from .utils import (is_fraction, make_fraction, fraction_to_string,
                          NotifyingList, ListenerInterface,
                          undefined_to_string, string_to_undefined,
                          DateTimeFormatter)

import time
import datetime
import sys

class ExifValueError(ValueError):
    """Exception raised when failing to parse the *value* of an EXIF tag.

    """
    def __init__(self, value, type_):
        """Instanciate the ExifValueError.

        Args:
        value -- the value that fails to be parsed
        type_ -- the EXIF type of the tag
        """
        self.value = value
        self.type = type_

    def __str__(self):
        return 'Invalid value for EXIF type [%s]: [%s]' %(self.type, self.value)


class ExifTag(ListenerInterface):
    """An EXIF tag.

    Here is a correspondance table between the EXIF types and the possible
    python types the value of a tag may take:

    - Ascii: :class:`datetime.datetime`, :class:`datetime.date`, string
    - Byte, SByte: bytes
    - Comment: string
    - Long, SLong: [list of] int
    - Short, SShort: [list of] int
    - Rational, SRational: [list of] :class:`fractions.Fraction` if available
      (Python ≥ 2.6) or :class:`pyexiv2.utils.Rational`      
    - Undefined: string
    """
    # According to the EXIF specification, the only accepted format for an Ascii
    # value representing a datetime is '%Y:%m:%d %H:%M:%S', but it seems that
    # others formats can be found in the wild.
    _datetime_formats = ('%Y:%m:%d %H:%M:%S',
                         '%Y-%m-%d %H:%M:%S',
                         '%Y-%m-%dT%H:%M:%SZ')

    _date_formats = ('%Y:%m:%d',)

    def __init__(self, key, value=None, _tag=None):
        """ The tag can be initialized with an optional value which expected
        type depends on the EXIF type of the tag.

        Args:
        key -- the key of the tag
        value -- the value of the tag
        """
        super().__init__()
        if _tag is not None:
            self._tag = _tag

        else:
            self._tag = libexiv2python._ExifTag(key)

        self._raw_value = None
        self._value = None
        self._value_cookie = False
        if value is not None:
            self._set_value(value)

    def _set_owner(self, metadata):
        self._tag._setParentImage(metadata._image)

    @staticmethod
    def _from_existing_tag(_tag):
        """Build a tag from an already existing libexiv2python._ExifTag.

        """
        tag = ExifTag(_tag._getKey(), _tag=_tag)
        # Do not set the raw_value property, as it would call _tag._setRawValue
        # (see https://bugs.launchpad.net/pyexiv2/+bug/582445).
        tag._raw_value = _tag._getRawValue()
        tag._value_cookie = True
        return tag

    @property
    def key(self):
        """The key of the tag in the dotted form
        ``familyName.groupName.tagName`` where ``familyName`` = ``exif``.

        """
        return self._tag._getKey()

    @property
    def type(self):
        """The EXIF type of the tag (one of Ascii, Byte, SByte, Comment, Short,
        SShort, Long, SLong, Rational, SRational, Undefined).

        """
        return self._tag._getType()

    @property
    def name(self):
        """The name of the tag (this is also the third part of the key).

        """
        return self._tag._getName()

    @property
    def label(self):
        """The title (label) of the tag.

        """
        return self._tag._getLabel()

    @property
    def description(self):
        """The description of the tag.

        """
        return self._tag._getDescription()

    @property
    def section_name(self):
        """The name of the tag's section.

        """
        return self._tag._getSectionName()

    @property
    def section_description(self):
        """The description of the tag's section.

        """
        return self._tag._getSectionDescription()

    def _get_raw_value(self):
        return self._raw_value

    def _set_raw_value(self, value):
        self._tag._setRawValue(value)
        self._raw_value = value
        self._value_cookie = True

    raw_value = property(fget=_get_raw_value, fset=_set_raw_value,
                         doc='The raw value of the tag as a string.')

    def _compute_value(self):
        """Lazy computation of the value from the raw value.

        """
        if self.type in ('Short', 'SShort', 'Long', 'SLong', 
                         'Rational', 'SRational', 'Double', 'Float'):
            # May contain multiple values
            values = self._raw_value.split()
            if len(values) > 1:
                # Make values a notifying list
                values = [self._convert_to_python(v) for v in values]
                self._value = NotifyingList(values)
                self._value.register_listener(self)
                self._value_cookie = False
                return

        self._value = self._convert_to_python(self._raw_value)
        self._value_cookie = False

    def _get_value(self):
        if self._value_cookie:
            self._compute_value()
        return self._value

    def _set_value(self, value):
        if isinstance(value, (list, tuple)):
            raw_values = [self._convert_to_string(v) for v in value]
            self.raw_value = ' '.join(raw_values)

        else:
            self.raw_value = self._convert_to_string(value)

        if isinstance(self._value, NotifyingList):
            self._value.unregister_listener(self)

        if isinstance(value, NotifyingList):
            # Already a notifying list
            self._value = value
            self._value.register_listener(self)

        elif isinstance(value, (list, tuple)):
            # Make the values a notifying list 
            self._value = NotifyingList(value)
            self._value.register_listener(self)

        else:
            # Single value
            self._value = value

        self._value_cookie = False

    value = property(fget=_get_value, fset=_set_value,
                     doc='The value of the tag as a python object.')

    @property
    def human_value(self):
        """A (read-only) human-readable representation
        of the value of the tag.

        """
        return self._tag._getHumanValue() or None

    def contents_changed(self):
        # Implementation of the ListenerInterface.
        # React on changes to the list of values of the tag.
        # self._value is a list of values and its contents changed.
        self._set_value(self._value)

    def _match_encoding(self, charset):
        # charset see:
        # http://www.exiv2.org/doc/classExiv2_1_1CommentValue.html
        # enum  	CharsetId {
        #           ascii, jis, unicode, undefined,
        #           invalidCharsetId, lastCharsetId } 
        encoding = sys.getdefaultencoding()
        if charset in ('Ascii', 'ascii'):
            encoding = 'ascii'

        elif charset in ('Jis', 'jis'):
            encoding = 'shift_jis'

        elif charset in ('Unicode', 'unicode'):
            encoding = 'utf-8'

        return encoding

    def _convert_to_python(self, value):
        """
        Convert one raw value to its corresponding python type.

        :param value: the raw value to be converted
        :type value: string

        :return: the value converted to its corresponding python type

        :raise ExifValueError: if the conversion fails
        """
        if self.type == 'Ascii':
            # The value may contain a Datetime
            for format in self._datetime_formats:
                try:
                    t = time.strptime(value, format)
                except ValueError:
                    continue
                else:
                    return datetime.datetime(*t[:6])
            # Or a Date (e.g. Exif.GPSInfo.GPSDateStamp)
            for format in self._date_formats:
                try:
                    t = time.strptime(value, format)
                except ValueError:
                    continue
                else:
                    return datetime.date(*t[:3])
            # Default to string.
            # There is currently no charset conversion.
            # TODO: guess the encoding and decode accordingly into unicode
            # where relevant.
            return value

        elif self.type in ('Byte', 'SByte'):
            if isinstance(value, bytes):
                return value.decode('utf-8')
            return value

        elif self.type == 'Comment':
            if isinstance(value, str):
                if value.startswith('charset='):
                    charset, val = value.split(' ', 1)
                    return val
                return value

            if value.startswith(b'charset='):          
                charset = charset.split('=')[1].strip('"')
                encoding = self._match_encoding(charset)
                return val.decode(encoding, 'replace')

            else:
                # No encoding defined.
                try:
                    return value.decode('utf-8')
                except UnicodeError:
                    pass

            return value

        elif self.type in ('Short', 'SShort'):
            try:
                return int(value)
            except ValueError:
                raise ExifValueError(value, self.type)

        elif self.type in ('Long', 'SLong'):
            try:
                return int(value)
            except ValueError:
                raise ExifValueError(value, self.type)

        elif self.type in ('Rational', 'SRational'):
            try:
                r = make_fraction(value)
            except (ValueError, ZeroDivisionError):
                raise ExifValueError(value, self.type)

            else:
                if self.type == 'Rational' and r.numerator < 0:
                    raise ExifValueError(value, self.type)
                return r

        elif self.type in ('Double', 'Float'):
            try:
                return float(value)
            except ValueError:
                raise ExifValueError(value, self.type)


        elif self.type == 'Undefined':
            # There is currently no charset conversion.
            # TODO: guess the encoding and decode accordingly into unicode
            # where relevant.
            return undefined_to_string(value)

        raise ExifValueError(value, self.type)

    def _convert_to_string(self, value):
        """
        Convert one value to its corresponding string representation, suitable
        to pass to libexiv2.

        :param value: the value to be converted

        :return: the value converted to its corresponding string representation
        :rtype: string

        :raise ExifValueError: if the conversion fails
        """
        if self.type == 'Ascii':
            if isinstance(value, datetime.datetime):
                return DateTimeFormatter.exif(value)

            elif isinstance(value, datetime.date):
                if self.key == 'Exif.GPSInfo.GPSDateStamp':
                    # Special case
                    return DateTimeFormatter.exif(value)

                else:
                    return '%s 00:00:00' % DateTimeFormatter.exif(value)

            else:
                return value

        elif self.type in ('Byte', 'SByte'):
            if isinstance(value, str):
                try:
                    return value.encode('utf-8')
                except UnicodeEncodeError:
                    raise ExifValueError(value, self.type)

            elif isinstance(value, bytes):
                return value

            else:
                raise ExifValueError(value, self.type)

        elif self.type == 'Comment':
            return self._convert_to_bytes(value)

        elif self.type == 'Short':
            if isinstance(value, int) and value >= 0:
                return str(value)

            else:
                raise ExifValueError(value, self.type)

        elif self.type == 'SShort':
            if isinstance(value, int):
                return str(value)

            else:
                raise ExifValueError(value, self.type)

        elif self.type == 'Long':
            if isinstance(value, int) and value >= 0:
                return str(value)

            else:
                raise ExifValueError(value, self.type)

        elif self.type == 'SLong':
            if isinstance(value, int):
                return str(value)

            else:
                raise ExifValueError(value, self.type)

        elif self.type == 'Rational':
            if is_fraction(value) and value.numerator >= 0:
                return fraction_to_string(value)

            else:
                raise ExifValueError(value, self.type)

        elif self.type == 'SRational':
            if is_fraction(value):
                return fraction_to_string(value)

            else:
                raise ExifValueError(value, self.type)

        elif self.type == 'Undefined':
            if isinstance(value, str):
                try:
                    return string_to_undefined(value)
                except UnicodeEncodeError:
                    raise ExifValueError(value, self.type)

            elif isinstance(value, bytes):
                return string_to_undefined(value)

            else:
                raise ExifValueError(value, self.type)

        raise ExifValueError(value, self.type)

    def _convert_to_bytes(self, value):
        if value is None:
            return

        if isinstance(value, str):
            if value.startswith('charset='):
                charset, val = value.split(' ', 1)
                charset = charset.split('=')[1].strip('"')
                encoding = self._match_encoding(charset)

            else:
                encoding = 'utf-8'
                charset = 'Unicode'

            try:
                val = value.encode(encoding)
            except UnicodeError:
                pass

            else:
                #self._set_raw_value('charset=%s %s' % (charset, val))
                return val

        elif isinstance(value, bytes):
            return value

        else:
            raise ExifValueError(value, self.type)

    def __str__(self):
        """
        :return: a string representation of the EXIF tag for debugging purposes
        :rtype: string
        """
        left = '%s [%s]' % (self.key, self.type)
        if self._raw_value is None:
            right = '(No value)'

        elif self.type == 'Undefined' and len(self._raw_value) > 100:
            right = '(Binary value suppressed)'

        else:
             right = self._raw_value

        return '<%s = %s>' % (left, right)

    # Support for pickling.
    def __getstate__(self):
        return (self.key, self.raw_value)

    def __setstate__(self, state):
        key, raw_value = state
        self._tag = libexiv2python._ExifTag(key)
        self.raw_value = raw_value


class ExifThumbnail(object):

    """
    A thumbnail image optionally embedded in the IFD1 segment of the EXIF data.

    The image is either a TIFF or a JPEG image.
    """

    def __init__(self, _metadata):
        self._metadata = _metadata

    @property
    def mime_type(self):
        """The mime type of the preview image (e.g. ``image/jpeg``)."""
        return self._metadata._image._getExifThumbnailMimeType()

    @property
    def extension(self):
        """The file extension of the preview image with a leading dot
        (e.g. ``.jpg``)."""
        return self._metadata._image._getExifThumbnailExtension()

    def write_to_file(self, path):
        """
        Write the thumbnail image to a file on disk.
        The file extension will be automatically appended to the path.

        :param path: path to write the thumbnail to (without an extension)
        :type path: string
        """
        self._metadata._image._writeExifThumbnailToFile(path)

    def _update_exif_tags_cache(self):
        # Update the cache of EXIF tags
        keys = self._metadata._image._exifKeys()
        self._metadata._keys['exif'] = keys
        #cached = self._metadata._tags['exif'].keys()
        for key in self._metadata._tags['exif'].keys():
            if key not in keys:
                del self._metadata._tags['exif'][key]

    def erase(self):
        """
        Delete the thumbnail from the EXIF data.
        Removes all Exif.Thumbnail.*, i.e. Exif IFD1 tags.
        """
        self._metadata._image._eraseExifThumbnail()
        self._update_exif_tags_cache()

    def set_from_file(self, path):
        """
        Set the EXIF thumbnail to the JPEG image path.
        This sets only the ``Compression``, ``JPEGInterchangeFormat`` and
        ``JPEGInterchangeFormatLength`` tags, which is not all the thumbnail
        EXIF information mandatory according to the EXIF standard
        (but it is enough to work with the thumbnail).

        :param path: path to a JPEG file to set the thumbnail to
        :type path: string
        """
        self._metadata._image._setExifThumbnailFromFile(path)
        self._update_exif_tags_cache()

    def _get_data(self):
        buf_ = self._metadata._image._getExifThumbnailData()
        return buf_

    def _set_data(self, data):
        self._metadata._image._setExifThumbnailFromData(data)
        self._update_exif_tags_cache()

    data = property(fget=_get_data, fset=_set_data,
                    doc='The raw thumbnail data. Setting it is restricted to ' +
                        'a buffer in the JPEG format.')

