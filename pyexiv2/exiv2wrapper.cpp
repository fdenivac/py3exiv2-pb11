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

#include "exiv2wrapper.hpp"

#include <fstream>
#include <iostream>

namespace py = pybind11;

const char *EXCEPTION_HINT = "Caught Exiv2 exception: ";
// std::stringstream error_log;

// Custom error codes for Exiv2 exceptions
#define METADATA_NOT_READ 101
#define NON_REPEATABLE 102
#define KEY_NOT_FOUND 103
#define INVALID_VALUE 104
#define EXISTING_PREFIX 105
#define BUILTIN_NS 106
#define NOT_REGISTERED 107

#if EXIV2_MAJOR_VERSION >= 1 || (EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION >= 28)
#define HAVE_CLASS_ERROR_CODE
#endif
#if EXIV2_MAJOR_VERSION >= 1 || (EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION == 27)
#define HAVE_EXIV2_ERROR_CODE
#endif
// Custom macros
#ifdef HAVE_CLASS_ERROR_CODE
#define CHECK_METADATA_READ \
    if (!_dataRead) throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, "metadata not read");
#else
#ifdef HAVE_EXIV2_ERROR_CODE
#define CHECK_METADATA_READ \
    if (!_dataRead) throw Exiv2::Error(Exiv2::kerErrorMessage, "metadata not read");
#else
#define HAVE_OLD_ERROR_CODE
#define CHECK_METADATA_READ \
    if (!_dataRead) throw Exiv2::Error(METADATA_NOT_READ);
#endif
#endif

#define WRAP_ERROR                                            \
  if (Exiv2::LogMsg::error >= Exiv2::LogMsg::level() && Exiv2::LogMsg::handler()) \
  Exiv2::LogMsg(Exiv2::LogMsg::error).os()

namespace exiv2wrapper
{

void Image::_instantiate_image()
{
    _exifThumbnail = 0;

    // If an exception is thrown, it has to be done outside of the
    // Py_{BEGIN,END}_ALLOW_THREADS block.
#ifdef HAVE_CLASS_ERROR_CODE
    Exiv2::Error error = Exiv2::Error(Exiv2::ErrorCode::kerSuccess);
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
    Exiv2::Error error = Exiv2::Error(Exiv2::kerSuccess);
#else
    Exiv2::Error error(0);
#endif
#endif

    // Release the GIL to allow other python threads to run
    // while opening the file.
    Py_BEGIN_ALLOW_THREADS

    try
    {
        if (_data != 0)
        {
            _image = Exiv2::ImageFactory::open(_data, _size);
        }
        else
        {
            _image = Exiv2::ImageFactory::open(_filename);
        }
    }

    catch (Exiv2::Error& err) 
    {
        //std::cout << " Caught Exiv2 exception '" << err.code() << "'\n";
        error = err;
    }
    // Re-acquire the GIL
    Py_END_ALLOW_THREADS

    if (error.code() == Exiv2::ErrorCode::kerSuccess)
    {
        assert(_image.get() != 0);
        _dataRead = false;
    }
    else
    {
        throw error;
    }
}

// Base constructor
Image::Image(const std::string& filename)
{
    _filename = filename;
    _data = 0;
    _instantiate_image();
}

// From buffer constructor
Image::Image(const std::string& buffer, unsigned long size)
{
    // Deep copy of the data buffer
    _data = new Exiv2::byte[size];
    for (unsigned long i = 0; i < size; ++i)
    {
        _data[i] = buffer[i];
    }

    _size = size;
    _instantiate_image();
}

// Copy constructor
Image::Image(const Image& image)
{
    _filename = image._filename;
    _instantiate_image();
}

Image::~Image()
{
    if (_data != 0)
    {
        delete[] _data;
    }
    if (_exifThumbnail != 0)
    {
        delete _exifThumbnail;
    }
}

void Image::readMetadata()
{
    // If an exception is thrown, it has to be done outside of the
    // Py_{BEGIN,END}_ALLOW_THREADS block.
#ifdef HAVE_CLASS_ERROR_CODE
    Exiv2::Error error = Exiv2::Error(Exiv2::ErrorCode::kerSuccess);
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
    Exiv2::Error error = Exiv2::Error(Exiv2::kerSuccess);
#else
    Exiv2::Error error(0);
#endif
#endif

    // Release the GIL to allow other python threads to run
    // while reading metadata.
    Py_BEGIN_ALLOW_THREADS

    try
    {
        _image->readMetadata();
        _exifData = &_image->exifData();
        _iptcData = &_image->iptcData();
        _xmpData = &_image->xmpData();
        _dataRead = true;
    }

    catch (Exiv2::Error& err) 
    {
        //std::cout << " Caught Exiv2 exception '" << err.code() << "'\n";
        error = err;
    }

    // Re-acquire the GIL
    Py_END_ALLOW_THREADS

    if (error.code() != Exiv2::ErrorCode::kerSuccess)
    {
        throw error;
    }
}

void Image::writeMetadata()
{
    CHECK_METADATA_READ

    // If an exception is thrown, it has to be done outside of the
    // Py_{BEGIN,END}_ALLOW_THREADS block.
#ifdef HAVE_CLASS_ERROR_CODE
    Exiv2::Error error = Exiv2::Error(Exiv2::ErrorCode::kerSuccess);
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
    Exiv2::Error error = Exiv2::Error(Exiv2::kerSuccess);
#else
    Exiv2::Error error(0);
#endif
#endif

    // Release the GIL to allow other python threads to run
    // while writing metadata.
    Py_BEGIN_ALLOW_THREADS

    try
    {
        _image->writeMetadata();
    }

    catch (Exiv2::Error& err) 
    {
        //std::cout << "Caught Exiv2 exception '" << err.code() << "'\n";
        error = err;
    }

    // Re-acquire the GIL
    Py_END_ALLOW_THREADS

    if (error.code() != Exiv2::ErrorCode::kerSuccess)
    {
        throw error;
    }
}

unsigned int Image::pixelWidth() const
{
    CHECK_METADATA_READ
    return _image->pixelWidth();
}

unsigned int Image::pixelHeight() const
{
    CHECK_METADATA_READ
    return _image->pixelHeight();
}

std::string Image::mimeType() const
{
    CHECK_METADATA_READ
    return _image->mimeType();
}

py::list Image::exifKeys()
{
    CHECK_METADATA_READ

    py::list keys;
    for(Exiv2::ExifMetadata::iterator i = _exifData->begin();
        i != _exifData->end();
        ++i)
    {
        keys.append(i->key());
    }
    return keys;
}

const ExifTag Image::getExifTag(std::string key)
{
    CHECK_METADATA_READ

    Exiv2::ExifKey exifKey = Exiv2::ExifKey(key);

    if(_exifData->findKey(exifKey) == _exifData->end())
#ifdef HAVE_CLASS_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::ErrorCode::kerInvalidKey, key);
    }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::kerInvalidKey, key);
    }
#else
    {
        throw Exiv2::Error(KEY_NOT_FOUND, key);
    }
#endif
#endif

    return ExifTag(key, &(*_exifData)[key], _exifData, _image->byteOrder());
}

void Image::deleteExifTag(std::string key)
{
    CHECK_METADATA_READ

    Exiv2::ExifKey exifKey = Exiv2::ExifKey(key);
    Exiv2::ExifMetadata::iterator datum = _exifData->findKey(exifKey);
    if(datum == _exifData->end())
#ifdef HAVE_CLASS_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::ErrorCode::kerInvalidKey, key);
    }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::kerInvalidKey, key);
    }
#else
    {
        throw Exiv2::Error(KEY_NOT_FOUND, key);
    }
#endif
#endif
    _exifData->erase(datum);
}

py::list Image::iptcKeys()
{
    CHECK_METADATA_READ

    py::list keys;
    for(Exiv2::IptcMetadata::iterator i = _iptcData->begin();
        i != _iptcData->end();
        ++i)
    {
        try {
            keys.append(i->key());
        } catch (py::error_already_set &) {
            continue;
        }
    }
    return keys;
}

const IptcTag Image::getIptcTag(std::string key)
{
    CHECK_METADATA_READ

    Exiv2::IptcKey iptcKey = Exiv2::IptcKey(key);

    if(_iptcData->findKey(iptcKey) == _iptcData->end())
#ifdef HAVE_CLASS_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::ErrorCode::kerInvalidKey, key);
    }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::kerInvalidKey, key);
    }
#else
    {
        throw Exiv2::Error(KEY_NOT_FOUND, key);
    }
#endif
#endif
    return IptcTag(key, _iptcData);
}

void Image::deleteIptcTag(std::string key)
{
    CHECK_METADATA_READ

    Exiv2::IptcKey iptcKey = Exiv2::IptcKey(key);
    Exiv2::IptcMetadata::iterator dataIterator = _iptcData->findKey(iptcKey);

    if (dataIterator == _iptcData->end())
#ifdef HAVE_CLASS_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::ErrorCode::kerInvalidKey, key);
    }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::kerInvalidKey, key);
    }
#else
    {
        throw Exiv2::Error(KEY_NOT_FOUND, key);
    }
#endif
#endif

    while (dataIterator != _iptcData->end())
    {
        if (dataIterator->key() == key)
        {
            dataIterator = _iptcData->erase(dataIterator);
        }
        else
        {
            ++dataIterator;
        }
    }
}

py::list Image::xmpKeys()
{
    CHECK_METADATA_READ

    py::list keys;
    for(Exiv2::XmpMetadata::iterator i = _xmpData->begin();
        i != _xmpData->end();
        ++i)
    {
        keys.append(i->key());
    }
    return keys;
}

const XmpTag Image::getXmpTag(std::string key)
{
    CHECK_METADATA_READ

    Exiv2::XmpKey xmpKey = Exiv2::XmpKey(key);

    if(_xmpData->findKey(xmpKey) == _xmpData->end())
#ifdef HAVE_CLASS_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::ErrorCode::kerInvalidKey, key);
    }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::kerInvalidKey, key);
    }
#else
    {
        throw Exiv2::Error(KEY_NOT_FOUND, key);
    }
#endif
#endif

    return XmpTag(key, &(*_xmpData)[key]);
}

void Image::deleteXmpTag(std::string key)
{
    CHECK_METADATA_READ

    Exiv2::XmpKey xmpKey = Exiv2::XmpKey(key);
    Exiv2::XmpMetadata::iterator i = _xmpData->findKey(xmpKey);
    if(i != _xmpData->end())
    {
        _xmpData->erase(i);
    }
    else
#ifdef HAVE_CLASS_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::ErrorCode::kerInvalidKey, key);
    }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
        {
            throw Exiv2::Error(Exiv2::kerInvalidKey, key);
        }
#else
        {
            throw Exiv2::Error(KEY_NOT_FOUND, key);
        }
#endif
#endif
}

const std::string Image::getComment() const
{
    CHECK_METADATA_READ
    return _image->comment();
}

void Image::setComment(const std::string& comment)
{
    CHECK_METADATA_READ
    _image->setComment(comment);
}

void Image::clearComment()
{
    CHECK_METADATA_READ
    _image->clearComment();
}


py::list Image::previews()
{
    CHECK_METADATA_READ

    py::list previews;
    Exiv2::PreviewManager pm(*_image);
    Exiv2::PreviewPropertiesList props = pm.getPreviewProperties();
    for (Exiv2::PreviewPropertiesList::const_iterator i = props.begin();
         i != props.end();
         ++i)
    {
        previews.append(Preview(pm.getPreviewImage(*i)));
    }

    return previews;
}

void Image::copyMetadata(Image& other, bool exif, bool iptc, bool xmp) const
{
    CHECK_METADATA_READ
    if (!other._dataRead) 
    {
#ifdef HAVE_CLASS_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, "metadata not read");
    }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
        {
            throw Exiv2::Error(Exiv2::kerErrorMessage, "metadata not read");
        }
#else
        {
            throw Exiv2::Error(METADATA_NOT_READ);
        }
#endif
#endif
    }

    if (exif)
        other._image->setExifData(*_exifData);
    if (iptc)
        other._image->setIptcData(*_iptcData);
    if (xmp)
        other._image->setXmpData(*_xmpData);
}

py::bytes Image::getDataBuffer() const
{
    std::string buffer;

    // Release the GIL to allow other python threads to run
    // while reading the image data.
    Py_BEGIN_ALLOW_THREADS

    Exiv2::BasicIo& io = _image->io();
    unsigned long size = (unsigned long)io.size();
    long pos = -1;

    if (io.isopen())
    {
        // Remember the current position in the stream
        pos = (long)io.tell();
        // Go to the beginning of the stream
        io.seek(0, Exiv2::BasicIo::beg);
    }
    else
    {
        io.open();
    }

    // Copy the data buffer in a string. Since the data buffer can contain null
    // characters ('\x00'), the string cannot be simply constructed like that:
    //     buffer = std::string((char*) previewImage.pData());
    // because it would be truncated after the first occurence of a null
    // character. Therefore, it has to be copied character by character.
    // First allocate the memory for the whole string...
    buffer.resize(size, ' ');
    // ... then fill it with the raw data.
    for (unsigned long i = 0; i < size; ++i)
    {
        io.read((Exiv2::byte*) &buffer[i], 1);
    }

    if (pos == -1)
    {
        // The stream was initially closed
        io.close();
    }
    else
    {
        // Reset to the initial position in the stream
        io.seek(pos, Exiv2::BasicIo::beg);
    }

    // Re-acquire the GIL
    Py_END_ALLOW_THREADS

    return py::bytes(buffer.c_str(), buffer.size());
}

Exiv2::ByteOrder Image::getByteOrder() const
{
    CHECK_METADATA_READ
    return _image->byteOrder();
}

Exiv2::ExifThumb* Image::_getExifThumbnail()
{
    CHECK_METADATA_READ
    if (_exifThumbnail == 0)
    {
        _exifThumbnail = new Exiv2::ExifThumb(*_exifData);
    }
    return _exifThumbnail;
}

const std::string Image::getExifThumbnailMimeType()
{
    return std::string(_getExifThumbnail()->mimeType());
}

const std::string Image::getExifThumbnailExtension()
{
    return std::string(_getExifThumbnail()->extension());
}

void Image::writeExifThumbnailToFile(const std::string& path)
{
    std::ignore = _getExifThumbnail()->writeFile(path);
}

py::list Image::getExifThumbnailData()
{
    Exiv2::DataBuf buffer = _getExifThumbnail()->copy();
    // Copy the data buffer in a list.
    py::list data;
#ifdef HAVE_CLASS_ERROR_CODE
    for(unsigned int i = 0; i < buffer.size(); ++i) {
        unsigned int datum = *buffer.data(i);
        data.append(datum);
    }
    return data;
#else
    // libexiv2 < 0.28
    for(unsigned int i = 0; i < buffer.size_; ++i)
    {
        unsigned int datum = buffer.pData_[i];
        data.append(datum);
    }
    return data;
#endif
}

void Image::eraseExifThumbnail()
{
    _getExifThumbnail()->erase();
}

void Image::setExifThumbnailFromFile(const std::string& path)
{
    _getExifThumbnail()->setJpegThumbnail(path);
}

void Image::setExifThumbnailFromData(const std::string& data)
{
    const Exiv2::byte* buffer = (const Exiv2::byte*) data.c_str();
    _getExifThumbnail()->setJpegThumbnail(buffer, data.size());
}

const std::string Image::getIptcCharset() const
{
    CHECK_METADATA_READ
    const char* charset = _iptcData->detectCharset();
    if (charset != 0)
    {
        return std::string(charset);
    }
    else
    {
        return std::string();
    }
}

const std::string Image::getXmpPacket(int format) const
{
  // Serialize the current XMP
    std::string xmpPacket;
    if (!_xmpData->empty() && !_image->writeXmpFromPacket()) {
        Exiv2::XmpParser::encode(xmpPacket, _image->xmpData(), format);
        }
  return xmpPacket;
}


py::bytes Image::getICC() const
{
    Exiv2::DataBuf buffer = _image->iccProfile();

    return py::bytes((char*)buffer.c_str(), buffer.size());
}


ExifTag::ExifTag(const std::string& key,
                 Exiv2::Exifdatum* datum, Exiv2::ExifData* data,
                 Exiv2::ByteOrder byteOrder):
    _key(key), _byteOrder(byteOrder)
{
    if (datum != 0 && data != 0)
    {
        _datum = datum;
        _data = data;
    }
    else
    {
        _datum = new Exiv2::Exifdatum(_key);
        _data = 0;
    }

// Conditional code, exiv2 0.21 changed APIs we need
// (see https://bugs.launchpad.net/pyexiv2/+bug/684177).
#if EXIV2_MAJOR_VERSION >= 1 || (EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION >= 21)
    Exiv2::ExifKey exifKey(key);
    _type = Exiv2::TypeInfo::typeName(exifKey.defaultTypeId());
    // Where available, extract the type from the metadata, it is more reliable
    // than static type information. The exception is for user comments, for
    // which we’d rather keep the 'Comment' type instead of 'Undefined'.
    if ((_data != 0) && (_type != "Comment"))
    {
        const char* typeName = _datum->typeName();
        if (typeName != 0)
        {
            _type = typeName;
        }
    }
    _name = exifKey.tagName();
    _label = exifKey.tagLabel();
    _description = exifKey.tagDesc();
    _sectionName = Exiv2::ExifTags::sectionName(exifKey);
    // The section description is not exposed in the API any longer
    // (see http://dev.exiv2.org/issues/744). For want of anything better,
    // fall back on the section’s name.
    _sectionDescription = _sectionName;
#else
    const uint16_t tag = _datum->tag();
    const Exiv2::IfdId ifd = _datum->ifdId();
    _type = Exiv2::TypeInfo::typeName(Exiv2::ExifTags::tagType(tag, ifd));
    // Where available, extract the type from the metadata, it is more reliable
    // than static type information. The exception is for user comments, for
    // which we’d rather keep the 'Comment' type instead of 'Undefined'.
    if ((_data != 0) && (_type != "Comment"))
    {
        const char* typeName = _datum->typeName();
        if (typeName != 0)
        {
            _type = typeName;
        }
    }
    _name = Exiv2::ExifTags::tagName(tag, ifd);
    _label = Exiv2::ExifTags::tagLabel(tag, ifd);
    _description = Exiv2::ExifTags::tagDesc(tag, ifd);
    _sectionName = Exiv2::ExifTags::sectionName(tag, ifd);
    _sectionDescription = Exiv2::ExifTags::sectionDesc(tag, ifd);
#endif
}

ExifTag::~ExifTag()
{
    if (_data == 0)
    {
        delete _datum;
    }
}

void ExifTag::setRawValue(const std::string& value)
{
    int result = _datum->setValue(value);
    if (result != 0)
#ifdef HAVE_CLASS_ERROR_CODE
    {
        std::string message("Invalid value: ");
        message += value;
        throw Exiv2::Error(Exiv2::ErrorCode::kerInvalidDataset, message);
    }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
    {
        std::string message("Invalid value: ");
        message += value;
        throw Exiv2::Error(Exiv2::kerInvalidDataset, message);
    }
#else
    {
        throw Exiv2::Error(INVALID_VALUE);
    }
#endif
#endif
}

void ExifTag::setParentImage(Image& image)
{
    Exiv2::ExifData* data = image.getExifData();
    if (data == _data)
    {
        // The parent image is already the one passed as a parameter.
        // This happens when replacing a tag by itself. In this case, don’t do
        // anything (see https://bugs.launchpad.net/pyexiv2/+bug/622739).
        return;
    }
    _data = data;

#if EXIV2_MAJOR_VERSION >= 1 || (EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION >= 28)
        Exiv2::Value::UniquePtr value = _datum->getValue();
#else
        Exiv2::Value::AutoPtr value = _datum->getValue();
#endif
    delete _datum;
    _datum = &(*_data)[_key.key()];
    _datum->setValue(value.get());

    _byteOrder = image.getByteOrder();
}

const std::string ExifTag::getKey()
{
    return _key.key();
}

const std::string ExifTag::getType()
{
    return _type;
}

const std::string ExifTag::getName()
{
    return _name;
}

const std::string ExifTag::getLabel()
{
    return _label;
}

const std::string ExifTag::getDescription()
{
    return _description;
}

const std::string ExifTag::getSectionName()
{
    return _sectionName;
}

const std::string ExifTag::getSectionDescription()
{
    return _sectionDescription;
}

const std::string ExifTag::getRawValue()
{
    return _datum->toString();
}

const std::string ExifTag::getHumanValue()
{
    return _datum->print(_data);
}

int ExifTag::getByteOrder()
{
    return _byteOrder;
}


IptcTag::IptcTag(const std::string& key, Exiv2::IptcData* data): _key(key)
{
    _from_data = (data != 0);

    if (_from_data)
    {
        _data = data;
    }
    else
    {
        _data = new Exiv2::IptcData();
        _data->add(Exiv2::Iptcdatum(_key));
    }

    Exiv2::IptcMetadata::iterator iterator = _data->findKey(_key);
    const uint16_t tag = iterator->tag();
    const uint16_t record = iterator->record();
    _type = Exiv2::TypeInfo::typeName(Exiv2::IptcDataSets::dataSetType(tag, record));
    _name = Exiv2::IptcDataSets::dataSetName(tag, record);
    _title = Exiv2::IptcDataSets::dataSetTitle(tag, record);
    _description = Exiv2::IptcDataSets::dataSetDesc(tag, record);
    // What is the photoshop name anyway? Where is it used?
    _photoshopName = Exiv2::IptcDataSets::dataSetPsName(tag, record);
    _repeatable = Exiv2::IptcDataSets::dataSetRepeatable(tag, record);
    _recordName = Exiv2::IptcDataSets::recordName(record);
    _recordDescription = Exiv2::IptcDataSets::recordDesc(record);

    if (_from_data)
    {
        // Check that we are not trying to assign multiple values to a tag that
        // is not repeatable.
        unsigned int nb_values = 0;
        for(Exiv2::IptcMetadata::iterator iterator = _data->begin();
            iterator != _data->end(); ++iterator)
        {
            if (iterator->key() == key)
            {
                ++nb_values;
                if (!_repeatable && (nb_values > 1))
#ifdef HAVE_CLASS_ERROR_CODE
                {
                    std::string mssg("Tag not repeatable: ");
                    mssg += key;
                    throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, mssg);
                }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
                {
                    std::string mssg("Tag not repeatable: ");
                    mssg += key;
                    throw Exiv2::Error(Exiv2::kerErrorMessage, mssg);
                }
#else
                {
                    throw Exiv2::Error(NON_REPEATABLE);
                }
#endif
#endif
            }
        }
    }
}

IptcTag::~IptcTag()
{
    if (!_from_data)
    {
        delete _data;
    }
}

void IptcTag::setRawValues(const py::list& values)
{
    if (!_repeatable && (py::len(values) > 1))
    {
        // The tag is not repeatable but we are trying to assign it more than
        // one value.
#ifdef HAVE_CLASS_ERROR_CODE
    {
        throw Exiv2::Error(Exiv2::ErrorCode::kerInvalidDataset, "Tag not repeatable");
    }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
        {
            throw Exiv2::Error(Exiv2::kerInvalidDataset, "Tag not repeatable");
        }
#else
        {
        throw Exiv2::Error(NON_REPEATABLE);
        }
#endif
#endif
    }

    unsigned int index = 0;
    size_t max = py::len(values);
    Exiv2::IptcMetadata::iterator iterator = _data->findKey(_key);
    while (index < max)
    {
        std::string value = values[index++].cast<std::string>();
        if (iterator != _data->end())
        {
            // Override an existing value
            int result = iterator->setValue(value);
            if (result != 0)
#ifdef HAVE_CLASS_ERROR_CODE
            {
                std::string mssg("Invalid value: ");
                mssg += value;
                // there's no invalid value error in libexiv2, so we use 
                // kerInvalidDataset wich raise a Python ValueError
                throw Exiv2::Error(Exiv2::ErrorCode::kerInvalidDataset, mssg);
            }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
            {
                std::string mssg("Invalid value: ");
                mssg += value;
                // there's no invalid value error in libexiv2, so we use 
                // kerInvalidDataset wich raise a Python ValueError
                throw Exiv2::Error(Exiv2::kerInvalidDataset, mssg);
            }
#else
            {
                throw Exiv2::Error(INVALID_VALUE);
            }
#endif
#endif
            // Jump to the next datum matching the key
            ++iterator;
            while ((iterator != _data->end()) && (iterator->key() != _key.key()))
            {
                ++iterator;
            }
        }
        else
        {
            // Append a new value
            Exiv2::Iptcdatum datum(_key);
            int result = datum.setValue(value);
            if (result != 0)
#ifdef HAVE_CLASS_ERROR_CODE
            {
                std::string mssg("Invalid value: ");
                mssg += value;
                throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, mssg);
            }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
            {
                std::string mssg("Invalid value: ");
                mssg += value;
                throw Exiv2::Error(Exiv2::kerErrorMessage, mssg);
            }
#else
            {
                throw Exiv2::Error(INVALID_VALUE);
            }
#endif
#endif
            int state = _data->add(datum);
            if (state == 6)
#ifdef HAVE_CLASS_ERROR_CODE
            {
                std::string mssg("Tag not repeatable: ");
                mssg += _key.key();
                throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, mssg);
            }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
            {
                std::string mssg("Tag not repeatable: ");
                mssg += _key.key();
                throw Exiv2::Error(Exiv2::kerErrorMessage, mssg);
            }
#else
            {
                throw Exiv2::Error(NON_REPEATABLE);
            }
#endif
#endif
            // Reset iterator that has been invalidated by appending a datum
            iterator = _data->end();
        }
    }
    // Erase the remaining values if any
    while (iterator != _data->end())
    {
        if (iterator->key() == _key.key())
        {
            iterator = _data->erase(iterator);
        }
        else
        {
            ++iterator;
        }
    }
}

void IptcTag::setParentImage(Image& image)
{
    Exiv2::IptcData* data = image.getIptcData();
    if (data == _data)
    {
        // The parent image is already the one passed as a parameter.
        // This happens when replacing a tag by itself. In this case, don’t do
        // anything (see https://bugs.launchpad.net/pyexiv2/+bug/622739).
        return;
    }
    const py::list values = getRawValues();
    delete _data;
    _from_data = true;
    _data = data;
    setRawValues(values);
}

const std::string IptcTag::getKey()
{
    return _key.key();
}

const std::string IptcTag::getType()
{
    return _type;
}

const std::string IptcTag::getName()
{
    return _name;
}

const std::string IptcTag::getTitle()
{
    return _title;
}

const std::string IptcTag::getDescription()
{
    return _description;
}

const std::string IptcTag::getPhotoshopName()
{
    return _photoshopName;
}

const bool IptcTag::isRepeatable()
{
    return _repeatable;
}

const std::string IptcTag::getRecordName()
{
    return _recordName;
}

const std::string IptcTag::getRecordDescription()
{
    return _recordDescription;
}

const py::list IptcTag::getRawValues()
{
    py::list values;
    for(Exiv2::IptcMetadata::iterator iterator = _data->begin();
        iterator != _data->end(); ++iterator)
    {
        if (iterator->key() == _key.key())
        {
            values.append(iterator->toString());
        }
    }
    return values;
}


XmpTag::XmpTag(const std::string& key, Exiv2::Xmpdatum* datum): _key(key)
{
    _from_datum = (datum != 0);

    if (_from_datum)
    {
        _datum = datum;
        _exiv2_type = datum->typeName();
    }
    else
    {
        _datum = new Exiv2::Xmpdatum(_key);
        _exiv2_type = Exiv2::TypeInfo::typeName(Exiv2::XmpProperties::propertyType(_key));
    }

    const char* title = Exiv2::XmpProperties::propertyTitle(_key);
    if (title != 0)
    {
        _title = title;
    }

    const char* description = Exiv2::XmpProperties::propertyDesc(_key);
    if (description != 0)
    {
        _description = description;
    }

    const Exiv2::XmpPropertyInfo* info = Exiv2::XmpProperties::propertyInfo(_key);
    if (info != 0)
    {
        _name = info->name_;
        _type = info->xmpValueType_;
    }
}

XmpTag::~XmpTag()
{
    if (!_from_datum)
    {
        delete _datum;
    }
}

void XmpTag::setTextValue(const std::string& value)
{
    _datum->setValue(value);
}

void XmpTag::setArrayValue(const py::list& values)
{
    // Reset the value
    _datum->setValue(0);

    for (auto value : values) {
        _datum->setValue(std::string(py::str(value)));
    }
}

void XmpTag::setLangAltValue(const py::dict& values)
{
    // Reset the value
    _datum->setValue(0);

    for (auto item : values) {
        auto key = item.first.cast<std::string>();
        auto value = item.second.cast<std::string>();
        _datum->setValue("lang=\"" + key + "\" " + value);
    }
}

void XmpTag::setParentImage(Image& image)
{
    Exiv2::Xmpdatum* datum = &(*image.getXmpData())[_key.key()];
    if (datum == _datum)
    {
        // The parent image is already the one passed as a parameter.
        // This happens when replacing a tag by itself. In this case, don’t do
        // anything (see https://bugs.launchpad.net/pyexiv2/+bug/622739).
        return;
    }

#if EXIV2_MAJOR_VERSION >= 1 || (EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION >= 28)
        Exiv2::Value::UniquePtr value = _datum->getValue();
#else
        Exiv2::Value::AutoPtr value = _datum->getValue();
#endif
    delete _datum;
    _from_datum = true;
    _datum = &(*image.getXmpData())[_key.key()];
    _datum->setValue(value.get());
}

const std::string XmpTag::getKey()
{
    return _key.key();
}

const std::string XmpTag::getExiv2Type()
{
    return _exiv2_type;
}

const std::string XmpTag::getType()
{
    return _type;
}

const std::string XmpTag::getName()
{
    return _name;
}

const std::string XmpTag::getTitle()
{
    return _title;
}

const std::string XmpTag::getDescription()
{
    return _description;
}

const std::string XmpTag::getTextValue()
{
    return dynamic_cast<const Exiv2::XmpTextValue*>(&_datum->value())->value_;
}

const py::list XmpTag::getArrayValue()
{
#ifdef HAVE_OLD_ERROR_CODE
    std::vector<std::string> value =
        dynamic_cast<const Exiv2::XmpArrayValue*>(&_datum->value())->value_;
    py::list rvalue;
    for(std::vector<std::string>::const_iterator i = value.begin();
        i != value.end(); ++i)
    {
        rvalue.append(*i);
    }
    return rvalue;
#else
    // We can't use &_datum->value())->value_ because value_ is private in
    // this context (change in libexiv2 0.27)
    const Exiv2::XmpArrayValue* xav = 
            dynamic_cast<const Exiv2::XmpArrayValue*>(&_datum->value());
    py::list rvalue;
    for(size_t i = 0; i < xav->count(); ++i)
    {
        std::string value = xav->toString(i);
        rvalue.append(value);
    }
    return rvalue;
#endif
}

const py::dict XmpTag::getLangAltValue()
{
    Exiv2::LangAltValue::ValueType value =
        dynamic_cast<const Exiv2::LangAltValue*>(&_datum->value())->value_;
    py::dict rvalue;
    for (Exiv2::LangAltValue::ValueType::const_iterator i = value.begin();
         i != value.end(); ++i)
    {
        rvalue[i->first.c_str()] = i->second;
    }
    return rvalue;
}


Preview::Preview(const Exiv2::PreviewImage& previewImage)
{
    _mimeType = previewImage.mimeType();
    _extension = previewImage.extension();
    _size = previewImage.size();
    _dimensions = py::make_tuple(previewImage.width(),
                                            previewImage.height());

    // Copy the data buffer in a string. Since the data buffer can contain null
    // characters ('\x00'), the string cannot be simply constructed like that:
    //     _data = std::string((char*) previewImage.pData());
    // because it would be truncated after the first occurence of a null
    // character. Therefore, it has to be copied character by character.
    const Exiv2::byte* pData = previewImage.pData();
    // First allocate the memory for the whole string...
    _data = std::string(_size, ' ');
    // ... then fill it with the raw data.
    for(unsigned int i = 0; i < _size; ++i)
    {
        _data[i] = pData[i];
    }
}

py::object Preview::getData() const
{
    return py::bytes((char*)_data.c_str(), _size);
}

void Preview::writeToFile(const std::string& path) const
{
    std::string filename = path + _extension;
    std::ofstream fd(filename.c_str(), std::ios::out | std::ios::binary);
    fd << _data;
    fd.close();
}


#ifdef HAVE_OLD_ERROR_CODE
void translateExiv2Error(Exiv2::Error const& error)
{
    // Use the Python 'C' API to set up an exception object
    const char* message = error.what();

    // The type of the Python exception depends on the error code
    // Warning: this piece of code should be updated in case the error codes
    // defined by Exiv2 (file 'src/error.cpp') are changed
    switch (error.code())
    {
        // Exiv2 error codes
        case 2:
            // {path}: Call to `{function}' failed: {strerror}
            // May be raised when reading a file
            PyErr_SetString(PyExc_RuntimeError, message);
            break;
        case 3:
            // This does not look like a {image type} image
            // May be raised by readMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 4:
            // Invalid dataset name `{dataset name}'
            // May be raised when instantiating an IptcKey from a string
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 5:
            // Invalid record name `{record name}'
            // May be raised when instantiating an IptcKey from a string
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 6:
            // Invalid key `{key}'
            // May be raised when instantiating an ExifKey, an IptcKey or an
            // XmpKey from a string
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 7:
            // Invalid tag name or ifdId `{tag name}', ifdId {ifdId}
            // May be raised when instantiating an ExifKey from a string
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 8:
            // Value not set
            // May be raised when calling value() on a datum
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 9:
            // {path}: Failed to open the data source: {strerror}
            // May be raised by readMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 10:
            // {path}: Failed to open file ({mode}): {strerror}
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 11:
            // {path}: The file contains data of an unknown image type
            // May be raised when opening an image
            PyErr_SetString(PyExc_TypeError, message);
            break;
        case 12:
            // The memory contains data of an unknown image type
            // May be raised when instantiating an image from a data buffer
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 13:
            // Image type {image type} is not supported
            // May be raised when creating a new image
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 14:
            // Failed to read image data
            // May be raised by readMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 15:
            // This does not look like a JPEG image
            // May be raised by readMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 17:
            // {old path}: Failed to rename file to {new path}: {strerror}
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 18:
            // {path}: Transfer failed: {strerror}
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 19:
            // Memory transfer failed: {strerror}
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 20:
            // Failed to read input data
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 21:
            // Failed to write image
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 22:
            // Input data does not contain a valid image
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 23:
            // Invalid ifdId {ifdId}
            // May be raised when instantiating an ExifKey from a tag and
            // IFD item string
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 26:
            // Offset out of range
            // May be raised by writeMetadata() (TIFF)
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 27:
            // Unsupported data area offset type
            // May be raised by writeMetadata() (TIFF)
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 28:
            // Invalid charset: `{charset name}'
            // May be raised when instantiating a CommentValue from a string
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 29:
            // Unsupported date format
            // May be raised when instantiating a DateValue from a string
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 30:
            // Unsupported time format
            // May be raised when instantiating a TimeValue from a string
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 31:
            // Writing to {image format} images is not supported
            // May be raised by writeMetadata() for certain image types
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 32:
            // Setting {metadata type} in {image format} images is not supported
            // May be raised when setting certain types of metadata for certain
            // image types that don't support them
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 33:
            // This does not look like a CRW image
            // May be raised by readMetadata() (CRW)
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 35:
            // No namespace info available for XMP prefix `{prefix}'
            // May be raised when retrieving property info for an XmpKey
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 36:
            // No prefix registered for namespace `{namespace}', needed for
            // property path `{property path}'
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 37:
            // Size of {type of metadata} JPEG segment is larger than
            // 65535 bytes
            // May be raised by writeMetadata() (JPEG)
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 38:
            // Unhandled Xmpdatum {key} of type {value type}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_TypeError, message);
            break;
        case 39:
            // Unhandled XMP node {key} with opt={XMP Toolkit option flags}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_TypeError, message);
            break;
        case 40:
            // XMP Toolkit error {error id}: {error message}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_RuntimeError, message);
            break;
        case 41:
            // Failed to decode Lang Alt property {property path}
            // with opt={XMP Toolkit option flags}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 42:
            // Failed to decode Lang Alt qualifier {qualifier path}
            // with opt={XMP Toolkit option flags}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 43:
            // Failed to encode Lang Alt property {key}
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 44:
            // Failed to determine property name from path {property path},
            // namespace {namespace}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 45:
            // Schema namespace {namespace} is not registered with
            // the XMP Toolkit
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 46:
            // No namespace registered for prefix `{prefix}'
            // May be raised when instantiating an XmpKey from a string
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 47:
            // Aliases are not supported. Please send this XMP packet
            // to ahuggel@gmx.net `{namespace}', `{property path}', `{value}'
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 48:
            // Invalid XmpText type `{type}'
            // May be raised when instantiating an XmpTextValue from a string
            PyErr_SetString(PyExc_TypeError, message);
            break;
        case 49:
            // TIFF directory {TIFF directory name} has too many entries
            // May be raised by writeMetadata() (TIFF)
            PyErr_SetString(PyExc_IOError, message);
            break;
        // Added in py3exiv2
        case 50:
            // Multiple TIFF array element tags %1 in one directory")
            // May be raised by readMetadata() (TIFF)
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 51:
            // TIFF array element tag %1 has wrong type") }, // %1=tag number
            // May be raised by readMetadata() (TIFF)
            PyErr_SetString(PyExc_TypeError, message);
            break;
        case 52:
            // %1 has invalid XMP value type `%2'
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_ValueError, message);
            break;

        // Custom error codes
        case METADATA_NOT_READ:
            PyErr_SetString(PyExc_IOError, "Image metadata has not been read yet");
            break;
        case NON_REPEATABLE:
            PyErr_SetString(PyExc_KeyError, "Tag is not repeatable");
            break;
        case KEY_NOT_FOUND:
            PyErr_SetString(PyExc_KeyError, "Tag not set");
            break;
        case INVALID_VALUE:
            PyErr_SetString(PyExc_ValueError, "Invalid value");
            break;
        case EXISTING_PREFIX:
            PyErr_SetString(PyExc_KeyError, "A namespace with this prefix already exists");
            break;
        case BUILTIN_NS:
            PyErr_SetString(PyExc_KeyError, "Cannot unregister a builtin namespace");
            break;
        case NOT_REGISTERED:
            PyErr_SetString(PyExc_KeyError, "No namespace registered under this name");
            break;

        // Default handler
        default:
            PyErr_SetString(PyExc_RuntimeError, message);
    }
}

#else
void translateExiv2Error(Exiv2::Error const& error)
{
    // Use the Python 'C' API to set up an exception object
    const char* message = error.what();

    // py::print("=!======= ERROR (HAVE_CLASS_ERROR_CODE): ", int(error.code()));
    WRAP_ERROR << "translateExiv2Error Err: " << int(error.code()) << " : " << message;

    // The type of the Python exception depends on the error code
    // Warning: this piece of code should be updated in case the error codes
    // defined by Exiv2 (file 'src/error.cpp') are changed
    switch (int(error.code()))
    {
        case 1:
            // kerErrorMessage Unidentified error
            PyErr_SetString(PyExc_RuntimeError, message);
            break;
        case 2:
            // kerCallFailed {path}: Call to `{function}' failed: {strerror}
            // May be raised when reading a file
            PyErr_SetString(PyExc_RuntimeError, message);
            break;
        case 3:
            // kerNotAnImage This does not look like a {image type} image
            // May be raised by readMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 4:
            // kerInvalidDataset Invalid dataset name `{dataset name}'
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 5:
            // kerInvalidRecord Invalid record name `{record name}'
            // May be raised when instantiating an IptcKey from a string
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 6:
            // kerInvalidKey Invalid key `{key}'
            // May be raised when instantiating an ExifKey, an IptcKey or an
            // XmpKey from a string
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 7:
            // kerInvalidTag 
            // Invalid tag name or ifdId `{tag name}', ifdId {ifdId}
            // May be raised when instantiating an ExifKey from a string
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 8:
            // kerValueNotSet Value not set
            // May be raised when calling value() on a datum
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 9:
            // kerDataSourceOpenFailed 
            // {path}: Failed to open the data source: {strerror}
            // May be raised by readMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 10:
            // kerFileOpenFailed
            // {path}: Failed to open file ({mode}): {strerror}
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 11:
            // kerFileOpenFailed
            // {path}: The file contains data of an unknown image type
            // May be raised when opening an image
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 12:
            // kerMemoryContainsUnknownImageType
            //The memory contains data of an unknown image type
            // May be raised when instantiating an image from a data buffer
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 13:
            // kerUnsupportedImageType Image type {image type} is not supported
            // May be raised when creating a new image
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 14:
            // kerFailedToReadImageData Failed to read image data
            // May be raised by readMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 15:
            // kerNotAJpeg This does not look like a JPEG image
            // May be raised by readMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 17:
            // kerFileRenameFailed 
            // {old path}: Failed to rename file to {new path}: {strerror}
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 18:
            // kerTransferFailed {path}: Transfer failed: {strerror}
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 19:
            // kerMemoryTransferFailed Memory transfer failed: {strerror}
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 20:
            // kerInputDataReadFailed Failed to read input data
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 21:
            // kerImageWriteFailed Failed to write image
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 22:
            // kerNoImageInInputData Input data does not contain a valid image
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 23:
            // kerInvalidIfdId Invalid ifdId {ifdId}
            // May be raised when instantiating an ExifKey from a tag and
            // IFD item string
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 24:
            // kerValueTooLarge
            // Entry::setValue: Value too large {tag}, {size}, {requested}
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 25:
            // kerDataAreaValueTooLarge
            // Entry::setDataArea: Value too large {tag}, {size}, {requested}
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 26:
            // kerOffsetOutOfRange Offset out of range
            // May be raised by writeMetadata() (TIFF)
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 27:
            // kerUnsupportedDataAreaOffsetType
            // Unsupported data area offset type
            // May be raised by writeMetadata() (TIFF)
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 28:
            // kerInvalidCharset Invalid charset: `{charset name}'
            // May be raised when instantiating a CommentValue from a string
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 29:
            // kerUnsupportedDateFormat Unsupported date format
            // May be raised when instantiating a DateValue from a string
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 30:
            // kerUnsupportedTimeFormat Unsupported time format
            // May be raised when instantiating a TimeValue from a string
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 31:
            // kerWritingImageFormatUnsupported 
            // Writing to {image format} images is not supported
            // May be raised by writeMetadata() for certain image types
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 32:
            // kerInvalidSettingForImage 
            // Setting {metadata type} in {image format} images is not supported
            // May be raised when setting certain types of metadata for certain
            // image types that don't support them
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 33:
            // kerNotACrwImage This does not look like a CRW image
            // May be raised by readMetadata() (CRW)
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 34:
            // kerFunctionNotSupported {function}: Not supported
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 35:
            // kerNoNamespaceInfoForXmpPrefix
            // No namespace info available for XMP prefix `{prefix}'
            // May be raised when retrieving property info for an XmpKey
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 36:
            // kerNoPrefixForNamespace
            // No prefix registered for namespace `{namespace}', needed for
            // property path `{property path}'
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 37:
            // kerTooLargeJpegSegment
            // Size of {type of metadata} JPEG segment is larger than
            // 65535 bytes
            // May be raised by writeMetadata() (JPEG)
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 38:
            // kerUnhandledXmpdatum
            // Unhandled Xmpdatum {key} of type {value type}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_TypeError, message);
            break;
        case 39:
            // kerUnhandledXmpNode
            // Unhandled XMP node {key} with opt={XMP Toolkit option flags}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_TypeError, message);
            break;
        case 40:
            // kerXMPToolkitError
            // XMP Toolkit error {error id}: {error message}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_RuntimeError, message);
            break;
        case 41:
            // kerDecodeLangAltPropertyFailed
            // Failed to decode Lang Alt property {property path}
            // with opt={XMP Toolkit option flags}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 42:
            // kerDecodeLangAltQualifierFailed
            // Failed to decode Lang Alt qualifier {qualifier path}
            // with opt={XMP Toolkit option flags}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 43:
            // kerEncodeLangAltPropertyFailed
            // Failed to encode Lang Alt property {key}
            // May be raised by writeMetadata()
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 44:
            // kerPropertyNameIdentificationFailed
            // Failed to determine property name from path {property path},
            // namespace {namespace}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 45:
            // kerSchemaNamespaceNotRegistered
            // Schema namespace {namespace} is not registered with
            // the XMP Toolkit
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 46:
            // kerNoNamespaceForPrefix
            // No namespace registered for prefix `{prefix}'
            // May be raised when instantiating an XmpKey from a string
            PyErr_SetString(PyExc_KeyError, message);
            break;
        case 47:
            // kerAliasesNotSupported
            // Aliases are not supported. Please send this XMP packet
            // to ahuggel@gmx.net `{namespace}', `{property path}', `{value}'
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 48:
            // kerInvalidXmpText
            // Invalid XmpText type `{type}'
            // May be raised when instantiating an XmpTextValue from a string
            PyErr_SetString(PyExc_TypeError, message);
            break;
        case 49:
            // kerTooManyTiffDirectoryEntries
            // TIFF directory {TIFF directory name} has too many entries
            // May be raised by writeMetadata() (TIFF)
            PyErr_SetString(PyExc_IOError, message);
            break;
        // Added in py3exiv2
        case 50:
            // kerMultipleTiffArrayElementTagsInDirectory
            // Multiple TIFF array element tags {number} in one directory")
            // May be raised by readMetadata() (TIFF)
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 51:
            // kerWrongTiffArrayElementTagType
            // TIFF array element tag {number} has wrong type") }, // %1=tag number
            // May be raised by readMetadata() (TIFF)
            PyErr_SetString(PyExc_TypeError, message);
            break;
        // Added in libexiv2 0.27
        case 52:
            // kerInvalidKeyXmpValue {key} has invalid XMP value type {type}
            // May be raised by readMetadata() when reading the XMP data
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 53:
            // kerInvalidIccProfile Not a valid ICC Profile
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 54:
            // kerInvalidXMP Not valid XMP
            PyErr_SetString(PyExc_TypeError, message);
            break;
        case 55:
            // kerTiffDirectoryTooLarge tiff directory length is too large
            PyErr_SetString(PyExc_ValueError, message);
            break;
        case 56:
            // kerInvalidTypeValue
            // Invalid type value detected in Image::printIFDStructure
            PyErr_SetString(PyExc_TypeError, message);
            break;
        case 57:
            // kerInvalidMalloc
            // Invalid memory allocation request
            PyErr_SetString(PyExc_MemoryError, message);
            break;
        case 58:
            // kerCorruptedMetadata Corrupted image metadata
            PyErr_SetString(PyExc_IOError, message);
            break;
        case 59:
            // kerArithmeticOverflow Arithmetic operation overflow
            PyErr_SetString(PyExc_OverflowError, message);
            break;
        case 60:
            // kerMallocFailed Memory allocation failed
            PyErr_SetString(PyExc_MemoryError, message);
            break;

        // Default handler
        default:
            PyErr_SetString(PyExc_RuntimeError, message);
    }
}
#endif

bool initialiseXmpParser()
{
    if (!Exiv2::XmpParser::initialize())
        return false;

    std::string prefix("py3exiv2");
    std::string name("www.py3exiv2.tuxfamily.org/");

    try
    {
        const std::string& ns = Exiv2::XmpProperties::ns(prefix);
    }

    catch (Exiv2::Error &)
    {
        // No namespace exists with the requested prefix, it is safe to
        // register a new one.
        Exiv2::XmpProperties::registerNs(name, prefix);
    }

    return true;
}

bool closeXmpParser()
{
    std::string name("www.py3exiv2.tuxfamily.org/");
    const std::string& prefix = Exiv2::XmpProperties::prefix(name);
    if (prefix != "")
    {
        Exiv2::XmpProperties::unregisterNs(name);
    }

    Exiv2::XmpParser::terminate();

    return true;
}

void registerXmpNs(const std::string& name, const std::string& prefix)
{
    try
    {
        const std::string& ns = Exiv2::XmpProperties::ns(prefix);
    }

    catch (Exiv2::Error &)
    {
        // No namespace exists with the requested prefix, it is safe to
        // register a new one.
        Exiv2::XmpProperties::registerNs(name, prefix);
        return;
    }
#ifdef HAVE_CLASS_ERROR_CODE
    {
        std::string mssg("Namespace already exists: ");
        mssg += prefix;
        throw Exiv2::Error(Exiv2::ErrorCode::kerInvalidKey, mssg);
    }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
    {
        std::string mssg("Namespace already exists: ");
        mssg += prefix;
        throw Exiv2::Error(Exiv2::kerInvalidKey, mssg);
    }
#else
    {
        throw Exiv2::Error(EXISTING_PREFIX, prefix);
    }
#endif
#endif
}

void unregisterXmpNs(const std::string& name)
{
    const std::string& prefix = Exiv2::XmpProperties::prefix(name);
    if (prefix != "")
    {
        Exiv2::XmpProperties::unregisterNs(name);
        try
        {
            std::ignore = Exiv2::XmpProperties::nsInfo(prefix);
        }
        catch (Exiv2::Error&)
        {
            // The namespace has been successfully unregistered.
            return;
        }
        // The namespace hasn’t been unregistered because it’s builtin.
#ifdef HAVE_CLASS_ERROR_CODE
        {
            std::string mssg("Can't unregister builtin namespace: ");
            mssg += name;
            throw Exiv2::Error(Exiv2::ErrorCode::kerInvalidKey, mssg);
        }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
        {
            std::string mssg("Can't unregister builtin namespace: ");
            mssg += name;
            throw Exiv2::Error(Exiv2::kerInvalidKey, mssg);
        }
#else
        {
            throw Exiv2::Error(BUILTIN_NS, name);
        }
#endif
#endif
    }
    else
#ifdef HAVE_CLASS_ERROR_CODE
    {
        std::string mssg("Namespace does not exists: ");
        mssg += name;
        throw Exiv2::Error(Exiv2::ErrorCode::kerInvalidKey, mssg);
    }
#else 
#ifdef HAVE_EXIV2_ERROR_CODE
    {
        std::string mssg("Namespace does not exists: ");
        mssg += name;
        throw Exiv2::Error(Exiv2::kerInvalidKey, mssg);
    }
#else
    {
        throw Exiv2::Error(NOT_REGISTERED, name);
    }
#endif
#endif
}

void unregisterAllXmpNs()
{
    // Unregister all custom namespaces.
    Exiv2::XmpProperties::unregisterNs();
}


void logHandler(int level, const char *msg)
{
    std::cout << "EVIV2LOG: " << msg << std::endl;
}


void initLog()
{
    Exiv2::LogMsg::setHandler(logHandler);
}

void setLogLevel(int level)
{
    if (level == 0)
        Exiv2::LogMsg::setLevel(Exiv2::LogMsg::debug);
    if (level == 1)
        Exiv2::LogMsg::setLevel(Exiv2::LogMsg::info);
    if (level == 2)
        Exiv2::LogMsg::setLevel(Exiv2::LogMsg::warn);
    if (level == 3)
        Exiv2::LogMsg::setLevel(Exiv2::LogMsg::error);
    if (level == 4)
        Exiv2::LogMsg::setLevel(Exiv2::LogMsg::mute);
}


} // End of namespace exiv2wrapper

