# Install all plugin qml component file to

function(INSTALL_PLUGIN_QUICK_MODULE DST_PATH)
    file(GLOB_RECURSE QMLS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.qml")

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(QML_BUILD_INSTALL_PATH ${PROJECT_BINARY_DIR}/../../${PROJECT_NAME})
        EXECUTE_PROCESS(COMMAND mkdir -p ${QML_BUILD_INSTALL_PATH})
        EXECUTE_PROCESS(COMMAND cp -f ${QMLS} ${QML_BUILD_INSTALL_PATH})
        message("--- Copy qml file to ${QML_BUILD_INSTALL_PATH}")

    else()
        install(FILES ${QMLS} DESTINATION ${DST_PATH}/${PROJECT_NAME})
        message("--- Copy qml file to ${DST_PATH}/${PROJECT_NAME}")
    endif()
endfunction(INSTALL_PLUGIN_QUICK_MODULE)
