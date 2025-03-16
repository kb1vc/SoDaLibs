
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



