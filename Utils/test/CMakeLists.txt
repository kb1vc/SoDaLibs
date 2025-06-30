
########### next target ###############

set(TST_FMT_SRCS
	FormatTest.cxx
	../src/Format.cxx
	)



add_executable(FormatTest ${TST_FMT_SRCS})

add_executable(FormatNaN FormatNaN.cxx ../src/Format.cxx)

add_executable(SplitTest SplitTest.cxx)
target_link_libraries(SplitTest sodautils)

add_executable(FormatHexTest  FormatHexTest.cxx)
target_link_libraries(FormatHexTest sodautils)

add_executable(MailBoxTest MailBoxTest.cxx)
target_link_libraries(MailBoxTest sodautils Threads::Threads)

add_executable(BarrierTest BarrierTest.cxx)
target_link_libraries(BarrierTest sodautils Threads::Threads)


add_executable(OptionsTest OptionsTest.cxx)
target_link_libraries(OptionsTest sodautils)

add_executable(PropertyTree_Test PropertyTree_Test.cxx)
target_link_libraries(PropertyTree_Test sodautils jsoncpp)


enable_testing()

add_test(NAME FastBarrierTest 
  COMMAND $<TARGET_FILE:BarrierTest> --maddur 35 --threads 20 --trials 100000)
set_tests_properties(FastBarrierTest PROPERTIES
  FAIL_REGULAR_EXPRESSION "Ooops, dang!"
  )

add_test(FastBarrierTest_should_timeout BarrierTest --maddur 1001000 --threads 10 --trials 100)
set_tests_properties(FastBarrierTest_should_timeout PROPERTIES
  PASS_REGULAR_EXPRESSION "Ooops, dang!"
  )


add_test(NAME MailBoxTest1
  COMMAND $<TARGET_FILE:MailBoxTest> -m 500 -t 20 -r 100)
set_tests_properties(MailBoxTest1 PROPERTIES
  FAIL_REGULAR_EXPRESSION "subscriber")

add_test(NAME MailBoxTest2
  COMMAND $<TARGET_FILE:MailBoxTest> -m 500 -t 20 -r 100 --noecho)
set_tests_properties(MailBoxTest2 PROPERTIES
  FAIL_REGULAR_EXPRESSION "subscriber")


add_test(NAME FastFormatTest 
  COMMAND $<TARGET_FILE:FormatTest>)

add_test(NAME FormatHexTest 
  COMMAND $<TARGET_FILE:FormatHexTest>)


add_test(NAME FormatNaN 
  COMMAND $<TARGET_FILE:FormatNaN>)

add_test(NAME OptionsTestSimple 
  COMMAND $<TARGET_FILE:OptionsTest>)

add_test(NAME PropertyTreeTest1
  COMMAND $<TARGET_FILE:PropertyTree_Test> ${CMAKE_CURRENT_LIST_DIR}/test1.yaml)



add_test(NAME SplitTest
  COMMAND $<TARGET_FILE:SplitTest> -m 500 -t 20 -r 100)
set_tests_properties(SplitTest PROPERTIES
  FAIL_REGULAR_EXPRESSION "FAIL")

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

