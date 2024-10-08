#!/usr/bin/python3
# -*- coding: utf-8 -*-

# ******************************************************************************
#
# Copyright (C) 2008-2011 Olivier Tilloy <olivier@tilloy.net>
# Copyright (C) 2015-2016 Vincent Vande Vyvre <vincent.vandevyvre@oqapy.eu>
#
# This file is part of the pyexiv2 distribution.
#
# pyexiv2 is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# pyexiv2 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with pyexiv2; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
#
# Maintainer: Vincent Vande Vyvre <vincent.vandevyvre@oqapy.eu>
#
# ******************************************************************************

import unittest

# Test cases to run
from test_readmetadatatestcase import ReadMetadataTestCase
from test_gps_coordinate import TestGPSCoordinate
from test_notifying_list import TestNotifyingList
from test_exif import TestExifTag
from test_iptc import TestIptcTag
from test_xmp import TestXmpTag, TestXmpNamespaces
from test_metadata import TestImageMetadata
from test_buffer import TestBuffer
from test_encoding import TestEncodings
from utils import TestConversions, TestFractions
from test_usercomment import TestUserCommentReadWrite, TestUserCommentAdd
from test_pickling import TestPicklingTags
from test_datetimeformatter import TestDateTimeFormatter


def run_unit_tests():
    # Instantiate a test suite containing all the test cases
    suite = unittest.TestSuite()
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(ReadMetadataTestCase))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestGPSCoordinate))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestNotifyingList))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestExifTag))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestIptcTag))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestXmpTag))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestXmpNamespaces))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestImageMetadata))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestBuffer))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestEncodings))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestConversions))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestFractions))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestUserCommentReadWrite))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestUserCommentAdd))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestPicklingTags))
    suite.addTest(unittest.defaultTestLoader.loadTestsFromTestCase(TestDateTimeFormatter))
    # Run the test suite
    return unittest.TextTestRunner(verbosity=2).run(suite)


if __name__ == '__main__':
    run_unit_tests()

