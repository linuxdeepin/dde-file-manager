/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

    #define AC_FILE_MENU_DEFAULT                            "default_file_menu" //默认菜单
    #define AC_FILE_MENU_DEFAULT_SIDEBAR                    "default_sidebar_menu" //侧边栏默认菜单
    #define AC_FILE_MENU_SIDEBAR_TAGITEM                    "sidebar_tagitem_menu" //侧边栏标签菜单
    #define AC_FILE_MENU_SIDEBAR_DEVICE_ITEM                "sidebar_deviceitem_menu"//侧边栏设备菜单
    #define AC_FILE_MENU_SIDEBAR_VAULT_ITEM                 "sidebar_vaultitem_menu"//侧边栏保险箱菜单
    #define AC_FILE_MENU_SIDEBAR_BOOK_MARK                  "sidebar_bookmark_menu" //侧边栏书签菜单
    #define AC_FILE_MENU_USER_SHARE                         "user_share_menu" //用户共享菜单
    #define AC_FILE_MENU_TOOLBAR_SEETINGS                   "toolbar_seetings_menu"//工具栏菜单
    #define AC_FILE_MENU_VAULT                              "vault_menu" //保险箱菜单
    #define AC_FILE_MENU_FILEVIEW                           "fileview_menu"//视图菜单
    #define AC_FILE_MENU_CRUMB_BAR                          "crumb_bar_menu"//面包屑菜单

    #define AC_COMPUTER_VIEW                                "computer_view" // 计算视图
    #define AC_COMPUTER_LIST_VIEW                           "computer_list_view"//计算机列表视图
    #define AC_COMPUTER_MIAN_WINDOW                         "computer_window"//计算机窗口

    #define AC_COMPUTER_TITLE_BAR                           "computer_title_bar"//计算机标题栏
    #define AC_COMPUTER_CUSTOM_TITLE_BAR                    "custom_title_bar"//自定义标题栏
    #define AC_COMPUTER_TITLE_BAR_ADRESS                    "AddressToolBar"//地址输入框
    #define AC_COMPUTER_TITLE_BAR_BACK_BTN                  "back_button"//回退按钮
    #define AC_COMPUTER_TITLE_BAR_FORWARD_BTN               "forword_button" //前进按钮
    #define AC_COMPUTER_TITLE_BAR_BTN_BOX                   "button_box" //按钮区域
    #define AC_COMPUTER_TITLE_BAR_SERACH_BTN                "search_button" //查询按钮
    #define AC_COMPUTER_TITLE_BAR_DETAIL_BTN                "detail_button" //详细按钮
    #define AC_COMPUTER_TITLE_BAR_CONTOLLER_TOOL_BAR        "ContollerToolBar" //控制区域
    #define AC_COMPUTER_TITLE_BAR_CRUMB_SERACH_FRAME        "crumb_search_frame" //面包屑区域

    #define AC_COMPUTER_STATUS_BAR                          "status_bar" //计算机状态栏
    #define AC_COMPUTER_STATUS_BAR_LOADING_INDICATOR        "loadingIndicator" //加载指示器
    #define AC_COMPUTER_STATUS_BAR_SCALE_SLIDER             "scale_slider"
    #define AC_COMPUTER_STATUS_BAR_ACCEPT_BUTTON            "accept_button" //接收按钮
    #define AC_COMPUTER_STATUS_BAR_REJECT_BUTTON            "reject_button" // 拒绝按钮
    #define AC_COMPUTER_STATUS_BAR_LINE_EDIT                "line_edit" //编辑区域

    #define AC_COMPUTER_RENAME_BAR                          "rename_bar" //重命名
    #define AC_COMPUTER_RENAME_BAR_SELECT_TYPE              "rename_types" //重命名类型
    #define AC_COMPUTER_RENAME_BAR_STACK_WIDGET             "stack_widget" //栈面板

    #define AC_COMPUTER_CRUMB_BAR                           "crumb_bar" //面包屑指示栏
    #define AC_COMPUTER_CRUMB_BAR_LEFT_ARROW                "left_arrow" //左箭头
    #define AC_COMPUTER_CRUMB_BAR_RIGHT_ARROW               "right_arrow" //右箭头
    #define AC_COMPUTER_CRUMB_BAR_LIST_VIEW                 "crumb_list_view" //列表
    #define AC_COMPUTER_CRUMB_BAR_SCROLL_BAR                "scroll_bar" //滑动模块

    #define AC_COMPUTER_COMPLETER_LIST_VIEW                 "CompleterListView"

    #define AC_FILE_VIEW                                    "file_view" //文件视图
    #define AC_FM_VIEW_MANAGER                              "fm_view_manager" //文件视图管理
    #define AC_DM_SIDE_BAR                                  "left_side_bar" //左边栏
    #define AC_DM_SIDE_BAR_VIEW                             "side_bar_view" //左边栏视图
    #define AC_DM_TOOLBAR                                   "DM_ToolBar" //工具栏
    #define AC_DM_RIGHT_VIEW                                "right_view" // 右边栏
    #define AC_DM_RIGHT_VIEW_TRASH_LABEL                    "trash_label" //回收站标签
    #define AC_DM_RIGHT_VIEW_TRASH_HOLDER                   "trash_holder" //回收站
    #define AC_DM_RIGHT_VIEW_TRASH_SPLIT_LINE               "trash_split_line" //回收站隔离线
    #define AC_DM_RIGHT_VIEW_EMPTY_TRASH_BUTTON             "EmptyTrashButton" //清空回收站
    #define AC_DM_RIGHT_VIEW_HOLDER                         "rightviewHolder" //右视图
    #define AC_DM_RIGHT_VIEW_DETAIL_VLINE                   "rightDetailVLine" //右视图隔离线
    #define AC_DM_RIGHT_VIEW_DETAIL_VIEW                    "right-detail-view" //有视图详情
    #define AC_DM_RIGHT_VIEW_MAIN_FRAME                     "main_frame" //主窗口
    #define AC_DM_RIGHT_VIEW_MAIN_FRAME_ICON_LABEL          "frame_icon"//主窗口图标
    #define AC_DM_RIGHT_VIEW_MAIN_FRAME_LINE                "frame_line" //主窗口隔离线
    #define AC_DM_RIGHT_VIEW_MAIN_FRAME_SCROLLAREA          "DFMRightDetailView-QScrollArea" //滑动区域

    #define AC_TAG_WIDGET                                   "tag-widget" //标签面板

    #define AC_COMPUTER_CENTRAL_WIDGET                      "CentralWidget" //计算机中心区域
    #define AC_DM_SPLITTER                                  "dm_splitter" //隔离线
    #define AC_VIEW_MID_WIDGET                              "mid_wiget" //中间区域

    #define AC_VIEW_TAB_BAR                                 "TabBar" //标签栏
    #define AC_VIEW_TAB_BAR_NEW_BUTTON                      "NewTabButton" //标签栏按钮
    #define AC_VIEW_TAB_BAR_TOP_LINE                        "top_line" //标签栏上线
    #define AC_VIEW_TAB_BAR_BOTTOM_LINE                     "bottom_line" //标签栏下线

    #define AC_CLOSE_ALL_DLG_INDICATOR                      "dialogs_close_all_dialog_indicator"
    #define AC_CLOSE_ALL_DLG_INDICATOR_MSG_LABEL            "dialog_indicator_message_label"
    #define AC_CLOSE_ALL_DLG_INDICATOR_CLOSE_BUTTON         "dialog_indicator_message_all_close_button"

    #define AC_TASK_DLG                                     "d_task_dialog" //任务对话框
    #define AC_TASK_DLG_TITLE_BAR                           "d_task_dialog_title_bars" //标题栏
    #define AC_TASK_DLG_TASK_LIST_WIDGET                    "task_dialog_task_list_widget" //任务列表面板
    #define AC_TASK_DLG_TASK_LIST_ITEM                      "task_dialog_task_list_item" // 任务列表
    #define AC_TASK_DLG_TASK_JOB_ITEM                       "task_dialog_task_job_item" //任务

    #define AC_FD_STATUS_BAR                                "dlg_status_bar" // 文件对话框状态栏
    #define AC_FD_STATUS_BAR_INTEL                          "statusBar" // 状态栏
    #define AC_FD_STATUS_BAR_FILE_NAME_EDIT                 "file_name_edit" //文件名
    #define AC_FD_STATUS_BAR_FILTERS                        "filters" //过滤
    #define AC_FD_STATUS_BAR_TITLE_LABEL                    "title_label" //标题标签
    #define AC_FD_STATUS_BAR_TITLE_CONTENT_LABEL            "content_label" //内容标签
    #define AC_FD_STATUS_BAR_TITLE_CONTENT_EDIT             "content_edit" //内容框
    #define AC_FD_STATUS_BAR_TITLE_CONTENT_BOX              "content_box" //内容列表

    #define AC_TAG_LABLE                                    "tag_label" //tag 标签
    #define AC_TAG_LEFT_LABLE                               "tag_left_label" //tag 左标签
    #define AC_TAG_ACTION_WIDGET                            "tag_action_widget" //tag 面板
    #define AC_TAG_ACTION_WIDGET_TOOL_TIP                   "tag_action_tool_tip" //tag 面板 提示
    #define AC_TAG_CRUMB_EDIT                               "tag_crumb_edit" //tag 编辑区域

    #define AC_VAULT_ACTIVE_WIDGET                          "vault_active_widget"          // 保险箱激活页面
    #define AC_VAULT_ACTIVE_STACK                           "vault_active_stack"           // 保险箱激活 功能页面容器
    #define AC_VAULT_ACTIVE_START_WIDGET                    "vault_active_start_widget"    // 保险箱激活 开始页面
    #define AC_VAULT_ACTIVE_START_TITLE_LABEL               "vault_active_start_title"     // 保险箱激活 开始页面 标题
    #define AC_VAULT_ACTIVE_START_CONTENT_LABEL             "vault_active_start_content"   // 保险箱激活 开始页面 内容
    #define AC_VAULT_ACTIVE_START_PICTURE                   "vault_active_start_picture"   // 保险箱激活 开始页面 图片
    #define AC_VAULT_ACTIVE_START_BUTTON                    "vault_active_start_button"    // 保险箱激活 开始页面 开启按钮
    #define AC_VAULT_ACTIVE_SET_PASSWORD_WIDGET             "vault_active_set_password_widget"             // 保险箱激活 设置密码页面
    #define AC_VAULT_ACTIVE_SET_PASSWORD_TITLE              "vault_active_set_password_title"              // 保险箱激活 设置密码页面 标题
    #define AC_VAULT_ACTIVE_SET_PASSWORD_TYPE_LABEL         "vault_active_set_password_type_label"         // 保险箱激活 设置密码页面 类型 标签
    #define AC_VAULT_ACTIVE_SET_PASSWORD_TYPE_COMBOX        "vault_active_set_password_type_combox"        // 保险箱激活 设置密码页面 类型 组合框
    #define AC_VAULT_ACTIVE_SET_PASSWORD_PASSWORD_LABEL     "vault_active_set_password_password_label"     // 保险箱激活 设置密码页面 设置密码 标签
    #define AC_VAULT_ACTIVE_SET_PASSWORD_PASSWORD_EDIT      "vault_active_set_password_password_edit"      // 保险箱激活 设置密码页面 设置密码 编辑框
    #define AC_VAULT_ACTIVE_SET_PASSWORD_REPASSWORD_LABEL   "vault_active_set_password_repassword_lable"   // 保险箱激活 设置密码页面 重复密码 标签
    #define AC_VAULT_ACTIVE_SET_PASSWORD_REPASSWORD_EDIT    "vault_active_set_password_repassword_edit"    // 保险箱激活 设置密码页面 重复密码 编辑框
    #define AC_VAULT_ACTIVE_SET_PASSWORD_HINT_LABEL         "vault_active_set_password_hint_label"         // 保险箱激活 设置密码页面 提示 标签
    #define AC_VAULT_ACTIVE_SET_PASSWORD_HINT_EDIT          "vault_active_set_password_hint_edit"          // 保险箱激活 设置密码页面 提示 编辑框
    #define AC_VAULT_ACTIVE_SET_PASSWORD_NEXT_BUTTON        "vault_active_set_password_next_button"        // 保险箱激活 设置密码页面 下一步 按钮
    #define AC_VAULT_ACTIVE_KEY_WIDGET                      "vault_active_key_widget"          // 保险箱激活 密钥页面
    #define AC_VAULT_ACTIVE_KEY_TITLE                       "vault_active_key_title"           // 保险箱激活 密钥页面 标题
    #define AC_VAULT_ACTIVE_KEY_CONTENT                     "vault_active_key_content"         // 保险箱激活 密钥页面 内容
    #define AC_VAULT_ACTIVE_KEY_KEY_BUTTON                  "vault_active_key_key_button"      // 保险箱激活 密钥页面 密钥 按钮
    #define AC_VAULT_ACTIVE_KEY_KEY_EDIT                    "vault_active_key_key_edit"        // 保险箱激活 密钥页面 密钥 编辑器框
    #define AC_VAULT_ACTIVE_KEY_QRCODE_BUTTON               "vault_active_key_qrcode_button"   // 保险箱激活 密钥页面 二维码 按钮
    #define AC_VAULT_ACTIVE_KEY_QRCODE_IMAGE                "vault_active_key_qrcode_image"    // 保险箱激活 密钥页面 二维码 图片
    #define AC_VAULT_ACTIVE_KEY_QRCODE_HINT                 "vault_active_key_qrcode_hint"     // 保险箱激活 密钥页面 二维码 提示
    #define AC_VAULT_ACTIVE_KEY_NEXT_BUTTON                 "vault_active_key_next_button"     // 保险箱激活 密钥页面 下一步 按钮
    #define AC_VAULT_DEFAULT_PATH_RADIOBTN                  "vault_default_path_radiobtn"      // 保险箱激活 密钥保存页面 默认保存路径单选按钮
    #define AC_VAULT_OTHER_PATH_RADIOBTN                    "vault_other_path_radiobtn"        // 保险箱激活 密钥保存页面 用户自定义保存路径单选按钮
    #define AC_VAULT_SELECT_FILE_SAVE_PATH_EDIT             "vault_select_file_save_path_edit" // 保险箱激活 密钥保存页面 用户选择保存的路径编辑框
    #define AC_VAULT_ACTIVE_ENCRYPT_WIDGET                  "vault_active_encrypy_widget"          // 保险箱激活 加密页面
    #define AC_VAULT_ACTIVE_ENCRYPT_TITLE                   "vault_active_encrypy_title"           // 保险箱激活 加密页面 标题
    #define AC_VAULT_ACTIVE_ENCRYPT_CONTENT                 "vault_active_encrypy_content"         // 保险箱激活 加密页面 内容
    #define AC_VAULT_ACTIVE_ENCRYPT_IMAGE                   "vault_active_encrypy_image"           // 保险箱激活 加密页面 图片
    #define AC_VAULT_ACTIVE_ENCRYPT_PROGRESS                "vault_active_encrypy_progress"        // 保险箱激活 加密页面 进度条
    #define AC_VAULT_ACTIVE_ENCRYPT_PROGRESS_HINT           "vault_active_encrypy_progress_hint"   // 保险箱激活 加密页面 进度条提示
    #define AC_VAULT_ACTIVE_ENCRYPT_FINISH_IMAGE            "vault_active_encrypy_finish_image"    // 保险箱激活 加密页面 加密完成图片
    #define AC_VAULT_ACTIVE_ENCRYPT_FINISH_HINT             "vault_active_encrypy_finish_hint"     // 保险箱激活 加密页面 加密完成提示
    #define AC_VAULT_ACTIVE_ENCRYPT_BUTTON                  "vault_active_encrypy_button"          // 保险箱激活 加密页面 按钮
    #define AC_VAULT_PASSWORD_UNLOCK_WIDGET                 "vault_password_unlock_widget"             // 保险箱解锁 密码解锁页面
    #define AC_VAULT_PASSWORD_UNLOCK_TITLE                  "vault_password_unlock_title"              // 保险箱解锁 密码解锁页面 标题
    #define AC_VAULT_PASSWORD_UNLOCK_CONTENT                "vault_password_unlock_content"            // 保险箱解锁 密码解锁页面 内容
    #define AC_VAULT_PASSWORD_UNLOCK_EDIT                   "vault_password_unlock_edit"               // 保险箱解锁 密码解锁页面 编辑框
    #define AC_VAULT_PASSWORD_UNLOCK_HINT_BUTTON            "vault_password_unlock_hint_button"        // 保险箱解锁 密码解锁页面 密码提示按钮
    #define AC_VAULT_PASSWORD_UNLOCK_FORGETPASSWORD_BUTTON  "vault_password_unlock_forgetpassword_button"  // 保险箱解锁 密码解锁页面 密码找回
    #define AC_VAULT_PASSWORD_UNLOCK_CANCEL_BUTTON          "vault_password_unlock_cancel_button"      // 保险箱解锁 密码解锁页面 取消按钮
    #define AC_VAULT_PASSWORD_UNLOCK_OK_BUTTON              "vault_password_unlock_ok_button"          // 保险箱解锁 密码解锁页面 确认按钮
    #define AC_VAULT_KEY_UNLOCK_WIDGET                      "vault_key_unlock_widget"          // 保险箱解锁 密钥解锁页面
    #define AC_VAULT_KEY_UNLOCK_TITLE                       "vault_key_unlock_title"           // 保险箱解锁 密钥解锁页面 标题
    #define AC_VAULT_KEY_UNLOCK_EDIT                        "vault_key_unlock_edit"            // 保险箱解锁 密钥解锁页面 编辑框
    #define AC_VAULT_KEY_UNLOCK_CANCEL_BUTTON               "vault_key_unlock_cancel_button"   // 保险箱解锁 密钥解锁页面 取消按钮
    #define AC_VAULT_KEY_UNLOCK_OK_BUTTON                   "vault_key_unlock_ok_button"       // 保险箱解锁 密钥解锁页面 确认按钮
    #define AC_VAULT_DELETE_WIDGET                          "vault_delete_widget"          // 保险箱删除 删除页面
    #define AC_VAULT_DELETE_TITLE                           "vault_delete_title"           // 保险箱删除 删除页面 标题
    #define AC_VAULT_DELETE_CONTENT                         "vault_delete_content"         // 保险箱删除 删除页面 内容
    #define AC_VAULT_DELETE_CANCEL_BUTTON                   "vault_delete_cancel_button"   // 保险箱删除 删除页面 取消按钮
    #define AC_VAULT_DELETE_CHANGE_BUTTON                   "vault_delete_change_button"   // 保险箱删除 删除页面 切换按钮
    #define AC_VAULT_DELETE_DELETE_BUTTON                   "vault_delete_delete_button"   // 保险箱删除 删除页面 删除按钮
    #define AC_VAULT_DELETE_PASSWORD_EDIT                   "vault_delete_password_edit"   // 保险箱删除 删除页面 密码编辑框
    #define AC_VAULT_DELETE_HINT_BUTTON                     "vault_delete_hint_button"     // 保险箱删除 删除页面 密码提示按钮
    #define AC_VAULT_DELETE_KEY_EDIT                        "vault_delete_key_edit"        // 保险箱删除 删除页面 密钥编辑框
    #define AC_VAULT_DELETE_PROGRESS                        "vault_delete_progress"        // 保险箱删除 删除页面 删除操作进度条
    #define AC_VAULT_SAVE_PATH_TYPE_COMBOBOX                "vault_save_path_type_combobox"     // 保险箱解锁 密码找回页面 密钥保存方式
    #define AC_VAULT_SAVE_PUBKEY_FILE_EDIT                  "vault_save_pubkey_file_edit"       // 保险箱解锁 密码找回页面 选择密钥文件路径
    #define AC_VAULT_RETURN_UNLOCK_PAGE_BTN                 "vault_return_unlock_page_btn"      // 保险箱解锁 密码找回页面 返回解锁页面
    #define AC_VAULT_VERIFY_PUKEY_FILE_BTN                  "vault_verify_pubkey_file_btn"      // 保险箱解锁 密码找回页面 验证密钥

#endif // LIB_FRAME_ACCESSIBLE_DEFINE_H
