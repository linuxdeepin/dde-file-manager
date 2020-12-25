# Actually for "Disk" or say "Partition"
# For replacing stuff in deviceinfo/
# Replacement of current UDisksDevice* / GvfsMountManager* / etc.
HEADERS += \
    $$PWD/dfmabstractdeviceinterface.h \
    $$PWD/dfmudisks2deviceinfo.h \
    $$PWD/dfmvfsdeviceinfo.h \
    $$PWD/dfmdeviceinfo.h \
    $$PWD/dfminvaliddeviceinfo.h \
    $$PWD/dfmdevicecontroller.h

SOURCES += \
    $$PWD/dfmudisks2deviceinfo.cpp \
    $$PWD/dfmvfsdeviceinfo.cpp \
    $$PWD/dfmdeviceinfo.cpp \
    $$PWD/dfminvaliddeviceinfo.cpp \
    $$PWD/dfmdevicecontroller.cpp \
    $$PWD/dfmabstractdeviceinterface.cpp
