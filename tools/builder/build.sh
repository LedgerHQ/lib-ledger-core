#!/bin/bash -e

# Defaults
JNI=false
POSTGRES=false

# Option strings
SHORT=jp
LONG=jni,postgres

# read the options
OPTS=$(getopt --options $SHORT --long $LONG --name "$0" -- "$@")

if [ $? != 0 ] ; then echo "Failed to parse options...exiting." >&2 ; exit 1 ; fi

eval set -- "$OPTS"

while true; do
  case $1 in
    --jni|-j)
      JNI=true
      shift
      ;; 
    --postgres|-p)
      POSTGRES=true
      shift
      ;;
    --)              # End of all options.
      shift
      break
      ;;
    -?*)
        printf 'WARN: Unknown option (ignored): %s\n' "$1" >&2
        exit -1
        ;;
    *)               # Default case: No more options, so break out of the loop.
        break
  esac

  shift
done

CMAKE_PARAMS="\
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DSYS_OPENSSL=ON \
  -DOPENSSL_SSL_LIBRARIES=/usr/lib/x86_64-linux-gnu \
  -DOPENSSL_INCLUDE_DIR=/usr/include/openssl \
  -DCMAKE_INSTALL_PREFIX=/usr/share/qt5"

if [ "$JNI" = true ] ; then
  # echo 'Enable JNI build'
  CMAKE_PARAMS="${CMAKE_PARAMS} -DTARGET_JNI=ON"
fi

if [ "$POSTGRES" = true ] ; then
  # echo 'Enable JNI build'
  CMAKE_PARAMS="${CMAKE_PARAMS} -DPG_SUPPORT=ON -DPostgreSQL_INCLUDE_DIR=/usr/include/postgresql"
fi
            
if [ "${BUILD_TYPE}" = "Release" ] ; then 
  # Case 1: Release flag
  CMAKE_PARAMS="${CMAKE_PARAMS} -DBUILD_TESTS=OFF"
elif [ "${BUILD_TYPE}" = "Debug" ] ; then
  # Case 2: Debug flags
  CMAKE_PARAMS="${CMAKE_PARAMS} -DBUILD_TESTS=ON"
else
  # Case 3: Unknown build type
  echo "Error: unknown build type '${BUILD_TYPE}' \
        (expected: 'Release' or 'Debug' (case sensitive))"
  exit 1
fi

echo "cmake parameters (${BUILD_TYPE}): ${CMAKE_PARAMS}"
cmake ${CMAKE_PARAMS} ${LIB_CORE_SRC_DIR}
# Avoid "clock skew detected" warning:
touch *
make -j$(nproc)
