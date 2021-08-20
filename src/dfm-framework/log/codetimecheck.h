#ifndef CODETIMECHECK_H
#define CODETIMECHECK_H

#include "dfm-framework/definitions/globaldefinitions.h"

#ifndef DPF_NO_CHECK_TIME //make use

#include <QMessageLogContext>

DPF_BEGIN_NAMESPACE
/**
 * @brief The CodeCheckTime class
 *  代码埋点时间检查模块，可加编译参数进行屏蔽
 *  DPF_NO_CHECK_TIME (cmake -DDPF_NO_CHECK_TIME)
 */
class CodeCheckTime
{
public:
    explicit CodeCheckTime() = delete;

    /**
     * @brief setLogCacheDayCount 设置日志缓存时间
     *  需要在调用其他函数之前调用
     * @param dayCount 日志缓存时间
     */
    static void setLogCacheDayCount(uint dayCount);

    /**
     * @brief logCacheDayCount 获取设置的日志缓存时间
     * @return uint 日志缓存时间，默认7天
     */
    static uint logCacheDayCount();

    /**
     * @brief begin 检查点-开始
     * @param context 日志打印上下文，可参照QMessageLogContext
     */
    static void begin(const QMessageLogContext &context);

    /**
     * @brief end 检查点-结束
     * @param context 日志打印上下文，可参照QMessageLogContext
     */
    static void end(const QMessageLogContext &context);
};

DPF_END_NAMESPACE

// QMessageLogContext类型的上下文构造
#define CodeCheckLogContext {__FILE__,__LINE__,__FUNCTION__,"TimeCheck"}
// 检查点Begin的宏定义，可在任意执行代码块中使用
#define dpfCheckTimeBegin() dpf::CodeCheckTime::begin(CodeCheckLogContext)
// 检查点End的宏定义，可在任意执行代码块中使用
#define dpfCheckTimeEnd() dpf::CodeCheckTime::end(CodeCheckLogContext)
#else // define DPF_NO_CHECK_TIME
// 检查点Begin的宏定义，可在任意执行代码块中使用
#define dpfCheckTimeBegin()
// 检查点End的宏定义，可在任意执行代码块中使用
#define dpfCheckTimeEnd()
#endif // DPF_NO_CHECK_TIME

#endif // CODETIMECHECK_H
