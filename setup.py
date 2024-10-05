"""
Setup file for the duino_bus module.
"""

from pathlib import Path
import sys
if sys.version_info < (3, 9):
    print('duino_bus requires Python 3.9 or newer.')
    sys.exit(1)

# pylint: disable=wrong-import-position
from setuptools import setup
from duino_bus.version import __version__

here = Path(__file__).parent
long_description = (here / "README.md").read_text()

setup(
        name='duino_bus',
        version=__version__,
        author='Dave Hylands',
        author_email='dhylands@gmail.com',
        description=('A bus abstraction for interfacing with serial like devices.'),
        license='MIT',
        keywords='cmd cli arduino',
        url='https://github.com/dhylands/duino_bus',
        download_url=f'https://github.com/dhylands/duino_bus/shell/tarball/v{__version__}',
        packages=['duino_bus',
                  'tests'],
        long_description=long_description,
        long_description_content_type='text/markdown',
        classifiers=[
                'Development Status :: 3 - Alpha',
                'Environment :: Console',
                'Intended Audience :: Developers',
                'License :: OSI Approved :: MIT License',
                'Natural Language :: English',
                'Operating System :: POSIX :: Linux',
                'Programming Language :: Python',
                'Programming Language :: Python :: 3',
                'Topic :: Software Development :: Embedded Systems',
                'Topic :: System :: Shells',
                'Topic :: Terminals :: Serial',
                'Topic :: Utilities',
        ],
        install_requires=[
                'pyserial',
                'crcmod',
        ],
)
