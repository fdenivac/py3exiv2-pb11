#!/usr/bin/env python3
# - * - coding: utf-8 - * -
#
# Package py3exiv2 (module name : pyexiv2) for python 3
#
#   This py3exiv2 version use pybind11 to bind exiv2 library to python.
#
#
# BUILD REQUIREMENTS
# ==================
# Windows requirements :
#   - Microsoft Visual Studio
#   - exiv2 includes,libraries
# Linux package requirements :
#   - build-essential
#   - libexiv2-dev

# BUILD
# ======
#  > cd <ROOT_PROJECT>
#  > python3 -m build
#
#
# INSTALL
# =======
#  * Install after build :
#       > pip install <ROOT_PROJECT>/dist/py3exiv2-1.0.0-cp311-cp311-win_amd64.whl
#  * Direct install invoking build :
#       > cd <ROOT_PROJECT>
#       > python3 setup.py install
#


import os
import shutil
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension


if os.name == "nt":

    exiv2_dir = os.environ.get("EXIV2_DIR")
    if not exiv2_dir:
        raise ValueError(
            'Build requires the Exiv2 tree path via environnement variable "EXIV2_DIR"'
        )

    exiv2_incdir = os.path.join(exiv2_dir, "include")
    exiv2_libdir = os.path.join(exiv2_dir, "lib")
    exiv2_dll = os.path.join(exiv2_dir, "bin", "exiv2.dll")

    if not (
        os.path.isdir(exiv2_incdir)
        and os.path.isdir(exiv2_libdir)
        and os.path.isfile(exiv2_dll)
    ):
        raise ValueError(
            f'"EXIV2_DIR" file (exiv2.dll) or directories ("include", "lib") not found in root : {exiv2_dir}'
        )

    # exiv2 local copy
    shutil.copy(exiv2_dll, "pyexiv2/")

    # prepare build
    incdirs = [exiv2_incdir]
    libdirs = [exiv2_libdir]
    altlibs = ["exiv2"]
    platname = "win_amd64"

else:
    # tested on Linux, but for others ??
    altlibs = ["exiv2"]
    libdirs = ["/usr/local/lib"]
    incdirs = ["/usr/local/include/exiv2"]
    platname = "linux_x86_64"

setup(
    packages=["pyexiv2"],
    options={
        "bdist_wheel": {
            "plat_name": platname,
        },
    },
    ext_modules=[
        Pybind11Extension(
            "pyexiv2.libexiv2python",
            ["pyexiv2/exiv2wrapper.cpp", "pyexiv2/exiv2wrapper_python.cpp"],
            include_dirs=incdirs,
            library_dirs=libdirs,
            libraries=altlibs,
        ),
    ],
)
