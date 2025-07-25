# SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

"""
SensCord SDK setup script.

The installation method is as follows.

# python setup.py install
"""
import sys

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup


_VERSION = '1.11.1'


setup(
    name="senscord",
    packages=["senscord", "senscord.external"],
    version=_VERSION,
    description="SensCord SDK",
    install_requires=[
        "msgpack>=1.0,<1.1" if sys.version_info >= (3,) else "msgpack==0.6.2",
    ])
