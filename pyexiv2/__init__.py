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
Manipulation of EXIF, IPTC and XMP metadata and thumbnails embedded in images.

The ImageMetadata class provides read/write access to all the metadata and
the various thumbnails embedded in an image file such as JPEG and TIFF files.

Metadata is accessed through tag classes (ExifTag, IptcTag, XmpTag)
and the tag values are conveniently wrapped in python objects.
For example, a tag containing a date/time information for the image
(e.g. Exif.Photo.DateTimeOriginal) will be represented by a python
datetime.datetime object.

This module is a python layer on top of the low-level python binding of the
C++ library Exiv2, libexiv2python.

A typical use of this binding would be:

>>> import pyexiv2
>>> metadata = pyexiv2.ImageMetadata('test/smiley.jpg')
>>> metadata.read()
>>> print(metadata.exif_keys)
['Exif.Image.ImageDescription', 'Exif.Image.XResolution',
 'Exif.Image.YResolution', 'Exif.Image.ResolutionUnit', 'Exif.Image.Software',
 'Exif.Image.DateTime', 'Exif.Image.Artist', 'Exif.Image.Copyright',
 'Exif.Image.ExifTag', 'Exif.Photo.Flash', 'Exif.Photo.PixelXDimension',
 'Exif.Photo.PixelYDimension']
>>> print(metadata['Exif.Image.DateTime'].value)
2004-07-13 21:23:44
>>> import datetime
>>> metadata['Exif.Image.DateTime'].value = datetime.datetime.today()
>>> metadata.write()
"""

from . import libexiv2python

from .metadata import ImageMetadata
from .exif import ExifValueError, ExifTag, ExifThumbnail
from .iptc import IptcValueError, IptcTag
from .xmp import (XmpValueError, XmpTag, register_namespace,
                         unregister_namespace, unregister_namespaces)
from .preview import Preview
from .utils import (FixedOffset, NotifyingList,
                           undefined_to_string, string_to_undefined,
                           GPSCoordinate)

def initLog():
    """Initialize exiv2 log system
    """
    libexiv2python._initLog()

def setLogLevel(level):
    """Set exiv2 log level
    """
    libexiv2python._setLogLevel(level)

def _make_version(_version_info):
    return '.'.join([str(i) for i in _version_info])


#: A tuple containing the three components of the version number: major, minor, micro.
version_info = (1, 0, 0)

#: The version of the module as a string (major.minor.micro).
__version__ = _make_version(version_info)

#: A tuple containing the three components of the version number of libexiv2: major, minor, micro.
exiv2_version_info = libexiv2python.exiv2_version_info

#: The version of libexiv2 as a string (major.minor.micro).
__exiv2_version__ = _make_version(exiv2_version_info)

