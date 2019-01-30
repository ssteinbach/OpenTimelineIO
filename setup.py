#! /usr/bin/env python

"""Test of C++/pybind + cmake
"""

import os
import re
import sys
import sysconfig
import platform
import subprocess
import unittest

from distutils.version import LooseVersion
from shutil import copyfile, copymode

from skbuild import (
    setup,
)
import setuptools

def test_otio():
    """Discovers and runs tests"""
    try:
        # Clear the environment of a preset media linker
        del os.environ['OTIO_DEFAULT_MEDIA_LINKER']
    except KeyError:
        pass
    return unittest.TestLoader().discover('tests')

setup(
    name='opentimelineio',
    version='0.9.1',
    author='Pixar Animation Studios',
    author_email='foo@bar.com',
    description='Blah',
    long_description='',
    test_suite='setup.test_otio',
    include_package_data=True,
    cmake_args=['-DCMAKE_SKIP_BUILD_RPATH=1'],
    packages=[
        'opentimelineio',
        # 'opentimelineio.console',
        # 'opentimelineio_contrib',
        # 'opentimelineio_contrib.adapters',
        'opentimelineview',
    ],
    package_dir={
        "":"python"
    },
    package_data={
        'opentimelineio': [
            'adapters/builtin_adapters.plugin_manifest.json',
            'lib/libopentimelineio.so',
            'lib/libopentime.so',
        ],
        # 'opentimelineio_contrib': [
        #     'adapters/contrib_adapters.plugin_manifest.json',
        # ]
    },
    # because we need to open() the adapters manifest, we aren't zip-safe
    zip_safe=False,
    tests_require=[
            'mock;python_version<"3.3"',
    ],
    entry_points={
        'console_scripts': [
            'otioview = opentimelineview.console:main',
            'otiocat = opentimelineio.console.otiocat:main',
            'otioconvert = opentimelineio.console.otioconvert:main',
            'otiostat = opentimelineio.console.otiostat:main',
        ],
    },
)
