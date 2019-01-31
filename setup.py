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


from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion
from distutils.version import LooseVersion
from shutil import copyfile, copymode

import setuptools

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions))

        if platform.system() == "Windows":
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)', out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

        # from cmake_example PR #16
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        cmake_args = [
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + os.path.join(extdir, 'opentimelineio') ,
            '-DPYTHON_EXECUTABLE=' + sys.executable,
            '-DCMAKE_INSTALL_RPATH=$ORIGIN',
            '-DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON',
            '-DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=OFF',
        ]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir)]
            if sys.maxsize > 2**32:
                cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            build_args += ['--', '-j2']

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get('CXXFLAGS', ''),
                                                              self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)


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
    packages=setuptools.find_packages(where="python"),
    ext_modules=[
        CMakeExtension('_opentimelineio'),
        CMakeExtension('_opentime')
    ],
    cmdclass=dict(build_ext=CMakeBuild),
    package_dir={
        "":"python"
    },
    package_data={
        'opentimelineio': [
            'adapters/builtin_adapters.plugin_manifest.json',
            # 'lib/libopentimelineio.so',
            # 'lib/libopentime.so',
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
