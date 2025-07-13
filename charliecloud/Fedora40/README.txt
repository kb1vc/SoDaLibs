The stuff here does a few things:

All steps are in separate scripts

0. Builds a base Fedora40 container that we'll use for the rest of the tests

in build_base.sh - it uses DockerBase to create fedora40_sodabase. 

1. Tests the build of the library with jsoncpp-devel already installed.

in build_w_jsoncpp.sh

2. Tests the build of the library without jsoncpp-devel already installed.

in build_wo_jsoncpp.sh

3. Builds an RPM.

in build_rpm.sh

4. Tests the RPM build.

in test_rpm.sh

ch-image build --rebuild --bind `pwd`/:/mnt/1 --bind `pwd`/../common_build_scripts:/mnt/0 -t fedora40_sodalibs_rpm_test -f DockerTestRPM
