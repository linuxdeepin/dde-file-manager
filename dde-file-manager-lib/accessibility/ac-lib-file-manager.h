/*
 * Copyright (C) 2020 UOS Technology Co., Ltd.
 *
 * Author:     max-lv <lvwujun@uniontech.com>
 *
 * Maintainer: max-lv <lvwujun@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIB_FRAME_ACCESSIBLE_DEFINE_H
#define LIB_FRAME_ACCESSIBLE_DEFINE_H

#include <QString>
#include <QObject>
#include "acintelfunctions.h"

// 使用宏定义，方便国际化操作
#ifdef ENABLE_ACCESSIBILITY
    #define AC_FILE_MENU_DEFAULT QObject::tr("default_file_menu") //默认菜单
    #define AC_FILE_MENU_DEFAULT_SIDEBAR QObject::tr("default_sidebar_menu") //侧边栏默认菜单
    #define AC_FILE_MENU_SIDEBAR_TAGITEM QObject::tr("sidebar_tagitem_menu") //侧边栏标签菜单
    #define AC_FILE_MENU_SIDEBAR_DEVICE_ITEM QObject::tr("sidebar_deviceitem_menu")//侧边栏设备菜单
    #define AC_FILE_MENU_SIDEBAR_VAULT_ITEM QObject::tr("sidebar_vaultitem_menu")//侧边栏保险箱菜单
    #define AC_FILE_MENU_SIDEBAR_BOOK_MARK QObject::tr("sidebar_bookmark_menu") //侧边栏书签菜单
    #define AC_FILE_MENU_USER_SHARE QObject::tr("user_share_menu") //用户共享菜单
    #define AC_FILE_MENU_TOOLBAR_SEETINGS QObject::tr("toolbar_seetings_menu")//工具栏菜单
    #define AC_FILE_MENU_VAULT QObject::tr("vault_menu") //保险箱菜单
    #define AC_FILE_MENU_FILEVIEW QObject::tr("fileview_menu")//视图菜单
    #define AC_FILE_MENU_CRUMB_BAR QObject::tr("crumb_bar_menu")//面包屑菜单

    #define AC_COMPUTER_VIEW QObject::tr("computer_view") // 计算视图
    #define AC_COMPUTER_LIST_VIEW QObject::tr("computer_list_view")//计算机列表视图
    #define AC_COMPUTER_MIAN_WINDOW QObject::tr("computer_window")//计算机窗口

    #define AC_COMPUTER_TITLE_BAR QObject::tr("computer_title_bar")//计算机标题栏
    #define AC_COMPUTER_CUSTOM_TITLE_BAR QObject::tr("custom_title_bar")//自定义标题栏
    #define AC_COMPUTER_TITLE_BAR_ADRESS QObject::tr("AddressToolBar")//地址输入框
    #define AC_COMPUTER_TITLE_BAR_BACK_BTN QObject::tr("back_button")//回退按钮
    #define AC_COMPUTER_TITLE_BAR_FORWARD_BTN QObject::tr("forword_button") //前进按钮
    #define AC_COMPUTER_TITLE_BAR_BTN_BOX QObject::tr("button_box") //按钮区域
    #define AC_COMPUTER_TITLE_BAR_SERACH_BTN QObject::tr("search_button") //查询按钮
    #define AC_COMPUTER_TITLE_BAR_DETAIL_BTN QObject::tr("detail_button") //详细按钮
    #define AC_COMPUTER_TITLE_BAR_CONTOLLER_TOOL_BAR QObject::tr("ContollerToolBar") //控制区域
    #define AC_COMPUTER_TITLE_BAR_CRUMB_SERACH_FRAME QObject::tr("crumb_search_frame") //面包屑区域

    #define AC_COMPUTER_STATUS_BAR QObject::tr("status_bar") //计算机状态栏
    #define AC_COMPUTER_STATUS_BAR_LOADING_INDICATOR QObject::tr("loadingIndicator") //加载指示器
    #define AC_COMPUTER_STATUS_BAR_SCALE_SLIDER QObject::tr("scale_slider")
    #define AC_COMPUTER_STATUS_BAR_ACCEPT_BUTTON QObject::tr("accept_button") //接收按钮
    #define AC_COMPUTER_STATUS_BAR_REJECT_BUTTON QObject::tr("reject_button") // 拒绝按钮
    #define AC_COMPUTER_STATUS_BAR_LINE_EDIT QObject::tr("line_edit") //编辑区域

    #define AC_COMPUTER_RENAME_BAR QObject::tr("rename_bar") //重命名
    #define AC_COMPUTER_RENAME_BAR_SELECT_TYPE QObject::tr("rename_types") //重命名类型
    #define AC_COMPUTER_RENAME_BAR_STACK_WIDGET QObject::tr("stack_widget") //栈面板

    #define AC_COMPUTER_CRUMB_BAR QObject::tr("crumb_bar") //面包屑指示栏
    #define AC_COMPUTER_CRUMB_BAR_LEFT_ARROW QObject::tr("left_arrow") //左箭头
    #define AC_COMPUTER_CRUMB_BAR_RIGHT_ARROW QObject::tr("right_arrow") //右箭头
    #define AC_COMPUTER_CRUMB_BAR_LIST_VIEW QObject::tr("crumb_list_view") //列表
    #define AC_COMPUTER_CRUMB_BAR_SCROLL_BAR QObject::tr("scroll_bar") //滑动模块

    #define AC_COMPUTER_COMPLETER_LIST_VIEW QObject::tr("CompleterListView")

    #define AC_FILE_VIEW QObject::tr("file_view") //文件视图
    #define AC_FM_VIEW_MANAGER QObject::tr("fm_view_manager") //文件视图管理
    #define AC_DM_SIDE_BAR QObject::tr("left_side_bar") //左边栏
    #define AC_DM_SIDE_BAR_VIEW QObject::tr("side_bar_view") //左边栏视图
    #define AC_DM_TOOLBAR QObject::tr("DM_ToolBar") //工具栏
    #define AC_DM_RIGHT_VIEW QObject::tr("right_view") // 右边栏
    #define AC_DM_RIGHT_VIEW_TRASH_LABEL QObject::tr("trash_label") //回收站标签
    #define AC_DM_RIGHT_VIEW_TRASH_HOLDER QObject::tr("trash_holder") //回收站
    #define AC_DM_RIGHT_VIEW_TRASH_SPLIT_LINE QObject::tr("trash_split_line") //回收站隔离线
    #define AC_DM_RIGHT_VIEW_EMPTY_TRASH_BUTTON QObject::tr("EmptyTrashButton") //清空回收站
    #define AC_DM_RIGHT_VIEW_HOLDER QObject::tr("rightviewHolder") //右视图
    #define AC_DM_RIGHT_VIEW_DETAIL_VLINE QObject::tr("rightDetailVLine") //右视图隔离线
    #define AC_DM_RIGHT_VIEW_DETAIL_VIEW QObject::tr("right-detail-view") //有视图详情
    #define AC_DM_RIGHT_VIEW_MAIN_FRAME QObject::tr("main_frame") //主窗口
    #define AC_DM_RIGHT_VIEW_MAIN_FRAME_ICON_LABEL QObject::tr("frame_icon")//主窗口图标
    #define AC_DM_RIGHT_VIEW_MAIN_FRAME_LINE QObject::tr("frame_line") //主窗口隔离线
    #define AC_DM_RIGHT_VIEW_MAIN_FRAME_SCROLLAREA QObject::tr("DFMRightDetailView-QScrollArea") //滑动区域

    #define AC_TAG_WIDGET QObject::tr("tag-widget") //标签面板

    #define AC_COMPUTER_CENTRAL_WIDGET QObject::tr("CentralWidget") //计算机中心区域
    #define AC_DM_SPLITTER QObject::tr("dm_splitter") //隔离线
    #define AC_VIEW_MID_WIDGET QObject::tr("mid_wiget") //中间区域

    #define AC_VIEW_TAB_BAR QObject::tr("TabBar") //标签栏
    #define AC_VIEW_TAB_BAR_NEW_BUTTON QObject::tr("NewTabButton") //标签栏按钮
    #define AC_VIEW_TAB_BAR_TOP_LINE QObject::tr("top_line") //标签栏上线
    #define AC_VIEW_TAB_BAR_BOTTOM_LINE QObject::tr("bottom_line") //标签栏下线

    #define AC_CLOSE_ALL_DLG_INDICATOR QObject::tr("dialogs_close_all_dialog_indicator")
    #define AC_CLOSE_ALL_DLG_INDICATOR_MSG_LABEL QObject::tr("dialog_indicator_message_label")
    #define AC_CLOSE_ALL_DLG_INDICATOR_CLOSE_BUTTON QObject::tr("dialog_indicator_message_all_close_button")

    #define AC_TASK_DLG QObject::tr("d_task_dialog") //任务对话框
    #define AC_TASK_DLG_TITLE_BAR QObject::tr("d_task_dialog_title_bars") //标题栏
    #define AC_TASK_DLG_TASK_LIST_WIDGET QObject::tr("task_dialog_task_list_widget") //任务列表面板
    #define AC_TASK_DLG_TASK_LIST_ITEM QObject::tr("task_dialog_task_list_item") // 任务列表
    #define AC_TASK_DLG_TASK_JOB_ITEM QObject::tr("task_dialog_task_job_item") //任务

    #define AC_FD_STATUS_BAR QObject::tr("dlg_status_bar") // 文件对话框状态栏
    #define AC_FD_STATUS_BAR_INTEL QObject::tr("statusBar") // 状态栏
    #define AC_FD_STATUS_BAR_FILE_NAME_EDIT QObject::tr("file_name_edit") //文件名
    #define AC_FD_STATUS_BAR_FILTERS QObject::tr("filters") //过滤
    #define AC_FD_STATUS_BAR_TITLE_LABEL QObject::tr("title_label") //标题标签
    #define AC_FD_STATUS_BAR_TITLE_CONTENT_LABEL QObject::tr("content_label") //内容标签
    #define AC_FD_STATUS_BAR_TITLE_CONTENT_EDIT QObject::tr("content_edit") //内容框
    #define AC_FD_STATUS_BAR_TITLE_CONTENT_BOX QObject::tr("content_box") //内容列表

    #define AC_TAG_LABLE QObject::tr("tag_label") //tag 标签
    #define AC_TAG_LEFT_LABLE QObject::tr("tag_left_label") //tag 左标签
    #define AC_TAG_ACTION_WIDGET QObject::tr("tag_action_widget") //tag 面板
    #define AC_TAG_ACTION_WIDGET_TOOL_TIP QObject::tr("tag_action_tool_tip") //tag 面板 提示
    #define AC_TAG_CRUMB_EDIT QObject::tr("tag_crumb_edit") //tag 编辑区域

    #define AC_VAULT_ACTIVE_WIDGET                          QObject::tr("vault_active_widget")          // 保险箱激活页面
    #define AC_VAULT_ACTIVE_STACK                           QObject::tr("vault_active_stack")           // 保险箱激活 功能页面容器
    #define AC_VAULT_ACTIVE_START_WIDGET                    QObject::tr("vault_active_start_widget")    // 保险箱激活 开始页面
    #define AC_VAULT_ACTIVE_START_TITLE_LABEL               QObject::tr("vault_active_start_title")     // 保险箱激活 开始页面 标题
    #define AC_VAULT_ACTIVE_START_CONTENT_LABEL             QObject::tr("vault_active_start_content")   // 保险箱激活 开始页面 内容
    #define AC_VAULT_ACTIVE_START_PICTURE                   QObject::tr("vault_active_start_picture")   // 保险箱激活 开始页面 图片
    #define AC_VAULT_ACTIVE_START_BUTTON                    QObject::tr("vault_active_start_button")    // 保险箱激活 开始页面 开启按钮
    #define AC_VAULT_ACTIVE_SET_PASSWORD_WIDGET             QObject::tr("vault_active_set_password_widget")             // 保险箱激活 设置密码页面
    #define AC_VAULT_ACTIVE_SET_PASSWORD_TITLE              QObject::tr("vault_active_set_password_title")              // 保险箱激活 设置密码页面 标题
    #define AC_VAULT_ACTIVE_SET_PASSWORD_TYPE_LABEL         QObject::tr("vault_active_set_password_type_label")         // 保险箱激活 设置密码页面 类型 标签
    #define AC_VAULT_ACTIVE_SET_PASSWORD_TYPE_COMBOX        QObject::tr("vault_active_set_password_type_combox")        // 保险箱激活 设置密码页面 类型 组合框
    #define AC_VAULT_ACTIVE_SET_PASSWORD_PASSWORD_LABEL     QObject::tr("vault_active_set_password_password_label")     // 保险箱激活 设置密码页面 设置密码 标签
    #define AC_VAULT_ACTIVE_SET_PASSWORD_PASSWORD_EDIT      QObject::tr("vault_active_set_password_password_edit")      // 保险箱激活 设置密码页面 设置密码 编辑框
    #define AC_VAULT_ACTIVE_SET_PASSWORD_REPASSWORD_LABEL   QObject::tr("vault_active_set_password_repassword_lable")   // 保险箱激活 设置密码页面 重复密码 标签
    #define AC_VAULT_ACTIVE_SET_PASSWORD_REPASSWORD_EDIT    QObject::tr("vault_active_set_password_repassword_edit")    // 保险箱激活 设置密码页面 重复密码 编辑框
    #define AC_VAULT_ACTIVE_SET_PASSWORD_HINT_LABEL         QObject::tr("vault_active_set_password_hint_label")         // 保险箱激活 设置密码页面 提示 标签
    #define AC_VAULT_ACTIVE_SET_PASSWORD_HINT_EDIT          QObject::tr("vault_active_set_password_hint_edit")          // 保险箱激活 设置密码页面 提示 编辑框
    #define AC_VAULT_ACTIVE_SET_PASSWORD_NEXT_BUTTON        QObject::tr("vault_active_set_password_next_button")        // 保险箱激活 设置密码页面 下一步 按钮
    #define AC_VAULT_ACTIVE_KEY_WIDGET                      QObject::tr("vault_active_key_widget")          // 保险箱激活 密钥页面
    #define AC_VAULT_ACTIVE_KEY_TITLE                       QObject::tr("vault_active_key_title")           // 保险箱激活 密钥页面 标题
    #define AC_VAULT_ACTIVE_KEY_CONTENT                     QObject::tr("vault_active_key_content")         // 保险箱激活 密钥页面 内容
    #define AC_VAULT_ACTIVE_KEY_KEY_BUTTON                  QObject::tr("vault_active_key_key_button")      // 保险箱激活 密钥页面 密钥 按钮
    #define AC_VAULT_ACTIVE_KEY_KEY_EDIT                    QObject::tr("vault_active_key_key_edit")        // 保险箱激活 密钥页面 密钥 编辑器框
    #define AC_VAULT_ACTIVE_KEY_QRCODE_BUTTON               QObject::tr("vault_active_key_qrcode_button")   // 保险箱激活 密钥页面 二维码 按钮
    #define AC_VAULT_ACTIVE_KEY_QRCODE_IMAGE                QObject::tr("vault_active_key_qrcode_image")    // 保险箱激活 密钥页面 二维码 图片
    #define AC_VAULT_ACTIVE_KEY_QRCODE_HINT                 QObject::tr("vault_active_key_qrcode_hint")     // 保险箱激活 密钥页面 二维码 提示
    #define AC_VAULT_ACTIVE_KEY_NEXT_BUTTON                 QObject::tr("vault_active_key_next_button")     // 保险箱激活 密钥页面 下一步 按钮
    #define AC_VAULT_ACTIVE_ENCRYPT_WIDGET                  QObject::tr("vault_active_encrypy_widget")          // 保险箱激活 加密页面
    #define AC_VAULT_ACTIVE_ENCRYPT_TITLE                   QObject::tr("vault_active_encrypy_title")           // 保险箱激活 加密页面 标题
    #define AC_VAULT_ACTIVE_ENCRYPT_CONTENT                 QObject::tr("vault_active_encrypy_content")         // 保险箱激活 加密页面 内容
    #define AC_VAULT_ACTIVE_ENCRYPT_IMAGE                   QObject::tr("vault_active_encrypy_image")           // 保险箱激活 加密页面 图片
    #define AC_VAULT_ACTIVE_ENCRYPT_PROGRESS                QObject::tr("vault_active_encrypy_progress")        // 保险箱激活 加密页面 进度条
    #define AC_VAULT_ACTIVE_ENCRYPT_PROGRESS_HINT           QObject::tr("vault_active_encrypy_progress_hint")   // 保险箱激活 加密页面 进度条提示
    #define AC_VAULT_ACTIVE_ENCRYPT_FINISH_IMAGE            QObject::tr("vault_active_encrypy_finish_image")    // 保险箱激活 加密页面 加密完成图片
    #define AC_VAULT_ACTIVE_ENCRYPT_FINISH_HINT             QObject::tr("vault_active_encrypy_finish_hint")     // 保险箱激活 加密页面 加密完成提示
    #define AC_VAULT_ACTIVE_ENCRYPT_BUTTON                  QObject::tr("vault_active_encrypy_button")          // 保险箱激活 加密页面 按钮
    #define AC_VAULT_PASSWORD_UNLOCK_WIDGET                 QObject::tr("vault_password_unlock_widget")             // 保险箱解锁 密码解锁页面
    #define AC_VAULT_PASSWORD_UNLOCK_TITLE                  QObject::tr("vault_password_unlock_title")              // 保险箱解锁 密码解锁页面 标题
    #define AC_VAULT_PASSWORD_UNLOCK_CONTENT                QObject::tr("vault_password_unlock_content")            // 保险箱解锁 密码解锁页面 内容
    #define AC_VAULT_PASSWORD_UNLOCK_EDIT                   QObject::tr("vault_password_unlock_edit")               // 保险箱解锁 密码解锁页面 编辑框
    #define AC_VAULT_PASSWORD_UNLOCK_HINT_BUTTON            QObject::tr("vault_password_unlock_hint_button")        // 保险箱解锁 密码解锁页面 密码提示按钮
    #define AC_VAULT_PASSWORD_UNLOCK_CANCEL_BUTTON          QObject::tr("vault_password_unlock_cancel_button")      // 保险箱解锁 密码解锁页面 取消按钮
    #define AC_VAULT_PASSWORD_UNLOCK_OK_BUTTON              QObject::tr("vault_password_unlock_ok_button")          // 保险箱解锁 密码解锁页面 确认按钮
    #define AC_VAULT_KEY_UNLOCK_WIDGET                      QObject::tr("vault_key_unlock_widget")          // 保险箱解锁 密钥解锁页面
    #define AC_VAULT_KEY_UNLOCK_TITLE                       QObject::tr("vault_key_unlock_title")           // 保险箱解锁 密钥解锁页面 标题
    #define AC_VAULT_KEY_UNLOCK_EDIT                        QObject::tr("vault_key_unlock_edit")            // 保险箱解锁 密钥解锁页面 编辑框
    #define AC_VAULT_KEY_UNLOCK_CANCEL_BUTTON               QObject::tr("vault_key_unlock_cancel_button")   // 保险箱解锁 密钥解锁页面 取消按钮
    #define AC_VAULT_KEY_UNLOCK_OK_BUTTON                   QObject::tr("vault_key_unlock_ok_button")       // 保险箱解锁 密钥解锁页面 确认按钮
    #define AC_VAULT_DELETE_WIDGET                          QObject::tr("vault_delete_widget")          // 保险箱删除 删除页面
    #define AC_VAULT_DELETE_TITLE                           QObject::tr("vault_delete_title")           // 保险箱删除 删除页面 标题
    #define AC_VAULT_DELETE_CONTENT                         QObject::tr("vault_delete_content")         // 保险箱删除 删除页面 内容
    #define AC_VAULT_DELETE_CANCEL_BUTTON                   QObject::tr("vault_delete_cancel_button")   // 保险箱删除 删除页面 取消按钮
    #define AC_VAULT_DELETE_CHANGE_BUTTON                   QObject::tr("vault_delete_change_button")   // 保险箱删除 删除页面 切换按钮
    #define AC_VAULT_DELETE_DELETE_BUTTON                   QObject::tr("vault_delete_delete_button")   // 保险箱删除 删除页面 删除按钮
    #define AC_VAULT_DELETE_PASSWORD_EDIT                   QObject::tr("vault_delete_password_edit")   // 保险箱删除 删除页面 密码编辑框
    #define AC_VAULT_DELETE_HINT_BUTTON                     QObject::tr("vault_delete_hint_button")     // 保险箱删除 删除页面 密码提示按钮
    #define AC_VAULT_DELETE_KEY_EDIT                        QObject::tr("vault_delete_key_edit")        // 保险箱删除 删除页面 密钥编辑框
    #define AC_VAULT_DELETE_PROGRESS                        QObject::tr("vault_delete_progress")        // 保险箱删除 删除页面 删除操作进度条

#else
    #define AC_FILE_MENU_DEFAULT "" //默认菜单
    #define AC_FILE_MENU_DEFAULT_SIDEBAR "" //侧边栏默认菜单
    #define AC_FILE_MENU_SIDEBAR_TAGITEM "" //侧边栏标签菜单
    #define AC_FILE_MENU_SIDEBAR_DEVICE_ITEM ""//侧边栏设备菜单
    #define AC_FILE_MENU_SIDEBAR_VAULT_ITEM ""//侧边栏保险箱菜单
    #define AC_FILE_MENU_SIDEBAR_BOOK_MARK "" //侧边栏书签菜单
    #define AC_FILE_MENU_USER_SHARE "" //用户共享菜单
    #define AC_FILE_MENU_TOOLBAR_SEETINGS ""//工具栏菜单
    #define AC_FILE_MENU_VAULT "" //保险箱菜单
    #define AC_FILE_MENU_FILEVIEW ""//视图菜单
    #define AC_FILE_MENU_CRUMB_BAR ""//面包屑菜单

    #define AC_COMPUTER_VIEW "" // 计算视图
    #define AC_COMPUTER_LIST_VIEW ""//计算机列表视图
    #define AC_COMPUTER_MIAN_WINDOW ""//计算机窗口

    #define AC_COMPUTER_TITLE_BAR ""//计算机标题栏
    #define AC_COMPUTER_CUSTOM_TITLE_BAR ""//自定义标题栏
    #define AC_COMPUTER_TITLE_BAR_ADRESS ""//地址输入框
    #define AC_COMPUTER_TITLE_BAR_BACK_BTN ""//回退按钮
    #define AC_COMPUTER_TITLE_BAR_FORWARD_BTN "" //前进按钮
    #define AC_COMPUTER_TITLE_BAR_BTN_BOX "" //按钮区域
    #define AC_COMPUTER_TITLE_BAR_SERACH_BTN "" //查询按钮
    #define AC_COMPUTER_TITLE_BAR_DETAIL_BTN "" //详细按钮
    #define AC_COMPUTER_TITLE_BAR_CONTOLLER_TOOL_BAR "" //控制区域
    #define AC_COMPUTER_TITLE_BAR_CRUMB_SERACH_FRAME "" //面包屑区域

    #define AC_COMPUTER_STATUS_BAR "" //计算机状态栏
    #define AC_COMPUTER_STATUS_BAR_LOADING_INDICATOR "" //加载指示器
    #define AC_COMPUTER_STATUS_BAR_SCALE_SLIDER ""
    #define AC_COMPUTER_STATUS_BAR_ACCEPT_BUTTON "" //接收按钮
    #define AC_COMPUTER_STATUS_BAR_REJECT_BUTTON "" // 拒绝按钮
    #define AC_COMPUTER_STATUS_BAR_LINE_EDIT "" //编辑区域

    #define AC_COMPUTER_RENAME_BAR "" //重命名
    #define AC_COMPUTER_RENAME_BAR_SELECT_TYPE "" //重命名类型
    #define AC_COMPUTER_RENAME_BAR_STACK_WIDGET "" //栈面板

    #define AC_COMPUTER_CRUMB_BAR "" //面包屑指示栏
    #define AC_COMPUTER_CRUMB_BAR_LEFT_ARROW "" //左箭头
    #define AC_COMPUTER_CRUMB_BAR_RIGHT_ARROW "" //右箭头
    #define AC_COMPUTER_CRUMB_BAR_LIST_VIEW "" //列表
    #define AC_COMPUTER_CRUMB_BAR_SCROLL_BAR "" //滑动模块

    #define AC_COMPUTER_COMPLETER_LIST_VIEW ""

    #define AC_FILE_VIEW "" //文件视图
    #define AC_FM_VIEW_MANAGER "" //文件视图管理
    #define AC_DM_SIDE_BAR "" //左边栏
    #define AC_DM_SIDE_BAR_VIEW "" //左边栏视图
    #define AC_DM_TOOLBAR "" //工具栏
    #define AC_DM_RIGHT_VIEW "" // 右边栏
    #define AC_DM_RIGHT_VIEW_TRASH_LABEL "" //回收站标签
    #define AC_DM_RIGHT_VIEW_TRASH_HOLDER "" //回收站
    #define AC_DM_RIGHT_VIEW_TRASH_SPLIT_LINE "" //回收站隔离线
    #define AC_DM_RIGHT_VIEW_EMPTY_TRASH_BUTTON "" //清空回收站
    #define AC_DM_RIGHT_VIEW_HOLDER "" //右视图
    #define AC_DM_RIGHT_VIEW_DETAIL_VLINE "" //右视图隔离线
    #define AC_DM_RIGHT_VIEW_DETAIL_VIEW "" //有视图详情
    #define AC_DM_RIGHT_VIEW_MAIN_FRAME "" //主窗口
    #define AC_DM_RIGHT_VIEW_MAIN_FRAME_ICON_LABEL ""//主窗口图标
    #define AC_DM_RIGHT_VIEW_MAIN_FRAME_LINE "" //主窗口隔离线
    #define AC_DM_RIGHT_VIEW_MAIN_FRAME_SCROLLAREA "" //滑动区域

    #define AC_TAG_WIDGET "" //标签面板

    #define AC_COMPUTER_CENTRAL_WIDGET "" //计算机中心区域
    #define AC_DM_SPLITTER "" //隔离线
    #define AC_VIEW_MID_WIDGET "" //中间区域

    #define AC_VIEW_TAB_BAR "" //标签栏
    #define AC_VIEW_TAB_BAR_NEW_BUTTON "" //标签栏按钮
    #define AC_VIEW_TAB_BAR_TOP_LINE "" //标签栏上线
    #define AC_VIEW_TAB_BAR_BOTTOM_LINE "" //标签栏下线

    #define AC_CLOSE_ALL_DLG_INDICATOR ""
    #define AC_CLOSE_ALL_DLG_INDICATOR_MSG_LABEL ""
    #define AC_CLOSE_ALL_DLG_INDICATOR_CLOSE_BUTTON ""

    #define AC_TASK_DLG "" //任务对话框
    #define AC_TASK_DLG_TITLE_BAR "" //标题栏
    #define AC_TASK_DLG_TASK_LIST_WIDGET "" //任务列表面板
    #define AC_TASK_DLG_TASK_LIST_ITEM "" // 任务列表
    #define AC_TASK_DLG_TASK_JOB_ITEM "" //任务

    #define AC_FD_STATUS_BAR "" // 文件对话框状态栏
    #define AC_FD_STATUS_BAR_INTEL "" // 状态栏
    #define AC_FD_STATUS_BAR_FILE_NAME_EDIT "" //文件名
    #define AC_FD_STATUS_BAR_FILTERS "" //过滤
    #define AC_FD_STATUS_BAR_TITLE_LABEL "" //标题标签
    #define AC_FD_STATUS_BAR_TITLE_CONTENT_LABEL "" //内容标签
    #define AC_FD_STATUS_BAR_TITLE_CONTENT_EDIT "" //内容框
    #define AC_FD_STATUS_BAR_TITLE_CONTENT_BOX "" //内容列表

    #define AC_TAG_LABLE "" //tag 标签
    #define AC_TAG_LEFT_LABLE "" //tag 左标签
    #define AC_TAG_ACTION_WIDGET "" //tag 面板
    #define AC_TAG_ACTION_WIDGET_TOOL_TIP "" //tag 面板 提示
    #define AC_TAG_CRUMB_EDIT "" //tag 编辑区域

    #define AC_VAULT_ACTIVE_WIDGET                          ""  // 保险箱激活页面
    #define AC_VAULT_ACTIVE_STACK                           ""  // 保险箱激活 功能页面容器
    #define AC_VAULT_ACTIVE_START_WIDGET                    ""  // 保险箱激活 开始页面
    #define AC_VAULT_ACTIVE_START_TITLE_LABEL               ""  // 保险箱激活 开始页面 标题
    #define AC_VAULT_ACTIVE_START_CONTENT_LABEL             ""  // 保险箱激活 开始页面 内容
    #define AC_VAULT_ACTIVE_START_PICTURE                   ""  // 保险箱激活 开始页面 图片
    #define AC_VAULT_ACTIVE_START_BUTTON                    ""  // 保险箱激活 开始页面 开启按钮
    #define AC_VAULT_ACTIVE_SET_PASSWORD_WIDGET             ""  // 保险箱激活 设置密码页面
    #define AC_VAULT_ACTIVE_SET_PASSWORD_TITLE              ""  // 保险箱激活 设置密码页面 标题
    #define AC_VAULT_ACTIVE_SET_PASSWORD_TYPE_LABEL         ""  // 保险箱激活 设置密码页面 类型 标签
    #define AC_VAULT_ACTIVE_SET_PASSWORD_TYPE_COMBOX        ""  // 保险箱激活 设置密码页面 类型 组合框
    #define AC_VAULT_ACTIVE_SET_PASSWORD_PASSWORD_LABEL     ""  // 保险箱激活 设置密码页面 设置密码 标签
    #define AC_VAULT_ACTIVE_SET_PASSWORD_PASSWORD_EDIT      ""  // 保险箱激活 设置密码页面 设置密码 编辑框
    #define AC_VAULT_ACTIVE_SET_PASSWORD_REPASSWORD_LABEL   ""  // 保险箱激活 设置密码页面 重复密码 标签
    #define AC_VAULT_ACTIVE_SET_PASSWORD_REPASSWORD_EDIT    ""  // 保险箱激活 设置密码页面 重复密码 编辑框
    #define AC_VAULT_ACTIVE_SET_PASSWORD_HINT_LABEL         ""  // 保险箱激活 设置密码页面 提示 标签
    #define AC_VAULT_ACTIVE_SET_PASSWORD_HINT_EDIT          ""  // 保险箱激活 设置密码页面 提示 编辑框
    #define AC_VAULT_ACTIVE_SET_PASSWORD_NEXT_BUTTON        ""  // 保险箱激活 设置密码页面 下一步 按钮
    #define AC_VAULT_ACTIVE_KEY_WIDGET                      ""  // 保险箱激活 密钥页面
    #define AC_VAULT_ACTIVE_KEY_TITLE                       ""  // 保险箱激活 密钥页面 标题
    #define AC_VAULT_ACTIVE_KEY_CONTENT                     ""  // 保险箱激活 密钥页面 内容
    #define AC_VAULT_ACTIVE_KEY_KEY_BUTTON                  ""  // 保险箱激活 密钥页面 密钥 按钮
    #define AC_VAULT_ACTIVE_KEY_KEY_EDIT                    ""  // 保险箱激活 密钥页面 密钥 编辑器框
    #define AC_VAULT_ACTIVE_KEY_QRCODE_BUTTON               ""  // 保险箱激活 密钥页面 二维码 按钮
    #define AC_VAULT_ACTIVE_KEY_QRCODE_IMAGE                ""  // 保险箱激活 密钥页面 二维码 图片
    #define AC_VAULT_ACTIVE_KEY_QRCODE_HINT                 ""  // 保险箱激活 密钥页面 二维码 提示
    #define AC_VAULT_ACTIVE_KEY_NEXT_BUTTON                 ""  // 保险箱激活 密钥页面 下一步 按钮
    #define AC_VAULT_ACTIVE_ENCRYPT_WIDGET                  ""  // 保险箱激活 加密页面
    #define AC_VAULT_ACTIVE_ENCRYPT_TITLE                   ""  // 保险箱激活 加密页面 标题
    #define AC_VAULT_ACTIVE_ENCRYPT_CONTENT                 ""  // 保险箱激活 加密页面 内容
    #define AC_VAULT_ACTIVE_ENCRYPT_IMAGE                   ""  // 保险箱激活 加密页面 图片
    #define AC_VAULT_ACTIVE_ENCRYPT_PROGRESS                ""  // 保险箱激活 加密页面 进度条
    #define AC_VAULT_ACTIVE_ENCRYPT_PROGRESS_HINT           ""  // 保险箱激活 加密页面 进度条提示
    #define AC_VAULT_ACTIVE_ENCRYPT_FINISH_IMAGE            ""  // 保险箱激活 加密页面 加密完成图片
    #define AC_VAULT_ACTIVE_ENCRYPT_FINISH_HINT             ""  // 保险箱激活 加密页面 加密完成提示
    #define AC_VAULT_ACTIVE_ENCRYPT_BUTTON                  ""  // 保险箱激活 加密页面 按钮
    #define AC_VAULT_PASSWORD_UNLOCK_WIDGET                 ""  // 保险箱解锁 密码解锁页面
    #define AC_VAULT_PASSWORD_UNLOCK_TITLE                  ""  // 保险箱解锁 密码解锁页面 标题
    #define AC_VAULT_PASSWORD_UNLOCK_CONTENT                ""  // 保险箱解锁 密码解锁页面 内容
    #define AC_VAULT_PASSWORD_UNLOCK_EDIT                   ""  // 保险箱解锁 密码解锁页面 编辑框
    #define AC_VAULT_PASSWORD_UNLOCK_HINT_BUTTON            ""  // 保险箱解锁 密码解锁页面 密码提示按钮
    #define AC_VAULT_PASSWORD_UNLOCK_CANCEL_BUTTON          ""  // 保险箱解锁 密码解锁页面 取消按钮
    #define AC_VAULT_PASSWORD_UNLOCK_OK_BUTTON              ""  // 保险箱解锁 密码解锁页面 确认按钮
    #define AC_VAULT_KEY_UNLOCK_WIDGET                      ""  // 保险箱解锁 密钥解锁页面
    #define AC_VAULT_KEY_UNLOCK_TITLE                       ""  // 保险箱解锁 密钥解锁页面 标题
    #define AC_VAULT_KEY_UNLOCK_EDIT                        ""  // 保险箱解锁 密钥解锁页面 编辑框
    #define AC_VAULT_KEY_UNLOCK_CANCEL_BUTTON               ""  // 保险箱解锁 密钥解锁页面 取消按钮
    #define AC_VAULT_KEY_UNLOCK_OK_BUTTON                   ""  // 保险箱解锁 密钥解锁页面 确认按钮
    #define AC_VAULT_DELETE_WIDGET                          ""  // 保险箱删除 删除页面
    #define AC_VAULT_DELETE_TITLE                           ""  // 保险箱删除 删除页面 标题
    #define AC_VAULT_DELETE_CONTENT                         ""  // 保险箱删除 删除页面 内容
    #define AC_VAULT_DELETE_CANCEL_BUTTON                   ""  // 保险箱删除 删除页面 取消按钮
    #define AC_VAULT_DELETE_CHANGE_BUTTON                   ""  // 保险箱删除 删除页面 切换按钮
    #define AC_VAULT_DELETE_DELETE_BUTTON                   ""  // 保险箱删除 删除页面 删除按钮
    #define AC_VAULT_DELETE_PASSWORD_EDIT                   ""  // 保险箱删除 删除页面 密码编辑框
    #define AC_VAULT_DELETE_HINT_BUTTON                     ""  // 保险箱删除 删除页面 密码提示按钮
    #define AC_VAULT_DELETE_KEY_EDIT                        ""  // 保险箱删除 删除页面 密钥编辑框
    #define AC_VAULT_DELETE_PROGRESS                        ""  // 保险箱删除 删除页面 删除操作进度条

#endif // #ifdef ENABLE_ACCESSIBILITY

#endif // LIB_FRAME_ACCESSIBLE_DEFINE_H
