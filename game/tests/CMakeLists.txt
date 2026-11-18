cmake_minimum_required(VERSION 3.28)

option(BLACKJACK_COVERAGE "Instrumentare pentru code coverage (GCC/Clang)" OFF)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)

CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v1.17.0
    VERSION 1.17.0
)

include(GoogleTest)

get_target_property(BACKEND_SOURCE_DIR Backend SOURCE_DIR)
get_target_property(BACKEND_SOURCES Backend SOURCES)

set(BACKEND_CPP_FILES "")
if(BACKEND_SOURCES)
    foreach(source IN LISTS BACKEND_SOURCES)
        if(source MATCHES "\\.cpp$" AND NOT source MATCHES "^\\$<")
            if(NOT IS_ABSOLUTE "${source}")
                set(source "${BACKEND_SOURCE_DIR}/${source}")
            endif()
            list(APPEND BACKEND_CPP_FILES "${source}")
        endif()
    endforeach()
endif()

if(NOT BACKEND_CPP_FILES)
    file(GLOB_RECURSE BACKEND_CPP_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Backend/*.cpp")
endif()

set(TEST_SOURCES
    unit/hand_score_tests.cpp
    unit/player_tests.cpp
    unit/dealer_tests.cpp
    unit/deck_tests.cpp
    unit/game_tests.cpp
    unit/listener_list_tests.cpp
    unit/hosted_game_tests.cpp
    unit/protocol_tests.cpp
    unit/snapshot_tests.cpp
    unit/game_events_tests.cpp
    unit/remote_game_tests.cpp
    integration/network_tests.cpp
)

add_executable(BackendTests ${TEST_SOURCES} ${BACKEND_CPP_FILES})

target_compile_features(BackendTests PRIVATE cxx_std_23)

target_include_directories(BackendTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    $<TARGET_PROPERTY:Backend,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Backend,INTERFACE_INCLUDE_DIRECTORIES>
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Backend
    ${CMAKE_SOURCE_DIR}/Backend/include
)

target_compile_definitions(BackendTests PRIVATE
    BLACKJACK_EXPORTS
    $<TARGET_PROPERTY:Backend,COMPILE_DEFINITIONS>
)

if(TARGET SFML::Network)
    target_link_libraries(BackendTests PRIVATE SFML::Network)
elseif(TARGET sfml-network)
    target_link_libraries(BackendTests PRIVATE sfml-network)
endif()

target_link_libraries(BackendTests PRIVATE GTest::gtest GTest::gmock GTest::gtest_main)

add_custom_command(TARGET BackendTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:BackendTests>
        $<TARGET_FILE_DIR:BackendTests>
    COMMAND_EXPAND_LISTS
)

if(MSVC)
    target_compile_options(BackendTests PRIVATE /utf-8 /bigobj)
endif()

if(BLACKJACK_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(BackendTests PRIVATE --coverage -O0 -g)
    target_link_options(BackendTests PRIVATE --coverage)

    find_program(GCOVR_EXECUTABLE gcovr)
    if(GCOVR_EXECUTABLE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(GCOVR_GCOV_ARGS --gcov-executable "llvm-cov gcov")
        else()
            set(GCOVR_GCOV_ARGS "")
        endif()

        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${GCOVR_EXECUTABLE}
                --root ${CMAKE_SOURCE_DIR}
                --filter "${CMAKE_SOURCE_DIR}/Backend/"
                --exclude "${CMAKE_SOURCE_DIR}/Tests/"
                ${GCOVR_GCOV_ARGS}
                --html-details ${CMAKE_BINARY_DIR}/coverage/index.html
                --xml ${CMAKE_BINARY_DIR}/coverage/coverage.xml
                --print-summary
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS BackendTests
            USES_TERMINAL
        )
    endif()
endif()

enable_testing()

gtest_discover_tests(BackendTests
    DISCOVERY_TIMEOUT 60
    PROPERTIES RESOURCE_LOCK blackjack_ports TIMEOUT 120
)
