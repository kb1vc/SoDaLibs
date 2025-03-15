#!/bin/bash

# For build integrity tests.
# Here's what we test:
# 
# 1. Local build with no yaml installed
#    Build Fedora_no_yaml.sif from Fedora_no_yaml.def
#    run build_kit.sh LOCAL
#    run test_kit LOCAL
# 2. Local build with yaml
#    Build Fedora_noyaml.sif
#    run build_kit.sh LOCAL
#    run test_kit.sh LOCAL
# 3. Install build with no yaml installed  
#    Fedora_noyaml_install.sif with post /mnt/build_kit.sh INSTALL
#    run test_kit.sh INSTALL
# 4. Install build with yaml
#    Fedora_yaml_install.sif with post /mnt/build_kit.sh INSTALL
#    run test_kit.sh INSTALL
# 5. Build with install from RPM
#    Fedora_rpm_install.sif  
#    run test_kit.sh INSTALL
#
# apptainer pull docker://fedora:37

# first build the "common container"
sudo rm -rf Fedora_CommonTools.sif
sudo apptainer build Fedora_CommonTools.sif Fedora_CommonTools.def

#  Recipes all build on Fedora_generic.def

# test 1
# 1. Local build with no yaml installed
#    Build Fedora_no_yaml.sif from Fedora_no_yaml.def
#    run build_kit.sh LOCAL
#    run test_kit LOCAL
echo "\n\nBUILD 1"
mkdir test_1
cp Fedora_generic.def test_1/Fedora_noyaml.def
# First items 1, 3
sudo apptainer build --bind `pwd`/test_1:/mnt test_1/Fedora_noyaml.sif test_1/Fedora_noyaml.def
echo "\n\n\n"
apptainer exec --bind `pwd`/test_1:/mnt --bind `pwd`/../:/source_tree  test_1/Fedora_noyaml.sif `pwd`/build_kit.sh LOCAL
echo "\n\n\n"
apptainer exec --bind `pwd`/test_1:/mnt --bind `pwd`/../:/source_tree  test_1/Fedora_noyaml.sif `pwd`/test_kit.sh LOCAL


# 2. Local build with yaml
#    Build Fedora_yaml.sif
#    run build_kit.sh LOCAL
#    run test_kit.sh LOCAL
echo "\n\nBUILD 2"
mkdir test_2
cat Fedora_generic.def | sed -e 's/#STEP1/dnf install --assumeyes yaml-cpp-devel/' > test_2/Fedora_yaml.def
sudo apptainer build --bind `pwd`/test_2:/mnt test_2/Fedora_yaml.sif test_2/Fedora_yaml.def
echo "\n\n\n"
apptainer exec --bind `pwd`/test_2:/mnt --bind `pwd`/../:/source_tree  test_2/Fedora_yaml.sif `pwd`/build_kit.sh LOCAL
echo "\n\n\n"
apptainer exec --bind `pwd`/test_2:/mnt --bind `pwd`/../:/source_tree  test_2/Fedora_yaml.sif `pwd`/test_kit.sh LOCAL TESTYAML

# 3. Install build with no yaml installed  
#    Fedora_noyaml_install.sif with post /mnt/build_kit.sh INSTALL
#    run test_kit.sh INSTALL
echo "BUILD3"
mkdir test_3
cat Fedora_generic.def | sed -e 's|#STEP1|/mnt/script/build_kit.sh INSTALL|' > test_3/Fedora_noyaml.def
# First items 1, 3
sudo apptainer build --bind `pwd`/test_3:/mnt --bind `pwd`/../:/mnt/source_tree --bind `pwd`:/mnt/script test_3/Fedora_noyaml.sif test_3/Fedora_noyaml.def
echo "\n\n\n"
echo "\n\n\n"
apptainer exec --bind `pwd`/test_3:/mnt test_3/Fedora_noyaml.sif `pwd`/test_kit.sh INSTALL

# test 4 installed build with yaml
# 4. Install build with yaml
#    Fedora_yaml_install.sif with post /mnt/build_kit.sh INSTALL
#    run build_rpm.sh
#    run test_kit.sh INSTALL
echo "BUILD4"
mkdir test_4
cat Fedora_generic.def | sed -e 's/#STEP1/dnf install --assumeyes yaml-cpp-devel/' | sed -e 's|#STEP2|/mnt/script/build_kit.sh INSTALL BUILD_KIT|' > test_4/Fedora_noyaml.def
# First items 1, 3
sudo apptainer build --bind `pwd`/test_4:/mnt --bind `pwd`/../:/mnt/source_tree --bind `pwd`:/mnt/script test_4/Fedora_noyaml.sif test_4/Fedora_noyaml.def
echo "\n\n\n"
echo "\n\n\n"
apptainer exec --bind `pwd`/test_4:/mnt test_4/Fedora_noyaml.sif `pwd`/test_kit.sh INSTALL


# 5. Build with install from RPM
#    Fedora_rpm_install.sif  
#    run test_kit.sh INSTALL
mkdir test_5
cp test_4/tool_tree/build/SoDaUtils*.rpm test_5/SoDaUtils_kit.rpm
cp test_4/tool_tree/build/SoDaUtils*.rpm ./
cat Fedora_generic.def | sed -e 's/#STEP1/dnf install --assumeyes yaml-cpp-devel/' | sed -e 's|#STEP2|dnf install --assumeyes /mnt/SoDaUtils_kit.rpm|' > test_5/Fedora_rpm.def
sudo apptainer build --bind `pwd`/test_5:/mnt test_5/Fedora_rpm.sif test_5/Fedora_rpm.def
echo "\n\n\n"
echo "\n\n\n"
apptainer exec --bind `pwd`/test_5:/mnt --bind `pwd`/../:/source_tree  test_5/Fedora_rpm.sif `pwd`/test_kit.sh INSTALL TESTYAML
