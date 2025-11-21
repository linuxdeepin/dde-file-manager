cmake_minimum_required(VERSION 3.10)

add_subdirectory(accesscontrol)
add_subdirectory(sharecontrol)
add_subdirectory(mountcontrol)
add_subdirectory(textindex)
add_subdirectory(diskencrypt)
add_subdirectory(tpmcontrol)

# 安全加固
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deepin-service-group@.service.d DESTINATION /etc/systemd/system/)
