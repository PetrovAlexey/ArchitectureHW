cmake_minimum_required(VERSION 3.2)

project(hl_event_server C CXX)

SET (EXAMPLE_BINARY "simple_server")
SET (WRITER_BINARY  "event_writer")
SET (TESTS "gtest")

find_package(OpenSSL)
find_package(GTest REQUIRED)
find_package(Threads)
find_package(ZLIB)
find_package(CppKafka REQUIRED)
find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL)
find_package(Boost COMPONENTS filesystem system program_options regex REQUIRED)


if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

include_directories(${Poco_INCLUDE_DIRS})
include_directories(${GTEST_INCLUDE_DIRS})
include_directories("/usr/include/mysql")
include_directories("/usr/local/include/mysql")
include_directories(${Boost_INCLUDE_DIR})
link_directories("/usr/local/lib")

add_executable(${EXAMPLE_BINARY} main.cpp 
                                 config/config.cpp 
                                 database/database.cpp
        database/person.cpp)

add_executable(${WRITER_BINARY} writer_main.cpp
        config/config.cpp
        database/database.cpp
        database/person.cpp)

add_executable(${TESTS} test/unittest.cpp config/config.cpp database/database.cpp
        database/person.cpp)


target_include_directories(${EXAMPLE_BINARY} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${EXAMPLE_BINARY} PRIVATE -Wall -Wextra -pedantic -Werror )

target_include_directories(${WRITER_BINARY} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${WRITER_BINARY} PRIVATE -Wall -Wextra -pedantic -Werror )


target_link_libraries(${EXAMPLE_BINARY} PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
        ${Poco_LIBRARIES}
        "libPocoData.so"
        "libPocoDataMySQL.so"
        "libmysqlclient.so"
        "rdkafka++"
        CppKafka::cppkafka
        ZLIB::ZLIB
        ${Boost_LIBRARIES})

target_link_libraries(${WRITER_BINARY} PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
        ${Poco_LIBRARIES}
        "libPocoData.so"
        "libPocoDataMySQL.so"
        "libmysqlclient.so"
        "rdkafka++"
        CppKafka::cppkafka
        ZLIB::ZLIB
        ${Boost_LIBRARIES})


target_link_libraries(${TESTS} ${CMAKE_THREAD_LIBS_INIT}
        ${Poco_LIBRARIES}
        "PocoData"
        "PocoDataMySQL"
        "CppKafka::cppkafka"
        "mysqlclient" ${GTEST_LIBRARIES})


set_target_properties(${EXAMPLE_BINARY} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${EXAMPLE_BINARY} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

install(TARGETS ${EXAMPLE_BINARY} RUNTIME DESTINATION bin)

set_target_properties(${WRITER_BINARY} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${WRITER_BINARY} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

install(TARGETS ${WRITER_BINARY} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT petrov.as@phystech.edu)
include(CPack)

