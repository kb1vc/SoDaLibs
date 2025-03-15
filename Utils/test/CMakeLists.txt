
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

if(yaml-cpp_FOUND)
  add_executable(PropertyTreeYAML_Test PropertyTreeYAML_Test.cxx)
  target_link_libraries(PropertyTreeYAML_Test sodautils sodaprops)
endif()

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

if(yaml-cpp_FOUND) 
  add_test(NAME PropertyTreeYAMLTest1
    COMMAND $<TARGET_FILE:PropertyTreeYAML_Test> ${CMAKE_CURRENT_LIST_DIR}/test1.yaml)
endif()


add_test(NAME SplitTest
  COMMAND $<TARGET_FILE:SplitTest> -m 500 -t 20 -r 100)
set_tests_properties(SplitTest PROPERTIES
  FAIL_REGULAR_EXPRESSION "FAIL")

