
add_executable(FFTTest FFTTest.cxx)


target_link_libraries(FFTTest   sodasignals sodautils::sodautils)
target_include_directories(FFTTest PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(FFTTest PRIVATE SODA_LIB_BUILD)

add_executable(FFTTiming FFTTiming.cxx)
target_link_libraries(FFTTiming sodasignals  sodautils)
target_include_directories(FFTTiming PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(FFTTiming PRIVATE SODA_LIB_BUILD)

add_executable(FilterTest FilterTest.cxx Checker.cxx)
target_link_libraries(FilterTest sodasignals  sodautils)
target_include_directories(FilterTest PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(FilterTest PRIVATE SODA_LIB_BUILD)

add_executable(OSFilterTest OSFilterTest.cxx Checker.cxx)
target_link_libraries(OSFilterTest sodasignals  sodautils)
target_include_directories(OSFilterTest PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(OSFilterTest PRIVATE SODA_LIB_BUILD)

add_executable(PeriodogramTest PeriodogramTest.cxx)
target_link_libraries(PeriodogramTest sodasignals  sodautils)
target_include_directories(PeriodogramTest PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(PeriodogramTest PRIVATE SODA_LIB_BUILD)

add_executable(ReSamplerTest ReSamplerTest.cxx Checker.cxx)
target_link_libraries(ReSamplerTest sodasignals  sodautils)
target_include_directories(ReSamplerTest PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(ReSamplerTest PRIVATE SODA_LIB_BUILD)

add_executable(ReSamplerSweep ReSamplerSweep.cxx)
target_link_libraries(ReSamplerSweep sodasignals  sodautils)
target_include_directories(ReSamplerSweep PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(ReSamplerSweep PRIVATE SODA_LIB_BUILD)

add_executable(ResampPerf ResampPerf.cxx)
target_link_libraries(ResampPerf sodasignals sodautils)
target_include_directories(ResampPerf PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(ResampPerf PRIVATE SODA_LIB_BUILD)

add_executable(PolyphaseResampTest PolyphaseResampTest.cxx Checker.cxx)
target_link_libraries(PolyphaseResampTest sodasignals sodautils)
target_include_directories(PolyphaseResampTest PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(PolyphaseResampTest PRIVATE SODA_LIB_BUILD)

add_executable(ReSamplerBufTest ReSamplerBufTest.cxx Checker.cxx)
target_link_libraries(ReSamplerBufTest sodasignals sodautils)
target_include_directories(ReSamplerBufTest PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(ReSamplerBufTest PRIVATE SODA_LIB_BUILD)

enable_testing()

add_test(NAME PeriodogramTest_1
  COMMAND $<TARGET_FILE:PeriodogramTest> --fsamp 48e3 -ftest 8e3 --psize 4096)
set_tests_properties(PeriodogramTest_1 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")


add_test(NAME ReSamplerTest_1
  COMMAND $<TARGET_FILE:ReSamplerTest> --fsin 48e3 --fsout 8e3)
set_tests_properties(ReSamplerTest_1 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerTest_2
  COMMAND $<TARGET_FILE:ReSamplerTest> --fsin 625e3 --fsout 48e3)
set_tests_properties(ReSamplerTest_2 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerTest_3
  COMMAND $<TARGET_FILE:ReSamplerTest> --fsin 120e3 --fsout 48e3)
set_tests_properties(ReSamplerTest_3 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerTest_4
  COMMAND $<TARGET_FILE:ReSamplerTest> --fsin 1.2e6 --fsout 48e3)
set_tests_properties(ReSamplerTest_4 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerTest_5
  COMMAND $<TARGET_FILE:ReSamplerTest> --fsin 1.25e6 --fsout 48e3)
set_tests_properties(ReSamplerTest_5 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerTest_6
  COMMAND $<TARGET_FILE:ReSamplerTest> --fsin 1.25e6 --fsout 44.1e3)
set_tests_properties(ReSamplerTest_6 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")


add_test(NAME ReSamplerTest_I1
  COMMAND $<TARGET_FILE:ReSamplerTest> --fsout 48e3 --fsin 8e3)
set_tests_properties(ReSamplerTest_I1 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerTest_I2
  COMMAND $<TARGET_FILE:ReSamplerTest> --fsout 625e3 --fsin 48e3)
set_tests_properties(ReSamplerTest_I2 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerTest_I3
  COMMAND $<TARGET_FILE:ReSamplerTest> --fsout 120e3 --fsin 48e3)
set_tests_properties(ReSamplerTest_I3 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerTest_I4
  COMMAND $<TARGET_FILE:ReSamplerTest> --fsout 1.2e6 --fsin 48e3)
set_tests_properties(ReSamplerTest_I4 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerTest_I5
  COMMAND $<TARGET_FILE:ReSamplerTest> --fsout 1.25e6 --fsin 48e3)
set_tests_properties(ReSamplerTest_I5 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_1
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsin 48e3 --fsout 8e3)
set_tests_properties(ReSamplerBufTest_1 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_2
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsin 625e3 --fsout 48e3)
set_tests_properties(ReSamplerBufTest_2 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_3
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsin 120e3 --fsout 48e3)
set_tests_properties(ReSamplerBufTest_3 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_4
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsin 1.2e6 --fsout 48e3)
set_tests_properties(ReSamplerBufTest_4 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_5
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsin 1.25e6 --fsout 48e3)
set_tests_properties(ReSamplerBufTest_5 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_6
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsin 2.5e6 --fsout 48e3)
set_tests_properties(ReSamplerBufTest_6 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_I1
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsout 48e3 --fsin 8e3)
set_tests_properties(ReSamplerBufTest_I1 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_I2
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsout 625e3 --fsin 48e3)
set_tests_properties(ReSamplerBufTest_I2 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_I3
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsout 120e3 --fsin 48e3)
set_tests_properties(ReSamplerBufTest_I3 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_I4
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsout 1.2e6 --fsin 48e3)
set_tests_properties(ReSamplerBufTest_I4 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_I5
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsout 1.25e6 --fsin 48e3)
set_tests_properties(ReSamplerBufTest_I5 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_exact_down
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsin 625e3 --fsout 48e3 --bufsize 30000 --expectedout 2304)
set_tests_properties(ReSamplerBufTest_exact_down PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME ReSamplerBufTest_exact_up
  COMMAND $<TARGET_FILE:ReSamplerBufTest> --fsin 48e3 --fsout 625e3 --bufsize 2304 --expectedout 30000)
set_tests_properties(ReSamplerBufTest_exact_up PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

if(ABSOLUTELY_NUTS)
add_test(NAME ReSamplerTest_I6
  COMMAND $<TARGET_FILE:ReSamplerTest> --fsout 125e6 --fsin 44.1e3)
set_tests_properties(ReSamplerTest_I6 PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")
endif()


add_test(NAME OSFilterTest
  COMMAND $<TARGET_FILE:OSFilterTest>)
set_tests_properties(OSFilterTest PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME FilterTest
  COMMAND $<TARGET_FILE:FilterTest>)
set_tests_properties(FilterTest PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")


add_test(NAME PolyphaseResampTest_8k_48k
  COMMAND $<TARGET_FILE:PolyphaseResampTest> --fsin 8e3 --fsout 48e3)
set_tests_properties(PolyphaseResampTest_8k_48k PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME PolyphaseResampTest_48k_625k
  COMMAND $<TARGET_FILE:PolyphaseResampTest> --fsin 48e3 --fsout 625e3)
set_tests_properties(PolyphaseResampTest_48k_625k PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME PolyphaseResampTest_48k_1200k
  COMMAND $<TARGET_FILE:PolyphaseResampTest> --fsin 48e3 --fsout 1.2e6)
set_tests_properties(PolyphaseResampTest_48k_1200k PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME PolyphaseResampTest_625k_48k
  COMMAND $<TARGET_FILE:PolyphaseResampTest> --fsin 625e3 --fsout 48e3)
set_tests_properties(PolyphaseResampTest_625k_48k PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")

add_test(NAME PolyphaseResampTest_48k_8k
  COMMAND $<TARGET_FILE:PolyphaseResampTest> --fsin 48e3 --fsout 8e3)
set_tests_properties(PolyphaseResampTest_48k_8k PROPERTIES
  PASS_REGULAR_EXPRESSION "PASSED")


############################################################################
#
# BSD 2-Clause License
# 
# Copyright (c) 2025, kb1vc
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

