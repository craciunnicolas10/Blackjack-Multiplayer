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

# Copiaza DLL-urile (Backend, SFML) si assets langa Frontend.exe si in build dir,
# ca aplicatia sa porneasca si fara pasul de Install.
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# Sursele sunt UTF-8 (texte cu diacritice); fără /utf-8, MSVC le citește în codepage-ul local.
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()
