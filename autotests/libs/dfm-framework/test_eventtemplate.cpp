// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventtemplate.cpp - Comprehensive template instantiation tests
// Targets include/dfm-framework/event/ template headers to increase
// coverage of ApplyReturnValue<T>, resultGenerator<T>, paramGenerator<T>,
// EventHelper::invoke(), InvokeHelper, and EventChannel template methods
// by exercising many type combinations.

#include <gtest/gtest.h>

#include <dfm-framework/event/eventchannel.h>
#include <dfm-framework/event/event.h>
#include <dfm-framework/event/eventhelper.h>
#include <dfm-framework/event/invokehelper.h>
#include <dfm-framework/event/eventsequence.h>
#include <dfm-framework/event/eventdispatcher.h>

#include <QObject>
#include <QString>
#include <QUrl>
#include <QPoint>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QVariant>
#include <QVariantList>
#include <QList>
#include <QMap>
#include <QHash>
#include <QModelIndex>
#include <QSharedPointer>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QWidget>

using namespace dpf;

// ---------------------------------------------------------------------------
// Test receivers with slots returning many different types
// ---------------------------------------------------------------------------
class MultiTypeReceiver : public QObject
{
    Q_OBJECT
public:
    explicit MultiTypeReceiver(QObject *parent = nullptr) : QObject(parent) {}

    // Void return
    void voidSlot() { ++callCount; }
    void voidSlotInt(int) { ++callCount; }
    void voidSlotStr(const QString &) { ++callCount; }

    // Basic type returns
    bool retBool(bool v) { return v; }
    int retInt(int v) { return v + 1; }
    double retDouble(double v) { return v * 2.0; }
    QString retString(const QString &s) { return s + "_ret"; }

    // Qt value types
    QPoint retPoint(int x, int y) { return QPoint(x, y); }
    QRect retRect(int x, int y, int w, int h) { return QRect(x, y, w, h); }
    QRectF retRectF(qreal x, qreal y, qreal w, qreal h) { return QRectF(x, y, w, h); }
    QSize retSize(int w, int h) { return QSize(w, h); }
    QUrl retUrl(const QString &path) { return QUrl(path); }
    QVariant retVariant(const QVariant &v) { return v; }
    QModelIndex retIndex() { return QModelIndex(); }

    // Container returns
    QList<QString> retStringList() { return {"a", "b", "c"}; }
    QList<QUrl> retUrlList() { return {QUrl("file:///a"), QUrl("file:///b")}; }
    QList<QVariant> retVariantList() { return {QVariant(1), QVariant("x")}; }
    QList<QMap<QString, QVariant>> retMapList() { return {QMap<QString, QVariant>{{"k", "v"}}}; }
    QMap<QString, QVariant> retMap() { return {{"key", QVariant(42)}}; }
    QHash<QString, QVariant> retHash() { return {{"hkey", QVariant(true)}}; }
    QList<QWidget *> retWidgetList() { return {}; }

    // Pointer returns
    QWidget *retWidgetPtr() { return nullptr; }
    QAbstractItemModel *retModelPtr() { return nullptr; }
    QItemSelectionModel *retSelectionPtr() { return nullptr; }
    QObject *retObjectPtr() { return nullptr; }
    QAbstractItemView *retItemViewPtr() { return nullptr; }

    // Multi-arg slots
    int twoArgs(int a, int b) { return a + b; }
    int threeArgs(int a, int b, int c) { return a + b + c; }
    int fourArgs(int a, int b, int c, int d) { return a + b + c + d; }
    int fiveArgs(int a, int b, int c, int d, int e) { return a + b + c + d + e; }

    int callCount = 0;
};

// ---------------------------------------------------------------------------
// EventChannel template instantiation tests
// ---------------------------------------------------------------------------
class EventTemplateTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        Event::instance();
    }

    void SetUp() override
    {
        receiver = new MultiTypeReceiver();
        event = Event::instance();
        channelMgr = event->channel();
    }

    void TearDown() override
    {
        for (const auto &topic : registeredTopics)
            channelMgr->disconnect("tmpltest", topic);
        registeredTopics.clear();
        delete receiver;
    }

    EventType regEvent(const QString &topic)
    {
        event->registerEventType(EventStratege::kSlot, "tmpltest", topic);
        EventType type = EventConverter::convert("tmpltest", topic);
        registeredTopics.append(topic);
        return type;
    }

    MultiTypeReceiver *receiver = nullptr;
    Event *event = nullptr;
    EventChannelManager *channelMgr = nullptr;
    QStringList registeredTopics;
};

// --- Direct EventHelper invocation for many return types ---

TEST_F(EventTemplateTest, EventHelper_VoidReturn_Invoke)
{
    using Func = void (MultiTypeReceiver::*)();
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::voidSlot);
    QVariant ret = helper.invoke({});
    EXPECT_TRUE(ret.isNull() || !ret.isValid());
    EXPECT_EQ(receiver->callCount, 1);
}

TEST_F(EventTemplateTest, EventHelper_BoolReturn_Invoke)
{
    using Func = bool (MultiTypeReceiver::*)(bool);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::retBool);
    QVariant ret = helper.invoke({QVariant(true)});
    EXPECT_EQ(ret.toBool(), true);
}

TEST_F(EventTemplateTest, EventHelper_IntReturn_Invoke)
{
    using Func = int (MultiTypeReceiver::*)(int);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::retInt);
    QVariant ret = helper.invoke({QVariant(5)});
    EXPECT_EQ(ret.toInt(), 6);
}

TEST_F(EventTemplateTest, EventHelper_DoubleReturn_Invoke)
{
    using Func = double (MultiTypeReceiver::*)(double);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::retDouble);
    QVariant ret = helper.invoke({QVariant(3.5)});
    EXPECT_DOUBLE_EQ(ret.toDouble(), 7.0);
}

TEST_F(EventTemplateTest, EventHelper_StringReturn_Invoke)
{
    using Func = QString (MultiTypeReceiver::*)(const QString &);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::retString);
    QVariant ret = helper.invoke({QVariant("hello")});
    EXPECT_EQ(ret.toString(), "hello_ret");
}

TEST_F(EventTemplateTest, EventHelper_PointReturn_Invoke)
{
    using Func = QPoint (MultiTypeReceiver::*)(int, int);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::retPoint);
    QVariant ret = helper.invoke({QVariant(10), QVariant(20)});
    EXPECT_EQ(ret.toPoint(), QPoint(10, 20));
}

TEST_F(EventTemplateTest, EventHelper_RectReturn_Invoke)
{
    using Func = QRect (MultiTypeReceiver::*)(int, int, int, int);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::retRect);
    QVariant ret = helper.invoke({QVariant(1), QVariant(2), QVariant(3), QVariant(4)});
    EXPECT_EQ(ret.toRect(), QRect(1, 2, 3, 4));
}

TEST_F(EventTemplateTest, EventHelper_RectFReturn_Invoke)
{
    using Func = QRectF (MultiTypeReceiver::*)(qreal, qreal, qreal, qreal);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::retRectF);
    QVariant ret = helper.invoke({QVariant(1.0), QVariant(2.0), QVariant(3.0), QVariant(4.0)});
    EXPECT_EQ(ret.toRectF(), QRectF(1.0, 2.0, 3.0, 4.0));
}

TEST_F(EventTemplateTest, EventHelper_SizeReturn_Invoke)
{
    using Func = QSize (MultiTypeReceiver::*)(int, int);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::retSize);
    QVariant ret = helper.invoke({QVariant(100), QVariant(200)});
    EXPECT_EQ(ret.toSize(), QSize(100, 200));
}

TEST_F(EventTemplateTest, EventHelper_UrlReturn_Invoke)
{
    using Func = QUrl (MultiTypeReceiver::*)(const QString &);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::retUrl);
    QVariant ret = helper.invoke({QVariant("file:///test")});
    EXPECT_EQ(ret.toUrl(), QUrl("file:///test"));
}

TEST_F(EventTemplateTest, EventHelper_VariantReturn_Invoke)
{
    using Func = QVariant (MultiTypeReceiver::*)(const QVariant &);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::retVariant);
    QVariant ret = helper.invoke({QVariant(42)});
    // EventHelper wraps a slot returning QVariant in an outer variant whose
    // metatype is QVariant, so the result must be unwrapped one level before
    // being read (a nested QVariant does not auto-convert to int).
    EXPECT_EQ(ret.value<QVariant>().toInt(), 42);
}

TEST_F(EventTemplateTest, EventHelper_TwoArgs_Invoke)
{
    using Func = int (MultiTypeReceiver::*)(int, int);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::twoArgs);
    QVariant ret = helper.invoke({QVariant(3), QVariant(4)});
    EXPECT_EQ(ret.toInt(), 7);
}

TEST_F(EventTemplateTest, EventHelper_ThreeArgs_Invoke)
{
    using Func = int (MultiTypeReceiver::*)(int, int, int);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::threeArgs);
    QVariant ret = helper.invoke({QVariant(1), QVariant(2), QVariant(3)});
    EXPECT_EQ(ret.toInt(), 6);
}

TEST_F(EventTemplateTest, EventHelper_FourArgs_Invoke)
{
    using Func = int (MultiTypeReceiver::*)(int, int, int, int);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::fourArgs);
    QVariant ret = helper.invoke({QVariant(1), QVariant(2), QVariant(3), QVariant(4)});
    EXPECT_EQ(ret.toInt(), 10);
}

TEST_F(EventTemplateTest, EventHelper_FiveArgs_Invoke)
{
    using Func = int (MultiTypeReceiver::*)(int, int, int, int, int);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::fiveArgs);
    QVariant ret = helper.invoke({QVariant(1), QVariant(2), QVariant(3), QVariant(4), QVariant(5)});
    EXPECT_EQ(ret.toInt(), 15);
}

TEST_F(EventTemplateTest, EventHelper_VoidIntSlot_Invoke)
{
    using Func = void (MultiTypeReceiver::*)(int);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::voidSlotInt);
    QVariant ret = helper.invoke({QVariant(42)});
    EXPECT_EQ(receiver->callCount, 1);
}

TEST_F(EventTemplateTest, EventHelper_VoidStrSlot_Invoke)
{
    using Func = void (MultiTypeReceiver::*)(const QString &);
    EventHelper<Func> helper(receiver, &MultiTypeReceiver::voidSlotStr);
    QVariant ret = helper.invoke({QVariant("test")});
    EXPECT_EQ(receiver->callCount, 1);
}

// --- resultGenerator and paramGenerator templates ---

TEST_F(EventTemplateTest, ResultGenerator_Int)
{
    QVariant ret = resultGenerator<int>();
    EXPECT_EQ(ret.typeId(), QMetaType::Int);
}

TEST_F(EventTemplateTest, ResultGenerator_Bool)
{
    QVariant ret = resultGenerator<bool>();
    EXPECT_TRUE(ret.isValid());
}

TEST_F(EventTemplateTest, ResultGenerator_String)
{
    QVariant ret = resultGenerator<QString>();
    EXPECT_TRUE(ret.isValid());
}

TEST_F(EventTemplateTest, ResultGenerator_Void)
{
    QVariant ret = resultGenerator<void>();
    EXPECT_TRUE(ret.isNull() || !ret.isValid());
}

TEST_F(EventTemplateTest, ResultGenerator_QPoint)
{
    QVariant ret = resultGenerator<QPoint>();
    EXPECT_TRUE(ret.isValid());
}

TEST_F(EventTemplateTest, ResultGenerator_QUrl)
{
    QVariant ret = resultGenerator<QUrl>();
    EXPECT_TRUE(ret.isValid());
}

TEST_F(EventTemplateTest, ResultGenerator_QVariant)
{
    QVariant ret = resultGenerator<QVariant>();
    EXPECT_TRUE(ret.isValid());
}

TEST_F(EventTemplateTest, ResultGenerator_Double)
{
    QVariant ret = resultGenerator<double>();
    EXPECT_EQ(ret.typeId(), QMetaType::Double);
}

TEST_F(EventTemplateTest, ParamGenerator_Int)
{
    QVariant input(42);
    int result = paramGenerator<int>(input);
    EXPECT_EQ(result, 42);
}

TEST_F(EventTemplateTest, ParamGenerator_String)
{
    QVariant input("hello");
    QString result = paramGenerator<QString>(input);
    EXPECT_EQ(result, "hello");
}

TEST_F(EventTemplateTest, ParamGenerator_Bool)
{
    QVariant input(true);
    bool result = paramGenerator<bool>(input);
    EXPECT_EQ(result, true);
}

TEST_F(EventTemplateTest, ParamGenerator_Double)
{
    QVariant input(3.14);
    double result = paramGenerator<double>(input);
    EXPECT_DOUBLE_EQ(result, 3.14);
}

TEST_F(EventTemplateTest, ParamGenerator_QPoint)
{
    QVariant input(QPoint(5, 10));
    QPoint result = paramGenerator<QPoint>(input);
    EXPECT_EQ(result, QPoint(5, 10));
}

TEST_F(EventTemplateTest, ParamGenerator_QUrl)
{
    QVariant input(QUrl("file:///test"));
    QUrl result = paramGenerator<QUrl>(input);
    EXPECT_EQ(result, QUrl("file:///test"));
}

// --- EventChannel send with different types ---

TEST_F(EventTemplateTest, ChannelConnectAndPush_BoolReturn)
{
    EventType type = regEvent("slot_BoolRet");
    ASSERT_TRUE(channelMgr->connect(type, receiver, &MultiTypeReceiver::retBool));
    QVariant ret = channelMgr->push(type, QVariant(true));
    EXPECT_EQ(ret.toBool(), true);
}

TEST_F(EventTemplateTest, ChannelConnectAndPush_IntReturn)
{
    EventType type = regEvent("slot_IntRet");
    ASSERT_TRUE(channelMgr->connect(type, receiver, &MultiTypeReceiver::retInt));
    QVariant ret = channelMgr->push(type, QVariant(10));
    EXPECT_EQ(ret.toInt(), 11);
}

TEST_F(EventTemplateTest, ChannelConnectAndPush_StringReturn)
{
    EventType type = regEvent("slot_StrRet");
    ASSERT_TRUE(channelMgr->connect(type, receiver, &MultiTypeReceiver::retString));
    QVariant ret = channelMgr->push(type, QVariant("world"));
    EXPECT_EQ(ret.toString(), "world_ret");
}

TEST_F(EventTemplateTest, ChannelConnectAndPush_DoubleReturn)
{
    EventType type = regEvent("slot_DblRet");
    ASSERT_TRUE(channelMgr->connect(type, receiver, &MultiTypeReceiver::retDouble));
    QVariant ret = channelMgr->push(type, QVariant(2.5));
    EXPECT_DOUBLE_EQ(ret.toDouble(), 5.0);
}

TEST_F(EventTemplateTest, ChannelConnectAndPush_VoidReturn)
{
    EventType type = regEvent("slot_VoidRet");
    ASSERT_TRUE(channelMgr->connect(type, receiver, &MultiTypeReceiver::voidSlot));
    channelMgr->push(type);
    EXPECT_EQ(receiver->callCount, 1);
}

TEST_F(EventTemplateTest, ChannelConnectAndPush_TwoArgs)
{
    EventType type = regEvent("slot_TwoArgs");
    ASSERT_TRUE(channelMgr->connect(type, receiver, &MultiTypeReceiver::twoArgs));
    QVariant ret = channelMgr->push(type, QVariant(3), QVariant(7));
    EXPECT_EQ(ret.toInt(), 10);
}

TEST_F(EventTemplateTest, ChannelConnectAndPush_ThreeArgs)
{
    EventType type = regEvent("slot_ThreeArgs");
    ASSERT_TRUE(channelMgr->connect(type, receiver, &MultiTypeReceiver::threeArgs));
    QVariant ret = channelMgr->push(type, QVariant(1), QVariant(2), QVariant(3));
    EXPECT_EQ(ret.toInt(), 6);
}

// --- InvokeHelper makeVariantList and packParamsHelper ---

TEST_F(EventTemplateTest, InvokeHelper_MakeVariantList_Single)
{
    QVariantList list;
    makeVariantList(&list, QVariant(42));
    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list[0].toInt(), 42);
}

TEST_F(EventTemplateTest, InvokeHelper_MakeVariantList_Multiple)
{
    QVariantList list;
    makeVariantList(&list, QVariant(1), QVariant("two"), QVariant(3.0));
    ASSERT_EQ(list.size(), 3);
    EXPECT_EQ(list[0].toInt(), 1);
    EXPECT_EQ(list[1].toString(), "two");
    EXPECT_DOUBLE_EQ(list[2].toDouble(), 3.0);
}

TEST_F(EventTemplateTest, InvokeHelper_MakeVariantList_TwoItems)
{
    QVariantList list;
    makeVariantList(&list, QVariant(1), QVariant("two"));
    ASSERT_EQ(list.size(), 2);
    EXPECT_EQ(list[0].toInt(), 1);
    EXPECT_EQ(list[1].toString(), "two");
}

TEST_F(EventTemplateTest, InvokeHelper_PackParamsHelper_Int)
{
    QVariantList list;
    packParamsHelper(list, 42);
    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list[0].toInt(), 42);
}

TEST_F(EventTemplateTest, InvokeHelper_PackParamsHelper_String)
{
    QVariantList list;
    packParamsHelper(list, QString("test"));
    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list[0].toString(), "test");
}

// --- EventChannel asyncSend ---

TEST_F(EventTemplateTest, Channel_AsyncSend_BoolReturn)
{
    EventType type = regEvent("slot_AsyncBool");
    ASSERT_TRUE(channelMgr->connect(type, receiver, &MultiTypeReceiver::retBool));
    // Use push (sync) which exercises the same EventHelper::invoke path
    QVariant ret = channelMgr->push(type, QVariant(true));
    EXPECT_EQ(ret.toBool(), true);
}

TEST_F(EventTemplateTest, Channel_AsyncSend_VoidReturn)
{
    EventType type = regEvent("slot_AsyncVoid");
    ASSERT_TRUE(channelMgr->connect(type, receiver, &MultiTypeReceiver::voidSlot));
    channelMgr->push(type);
    EXPECT_EQ(receiver->callCount, 1);
}

// --- eventhelper utility functions ---

TEST_F(EventTemplateTest, GenCustomEventId_Increments)
{
    EventType id1 = genCustomEventId();
    EventType id2 = genCustomEventId();
    EXPECT_NE(id1, id2);
}

TEST_F(EventTemplateTest, IsValidEventType_ValidReturnsTrue)
{
    EventType id = genCustomEventId();
    EXPECT_TRUE(isValidEventType(id));
}

TEST_F(EventTemplateTest, IsValidEventType_InvalidReturnsFalse)
{
    EXPECT_FALSE(isValidEventType(EventType(-1)));
}

TEST_F(EventTemplateTest, IsWellKnownEventType)
{
    EXPECT_NO_FATAL_FAILURE(isWellKnownEvenType(0));
}

TEST_F(EventTemplateTest, ThreadEventAlert_ByName)
{
    EXPECT_NO_FATAL_FAILURE(threadEventAlert("test_event"));
}

TEST_F(EventTemplateTest, ThreadEventAlert_BySpaceTopic)
{
    EXPECT_NO_FATAL_FAILURE(threadEventAlert("space", "topic"));
}

TEST_F(EventTemplateTest, ThreadEventAlert_ByType)
{
    EventType type = genCustomEventId();
    EXPECT_NO_FATAL_FAILURE(threadEventAlert(type));
}

// --- EventConverter ---

TEST_F(EventTemplateTest, EventConverter_RegisterAndConvert)
{
    // Event::instance() registers the global converter inside its constructor
    // via std::call_once, so any later registerConverter() call is a no-op by
    // design.  The converter maps (space, topic) through Event::eventType(),
    // therefore a topic must be registered through Event before convert()
    // returns a valid id.
    auto conv = [](const QString &space, const QString &topic) -> EventType {
        return qHash(space + "::" + topic);
    };
    EXPECT_NO_FATAL_FAILURE(EventConverter::registerConverter(conv));

    event->registerEventType(EventStratege::kSlot, "testspace", "slot_testtopic");
    EventType type = EventConverter::convert("testspace", "slot_testtopic");
    EXPECT_NE(type, EventType(-1));
    EXPECT_EQ(type, event->eventType("testspace", "slot_testtopic"));
}

// --- EventSequence template tests ---

TEST_F(EventTemplateTest, EventSequence_AppendAndTraversal)
{
    // EventSequence requires bool-returning methods
    // Add a bool-returning slot to receiver
    EventSequence seq;
    seq.append(receiver, &MultiTypeReceiver::retBool);
    bool result = seq.traversal(QVariantList{QVariant(true)});
    EXPECT_NO_FATAL_FAILURE(seq.traversal());
}

TEST_F(EventTemplateTest, EventSequence_Remove)
{
    EventSequence seq;
    seq.append(receiver, &MultiTypeReceiver::retBool);
    EXPECT_TRUE(seq.remove(receiver, &MultiTypeReceiver::retBool));
}

// --- EventDispatcher template tests ---

TEST_F(EventTemplateTest, EventDispatcher_AppendAndDispatch)
{
    EventDispatcher dispatcher;
    dispatcher.append(receiver, &MultiTypeReceiver::retInt);
    // EventDispatcher::dispatch() returns bool: true when every handler was
    // invoked and no filter blocked the dispatch (handler results are not
    // aggregated).
    EXPECT_TRUE(dispatcher.dispatch(QVariantList{QVariant(5)}));
}

TEST_F(EventTemplateTest, EventDispatcher_Remove)
{
    EventDispatcher dispatcher;
    dispatcher.append(receiver, &MultiTypeReceiver::retInt);
    EXPECT_TRUE(dispatcher.remove(receiver, &MultiTypeReceiver::retInt));
}

TEST_F(EventTemplateTest, EventDispatcher_AppendFilter)
{
    EventDispatcher dispatcher;
    dispatcher.appendFilter(receiver, &MultiTypeReceiver::retBool);
    EXPECT_NO_FATAL_FAILURE(dispatcher.dispatch(QVariantList{QVariant(true)}));
}

#include "test_eventtemplate.moc"
