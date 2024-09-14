set(BIN_NAME dfm-framework)

set(QT_VERSION_MAJOR 5)
set(DTK_VERSION_MAJOR "")

include(dfm-framework.cmake)

# install headers
install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/${BIN_NAME}
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
