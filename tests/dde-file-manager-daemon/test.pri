#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------
#system($$PWD/../vendor/prebuild)
#include($$PWD/../vendor/vendor.pri)

INCLUDEPATH +=  $$PWD \
                $$PWD/../../3rdparty/cpp-stub \
                $$PWD/../../3rdparty/stub-ext


SOURCES += \
    $$PWD/main.cpp \
#    $$PWD/vault/ut_vaultmanager.cpp \
#    $$PWD/vault/ut_vaultclock.cpp \
    $$PWD/accesscontrol/ut_accesscontrolmanager.cpp \
#    $$PWD/app/ut_filemanagerdaemon.cpp \
#    $$PWD/app/ut_policykithelper.cpp \
#    $$PWD/client/ut_filemanagerclient.cpp \
#    $$PWD/controllers/ut_appcontroller.cpp \
#    $$PWD/controllers/ut_fileoperation.cpp \
#    $$PWD/fileoperationjob/ut_basejob.cpp \
#    $$PWD/fileoperationjob/ut_copyjob.cpp \
#    $$PWD/fileoperationjob/ut_createfolderjob.cpp \
#    $$PWD/fileoperationjob/ut_createtemplatefilejob.cpp \
#    $$PWD/fileoperationjob/ut_deletejob.cpp \
#    $$PWD/fileoperationjob/ut_movejob.cpp \
#    $$PWD/fileoperationjob/ut_renamejob.cpp \
#    $$PWD/tag/ut_tagmanagerdaemon.cpp \
#    $$PWD/usershare/ut_usersharemanager.cpp

!CONFIG(DISABLE_TSAN_TOOL) {
    #DEFINES += TSAN_THREAD #互斥
    DEFINES += ENABLE_TSAN_TOOL
    contains(DEFINES, TSAN_THREAD){
       QMAKE_CXXFLAGS+="-fsanitize=thread"
       QMAKE_CFLAGS+="-fsanitize=thread"
       QMAKE_LFLAGS+="-fsanitize=thread"
    } else {
       QMAKE_CXXFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_CFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_LFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
    }
}
