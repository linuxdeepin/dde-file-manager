set (QCH_INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/qt${QT_VERSION_MAJOR}/doc CACHE STRING "QCH install location")
set (DOXYGEN_GENERATE_HTML "YES" CACHE STRING "Doxygen HTML output")
set (DOXYGEN_GENERATE_XML "NO" CACHE STRING "Doxygen XML output")
set (DOXYGEN_GENERATE_QHP "YES" CACHE STRING "Doxygen QHP output")
set (DOXYGEN_FILE_PATTERNS *.cpp *.h *.md *.zh_CN.dox CACHE STRING "Doxygen File Patterns")
set (DOXYGEN_PROJECT_NUMBER ${CMAKE_PROJECT_VERSION} CACHE STRING "") # Should be the same as this project is using.
set (DOXYGEN_EXTRACT_STATIC YES)
set (DOXYGEN_OUTPUT_LANGUAGE "Chinese")
set (DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/docs/)

if (DFM_BUILD_WITH_QT6)
    find_package(Qt6 COMPONENTS ToolsTools CONFIG REQUIRED)
    if (TARGET Qt6::qhelpgenerator)
        get_target_property(QHelpGenerator_EXECUTABLE Qt6::qhelpgenerator LOCATION)
        set (DOXYGEN_QHG_LOCATION "${QHelpGenerator_EXECUTABLE}")
    else ()
        message ("error: can't found qhelpgenerator from Qt6 tools.")
    endif ()
else ()
    set (DOXYGEN_QHG_LOCATION "qhelpgenerator")
endif ()

set (DOXYGEN_QHP_NAMESPACE "org.deepin.dtk.io")
set (DOXYGEN_QCH_FILE "${PROJECT_NAME}.qch")
set (DOXYGEN_QHP_VIRTUAL_FOLDER ${PROJECT_NAME})
set (DOXYGEN_HTML_EXTRA_STYLESHEET "" CACHE STRING "Doxygen custom stylesheet for HTML output")
set (DOXYGEN_TAGFILES "qtcore.tags=qthelp://doc.qt.io/qt-${QT_VERSION_MAJOR}/" CACHE STRING "Doxygen tag files")
set (DOXYGEN_MACRO_EXPANSION "YES")
set (DOXYGEN_EXPAND_ONLY_PREDEF "YES")
set (DOXYGEN_PREDEFINED
    "DPF_BEGIN_NAMESPACE=namespace dpf {"
    "DPF_END_NAMESPACE=}"
    "DPF_USE_NAMESPACE=using dpf;"
    "BEGEN_DFMEXT_NAMESPACE=namespace dfmext {"
    "END_DFMEXT_NAMESPACE=}"
    "USING_DFMEXT_NAMESPACE=using dfmext;"
    "DFMBASE_BEGIN_NAMESPACE=namespace dfmbase {"
    "DFMBASE_USE_NAMESPACE=}"
    "DFMBASE_USE_NAMESPACE=using dfmbase;"
    "DFMGUI_BEGIN_NAMESPACE=namespace dfmgui {"
    "DFMGUI_USE_NAMESPACE=}"
    "DFMGUI_USE_NAMESPACE=using dfmgui;"
    "DFMGLOBAL_BEGIN_NAMESPACE=namespace dfmbase::Global {"
    "DFMGLOBAL_END_NAMESPACE=}"
    "DFMGLOBAL_USE_NAMESPACE=using dfmbase::Global;"
)

doxygen_add_docs (doxygen
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/docs
    ALL
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generate documentation via Doxygen"
)

install (FILES ${PROJECT_BINARY_DIR}/docs/html/${PROJECT_NAME}.qch DESTINATION ${QCH_INSTALL_DESTINATION})
