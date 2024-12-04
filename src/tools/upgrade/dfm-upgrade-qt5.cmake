set(BIN_NAME dfm-upgrade)

set(QT_VERSION_MAJOR 5)
set(DTK_VERSION_MAJOR "")

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

configure_upgrade_library(${BIN_NAME})

target_link_libraries(${BIN_NAME}
    DFM${DTK_VERSION_MAJOR}::base
    DFM${DTK_VERSION_MAJOR}::framework
    Qt5::Core
) 