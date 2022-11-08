file(GLOB_RECURSE SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

file(GLOB LIB_HEADER
    ${CMAKE_CURRENT_LIST_DIR}/../include/*.h
)
file(GLOB LIB_MENU_HEADER
    ${CMAKE_CURRENT_LIST_DIR}/../include/menu/*.h
)
file(GLOB LIB_EMBLEMICON_HEADER
    ${CMAKE_CURRENT_LIST_DIR}/../include/emblemicon/*.h
)

add_library(${BIN_NAME} SHARED
    ${LIB_HEADER}
    ${LIB_MENU_HEADER}
    ${LIB_EMBLEMICON_HEADER}
    ${SRCS}
)


target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}
)


if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

add_library(DFM::extension ALIAS ${PROJECT_NAME})

target_compile_definitions(${BIN_NAME} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")


install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${LIB_HEADER} DESTINATION include/${PROJECT_NAME})
install(FILES ${LIB_MENU_HEADER} DESTINATION include/${PROJECT_NAME}/menu)
install(FILES ${LIB_EMBLEMICON_HEADER} DESTINATION include/${PROJECT_NAME}/emblemicon)
