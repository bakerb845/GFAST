#!/bin/sh

# USE_INTEL=true
USE_INTEL=false

# USE_AMQ=true
USE_AMQ=false

# USE_KAFKA=true
USE_KAFKA=false

# Build in build/ dir to preserve standalone make system
if [ ! -d build ]; then
   mkdir build
fi
cd build

if [ -f Makefile ]; then
   make clean
fi
if [ -f CMakeCache.txt ]; then
   echo "Removing CMakeCache.txt"
   rm CMakeCache.txt
fi
if [ -d CMakeFiles ]; then
   echo "Removing CMakeFiles"
   rm -rf CMakeFiles
fi

# This is a dir where I've put packages like:
#    iniparser, compearth, ISCL, etc.
# PKG_DIR=/home/mth/mth/zpkg
PKG_DIR=/home/ulbergc/WORK/GFAST_third_party

#EW_DIR=/opt/earthworm/earthworm_svn
# EW_DIR=/opt/earthworm/earthworm_7.10
# EW_DIR=/app/eewdata/ew/earthworm_7.9
EW_DIR=/home/ulbergc/WORK/GFAST_third_party/ew_v7.10

if $USE_INTEL; then
  MKL_INC=/opt/intel/mkl/include
  MKL_LIB_ROOT=/opt/intel/mkl/lib/intel64
  IPP_LIB_ROOT=/opt/intel/ipp/lib/intel64
  MKL_LIBS="${MKL_LIB_ROOT}/libmkl_intel_lp64.so;${MKL_LIB_ROOT}/libmkl_core.so;${MKL_LIB_ROOT}/libmkl_sequential.so"
  IPP_LIBS="${IPP_LIB_ROOT}/libipps.so;${IPP_LIB_ROOT}/libippvm.so;${IPP_LIB_ROOT}/libippcore.so"

  LINEAR="-DGFAST_USE_INTEL=TRUE \
          -DMKL_LIBRARY=${MKL_LIBS} \
          -DMKL_INCLUDE_DIR=${MKL_INC} \
          -DIPP_LIBRARY=${IPP_LIBS}
         "
else
  LINEAR="-DGFAST_USE_INTEL=FALSE \
          -DLAPACKE_INCLUDE_DIR=/usr/include/lapacke \
          -DLAPACKE_LIBRARY=/lib64/liblapacke.so \
          -DLAPACK_LIBRARY=/lib64/liblapack.so \
          -DBLAS_LIBRARY=/lib64/libblas.so \
          -DCBLAS_INCLUDE_DIR=/usr/include \
          -DCBLAS_LIBRARY=/lib64/atlas/libsatlas.so \
         "
fi

if $USE_AMQ; then
  ACTIVEMQ="-DGFAST_USE_AMQ=TRUE \
            -DLIBAMQ_INCLUDE_DIR=/usr/include/activemq-cpp-3.9.3 \
            -DLIBAMQ_LIBRARY=/usr/lib64/libactivemq-cpp.so \
            -DLSSL_LIBRARY=/usr/lib64/libssl.so.10 \
            -DLCRYPTO_LIBRARY=/usr/lib64/libcrypto.so.10 \
            -DAPR_INCLUDE_DIR=/usr/include/apr-1 \
           "
else
  ACTIVEMQ="-DGFAST_USE_AMQ=FALSE"
fi

if $USE_KAFKA; then
  KAFKA="-DGFAST_USE_KAFKA=TRUE \
         -DKAFKA_INCLUDE_DIR=/usr/include/librdkafka \
         -DKAFKA_LIBRARY=$LIB_ROOT/librdkafka.so 
        "
else
  KAFKA="-DGFAST_USE_KAFKA=FALSE"
fi

cmake ../ $LINEAR -DCMAKE_BUILD_TYPE=DEBUG \
  -DCMAKE_INSTALL_PREFIX=./ \
  -DCMAKE_C_FLAGS="-g3 -O2 -Wno-reserved-id-macro -Wno-padded -Wno-unknown-pragmas -fopenmp" \
  -DCMAKE_CXX_FLAGS="-g3 -O2 -fopenmp" \
  -DGFAST_INSTANCE="PNSN" \
  -DBUILDER="Builder" \
  $ACTIVEMQ \
  $KAFKA \
  -DGFAST_USE_EW=TRUE \
  -DUW_AMAZON=FALSE \
  -DH5_C_INCLUDE_DIR=/usr/include \
  -DH5_LIBRARY=/usr/lib64/libhdf5_cpp.so \
  -DINIPARSER_INCLUDE_DIR=${PKG_DIR}/iniparser/src \
  -DINIPARSER_LIBRARY=${PKG_DIR}/iniparser/libiniparser.a \
  -DCOMPEARTH_INCLUDE_DIR=${PKG_DIR}/mtbeach/c_src/include \
  -DCOMPEARTH_LIBRARY=${PKG_DIR}/mtbeach/c_src/lib/libcompearth_shared.so \
  -DISCL_INCLUDE_DIR=${PKG_DIR}/iscl/include \
  -DISCL_LIBRARY=${PKG_DIR}/iscl/lib/libiscl_shared.so \
  -DEW_BUILD_FLAGS="-Dlinux -D_LINUX -D_INTEL -D_USE_SCHED -D_USE_PTHREADS" \
  -DEW_INCLUDE_DIR=${EW_DIR}/include \
  -DEW_LIBRARY="${EW_DIR}/lib/libew_mt.a;${EW_DIR}/lib/libew_util.a" \
  -DLIBXML2_LIBRARY=/usr/lib64/libxml2.so.2 \
  -DLIBXML2_INCLUDE_DIR=/usr/include/libxml2 \
  -DZLIB_INCLUDE_DIR:PATH=/usr/include \
  -DZLIB_LIBRARY=/usr/lib64/libz.so
