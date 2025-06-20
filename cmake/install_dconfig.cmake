# expect that all dconfigs of plugin is saved in ./assets/configs
function(INSTALL_DCONFIG CONFIG_NAME)
    set(DConfigPath ${CMAKE_SOURCE_DIR}/assets/configs)
    message("DConfigPath: ${DConfigPath}")
    set(DFMAppId "org.deepin.dde.file-manager")
    set(ConfigName ${CONFIG_NAME})

    if (DEFINED DSG_DATA_DIR)
        message("-- DConfig is supported by DTK")
        message("---- AppId: ${DFMAppId}")
        message("---- Base: ${DConfigPath}")
        message("---- Files: ${DConfigPath}/${ConfigName}")
        if(COMMAND dtk_add_config_meta_files)
            message(STATUS "Using dtk_add_config_meta_files")
            dtk_add_config_meta_files(APPID ${DFMAppId}
                BASE ${DConfigPath}
                FILES ${DConfigPath}/${ConfigName})
        else()
            message(STATUS "Using dconfig_meta_files")
            dconfig_meta_files(APPID ${DFMAppId}
                BASE ${DConfigPath}
                FILES ${DConfigPath}/${ConfigName})
        endif()
    else()
        set(DSG_DATA_DIR ${CMAKE_INSTALL_PREFIX}/share/dsg)
        message("-- DConfig is NOT supported by DTK, install files into target path")
        message("---- InstallTargetDir: ${DSG_DATA_DIR}/configs")
        install(FILES ${DConfigPath}/${ConfigName} DESTINATION ${DSG_DATA_DIR}/configs/${DFMAppId})
    endif()
endfunction()
