from setuptools import setup, Extension

cjson_ext = Extension("cjson", sources=["cjson.c"], extra_compile_args=["-O3"])

setup(name="cjson", version="0.1.0", ext_modules=[cjson_ext], )