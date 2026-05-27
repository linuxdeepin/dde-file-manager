cmake_minimum_required(VERSION 3.10)

# Add all service test subdirectories
add_subdirectory(accesscontrol)
add_subdirectory(sharecontrol)
add_subdirectory(mountcontrol)
add_subdirectory(textindex)
if(DFM_TEST_ENABLE_DISKENCRYPT)
    add_subdirectory(diskencrypt)
endif()
