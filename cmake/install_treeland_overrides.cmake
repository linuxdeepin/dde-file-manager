# expect that treeland dsg override files are saved in ./assets/treeland
function(INSTALL_TREELAND_OVERRIDES SUBPATH)
    set(TreelandRoot ${CMAKE_SOURCE_DIR}/assets/treeland)
    set(TreelandAppId "org.deepin.dde.treeland")
    set(TreelandMetaName "org.deepin.dde.treeland.app")
    set(TreelandBase ${TreelandRoot}/${TreelandAppId}/${TreelandMetaName})

    file(GLOB TreelandOverrideFiles "${TreelandBase}/${SUBPATH}/*.json")
    list(LENGTH TreelandOverrideFiles TreelandOverrideCount)

    message(STATUS "Treeland override install request")
    message(STATUS "---- AppId: ${TreelandAppId}")
    message(STATUS "---- MetaName: ${TreelandMetaName}")
    message(STATUS "---- Base: ${TreelandBase}")
    message(STATUS "---- SubPath: ${SUBPATH}")
    message(STATUS "---- MatchedFiles: ${TreelandOverrideCount}")
    message(STATUS "Treeland override: ${SUBPATH}, files=${TreelandOverrideCount}")

    if(TreelandOverrideCount EQUAL 0)
        message(WARNING "No treeland override files found for subpath: ${SUBPATH}")
        return()
    endif()

    find_package(Dtk6 REQUIRED COMPONENTS Core)

    if(COMMAND dtk_add_config_override_files)
        message(STATUS "Using dtk_add_config_override_files")
        dtk_add_config_override_files(
            APPID ${TreelandAppId}
            META_NAME ${TreelandMetaName}
            BASE ${TreelandBase}
            FILES ${TreelandOverrideFiles}
        )
    else()
        if(NOT DEFINED DSG_DATA_DIR)
            set(DSG_DATA_DIR ${CMAKE_INSTALL_PREFIX}/share/dsg)
        endif()
        message(WARNING "dtk_add_config_override_files not found, fallback to manual install")
        install(
            FILES ${TreelandOverrideFiles}
            DESTINATION ${DSG_DATA_DIR}/configs/overrides/${TreelandAppId}/${TreelandMetaName}/${SUBPATH}
        )
    endif()
endfunction()
