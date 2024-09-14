cmake_minimum_required(VERSION 3.10)

# If do't define version number, specify the version number
set(VERSION "1.0.0" CACHE STRING "define project version")

set(BASE_NAME declarative)
set(BIN_NAME dfm-${BASE_NAME})

if(NOT VERSION)
    set(VERSION "1.0.0")
endif()

if(NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 1)
endif()

# signals and slots keyword is disable, use Q_SIGNALS and Q_SLOTS instead
add_compile_definitions(QT_NO_SIGNALS_SLOTS_KEYWORDS)

# add qrc resources
set(QRC_FILES)

if(DFM_BUILD_WITH_QT6)
    qt6_add_resources(QRC_RESOURCES ${QRC_FILES})
else()
    qt5_add_resources(QRC_RESOURCES ${QRC_FILES})
endif()

# add depends
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Quick REQUIRED)

# TODO：放到公共目录？
# 通过 Qt cmake 接口注册组件
qt_add_qml_module(${BIN_NAME}
    URI org.deepin.filemanager.${BASE_NAME}
    VERSION 1.0
    SHARED
    RESOURCE_PREFIX /
    OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DIR}/qml/org/deepin/filemanager/${BASE_NAME}
    QML_FILES ActionMenu.qml AnimationHSpliter.qml
)

target_include_directories(${BIN_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(${BIN_NAME} PUBLIC Qt6::Core Qt6::Quick)

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# install module
install(DIRECTORY ${DFM_BUILD_PLUGIN_DIR}/qml/org/deepin/filemanager/${BASE_NAME}
        DESTINATION ${DFM_QML_MODULE}/org/deepin/filemanager)
