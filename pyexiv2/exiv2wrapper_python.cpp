// *****************************************************************************
/*
 * Copyright (C) 2006-2012 Olivier Tilloy <olivier@tilloy.net>
 * Copyright (C) 2015-2023 Vincent Vande Vyvre <vincent.vandevyvre@oqapy.eu>
 * Copyright (C) 2024 fdenivac <fdenivac@gmail.com>
 *
 * This file is part of the py3exiv2 distribution.
 *
 * py3exiv2 is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * py3exiv2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with py3exiv2; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  Maintainer: Vincent Vande Vyvre <vincent.vandevyvre@oqapy.eu>
 */
// *****************************************************************************

#include <pybind11/pybind11.h>

#include <string>
#include <sstream>
#include <iostream>

#include "exiv2wrapper.hpp"

#include "exiv2/exv_conf.h"
#include "exiv2/version.hpp"


using namespace exiv2wrapper;

namespace py = pybind11;


PYBIND11_MODULE(libexiv2python, m)
{
    m.attr("exiv2_version_info") = \
         py::make_tuple(EXIV2_MAJOR_VERSION,
                        EXIV2_MINOR_VERSION,
                        EXIV2_PATCH_VERSION);

    // Set exceptions :
    //  py::register_exception<Exiv2::Error>(m, "Exiv2Error");
    // is OK, but here, we need to translate some error code to various exceptions
    static py::exception<Exiv2::Error> exc(m, "Exiv2Error");
    py::register_exception_translator([](std::exception_ptr p) {
        try {
            if (p) std::rethrow_exception(p);
        } catch (const  Exiv2::Error &e) {
            translateExiv2Error(e);
        }
    });
    


    // Swallow all warnings and error messages written by libexiv2 to stderr
    // (if it was compiled with DEBUG or without SUPPRESS_WARNINGS).
    // See https://bugs.launchpad.net/pyexiv2/+bug/507620.
    std::cerr.rdbuf(NULL);

    py::class_<ExifTag>(m, "_ExifTag")
        .def(py::init<std::string>())

        .def("_setRawValue", &ExifTag::setRawValue)
        .def("_setParentImage", &ExifTag::setParentImage)

        .def("_getKey", &ExifTag::getKey)
        .def("_getType", &ExifTag::getType)
        .def("_getName", &ExifTag::getName)
        .def("_getLabel", &ExifTag::getLabel)
        .def("_getDescription", &ExifTag::getDescription)
        .def("_getSectionName", &ExifTag::getSectionName)
        .def("_getSectionDescription", &ExifTag::getSectionDescription)
        .def("_getRawValue", &ExifTag::getRawValue)
        .def("_getHumanValue", &ExifTag::getHumanValue)
        .def("_getByteOrder", &ExifTag::getByteOrder)
    ;

    py::class_<IptcTag>(m, "_IptcTag")
        .def(py::init<std::string>())

        .def("_setRawValues", &IptcTag::setRawValues)
        .def("_setParentImage", &IptcTag::setParentImage)

        .def("_getKey", &IptcTag::getKey)
        .def("_getType", &IptcTag::getType)
        .def("_getName", &IptcTag::getName)
        .def("_getTitle", &IptcTag::getTitle)
        .def("_getDescription", &IptcTag::getDescription)
        .def("_getPhotoshopName", &IptcTag::getPhotoshopName)
        .def("_isRepeatable", &IptcTag::isRepeatable)
        .def("_getRecordName", &IptcTag::getRecordName)
        .def("_getRecordDescription", &IptcTag::getRecordDescription)
        .def("_getRawValues", &IptcTag::getRawValues)
    ;

    py::class_<XmpTag>(m, "_XmpTag")
        .def(py::init<std::string>())

        .def("_setTextValue", &XmpTag::setTextValue)
        .def("_setArrayValue", &XmpTag::setArrayValue)
        .def("_setLangAltValue", &XmpTag::setLangAltValue)
        .def("_setParentImage", &XmpTag::setParentImage)

        .def("_getKey", &XmpTag::getKey)
        .def("_getExiv2Type", &XmpTag::getExiv2Type)
        .def("_getType", &XmpTag::getType)
        .def("_getName", &XmpTag::getName)
        .def("_getTitle", &XmpTag::getTitle)
        .def("_getDescription", &XmpTag::getDescription)
        .def("_getTextValue", &XmpTag::getTextValue)
        .def("_getArrayValue", &XmpTag::getArrayValue)
        .def("_getLangAltValue", &XmpTag::getLangAltValue)
    ;

    py::class_<Preview>(m, "_Preview")
        .def(py::init<Exiv2::PreviewImage>())

        .def_readonly("mime_type", &Preview::_mimeType)
        .def_readonly("extension", &Preview::_extension)
        .def_readonly("size", &Preview::_size)
        .def_readonly("dimensions", &Preview::_dimensions)
        .def_readonly("data", &Preview::_data)

        .def("get_data", &Preview::getData)
        .def("write_to_file", &Preview::writeToFile)
    ;

    py::class_<Image>(m, "_Image")
        .def(py::init<std::string>())
        .def(py::init<std::string, long>())

        .def("_readMetadata", &Image::readMetadata)
        .def("_writeMetadata", &Image::writeMetadata)

        .def("_getPixelWidth", &Image::pixelWidth)
        .def("_getPixelHeight", &Image::pixelHeight)

        .def("_getMimeType", &Image::mimeType)

        .def("_exifKeys", &Image::exifKeys)
        .def("_getExifTag", &Image::getExifTag)
        .def("_deleteExifTag", &Image::deleteExifTag)

        .def("_iptcKeys", &Image::iptcKeys)
        .def("_getIptcTag", &Image::getIptcTag)
        .def("_deleteIptcTag", &Image::deleteIptcTag)

        .def("_xmpKeys", &Image::xmpKeys)
        .def("_getXmpTag", &Image::getXmpTag)
        .def("_deleteXmpTag", &Image::deleteXmpTag)

        .def("_getComment", &Image::getComment)
        .def("_setComment", &Image::setComment)
        .def("_clearComment", &Image::clearComment)

        .def("_previews", &Image::previews)

        .def("_copyMetadata", &Image::copyMetadata)

        .def("_getDataBuffer", &Image::getDataBuffer)

        .def("_getExifThumbnailMimeType", &Image::getExifThumbnailMimeType)
        .def("_getExifThumbnailExtension", &Image::getExifThumbnailExtension)
        .def("_writeExifThumbnailToFile", &Image::writeExifThumbnailToFile)
        .def("_getExifThumbnailData", &Image::getExifThumbnailData)
        .def("_eraseExifThumbnail", &Image::eraseExifThumbnail)
        .def("_setExifThumbnailFromFile", &Image::setExifThumbnailFromFile)
        .def("_setExifThumbnailFromData", &Image::setExifThumbnailFromData)

        .def("_getIptcCharset", &Image::getIptcCharset)

        .def("_getXmpPacket", &Image::getXmpPacket)
        .def("_getICC", &Image::getICC)
    ;

    m.doc() = "Expose the Exiv2 API to Python.";
    m.def("_initLog", initLog);
    m.def("_setLogLevel", setLogLevel);
    
    m.def("_initialiseXmpParser", initialiseXmpParser);
    m.def("_closeXmpParser", closeXmpParser);
    m.def("_registerXmpNs", registerXmpNs, py::arg("name"), py::arg("prefix"));
    m.def("_unregisterXmpNs", unregisterXmpNs, py::arg("name"));
    m.def("_unregisterAllXmpNs", unregisterAllXmpNs);

};

