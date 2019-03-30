import io
import os.path as op

from setuptools import setup
from subprocess import check_output, CalledProcessError

here = op.abspath(op.dirname(__file__))

# get the version from git
try:
    cmd = ['git', 'describe', '--dirty', '--always', '--tags']
    version = check_output(cmd, cwd=here).decode('utf-8')
except CalledProcessError:
    version = 'unknown'
# get the long description from the README file
with io.open(op.join(here, 'README.md'), mode='rt', encoding='utf-8') as f:
    long_description = f.read()

setup(
    name='peary',
    version=version,
    description='Peary client python library',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://gitlab.cern.ch/Caribou/peary',
    author='Moritz Kiehn',
    author_email='msmk@cern.ch',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Environment :: Console',
        'Environment :: Console :: Curses',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)',
        'Programming Language :: Python :: 3 :: Only',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Topic :: Scientific/Engineering',
    ],
    packages=['peary'],
    install_requires=[],
    python_requires='>=3.4',
    entry_points = {
        'console_scripts': [
            'pearyc = peary.shell:main',
        ],
    }
)
