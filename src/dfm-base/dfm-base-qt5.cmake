set(BIN_NAME dfm-base)

set(QT_VERSION_MAJOR 5)
set(DTK_VERSION_MAJOR "")

include(dfm-base.cmake)

# install header files
install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/${BIN_NAME}
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
