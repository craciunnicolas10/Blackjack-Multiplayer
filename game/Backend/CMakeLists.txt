cmake_minimum_required(VERSION 3.28)
project(Backend)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCES})

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION bin
    RUNTIME DESTINATION bin
)

target_compile_definitions(${PROJECT_NAME} PRIVATE BLACKJACK_EXPORTS)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Jocul in retea (Backend/net). API-ul nu expune tipuri SFML; PUBLIC doar ca biblioteca sa
# ajunga langa orice executabil care foloseste Backend-ul (Frontend, teste).
target_link_libraries(${PROJECT_NAME} PUBLIC SFML::Network)
