cmake_minimum_required (VERSION 2.8)
project (cptnHookLite CXX)

# Check if this is an out-of-source build
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
   message(FATAL_ERROR "In source build is not supported. Please build in a different directory.")
endif()

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -O3")

macro(add_cptnHookLite_plugin name)
   set (srcs ${ARGN})

   include_directories(
      "${CMAKE_CURRENT_SOURCE_DIR}/inc")
   
   add_library( ${name} SHARED ${srcs} )

   set_target_properties( ${name} PROPERTIES
      LINKER_LANGUAGE CXX
      PREFIX "")

   if (USE_LIBUNWIND)
      set(LIBUNWINDNAME "unwind")
   endif()
      
   target_link_libraries(${name} "z" "dl" "m" ${LIBUNWINDNAME})
      
endmacro(add_cptnHookLite_plugin)

set (USER_LIBS
   z
)

add_cptnHookLite_plugin(libcptnHookLite
   src/cptnHookLite.cpp
)

macro (add_cptnHookLite_exe name)
   set (libs ${ARGN})
   add_executable (${name} test/${name}.cpp)
   target_link_libraries(${name} ${libs})   
endmacro (add_cptnHookLite_exe)

# The tests -----------------------------------------------------------

# The executables
file(COPY ${CMAKE_SOURCE_DIR}/python/cptnhook DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY ${CMAKE_SOURCE_DIR}/python/cptnhookAnalyze DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_cptnHookLite_exe (writerReader z)
add_cptnHookLite_exe (reader z)
add_cptnHookLite_exe (simpleProg)

include( CTest )
set (CTEST_PROJECT_NAME "cptnHookLite")

set(CTEST_SOURCE_DIRECTORY "$ENV{CMAKE_SOURCE_DIR}")
set(CTEST_BINARY_DIRECTORY "$ENV{CMAKE_BINARY_DIR}")

macro(add_cptnHookLite_generic_test TEST_NAME TEST_REF)

#    # By default use the name of the test
#    if(${TEST_NAME} MATCHES "^bwlist")
#      set(TEST_ENABLED_CHECKERS "sas.CodingConventions.ROOT")
#    elseif(${TEST_NAME} MATCHES "^commentDisabler")
#      set(TEST_ENABLED_CHECKERS "sas.Example.Varname:sas.CodingConventions.General.NoUsingNamespaceInHeaders:sas.ThreadSafety.ConstCast")
#    elseif(${TEST_NAME} MATCHES "^sas")
#      set(TEST_ENABLED_CHECKERS ${TEST_NAME})
#    else()
#      set(TEST_ENABLED_CHECKERS "")
#    endif()

   set(FULL_TEST_REF "${CMAKE_SOURCE_DIR}/test/${TEST_REF}")
   string(REPLACE ".cpp" ".ref" FULL_TEST_REF ${FULL_TEST_REF})
   add_test(NAME "${TEST_NAME}"
     COMMAND "bin/cptnhook --root -o myTest -- " 
     WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
 endmacro(add_cptnHookLite_generic_test)

 macro(add_cptnhook_test TEST_NAME)
   add_cptnHookLite_generic_test("${TEST_NAME}" "${TEST_NAME}.ref")
 endmacro(add_cptnhook_test)


enable_testing()

add_cptnhook_test("writerReader")
add_cptnhook_test("simpleProg")

