/**
 * Copyright (C) 2020 UOS Technology Co., Ltd.
 *
 * This is a static file that used to define the UI object name that will used to auto-test
 **/

#ifndef LIB_FRAME_ACCESSIBLE_DEFINE_H
#define LIB_FRAME_ACCESSIBLE_DEFINE_H

#include <QString>
#include <QObject>

// 使用宏定义，方便国际化操作

#define DIALOGS_CLOSE_ALL_DIALOG_INDICATOR QObject::tr("dialogs_close_all_dialog_indicator")
#define DIALOGS_CLOSE_ALL_DIALOG_INDICATOR_MSG_LABEL QObject::tr("dialog_indicator_message_label")
#define DIALOGS_CLOSE_ALL_DIALOG_INDICATOR_CLOSE_BUTTON QObject::tr("dialog_indicator_message_all_close_button")

#define DIALOGS_TASK_DIALOG QObject::tr("d_task_dialog")
#define DIALOGS_TASK_DIALOG_TITLE_BAR QObject::tr("d_task_dialog_title_bars")
#define DIALOGS_TASK_DIALOG_TASK_LIST_WIDGET QObject::tr("task_dialog_task_list_widget")
#define DIALOGS_TASK_DIALOG_TASK_LIST_ITEM QObject::tr("task_dialog_task_list_item")
#define DIALOGS_TASK_DIALOG_TASK_JOB_ITEM QObject::tr("task_dialog_task_job_item")

#endif // LIB_FRAME_ACCESSIBLE_DEFINE_H
