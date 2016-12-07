TEMPLATE = subdirs

include(../common/common.pri)

!isEmpty(MENU_DIALOG_PLUGIN){
    SUBDIRS +=　showfile
}
