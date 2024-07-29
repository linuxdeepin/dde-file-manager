cmake_minimum_required(VERSION 3.10)

# add sub dir for business plugins

if(OPT_ENABLE_QML)
    add_subdirectory(filemanager)
    # TODO: adapt others
else()
    add_subdirectory(desktop)
    add_subdirectory(common)
    add_subdirectory(filemanager)
    add_subdirectory(filedialog)
    add_subdirectory(server)
    if (NOT OPT_ENABLE_QT6)
        add_subdirectory(daemon)
    endif()
endif()
