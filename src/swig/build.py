import distutils
from distutils.core import setup, Extension

setup(name = "Radare",
      version = "0.1",
      ext_modules = [Extension("_radare", ["radare.i", "radare.c"])])
