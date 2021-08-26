#ifndef EVENTCALLPROXY_H
#define EVENTCALLPROXY_H

#include "event.h"
#include "eventhandler.h"

#include "dfm-framework/log/frameworklog.h"

#include <QObject>
#include <QDebug>
#include <QHash>
#include <functional>

DPF_BEGIN_NAMESPACE

class EventCallProxy final
{
    struct HandlerInfo;
    typedef std::function<EventHandler*()> CreateFunc;
    typedef std::function<void(const HandlerInfo &info, const Event &event)> FilterExportFunc;

    struct HandlerInfo
    {
        QString className; //类名称
        EventHandler* ins; //实例
        QStringList topics; //订阅主题列表
        FilterExportFunc process; //过滤转发处理函数
    };

    //声明即定义
    inline static QList<HandlerInfo> eventHandlers{};

public:

    virtual ~EventCallProxy();

    template<class CT>
    static CT* regEventHandler(const QString& className, const QStringList &topics)
    {
        CT *ins = new CT();

        FilterExportFunc filterEventFunc = nullptr;
        //如果为异步构造类
        if (ins->processType() == EventHandler::Asynch) {
            //过滤及异步执行逻辑
            filterEventFunc = [=](const HandlerInfo &info, const Event &event) {
                dpfCritical() << "Call handler asynch: " << info.className;
                if (!info.topics.contains(event.topic())) return;
                //显式调用
                QtConcurrent::run(dynamic_cast<AsynchEventHandler*>(info.ins),
                                  &AsynchEventHandler::eventProcess,
                                  event);
            };
            dpfCritical() << "Create asynch handler: " << className
                          << "and topics:" << topics;
        }

        //如果为同步构造类
        if (ins->processType() == EventHandler::Synch) {
            //过滤及同步回调逻辑
            filterEventFunc = [=](const HandlerInfo &info, const Event &event) {
                dpfCritical() << "Call handler synch: " << info.className;
                if (!info.topics.contains(event.topic())) return;
                info.ins->eventProcess(event);
            };
            dpfCritical() << "Create synch handler: " << className
                          << "and topics:" << topics;
        }
        eventHandlers.append(HandlerInfo{className, ins, topics, filterEventFunc});
        return ins;
    }

    static void callEvent(const Event& event)
    {
        for(HandlerInfo &val : eventHandlers) {
            //发送端主题过滤
            if (!val.topics.contains(event.topic()))
                continue;
            if(val.process != nullptr)
                (val.process)(val,event);
        }
    }
};

DPF_END_NAMESPACE

/**
 * @brief DPF_EVENT_HANDLER 事件处理类声明宏
 * @code
 *  class MyEventHandler: public SynchEventHandler
 *  {
 *      ...
 *  }; DPF_EVENT_HANDLER(MyEventHandler,"WindowEvent","LauchEvent","FileEvent")
 *
 * @endcode
 * @param x 事件处理实现类
 * @param ... topics,可传递多个订阅主题
 */
#define DPF_EVENT_HANDLER(x,...) \
    DPF_BEGIN_NAMESPACE \
    namespace GlobalPrivate { \
    static x* x##_regInstence = EventCallProxy::regEventHandler<x>(#x,{__VA_ARGS__}); \
    } \
    DPF_END_NAMESPACE \

/**
 * @brief DPF_EVENT_CALL 事件传递调用
 * @param event 构造的事件传递数据包
 */
#define DPF_EVENT_CALL(event) \
    EventCallProxy::callEvent(event);

#endif // EVENTCALLPROXY_H
