#ifndef FRAMEWORKLOG_H
#define FRAMEWORKLOG_H

#include "dfm-framework/definitions/globaldefinitions.h"

#include "dfm-framework/log/codetimecheck.h"

#include <QDebug>
#include <QLoggingCategory>

/**
 * @brief Framework 可进行相关调用，如下
 * @code
 * qCDebug(Framework) << "hello";    //调试信息打印
 * qCDInfo(Framework) << "hello";    //信息打印
 * qCDWarning(Framework) << "hello";     //警告打印
 * qCCritical(Framework) << "hello";     //关键日志打印
 * @endcode
 */
Q_DECLARE_LOGGING_CATEGORY(Framework)

/**
 * @brief FrameworkLog 宏函数，可进行相关打印调用，如下
 * @code
 * dpfDebug() << "hello";    //调试信息打印
 * dpfInfo() << "hello";    //信息打印
 * dpfWarning() << "hello";     //警告打印
 * dpfCritical() << "hello";     //关键日志打印
 * @endcode
 */
#define dpfDebug() qCDebug(Framework)
#define dpfInfo() qCDInfo(Framework)
#define dpfWarning() qCDWarning(Framework)
#define dpfCritical() qCCritical(Framework)

DPF_BEGIN_NAMESPACE

/**
 * @brief The FrameworkLog class
 *  框架日志打印模块，内部封装输出重定向与日志格式化
 */
class FrameworkLog
{

public:
    explicit FrameworkLog() = delete;

    /**
     * @brief enableFrameworkLog 开启框架日志打印
     * @param enabled true为开启,false则关闭
     */
    static void enableFrameworkLog(bool enabled = true);

    /**
     * @brief setLogCacheDayCount 设置日志缓存时间，
     *  需要在调用其他函数之前调用
     * @param uint 缓存的天数
     */
    static void setLogCacheDayCount(uint dayCount);

    /**
     * @brief logCacheDayCount 获取设置的日志缓存时间
     * @return uint 缓存的天数,默认缓存7天
     */
    static uint logCacheDayCount();

    /**
     * @brief initialize 初始化框架日志打印模块
     */
    static void initialize();
};

DPF_END_NAMESPACE

#endif // FRAMEWORKLOG_H
