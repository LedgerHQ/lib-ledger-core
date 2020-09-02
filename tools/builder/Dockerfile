ARG BUILD_TYPE=Debug
ARG LIB_CORE_SRC_DIR=/libcore/source
ARG LIB_CORE_BUILD_DIR=/libcore/build
ARG LIB_CORE_JAR_BUILD_DIR=/build-jar
ARG LIBCORE_AWS_BASE_URL=https://s3-eu-west-1.amazonaws.com/ledger-lib-ledger-core

# Compile
# (generates libledger-core.so)

FROM debian:stretch AS builder

ARG CMAKE_VERSION=3.15.2

ARG BUILD_TYPE
ARG DL_LIBCORE
ARG LIB_CORE_SRC_DIR
ARG LIB_CORE_BUILD_DIR
ARG LIBCORE_AWS_BASE_URL
ARG LIBCORE_VERSION

RUN apt-get update -qqy \
    && apt-get install -qqy \
        build-essential \
        apt-transport-https \
        curl \
        git \
        ssh \
        libssl-dev \
        libx11-xcb-dev \
        qt5-default \
        libqt5websockets5 \
        libqt5websockets5-dev \
        postgresql-9.6 \
        libpq-dev \
        postgresql-server-dev-all \
        sqlite3 \
        libsqlite3-dev \
        # JDK is required as JNI is enabled:
        openjdk-8-jdk \
    && rm -rf /var/lib/apt/lists/*

RUN if [ "${DL_LIBCORE}" = "ON" ]; then exit 0; fi && \
    curl -LJO https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.tar.gz \
    && tar -zxf cmake-${CMAKE_VERSION}.tar.gz \
    && cd cmake-${CMAKE_VERSION} \
    && ./bootstrap \
    && make -j$(nproc) \
    && make install

ENV BUILD_TYPE=$BUILD_TYPE

COPY . ${LIB_CORE_SRC_DIR}

# Init submodules
# RUN cd ${LIB_CORE_SRC_DIR} \
#     # Use https for git to avoid permissions issues
#     && git config --global url."https://github.com/".insteadOf git@github.com: \
#     && git config --global url."https://".insteadOf git:// \
#     && git submodule update --init --recursive --jobs=$(nproc) \
#     && ( ( cd core/lib/leveldb && git checkout bitcoin-fork ) || exit 0 )

WORKDIR ${LIB_CORE_BUILD_DIR}


# Option 1: Compile
RUN if [ "$DL_LIBCORE" = "ON" ]; then exit 0; fi \
    && ${LIB_CORE_SRC_DIR}/tools/builder/build.sh --jni --postgres \
    && mv core/src/libledger-core.so libledger-core.so

# Option 2: Download
RUN if [ "${DL_LIBCORE}" = "ON" ]; then \
        curl -k "${LIBCORE_AWS_BASE_URL}/${LIBCORE_VERSION}/linux/jni/libledger-core_jni.so" \
             --max-time 600 \
             --output libledger-core.so \
    ; fi

# Safeguard to ensure that libledger-core.so exists at this stage
RUN stat ${LIB_CORE_BUILD_DIR}/libledger-core.so


# Encapsulate compiled library in jar file
# (generates ledger-lib-core.jar)

FROM debian:stretch AS jar_generator

ARG LIB_CORE_SRC_DIR
ARG LIB_CORE_BUILD_DIR
ARG LIB_CORE_JAR_BUILD_DIR

ARG JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
ARG SBT_DEB=sbt-1.3.10.deb

RUN mkdir -p ${LIB_CORE_BUILD_DIR} ${LIB_CORE_JAR_BUILD_DIR}

RUN apt-get update -qqy \
    && apt-get install -qqy apt-transport-https curl openjdk-8-jdk \
    && rm -rf /var/lib/apt/lists/* 

RUN curl -L -o ${SBT_DEB} https://dl.bintray.com/sbt/debian/${SBT_DEB} \
    && dpkg -i ${SBT_DEB} \
    && rm ${SBT_DEB}

# Generate interfaces
WORKDIR ${LIB_CORE_SRC_DIR}

COPY --from=builder ${LIB_CORE_SRC_DIR} .

RUN chmod +x tools/generate_interfaces.sh \
    && ./tools/generate_interfaces.sh

# Package
WORKDIR ${LIB_CORE_JAR_BUILD_DIR}

RUN cp -v ${LIB_CORE_SRC_DIR}/api/core/java/* . \
    && cp -v ${LIB_CORE_SRC_DIR}/api/core/scala/* .

COPY --from=builder ${LIB_CORE_BUILD_DIR}/libledger-core.so \
                    src/main/resources/resources/djinni_native_libs/libledger-core.so

RUN sbt package

# Move the jar file to build directory and rename it
RUN find . -name \*.jar -exec mv {} ${LIB_CORE_BUILD_DIR}/ledger-lib-core.jar \;


# Make libledger-core.so and ledger-lib-core.jar available 
# to host or to another Docker image

FROM scratch

ENTRYPOINT [ "cp" , "-rf" , ".", "/build" ]

ARG LIB_CORE_BUILD_DIR

WORKDIR ${LIB_CORE_BUILD_DIR}

COPY --from=builder ${LIB_CORE_BUILD_DIR}/libledger-core.so .
COPY --from=jar_generator ${LIB_CORE_BUILD_DIR}/ledger-lib-core.jar .

RUN mkdir /build
