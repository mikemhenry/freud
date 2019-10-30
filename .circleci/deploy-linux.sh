#!/bin/bash

set -e
set -u

# PYPI_USERNAME - (Required) Username for the publisher's account on PyPI
# PYPI_PASSWORD - (Required, Secret) Password for the publisher's account on PyPI

cat << EOF >> ~/.pypirc
[distutils]
index-servers=
    pypi
    testpypi

[pypi]
username: ${PYPI_USERNAME}
password: ${PYPI_PASSWORD}

[testpypi]
repository: https://test.pypi.org/legacy/
username: ${PYPI_USERNAME}
password: ${PYPI_PASSWORD}
EOF

if [ -z $1 ]; then
    echo "A repository (\"pypi\" or \"testpypi\") must be provided as the first argument."
    exit 1
fi

# Build TBB
git clone https://github.com/01org/tbb.git
cd tbb
make
BUILD_DIR=$(find build -name linux*release)
cd ${BUILD_DIR}
source tbbvars.sh
cd ~/

# Build wheels
for PYBIN in /opt/python/cp3*/bin; do
  # Split the echo command and the version since python2
  # --version doesn't return the value, just prints it.
  echo "Building for "
  ${PYBIN}/python --version

  "${PYBIN}/python" -m pip install cython --no-deps --ignore-installed -q --progress-bar=off
  rm -rf numpy-1.10.4
  curl -sSLO https://github.com/numpy/numpy/archive/v1.10.4.tar.gz
  tar -xzf v1.10.4.tar.gz
  cd numpy-1.10.4
  rm -f numpy/random/mtrand/mtrand.c
  rm -f PKG-INFO
  "${PYBIN}/python" -m pip install . --no-deps --ignore-installed -v --progress-bar=off -q

  # Force installation of version of SciPy (1.2) that works with
  # old NumPy (1.3 requires newer).
  "${PYBIN}/pip" install scipy==1.2.1 --progress-bar=off
  "${PYBIN}/pip" wheel ~/ci/freud/ -w ~/wheelhouse/ --no-deps --no-build-isolation --no-use-pep517
done

# Update RPath for wheels
for whl in ~/wheelhouse/freud*.whl; do
  auditwheel repair "$whl" -w ~/ci/freud/wheelhouse/
done

# Install from and test all wheels
for PYBIN in /opt/python/*/bin/; do
  "${PYBIN}/python" -m pip install freud_analysis --no-deps --no-index -f ~/ci/freud/wheelhouse
  "${PYBIN}/python" -m pip install rowan sympy
  cd ~/ci/freud/tests
  "${PYBIN}/python" -m unittest discover . -v
done

# Build source distribution using whichever Python appears last
cd ..
"${PYBIN}/python" setup.py sdist --dist-dir ~/ci/freud/wheelhouse/

"${PYBIN}/pip" install --user twine

"${PYBIN}/python" -m twine upload --skip-existing --repository $1 ~/ci/freud/wheelhouse/*