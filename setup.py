# -*- coding: utf-8 -*-
# occiput - Tomographic Vision
# Harvard University, Martinos Center for Biomedical Imaging
# Aalto University, Department of Computer Science


# Use old Python build system, otherwise the extension libraries cannot be found. FIXME
import sys
from setuptools import setup, Extension
from glob import glob
import os
import sysconfig
from Cython.Distutils import build_ext

def get_ext_filename_without_platform_suffix(filename):
    name, ext = os.path.splitext(filename)
    ext_suffix = sysconfig.get_config_var('EXT_SUFFIX')

    if ext_suffix == ext:
        return filename

    ext_suffix = ext_suffix.replace(ext, '')
    idx = name.find(ext_suffix)

    if idx == -1:
        return filename
    else:
        return name[:idx] + ext


class BuildExtWithoutPlatformSuffix(build_ext):
    def get_ext_filename(self, ext_name):
        filename = super().get_ext_filename(ext_name)
        return get_ext_filename_without_platform_suffix(filename)

petlink32_c_module = Extension('occiput.DataSources.petlink.petlink32_c',
                               [os.path.join('occiput', 'DataSources','petlink', 'petlink32_c.c')])
test_simplewrap_module = Extension('occiput.test.tests_simplewrap.test_simplewrap_c',
                                   [os.path.join('occiput', 'test','tests_simplewrap', 'test_simplewrap_c.c')])
test_matrices_module = Extension('occiput.test.tests_simplewrap.test_matrices_c',
                                 [os.path.join('occiput', 'test','tests_simplewrap', 'test_matrices_c.c')])
build_ext_fun = BuildExtWithoutPlatformSuffix


setup(
    name="occiput",
    version="1.2.1",
    author="Stefano Pedemonte and Michele Scipioni",
    author_email="occiput.reconstruction@gmail.com",
    packages=[
        "occiput",
        "occiput.test",
        "occiput.notebooks",
        "occiput.Core",
        "occiput.Reconstruction",
        "occiput.Reconstruction.PET",
        "occiput.Reconstruction.SPECT",
        #              'occiput.Reconstruction.CT',
        "occiput.Reconstruction.MR",
        "occiput.Transformation",
        "occiput.Registration",
        #              'occiput.Registration.Affine',
        "occiput.Registration.TranslationRotation",
        #              'occiput.Classification',
        "occiput.DataSources",
        "occiput.DataSources.Synthetic",
        "occiput.DataSources.FileSources",
        "occiput.Visualization",
    ],
    data_files=[
        (os.path.join('occiput','Visualization', 'DisplayNode', 'static'),
         glob(os.path.join('occiput','Visualization','DisplayNode', 'static', '*.*'))),
        (os.path.join('occiput', 'Visualization','DisplayNode', 'static', 'openseadragon'),
         glob(os.path.join('occiput','Visualization', 'DisplayNode', 'static', 'openseadragon', '*.*'))),
        (os.path.join('occiput','Visualization', 'DisplayNode', 'static', 'openseadragon', 'images'),
         glob(os.path.join('occiput', 'Visualization','DisplayNode', 'static', 'openseadragon', 'images', '*.*'))),
        (os.path.join('occiput', 'Visualization','DisplayNode', 'static', 'tipix'),
         glob(os.path.join('occiput','Visualization', 'DisplayNode', 'static', 'tipix', '*.*'))),
        (os.path.join('occiput','Visualization', 'DisplayNode', 'static', 'tipix', 'js'),
         glob(os.path.join('occiput','Visualization', 'DisplayNode', 'static', 'tipix', 'js', '*.*'))),
        (os.path.join('occiput','Visualization', 'DisplayNode', 'static', 'tipix', 'style'),
         glob(os.path.join('occiput', 'Visualization','DisplayNode', 'static', 'tipix', 'style', '*.*'))),
        (os.path.join('occiput', 'Visualization','DisplayNode', 'static', 'tipix', 'images'),
         glob(os.path.join('occiput', 'Visualization','DisplayNode', 'static', 'tipix', 'images', '*.*')))
    ],
    package_data={
        "occiput": [
            "Data/*.pdf",
            "Data/*.png",
            "Data/*.jpg",
            "Data/*.svg",
            "Data/*.nii",
            "Data/*.dcm",
            "Data/*.h5",
            "Data/*.txt",
            "Data/*.dat",
        ]
    },
    cmdclass={'build_ext': build_ext_fun},
    ext_modules=[petlink32_c_module, test_simplewrap_module, test_matrices_module],
    url="https://github.com/TomographyLab/Occiput/",
    license="LICENSE",
    description="Tomographic Vision - PET, SPECT, CT, MRI reconstruction and processing.",
    long_description=open("README.rst").read(),
    keywords=[
        "PET",
        "SPECT",
        "MRI",
        "computer vision",
        "artificial intelligence",
        "emission tomography",
        "transmission tomography",
        "tomographic reconstruction",
        "nuclear magnetic resonance",
    ],
    classifiers=[
        "Programming Language :: Python",
        "Development Status :: 4 - Beta",
        "Environment :: Other Environment",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: BSD License",
        "Operating System :: OS Independent",
        "Topic :: Scientific/Engineering :: Medical Science Apps.",
        "Topic :: Scientific/Engineering :: Mathematics",
        "Topic :: Scientific/Engineering :: Bio-Informatics",
    ],
    install_requires=[
        "numpy >= 1.12.0",
        "matplotlib >= 1.4.0",
        "ipy_table >= 1.11.0",
        "nibabel >= 2.0.0",
        "pydicom >= 0.9.0",
        "nipy >= 0.4.0",
        "jupyter >= 1.0.0",
        "h5py >= 2.3.0",
        "scipy >= 0.14.0",
        "pillow >= 2.8.0",
        "svgwrite >= 1.1.0",
    ],
    zip_safe=False
)
