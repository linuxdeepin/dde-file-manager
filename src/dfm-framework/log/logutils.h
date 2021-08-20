#ifndef LOGUTILS_H
#define LOGUTILS_H

#include "dfm-framework/definitions/globaldefinitions.h"

#include <QString>
#include <QDateTime>

DPF_BEGIN_NAMESPACE

class LogUtils
{
public:
    explicit LogUtils() = delete;

    /**
     * @brief checkAppCacheLogDir
     *  检查应用程序缓存日志的文件夹
     * @param subDirName 日志目录下的子目录
     *  默认为空，则检查顶层目录
     */
    static void checkAppCacheLogDir(const QString &subDirName = "");

    /**
     * @brief appCacheLogPath
     *  获取当前应用程序的缓存日志路径，
     * @return QString 返回的结果总是一个Dir类型的字符路径
     */
    static QString appCacheLogPath();

    /**
     * @brief localDateTime 获取年/月/日/时间
     * @return QString 格式化字符串后的时间
     */
    static QString localDateTime();

    /**
     * @brief localDate 获取年/月/日
     * @return QString 格式化字符串后的时间
     */
    static QString localDate();

    /**
     * @brief localDate 获取年/月/日/时间，
     *  年/月/日/与时间之间以逗号分割
     * @return QString 格式化字符串后的时间
     */
    static QString localDataTimeCSV();

    /**
     * @brief lastTimeStamp 获取输入时间之前指定天数时间戳，
     * 最小单位s
     * @param dateTime 时间
     * @param dayCount 向前的天数
     * @return uint 时间戳
     */
    static uint lastTimeStamp(const QDateTime &dateTime, uint dayCount);

    /**
     * @brief lastDateTime 获取输入时间之前指定天数时间，
     * 最小单位s
     * @param dateTime 时间
     * @param dayCount 向前的天数
     * @return QDateTime 时间
     */
    static QDateTime lastDateTime(const QDateTime &dateTime, uint dayCount);

    /**
     * @brief containLastDay 判断时间是否包含时间(天)范围
     * 最小单位s
     * @param src 基准时间
     * @param dst 对比时间
     * @param dayCount 往前推的天数
     * @return bool 是否包含的结果
     *
     * |------dst--------src----|
     * |------dayCount----|
     * return true;
     *
     * |-----------dst-------------src|
     *                  |-dayCount--|
     * return false
     */
    static bool containLastDay(const QDateTime &src, const QDateTime &dst, uint dayCount);

    /**
     * @brief toDayZero 获取今天的00:00:00的时间
     * @return
     */
    static QDateTime toDayZero();

};

DPF_END_NAMESPACE

#endif // LOGUTILS_H
