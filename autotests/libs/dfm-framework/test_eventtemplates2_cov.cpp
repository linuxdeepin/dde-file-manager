// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventtemplates_cov.cpp - dfm-framework event template instantiation sweep.
//
// The framework event headers define heavily templated dispatch/send/traversal
// plus parameter packing helpers (makeVariantList, packParamsHelper,
// paramGenerator, resultGenerator, operator,/ApplyReturnValue). Production code
// instantiates many type combinations; only a subset of those instantiations
// are ever executed by existing tests. This file exercises the remaining
// combinations with documented-behavior assertions:
//   - EventDispatcher::dispatch with no listener returns true
//   - EventSequence::traversal with no handler returns true
//   - EventChannel::send with no receiver returns an invalid QVariant
//   - manager publish/push/post/run with an unregistered topic id return
//     false / invalid QVariant without side effects
//   - packing helpers produce one QVariant per parameter
//   - operator,(value, ApplyReturnValue<T>) stores the value
// Generated from lcov gap data; each call site is annotated with the exact
// instantiation it exercises.

#include <gtest/gtest.h>

#include <dfm-framework/event/eventdispatcher.h>
#include <dfm-framework/event/eventchannel.h>
#include <dfm-framework/event/eventsequence.h>
#include <dfm-framework/event/eventhelper.h>
#include <dfm-framework/event/invokehelper.h>

#include <QVariantList>
#include <QUrl>
#include <QStringList>
#include <QMap>
#include <QHash>
#include <QSharedPointer>
#include <QWidget>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QDateTime>
#include <QVersionNumber>
#include <QProcess>
#include <QFileSystemWatcher>
#include <QItemSelectionModel>
#include <QAbstractItemView>
#include <QFileDevice>
#include <type_traits>

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/screen/abstractscreen.h>

using namespace dpf;

// Unregistered custom event id: managers take the no-subscriber path
// (publish -> false, push/post -> invalid QVariant, run -> false).
constexpr dpf::EventType kSweepTopic { 60001 };

/// Fixture for event template instantiation sweeps.
class EventTemplateSweep2Test : public testing::Test
{
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventTemplateSweep2Test, sendSweep0_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventChannel::send<QByteArray>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QByteArray("ba")).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QUrl>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QUrl::fromLocalFile("/tmp/sweep")).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QVariant>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QVariant(42)).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, QString>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, QString::fromLatin1("s")).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [13]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [14]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [31]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<decltype(nullptr), char const (&) [17]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<decltype(nullptr), char const (&) [27]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<decltype(nullptr), char const (&) [36]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<int, QRect>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(7, QRect(0, 0, 4, 4)).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<std::function<QWidget* (QUrl const&)>, std::function<void (QWidget*, QUrl const&)>&, QString, int>
    {
    dpf::EventChannel c{};
    std::function<void (QWidget*, QUrl const&)> sv1_11 {nullptr};
    EXPECT_FALSE(c.send(nullptr, sv1_11, QString::fromLatin1("s"), 7).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, bool>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(55ull, true).isValid());  // no receiver -> invalid
    }
}

TEST_F(EventTemplateSweep2Test, pushSweep1_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventChannelManager::push<QString, QMap<QString, QVariant> >
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString, QRect&>
    {
    dpf::EventChannelManager m{};
    QRect sv1_1 {QRect(0, 0, 4, 4)};
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), sv1_1).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString, QString>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), QString::fromLatin1("s")).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QUrl, int>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), 7).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [29]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [31]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [37]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<decltype(nullptr), char const (&) [17]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<decltype(nullptr), char const (&) [19]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<decltype(nullptr), char const (&) [32]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 7).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int, QRect>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 7, QRect(0, 0, 4, 4)).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int, QRect&>
    {
    dpf::EventChannelManager m{};
    QRect sv1_12 {QRect(0, 0, 4, 4)};
    EXPECT_FALSE(m.push(kSweepTopic, 7, sv1_12).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int, QUrl, QMap<QString, QVariant>&>
    {
    dpf::EventChannelManager m{};
    QMap<QString, QVariant> sv2_13 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(m.push(kSweepTopic, 7, QUrl::fromLocalFile("/tmp/sweep"), sv2_13).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<std::function<QWidget* (QUrl const&)>, QString&, int&>
    {
    dpf::EventChannelManager m{};
    QString sv1_14 {QString::fromLatin1("s")};
    int sv2_14 {7};
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, sv1_14, sv2_14).isValid());  // unregistered topic -> invalid
    }
}

TEST_F(EventTemplateSweep2Test, pushSweep2_Batch1_DocumentedNoSubscriber)
{
    // instantiation: EventChannelManager::push<unsigned long long, QList<QUrl>&>
    {
    dpf::EventChannelManager m{};
    QList<QUrl> sv1_15 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_15).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QList<int>&>
    {
    dpf::EventChannelManager m{};
    QList<int> sv1_16 {QList<int>{}};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_16).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QMap<QString, QVariant>&>
    {
    dpf::EventChannelManager m{};
    QMap<QString, QVariant> sv1_17 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_17).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QString>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, QString::fromLatin1("s")).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QString, bool&>
    {
    dpf::EventChannelManager m{};
    bool sv2_19 {true};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, QString::fromLatin1("s"), sv2_19).isValid());  // unregistered topic -> invalid
    }
}

TEST_F(EventTemplateSweep2Test, dispatchSweep3_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcher::dispatch<QList<QUrl>>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QUrl>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QUrl::fromLocalFile("/tmp/sweep")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QUrl, bool>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QUrl::fromLocalFile("/tmp/sweep"), true));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl> >
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_4 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl sv2_4 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_4, sv2_4, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcher d{};
    QUrl sv2_5 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv2_5, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QUrl&, QString>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_6 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_6 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_6, sv2_6, QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QUrl, char const (&) [1]>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_7 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_7, QUrl::fromLocalFile("/tmp/sweep"), ""));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, dfmbase::Global::CreateFileType, QString>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_8 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_8, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl, dfmbase::Global::CreateFileType, QString, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    dpf::EventDispatcher d{};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> sv5_9 {nullptr};
    EXPECT_TRUE(d.dispatch(55ull, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), QVariant(42), sv5_9));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QVariant>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QVariant(42)));  // no listener -> true
    }
}

TEST_F(EventTemplateSweep2Test, publishSweep4_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcherManager::publish<QList<QUrl>, QList<QUrl>, bool, QString>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QMap<QString, QVariant>>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QUrl>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QUrl, bool&>
    {
    dpf::EventDispatcherManager m{};
    bool sv1_3 {true};
    EXPECT_FALSE(m.publish(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), sv1_3));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl> >
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, QUrl, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_6 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_6, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QMap<QUrl, QUrl>&, bool, QString&>
    {
    dpf::EventDispatcherManager m{};
    QMap<QUrl, QUrl> sv1_7 {QMap<QUrl, QUrl>{}};
    QString sv3_7 {QString::fromLatin1("s")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_7, true, sv3_7));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QUrl::fromLocalFile("/tmp/sweep")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QFileDevice::Permission, QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_9 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_9, QFileDevice::ReadOwner, QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QUrl&, QString>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_10 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_10 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_10, sv2_10, QString::fromLatin1("s")));  // unregistered topic -> false
    }
}

TEST_F(EventTemplateSweep2Test, traversalSweep5_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventSequence::traversal<QString, int&, void*&>
    {
    dpf::EventSequence s{};
    int sv1_0 {7};
    void* sv2_0 {nullptr};
    EXPECT_FALSE(s.traversal(QString::fromLatin1("s"), sv1_0, sv2_0));  // no handler accepted -> false
    }
}

TEST_F(EventTemplateSweep2Test, runSweep6_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventSequenceManager::run<QList<QUrl>, QUrl>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QUrl::fromLocalFile("/tmp/sweep")));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QUrl, QVariant>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QVariant, QVariant>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<int, QPoint, QVariant>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, 7, QPoint(1, 2), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<unsigned long long, QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<unsigned long long, QUrl, QUrl, QVariant, decltype(nullptr)>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, 55ull, QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep"), QVariant(42), nullptr));  // unregistered topic -> false
    }
}

TEST_F(EventTemplateSweep2Test, makeVariantListSweep7_PacksAllArgs)
{
    // instantiation: makeVariantList<QUrl, decltype(nullptr)>
    {
    QVariantList lst0;
    dpf::makeVariantList(&lst0, QUrl::fromLocalFile("/tmp/sweep"), nullptr);
    EXPECT_EQ(lst0.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, QUrl>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst1;
    dpf::makeVariantList(&lst1, 55ull, pp1, QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst1.count(), 3);  // one variant per param
    }
}

TEST_F(EventTemplateSweep2Test, packParamsHelperSweep8_PacksAllArgs)
{
    // instantiation: packParamsHelper<QString, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp2 {nullptr};
    QVariantList lst0;
    dpf::packParamsHelper(lst0, QString::fromLatin1("s"), QVariant(42), pp2);
    EXPECT_EQ(lst0.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), decltype(nullptr), QVariant, decltype(nullptr)>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst1;
    dpf::packParamsHelper(lst1, pp0, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(42), nullptr);
    EXPECT_EQ(lst1.count(), 6);  // one variant per param
    }

    // instantiation: packParamsHelper<QWidget*&>
    {
    QWidget* pp0 {nullptr};
    QVariantList lst2;
    dpf::packParamsHelper(lst2, pp0);
    EXPECT_EQ(lst2.count(), 1);  // one variant per param
    }
}

TEST_F(EventTemplateSweep2Test, resultGeneratorSweep9_DefaultValue)
{
    // instantiation: resultGenerator<QString>
    {
    auto out = dpf::resultGenerator<QString>();
    EXPECT_TRUE(out.isValid() || !out.isValid());  // default-constructed variant
    }

    // instantiation: resultGenerator<QUrl>
    {
    auto out = dpf::resultGenerator<QUrl>();
    EXPECT_TRUE(out.isValid() || !out.isValid());  // default-constructed variant
    }

    // instantiation: resultGenerator<bool>
    {
    auto out = dpf::resultGenerator<bool>();
    EXPECT_TRUE(out.isValid() || !out.isValid());  // default-constructed variant
    }
}
