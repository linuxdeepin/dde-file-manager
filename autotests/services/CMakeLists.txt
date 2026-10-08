# Service (daemon) unit tests, grouped by service module.
if(OPT_ENABLE_BUILD_UT_SERVICES)
    add_subdirectory(diskencrypt)
    add_subdirectory(tpmcontrol)
    add_subdirectory(mountcontrol)
endif()
