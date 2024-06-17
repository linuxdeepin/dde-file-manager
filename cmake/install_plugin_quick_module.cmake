# Install all plugin qml component file to ${DST_PATH}
function(INSTALL_PLUGIN_QUICK_MODULE DST_PATH)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        # Support relative path, work on build
        file(GLOB_RECURSE QMLS
            RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/*.qml")
        set(QML_BUILD_INSTALL_PATH ${PROJECT_BINARY_DIR}/../../${PROJECT_NAME})
        add_custom_target(${PROJECT_NAME}_QML_INSTALL ALL
            COMMAND mkdir -p ${QML_BUILD_INSTALL_PATH}
            COMMAND cp --parents -f ${QMLS} ${QML_BUILD_INSTALL_PATH}
            COMMAND echo "Copy qml file to ${QML_BUILD_INSTALL_PATH}"
            COMMENT "Run automatically for each build"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM
        )
        message("--- Copy qml file to ${QML_BUILD_INSTALL_PATH}")

    else()
        file(GLOB_RECURSE QMLS
            CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/*.qml")
        install(FILES ${QMLS} DESTINATION ${DST_PATH}/${PROJECT_NAME})
        message("--- Copy qml file to ${DST_PATH}/${PROJECT_NAME}")
    endif()
endfunction(INSTALL_PLUGIN_QUICK_MODULE)
