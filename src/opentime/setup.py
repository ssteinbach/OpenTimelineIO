from setuptools import setup, Extension

libopentime = Extension(
    'opentime',
    libraries=['opentime'],
    library_dirs=['/home/steinbach/workspace/OpenTimelineIO/src/opentime'],
    sources=['py_opentime.c']
)

setup(
    name='OpenTime',
    version='0.1',
    description='C backed version of opentime.',
    author='Pixar Animation Studios',
    author_email='opentimlineio@pixar.com',
    url='http://opentimeline.io',
    long_description='API for manipulating time for editorial uses.',
    ext_modules=[libopentime]
)
