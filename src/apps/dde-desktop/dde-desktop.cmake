find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets DBus REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    DFM${DTK_VERSION_MAJOR}::base
    DFM${DTK_VERSION_MAJOR}::framework
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    ${Dtk6Widget_LIBRARIES}
)

if(${DTK_VERSION_MAJOR} EQUAL "6")
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${Dtk6Widget_LIBRARIES}
    )
else()
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${DtkWidget_LIBRARIES}
    )
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# dbus
install(FILES data/com.deepin.dde.desktop.service DESTINATION share/dbus-1/services)

# computer and trash
install(FILES data/applications/dfm-open.sh DESTINATION bin/)
install(FILES data/applications/dde-computer.desktop DESTINATION share/applications/)
install(FILES data/applications/dde-trash.desktop DESTINATION share/applications/)
install(FILES data/applications/dde-home.desktop DESTINATION share/applications/)
install(FILES data/applications/dde-desktop.desktop DESTINATION share/applications)
