cmake_minimum_required(VERSION 3.28)

project(BlackJackApp)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/CPM.cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake
    )
endif()

if(NOT DEFINED ENV{CPM_SOURCE_CACHE} AND NOT DEFINED CPM_SOURCE_CACHE)
    set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpm-cache" CACHE PATH "CPM source cache")
endif()

include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions($<$<CONFIG:Debug>:BLACKJACK_DEBUG_BUILD>)

add_subdirectory(Backend)

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
include(cmake/Dependencies.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(Frontend)

foreach (target Backend Frontend)
    if (MSVC)
        target_compile_options(${target} PRIVATE
            $<$<CONFIG:Release>:/O2>
            $<$<CONFIG:Release>:/GL>
            $<$<CONFIG:Release>:/Gy>
            $<$<CONFIG:Release>:/Gw>
        )

        target_link_options(${target} PRIVATE
            $<$<CONFIG:Release>:/LTCG>
            $<$<CONFIG:Release>:/OPT:REF>
            $<$<CONFIG:Release>:/OPT:ICF>
            $<$<CONFIG:Release>:/DEBUG:NONE>
        )
    endif()
endforeach()
