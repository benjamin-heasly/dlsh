# This runs in an Ubuntu environment.
FROM ubuntu:22.04

# Set up a dev environment.
RUN export DEBIAN_FRONTEND=noninteractive \
    && apt update \
    && apt install --no-install-recommends --yes git wget ca-certificates build-essential cmake zlib1g-dev liblz4-dev \
    && apt-get clean \
    && apt-get autoremove \
    && rm -rf /var/lib/apt/lists/*

# Install TCL 9 to the build environment.
WORKDIR /work
RUN git clone --depth=1 --branch core-9-0-1-rc https://github.com/tcltk/tcl.git \
  && cd tcl/unix/ \
  && ./configure \
  && make \
  && make install

# Install the Jansson JSON dependency.
WORKDIR /work
RUN git clone https://github.com/akheron/jansson.git \
  && cd jansson/ \
  && git checkout v2.14 \
  && mkdir build \
  && cd build/ \
  && cmake .. \
  && make \
  && make install

# Install the libharu PDF dependency -- both static and dynamic builds.
WORKDIR /work
RUN git clone https://github.com/libharu/libharu.git \
  && cd libharu \
  && git checkout v2.4.4 \
  && cmake -DBUILD_SHARED_LIBS=OFF -B build \
  && cmake --build build \
  && cmake --install build \
  && cmake -DBUILD_SHARED_LIBS=ON -B build \
  && cmake --build build \
  && cmake --install build

ADD . /work/dlsh/
WORKDIR /work/dlsh/
RUN cmake -DCMAKE_INSTALL_PREFIX=/work/package/ -B build \
  && cmake --build build \
  && cmake --install build


# docker build . -t dlsh:local

# docker run --rm -ti dlsh:local ls -alth /work/package/lib
# docker run --rm -ti dlsh:local ls -alth /work/package/include