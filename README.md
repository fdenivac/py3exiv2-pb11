# py3exiv2-pb11

This is a py3exiv2 fork of original bazaar repository hosted by Launchpad.net (version 0.12.0) :

    https://launchpad.net/py3exiv2

py3exiv2 is a python 3 binding to exiv2 library (http://exiv2.org/), the C++
library for manipulation of EXIF, IPTC, XMP image metadata, ICC Profile.

It is a python 3 module that allows your python scripts to read and write
metadata (EXIF, IPTC, XMP, thumbnails) embedded in image files
(JPEG, TIFF, ...).

In this fork :
 * the python binding is done using pybind11 package, replacing boost-python.
 * added methods access to profile ICC and XMP packet
 * some improvements for XMP types

Point your browser to http://exiv2.org/metadata.html for the complete metadata
tag reference.

pyexiv2 depends on the following libraries:

 * python (≥ 3.7)
 * python package pybind11 (≥ 2.10)
 * exiv2 (http://www.exiv2.org/)


## Typical usage

Note that module name is "pyexiv2".

      >>> import pyexiv2
      >>> metadata = pyexiv2.ImageMetadata('test/smiley.jpg')
      >>> metadata.read()
      >>> print(metadata.exif_keys)
      ['Exif.Image.ImageDescription', 'Exif.Image.XResolution', 'Exif.Image.YResolution', 
      'Exif.Image.DateTime', 'Exif.Image.Artist', 'Exif.Image.Copyright',
      'Exif.Photo.Flash', 'Exif.Photo.PixelXDimension', 'Exif.Photo.PixelYDimension']
      >>> print(metadata['Exif.Image.DateTime'].value)
      2004-07-13 21:23:44
      >>> import datetime
      >>> metadata['Exif.Image.DateTime'].value = datetime.datetime.today()
      >>> metadata.write()



## Building and installing

### Build requirements
- C++11 compiler
- Exiv2 binaries, include and library

See Supported Compilers on pybind11 documentation : <https://pybind11-jagerman.readthedocs.io/en/stable/intro.html#supported-compilers>


### Build

* Clone project

* **Linux specific :**
   * Install build package
      ```
      sudo apt-get install build-essential
      ```
   * install python 3 development package
      ```
      sudo apt-get install python3-dev
      ```
   * install exiv2 development package
      ```
      sudo apt-get install libexiv2-dev
      ```

* **Windows specific :**

   * install Visual Studio Community (C++ compoments)

   * Prepare Exiv2 include and library directories. The minimal structure for directory containing include and library must be :
      ```
      <EXIV2_DIR>
      ├── include
      ├── lib
      ├── ...
      ```

      You can download precompiled exiv2 libraries from exiv2 site : 
         *  http://pre-release.exiv2.org/download.html

      or from _LeoHsiao1/pyexiv2_ repository :
         *  https://github.com/LeoHsiao1/pyexiv2/tree/master/pyexiv2/lib

   * Declare environment variable EXIV2_DIR equal to exiv2 dev root directory.

      ```
      > set EXIV2_DIR=E:\Devs\exiv2-0.28.3-2019msvc64
      ```

* Set build environment : source compiler and python environment

* Build via :

    ```
    > cd <PY3EXIV2_DIR>
    > python -m build
    ```

* Build and install via
    ```
    > pip install py3exiv2
    ```

* Install from  wheel
    ```
    > cd <PY3EXIV2_DIR>
    > pip install dist/py3exiv2-1.0.0-cp311-cp311-win_amd64.whl
    ```





## Documentation

Refer to the internal documentation for a guide on how to use py3exiv2.
In a python interpreter, type:

    >>> import pyexiv2
    >>> help(pyexiv2)

or you can find the original API documentation (not up-to-date) at: 
    http://python3-exiv2.readthedocs.org/en/latest


## Copyright

 * Copyright (C) 2006-2011 Olivier Tilloy <olivier@tilloy.net>
 * Copyright (C) 2015-2019 Vincent Vande Vyvre <vincent.vandevyvre@oqapy.eu>
 * Copyright (C) 2024 fdenivac <fdenivac@gmail.com>



## Developers

py3exiv2 is Free Software, meaning that you are encouraged to play with it,
modify it to suit your needs and contribute back your changes and bug fixes.

For this fork :

 * Latest version of the code : "https://github.com/fdenivac/py3exiv2"

 * Bug tracking : "https://github.com/fdenivac/py3exiv2/issues"

Feedback, bug reports and patches are welcome!


## ChangeLog

* Change in 1.0.0 Thu, 24 Aug 2024
   * exiv2 binded with pybind11
   * functions to get ICC profile, XMP packet
   * improve XMP support types

* Change in 0.12.0 Thu, 28 Aug 2023
   * Adapt to Exiv2 0.28

* Change in 0.9.3 Thu, 24 Dec 2020
   * Fix minor issues

* Change in 0.9.0  Wed, 09 Dec 2020
   * Add support for tag Xmp.mwg-rs.Regions/mwg-rs:

* Change in 0.8.0  Mon, 28 Sep 2020
   * Add support for tag XmpSeq <property>Detail

* Change in 0.7.2  Wed, 25 Mar 2020
   * Add a datetime conversion to Python with iso format, update some unittests
  
* Change in 0.7.1  Sat, 28 May 2019
   * Minor changes in order to compile on OS X platform

* Change in 0.7.0  Sat, 13 Apr 2019
   * Add the initialiseXmpParser and closeXmpParser functions

* Change in 0.6.0  Tue, 05 Feb 2019
   * Add the streaming of the preview data

* Change in 0.5.0  Wed, 30 Jan 2019
   * Update for libexiv2-0.27

* Change in 0.4.0  Mon, 09 Jul 2018
   * Add the property ImageMetadata.buffer

* Change in 0.3.0  Thu, 17 May 2018
   * Add some convenient functions

* Change in 0.2.0  Fri, 12 Aug 2016
   * Add Preview.data feature

* Change in 0.1.0  Mon, 26 Jan 2015
   * First stable release
