FROM fedora:40

RUN dnf --assumeyes update
RUN dnf --assumeyes install gcc-c++ git cmake make
RUN dnf --assumeyes install fftw-devel

RUN mkdir /soda_testbuild
WORKDIR /soda_testbuild
RUN /mnt/0/get_sodalibs.sh

WORKDIR /soda_testbuild/SoDaLibs
RUN mkdir build
WORKDIR /soda_testbuild/SoDaLibs/build
RUN /mnt/0/build_sodalibs.sh
RUN mkdir -p /soda_testbuild/example_test/SigExamps
RUN mkdir -p /soda_testbuild/example_test/UtilExamps
WORKDIR /soda_testbuild/example_test
RUN cp /usr/local/share/SoDaSignals/examples/* ./SigExamps/build
RUN cp /usr/local/share/SoDaUtils/examples/* ./UtilExamps/build
RUN /mnt/0/build_utils_examps.sh
WORKDIR  /soda_testbuild/example_test/SigExamps/build
RUN /mnt/0/build_signals_examps.sh

