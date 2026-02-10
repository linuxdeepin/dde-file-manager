// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_LOG_DEFINES_H
#define DFM_LOG_DEFINES_H

#include <QLoggingCategory>

// ====== Log API Statement ======
// e.g. DFM_REGISTER_LOG_CATEGORY(fmplugin-optical)
// Written in the plugin's meta cpp file
#define DFM_LOG_REGISTER_CATEGORY(name)
#define DFM_LOG_REISGER_CATEGORY(name)   // error typo: compat
// Written in the plugin's global header file
#define DFM_LOG_USE_CATEGORY(name)
// Use fmDebug() instead of qCDebug(categoty, ...)
#define fmDebug(...)
#define fmInfo(...)
#define fmWarning(...)
#define fmCritical(...)

// ====== Disbale qDebug(), qInfo(), qWaring(), qCritical() ======
// Use qCDebug() instead of qDebug()
#define DFM_NO_QDEBUG_MACRO \
    while (false)           \
    QMessageLogger().noDebug

#if defined(DFM_DISABLE_DEBUG_MACRO)
#    undef qDebug
#    define qDebug DFM_NO_QDEBUG_MACRO
#    undef qInfo
#    define qInfo DFM_NO_QDEBUG_MACRO
#    undef qWarning
#    define qWarning DFM_NO_QDEBUG_MACRO
#    undef qCritical
#    define qCritical DFM_NO_QDEBUG_MACRO
#endif

// ====== Log API Defines ======
// Wrap qCDebug for all plugins
// Note: Used in the plugin's namespace!!!
#undef DFM_LOG_REGISTER_CATEGORY
#define DFM_LOG_REGISTER_CATEGORY2(name)                                                    \
    const QLoggingCategory &__log##name()                                                   \
    {                                                                                       \
        static const QLoggingCategory category("org.deepin.dde.filemanager.plugin." #name); \
        return category;                                                                    \
    }
#define DFM_LOG_REGISTER_CATEGORY(nameMacro) DFM_LOG_REGISTER_CATEGORY2(nameMacro)

#undef DFM_LOG_REISGER_CATEGORY
#define DFM_LOG_REISGER_CATEGORY(nameMacro) DFM_LOG_REGISTER_CATEGORY(nameMacro)

// Note: Used in the plugin's namespace!!!
#undef DFM_LOG_USE_CATEGORY
#define DFM_LOG_USE_CATEGORY2(name)                       \
    extern const QLoggingCategory &__log##name();         \
    inline const QLoggingCategory &__getLogCategoryName() \
    {                                                     \
        return __log##name();                             \
    }
#define DFM_LOG_USE_CATEGORY(nameMacro) DFM_LOG_USE_CATEGORY2(nameMacro)

#undef fmDebug
#define fmDebug(...)                                                                                                           \
    for (bool qt_category_enabled = __getLogCategoryName().isDebugEnabled(); qt_category_enabled; qt_category_enabled = false) \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, __getLogCategoryName().categoryName()).debug(__VA_ARGS__)

#undef fmInfo
#define fmInfo(...)                                                                                                           \
    for (bool qt_category_enabled = __getLogCategoryName().isInfoEnabled(); qt_category_enabled; qt_category_enabled = false) \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, __getLogCategoryName().categoryName()).info(__VA_ARGS__)

#undef fmWarning
#define fmWarning(...)                                                                                                           \
    for (bool qt_category_enabled = __getLogCategoryName().isWarningEnabled(); qt_category_enabled; qt_category_enabled = false) \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, __getLogCategoryName().categoryName()).warning(__VA_ARGS__)

#undef fmCritical
#define fmCritical(...)                                                                                                           \
    for (bool qt_category_enabled = __getLogCategoryName().isCriticalEnabled(); qt_category_enabled; qt_category_enabled = false) \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, __getLogCategoryName().categoryName()).critical(__VA_ARGS__)

#endif   // DFM_LOG_DEFINES_H
