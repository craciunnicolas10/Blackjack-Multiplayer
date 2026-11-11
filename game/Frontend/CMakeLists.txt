cmake_minimum_required(VERSION 3.28)
project(Frontend)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION bin)

install(TARGETS sfml-graphics sfml-window sfml-system
    LIBRARY DESTINATION bin
)

target_link_libraries(${PROJECT_NAME} PRIVATE Backend SFML::Graphics)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
