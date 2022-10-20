cmake_minimum_required(VERSION 3.10)

include_directories(commonstubs)

# add sub dir for business plugins
add_subdirectory(filemanager)
add_subdirectory(filedialog)
add_subdirectory(desktop)
add_subdirectory(common)
#add_subdirectory(daemon)
