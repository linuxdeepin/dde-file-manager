#ifndef FRAMEWORKLOG_H
#define FRAMEWORKLOG_H

#include "dfm-framework/definitions/globaldefinitions.h"

#include <QDebug>
#include <QLoggingCategory>

/**
 * @brief FrameworkLog 可进行相关调用，如下
 * @code
 * qCDebug(FrameworkLog) << "hello";    //调试信息打印
 * qCDInfo(FrameworkLog) << "hello";    //信息打印
 * qCDWarning(FrameworkLog) << "hello";     //警告打印
 * qCCritical(FrameworkLog) << "hello";     //关键日志打印
 * @endcode
 */
Q_DECLARE_LOGGING_CATEGORY(FrameworkLog)
Q_DECLARE_LOGGING_CATEGORY(TimeCheck)

#define qPointCheckTime() qCCritical(TimeCheck) << localDateTime() << __FUNCTION__ << "end";

DPF_BEGIN_NAMESPACE

/**
 * @brief enableFrameworkLog 开启框架日志打印
 * @param enabled true为开启,false则关闭
 */
extern void enableFrameworkLog(bool enabled = true);

/**
 * @brief enableTimeCheck 开启框架时间检查
 * @param enabled true为开启,false则为关闭
 */
extern void enableTimeCheck(bool enabled = true);

/**
 * @brief localDataTime 获取本地事件
 * @return QString 格式化时间字符串
 */
extern QString localDateTime();

DPF_END_NAMESPACE

#endif // FRAMEWORKLOG_H
