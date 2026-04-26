"""
Setup script for PyDNG Python bindings using scikit-build-core

This setup.py is provided for compatibility, but pyproject.toml is the primary configuration.
"""

from setuptools import find_packages, setup

version = "0.1.0"

try:
    with open("README.md", "r", encoding="utf-8") as f:
        long_description = f.read()
except Exception:
    long_description = "Python bindings for Adobe DNG SDK"

setup(
    name="dngpy",
    version=version,
    description="Python bindings for Adobe DNG SDK",
    long_description=long_description,
    long_description_content_type="text/markdown",
    python_requires=">=3.8",
    install_requires=[
        "numpy>=1.15.0",
    ],
    package_dir={"": "src"},
    packages=find_packages(where="src"),
    package_data={
        "dngpy": ["py.typed", "__init__.pyi", "_native.pyi"],
    },
)
