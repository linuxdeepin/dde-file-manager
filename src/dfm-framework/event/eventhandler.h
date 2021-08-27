#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "event.h"

#include "dfm-framework/log/frameworklog.h"
#include "dfm-framework/log/codetimecheck.h"

#include "dfm-framework/dfm_framework_global.h"

#include <QObject>
#include <QtConcurrent>

DPF_BEGIN_NAMESPACE

class EventHandlerPrivate;

/**
 * @brief The EventHandler class
 *  事件的处理类型基类，此类为事件驱动，
 *  禁止通过public等接口进行调用，防止逻辑关联顺序的错误。
 */
class EventHandler : public QObject
{
    Q_OBJECT
    QSharedPointer<const EventHandlerPrivate> d;
public:
    enum Type
    {
        Synch,
        Asynch
    };

    /**
     * 删除默认构造、拷贝构造、移动拷贝构造以防止
     * 1.未显式指定Type时，处理过程的未定义的行为
     * 2.拷贝浅拷贝等诱发的崩溃问题
     * 3.代码块边界与线程同步问题(特别是Qt线程机制)
     */
    EventHandler(const EventHandler &) = delete;
    EventHandler(const EventHandler &&) = delete;

    /**
     * @brief EventHandler 构造函数
     */
    explicit EventHandler();

    /**
     * @brief processType 必须重载实现的获取Handler的处理类型
     * @return 处理的类型
     *  Type::Synch 表示同步
     *  Type::Asynch 表示异步
     */
    virtual Type processType() = 0;

    /**
     * @brief eventProcess 事件处理入口
     */
    virtual void eventProcess(const Event&) = 0;

Q_SIGNALS:
    void handError(const QString &error);
    void handInfo(const QString &info);
};

/**
 * @brief The SynchEventHandler class
 *  同步处理类，禁止多重继承（二义性互斥），该类禁止更改
 */
class SynchEventHandler : virtual public EventHandler
{
    Q_OBJECT
public:
    virtual Type processType() override
    {
        return EventHandler::Type::Synch;
    }
    /**
     * @brief eventProcess
     *  该函数将遵循回调函数进行调用
     * @param event
     */
    virtual void eventProcess(const Event &event) override
    {
        Q_UNUSED(event);
        // 内部可存在异步线程进行同步处理提速
        // QtConcurrent::map等
        // auto controller = QtConcurrent::run([=](){
        //      出现处理错误 可使用 emit handError("what?");
        //      外抛信息可使用 emit handInfo("");
        //      dpfCritical() << event;
        // });
        // 阻塞同步，否则将导致同步未定义行为
        // controller.waitForFinished();
    }
};

/**
 * @brief The SynchEventHandler class
 *  异步同步处理类，禁止多重继承（二义性互斥），该类禁止更改
 */
class AsynchEventHandler : virtual public EventHandler
{
    Q_OBJECT
public:
    virtual Type processType() override
    {
        return EventHandler::Type::Asynch;
    }

    /**
     * @brief eventProcess
     *  该函数将被放在异步线程中抛出
     *  阻塞主界面的耗时操作
     * @param event 传递的事件
     */
    virtual void eventProcess(const Event &event) override
    {
        Q_UNUSED(event);
        //禁用QWidget树族在该函数中操作
    }
};

DPF_END_NAMESPACE

#endif // EVENTHANDLER_H
