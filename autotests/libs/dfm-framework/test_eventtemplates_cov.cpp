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
class EventTemplateSweepTest : public testing::Test
{
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventTemplateSweepTest, asyncSendSweep0_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventChannel::asyncSend<QString, QString>
    {
    dpf::EventChannel c{};
    
    auto cfut = c.asyncSend(QString::fromLatin1("s"), QString::fromLatin1("s"));
    cfut.waitForFinished();
    EXPECT_FALSE(cfut.result().isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::asyncSend<int>
    {
    dpf::EventChannel c{};
    
    auto cfut = c.asyncSend(7);
    cfut.waitForFinished();
    EXPECT_FALSE(cfut.result().isValid());  // no receiver -> invalid
    }
}

TEST_F(EventTemplateSweepTest, sendSweep1_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventChannel::send<Dtk::Widget::DLabel*, char const (&) [10]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<Dtk::Widget::DLabel*, char const (&) [15]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QByteArray>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QByteArray("ba")).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QFrame*, char const (&) [11]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QHash<QString, QVariant>>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QHash<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QList<QString>, QString>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QList<QString>{ QString::fromLatin1("s") }, QString::fromLatin1("s")).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QList<QString>, char const (&) [1]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QList<QString>{ QString::fromLatin1("s") }, "").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QList<QString>, int, QPoint>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QList<QString>{ QString::fromLatin1("s") }, 7, QPoint(1, 2)).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QList<QUrl>>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QList<QUrl>, QHash<QString, QVariant> >
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QHash<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QList<QUrl>, QHash<QString, QVariant>&>
    {
    dpf::EventChannel c{};
    QHash<QString, QVariant> sv1_10 {QHash<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(c.send(QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv1_10).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QMap<QString, QVariant>>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QString>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QString::fromLatin1("s")).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QString, QList<QString>&>
    {
    dpf::EventChannel c{};
    QList<QString> sv1_13 {QList<QString>{ QString::fromLatin1("s") }};
    EXPECT_FALSE(c.send(QString::fromLatin1("s"), sv1_13).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QString, QMap<QString, QVariant> >
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QString::fromLatin1("s"), QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}).isValid());  // no receiver -> invalid
    }
}

TEST_F(EventTemplateSweepTest, sendSweep2_Batch1_DocumentedNoSubscriber)
{
    // instantiation: EventChannel::send<QString, QMap<QString, QVariant>&>
    {
    dpf::EventChannel c{};
    QMap<QString, QVariant> sv1_15 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(c.send(QString::fromLatin1("s"), sv1_15).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QString, QRect&>
    {
    dpf::EventChannel c{};
    QRect sv1_16 {QRect(0, 0, 4, 4)};
    EXPECT_FALSE(c.send(QString::fromLatin1("s"), sv1_16).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QString, QString>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QString::fromLatin1("s"), QString::fromLatin1("s")).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QString, QString&>
    {
    dpf::EventChannel c{};
    QString sv1_18 {QString::fromLatin1("s")};
    EXPECT_FALSE(c.send(QString::fromLatin1("s"), sv1_18).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QString, QVariant>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QString::fromLatin1("s"), QVariant(42)).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QString, bool&, QWidget*&>
    {
    dpf::EventChannel c{};
    bool sv1_20 {true};
    QWidget* sv2_20 {nullptr};
    EXPECT_FALSE(c.send(QString::fromLatin1("s"), sv1_20, sv2_20).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QString, char const (&) [13]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QString::fromLatin1("s"), "xxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QString, dfmbase::Global::ViewMode>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QString::fromLatin1("s"), dfmbase::Global::ViewMode::kIconMode).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QString, std::function<void (unsigned long long, QUrl const&, std::function<void ()>)>&>
    {
    dpf::EventChannel c{};
    std::function<void (unsigned long long, QUrl const&, std::function<void ()>)> sv1_23 {nullptr};
    EXPECT_FALSE(c.send(QString::fromLatin1("s"), sv1_23).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QUrl>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QUrl::fromLocalFile("/tmp/sweep")).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QUrl, QMap<QString, QVariant>&>
    {
    dpf::EventChannel c{};
    QMap<QString, QVariant> sv1_25 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(c.send(QUrl::fromLocalFile("/tmp/sweep"), sv1_25).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QUrl, QVariant>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QUrl::fromLocalFile("/tmp/sweep"), QVariant(42)).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QUrl, bool&>
    {
    dpf::EventChannel c{};
    bool sv1_27 {true};
    EXPECT_FALSE(c.send(QUrl::fromLocalFile("/tmp/sweep"), sv1_27).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QUrl, int>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QUrl::fromLocalFile("/tmp/sweep"), 7).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QVariant>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QVariant(42)).isValid());  // no receiver -> invalid
    }
}

TEST_F(EventTemplateSweepTest, sendSweep3_Batch2_DocumentedNoSubscriber)
{
    // instantiation: EventChannel::send<QVariant, QVariant>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QVariant(42), QVariant(42)).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QVariant, QVariant, QVariant>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QVariant(42), QVariant(42), QVariant(42)).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QVariant, QVariant, QVariant, int>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(QVariant(42), QVariant(42), QVariant(42), 7).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, QString>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, QString::fromLatin1("s")).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [11]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [12]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxx").isValid());  // no receiver -> invalid
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

    // instantiation: EventChannel::send<QWidget*, char const (&) [15]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [16]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [17]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [19]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [21]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [22]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [23]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }
}

TEST_F(EventTemplateSweepTest, sendSweep4_Batch3_DocumentedNoSubscriber)
{
    // instantiation: EventChannel::send<QWidget*, char const (&) [24]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [25]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [26]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [27]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [28]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [29]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [30]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [31]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [32]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [36]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [37]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<QWidget*, char const (&) [8]>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, "xxxxxxx").isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<bool>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(true).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<bool, int&, bool&>
    {
    dpf::EventChannel c{};
    int sv1_58 {7};
    bool sv2_58 {true};
    EXPECT_FALSE(c.send(true, sv1_58, sv2_58).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<bool, int, bool>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(true, 7, true).isValid());  // no receiver -> invalid
    }
}

TEST_F(EventTemplateSweepTest, sendSweep5_Batch4_DocumentedNoSubscriber)
{
    // instantiation: EventChannel::send<int>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(7).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<int, QList<QUrl>&>
    {
    dpf::EventChannel c{};
    QList<QUrl> sv1_61 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(c.send(7, sv1_61).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<int, QPoint>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(7, QPoint(1, 2)).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<int, QRect>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(7, QRect(0, 0, 4, 4)).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<int, QRect&>
    {
    dpf::EventChannel c{};
    QRect sv1_64 {QRect(0, 0, 4, 4)};
    EXPECT_FALSE(c.send(7, sv1_64).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<int, QUrl&>
    {
    dpf::EventChannel c{};
    QUrl sv1_65 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(c.send(7, sv1_65).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<int, QUrl, QMap<QString, QVariant>&>
    {
    dpf::EventChannel c{};
    QMap<QString, QVariant> sv2_66 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(c.send(7, QUrl::fromLocalFile("/tmp/sweep"), sv2_66).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<int, int>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(7, 7).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<std::function<QMap<QString, QMultiMap<QString, std::pair<QString, QString> > > (QUrl const&)>, QString>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, QString::fromLatin1("s")).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<std::function<QWidget* (QUrl const&)>, QString>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(nullptr, QString::fromLatin1("s")).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<std::function<QWidget* (QUrl const&)>, QString&, int&>
    {
    dpf::EventChannel c{};
    QString sv1_70 {QString::fromLatin1("s")};
    int sv2_70 {7};
    EXPECT_FALSE(c.send(nullptr, sv1_70, sv2_70).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<std::function<QWidget* (QUrl const&)>, int&>
    {
    dpf::EventChannel c{};
    int sv1_71 {7};
    EXPECT_FALSE(c.send(nullptr, sv1_71).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<std::function<QWidget* (QUrl const&)>, std::function<void (QWidget*, QUrl const&)>&, QString, int>
    {
    dpf::EventChannel c{};
    std::function<void (QWidget*, QUrl const&)> sv1_72 {nullptr};
    EXPECT_FALSE(c.send(nullptr, sv1_72, QString::fromLatin1("s"), 7).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(55ull).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, QList<QString> >
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(55ull, QList<QString>{ QString::fromLatin1("s") }).isValid());  // no receiver -> invalid
    }
}

TEST_F(EventTemplateSweepTest, sendSweep6_Batch5_DocumentedNoSubscriber)
{
    // instantiation: EventChannel::send<unsigned long long, QList<QString>&>
    {
    dpf::EventChannel c{};
    QList<QString> sv1_75 {QList<QString>{ QString::fromLatin1("s") }};
    EXPECT_FALSE(c.send(55ull, sv1_75).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, QList<QUrl>&>
    {
    dpf::EventChannel c{};
    QList<QUrl> sv1_76 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(c.send(55ull, sv1_76).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, QList<QUrl>&, QList<QUrl>&>
    {
    dpf::EventChannel c{};
    QList<QUrl> sv1_77 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QList<QUrl> sv2_77 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(c.send(55ull, sv1_77, sv2_77).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, QList<int>&>
    {
    dpf::EventChannel c{};
    QList<int> sv1_78 {QList<int>{}};
    EXPECT_FALSE(c.send(55ull, sv1_78).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, QMap<QString, QVariant>&>
    {
    dpf::EventChannel c{};
    QMap<QString, QVariant> sv1_79 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(c.send(55ull, sv1_79).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, QString>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(55ull, QString::fromLatin1("s")).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, QString, bool>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(55ull, QString::fromLatin1("s"), true).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, QString, bool&>
    {
    dpf::EventChannel c{};
    bool sv2_82 {true};
    EXPECT_FALSE(c.send(55ull, QString::fromLatin1("s"), sv2_82).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, QUrl&>
    {
    dpf::EventChannel c{};
    QUrl sv1_83 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(c.send(55ull, sv1_83).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, QUrl&, QVariant>
    {
    dpf::EventChannel c{};
    QUrl sv1_84 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(c.send(55ull, sv1_84, QVariant(42)).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, QUrl, QVariant>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(55ull, QUrl::fromLocalFile("/tmp/sweep"), QVariant(42)).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, QVariant>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(55ull, QVariant(42)).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, bool>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(55ull, true).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, bool&>
    {
    dpf::EventChannel c{};
    bool sv1_88 {true};
    EXPECT_FALSE(c.send(55ull, sv1_88).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, bool&, bool&>
    {
    dpf::EventChannel c{};
    bool sv1_89 {true};
    bool sv2_89 {true};
    EXPECT_FALSE(c.send(55ull, sv1_89, sv2_89).isValid());  // no receiver -> invalid
    }
}

TEST_F(EventTemplateSweepTest, sendSweep7_Batch6_DocumentedNoSubscriber)
{
    // instantiation: EventChannel::send<unsigned long long, int>
    {
    dpf::EventChannel c{};
    
    EXPECT_FALSE(c.send(55ull, 7).isValid());  // no receiver -> invalid
    }

    // instantiation: EventChannel::send<unsigned long long, int&>
    {
    dpf::EventChannel c{};
    int sv1_91 {7};
    EXPECT_FALSE(c.send(55ull, sv1_91).isValid());  // no receiver -> invalid
    }
}

TEST_F(EventTemplateSweepTest, postSweep8_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventChannelManager::post<QString, QString>
    {
    dpf::EventChannelManager m{};
    
    auto qf = m.post(kSweepTopic, QString::fromLatin1("s"), QString::fromLatin1("s"));
    qf.waitForFinished();
    EXPECT_FALSE(qf.isRunning());  // unregistered topic: future not running
    }

    // instantiation: EventChannelManager::post<int>
    {
    dpf::EventChannelManager m{};
    
    auto qf = m.post(kSweepTopic, 7);
    qf.waitForFinished();
    EXPECT_FALSE(qf.isRunning());  // unregistered topic: future not running
    }
}

TEST_F(EventTemplateSweepTest, pushSweep9_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventChannelManager::push<Dtk::Widget::DLabel*, char const (&) [10]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<Dtk::Widget::DLabel*, char const (&) [15]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QByteArray>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QByteArray("ba")).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QFrame*, char const (&) [11]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QHash<QString, QVariant>>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QHash<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QList<QString>, QString>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QList<QString>{ QString::fromLatin1("s") }, QString::fromLatin1("s")).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QList<QString>, char const (&) [1]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QList<QString>{ QString::fromLatin1("s") }, "").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QList<QString>, int, QPoint>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QList<QString>{ QString::fromLatin1("s") }, 7, QPoint(1, 2)).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QList<QUrl>>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QList<QUrl>, QHash<QString, QVariant> >
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QHash<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QList<QUrl>, QHash<QString, QVariant>&>
    {
    dpf::EventChannelManager m{};
    QHash<QString, QVariant> sv1_10 {QHash<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(m.push(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv1_10).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QMap<QString, QVariant>>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QMap<QString, QVariant>&>
    {
    dpf::EventChannelManager m{};
    QMap<QString, QVariant> sv0_12 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(m.push(kSweepTopic, sv0_12).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s")).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString, QList<QString>&>
    {
    dpf::EventChannelManager m{};
    QList<QString> sv1_14 {QList<QString>{ QString::fromLatin1("s") }};
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), sv1_14).isValid());  // unregistered topic -> invalid
    }
}

TEST_F(EventTemplateSweepTest, pushSweep10_Batch1_DocumentedNoSubscriber)
{
    // instantiation: EventChannelManager::push<QString, QMap<QString, QVariant> >
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString, QMap<QString, QVariant>&>
    {
    dpf::EventChannelManager m{};
    QMap<QString, QVariant> sv1_16 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), sv1_16).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString, QRect&>
    {
    dpf::EventChannelManager m{};
    QRect sv1_17 {QRect(0, 0, 4, 4)};
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), sv1_17).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString, QString>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), QString::fromLatin1("s")).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString, QString&>
    {
    dpf::EventChannelManager m{};
    QString sv1_19 {QString::fromLatin1("s")};
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), sv1_19).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString, QVariant>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), QVariant(42)).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString, bool&, QWidget*&>
    {
    dpf::EventChannelManager m{};
    bool sv1_21 {true};
    QWidget* sv2_21 {nullptr};
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), sv1_21, sv2_21).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString, char const (&) [13]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), "xxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString, dfmbase::Global::ViewMode>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), dfmbase::Global::ViewMode::kIconMode).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QString, std::function<void (unsigned long long, QUrl const&, std::function<void ()>)>&>
    {
    dpf::EventChannelManager m{};
    std::function<void (unsigned long long, QUrl const&, std::function<void ()>)> sv1_24 {nullptr};
    EXPECT_FALSE(m.push(kSweepTopic, QString::fromLatin1("s"), sv1_24).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QUrl>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep")).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QUrl, QMap<QString, QVariant>&>
    {
    dpf::EventChannelManager m{};
    QMap<QString, QVariant> sv1_26 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(m.push(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), sv1_26).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QUrl, QVariant>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), QVariant(42)).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QUrl, bool&>
    {
    dpf::EventChannelManager m{};
    bool sv1_28 {true};
    EXPECT_FALSE(m.push(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), sv1_28).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QUrl, int>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), 7).isValid());  // unregistered topic -> invalid
    }
}

TEST_F(EventTemplateSweepTest, pushSweep11_Batch2_DocumentedNoSubscriber)
{
    // instantiation: EventChannelManager::push<QVariant>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QVariant(42)).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QVariant, QVariant>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QVariant(42), QVariant(42)).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QVariant, QVariant, QVariant>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QVariant(42), QVariant(42), QVariant(42)).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QVariant, QVariant, QVariant, int>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, QVariant(42), QVariant(42), QVariant(42), 7).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, QString>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, QString::fromLatin1("s")).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [11]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [12]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [13]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [14]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [15]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [16]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [17]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [19]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [21]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [22]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }
}

TEST_F(EventTemplateSweepTest, pushSweep12_Batch3_DocumentedNoSubscriber)
{
    // instantiation: EventChannelManager::push<QWidget*, char const (&) [23]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [24]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [25]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [26]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [27]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [28]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [29]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [30]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [31]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [32]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [36]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [37]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<QWidget*, char const (&) [8]>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, "xxxxxxx").isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<bool>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, true).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<bool, int&, bool&>
    {
    dpf::EventChannelManager m{};
    int sv1_59 {7};
    bool sv2_59 {true};
    EXPECT_FALSE(m.push(kSweepTopic, true, sv1_59, sv2_59).isValid());  // unregistered topic -> invalid
    }
}

TEST_F(EventTemplateSweepTest, pushSweep13_Batch4_DocumentedNoSubscriber)
{
    // instantiation: EventChannelManager::push<bool, int, bool>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, true, 7, true).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 7).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int&>
    {
    dpf::EventChannelManager m{};
    int sv0_62 {7};
    EXPECT_FALSE(m.push(kSweepTopic, sv0_62).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int, QList<QUrl>&>
    {
    dpf::EventChannelManager m{};
    QList<QUrl> sv1_63 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.push(kSweepTopic, 7, sv1_63).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int, QPoint>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 7, QPoint(1, 2)).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int, QRect>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 7, QRect(0, 0, 4, 4)).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int, QRect&>
    {
    dpf::EventChannelManager m{};
    QRect sv1_66 {QRect(0, 0, 4, 4)};
    EXPECT_FALSE(m.push(kSweepTopic, 7, sv1_66).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int, QUrl&>
    {
    dpf::EventChannelManager m{};
    QUrl sv1_67 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.push(kSweepTopic, 7, sv1_67).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int, QUrl, QMap<QString, QVariant>&>
    {
    dpf::EventChannelManager m{};
    QMap<QString, QVariant> sv2_68 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(m.push(kSweepTopic, 7, QUrl::fromLocalFile("/tmp/sweep"), sv2_68).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<int, int>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 7, 7).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<std::function<QMap<QString, QMultiMap<QString, std::pair<QString, QString> > > (QUrl const&)>, QString>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, QString::fromLatin1("s")).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<std::function<QWidget* (QUrl const&)>, QString>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, QString::fromLatin1("s")).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<std::function<QWidget* (QUrl const&)>, QString&, int&>
    {
    dpf::EventChannelManager m{};
    QString sv1_72 {QString::fromLatin1("s")};
    int sv2_72 {7};
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, sv1_72, sv2_72).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<std::function<QWidget* (QUrl const&)>, int&>
    {
    dpf::EventChannelManager m{};
    int sv1_73 {7};
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, sv1_73).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<std::function<QWidget* (QUrl const&)>, std::function<void (QWidget*, QUrl const&)>&, QString, int>
    {
    dpf::EventChannelManager m{};
    std::function<void (QWidget*, QUrl const&)> sv1_74 {nullptr};
    EXPECT_FALSE(m.push(kSweepTopic, nullptr, sv1_74, QString::fromLatin1("s"), 7).isValid());  // unregistered topic -> invalid
    }
}

TEST_F(EventTemplateSweepTest, pushSweep14_Batch5_DocumentedNoSubscriber)
{
    // instantiation: EventChannelManager::push<unsigned long long>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 55ull).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QList<QString> >
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, QList<QString>{ QString::fromLatin1("s") }).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QList<QString>&>
    {
    dpf::EventChannelManager m{};
    QList<QString> sv1_77 {QList<QString>{ QString::fromLatin1("s") }};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_77).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QList<QUrl>&>
    {
    dpf::EventChannelManager m{};
    QList<QUrl> sv1_78 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_78).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QList<QUrl>&, QList<QUrl>&>
    {
    dpf::EventChannelManager m{};
    QList<QUrl> sv1_79 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QList<QUrl> sv2_79 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_79, sv2_79).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QList<int>&>
    {
    dpf::EventChannelManager m{};
    QList<int> sv1_80 {QList<int>{}};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_80).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QMap<QString, QVariant>&>
    {
    dpf::EventChannelManager m{};
    QMap<QString, QVariant> sv1_81 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_81).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QString>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, QString::fromLatin1("s")).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QString, bool>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, QString::fromLatin1("s"), true).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QString, bool&>
    {
    dpf::EventChannelManager m{};
    bool sv2_84 {true};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, QString::fromLatin1("s"), sv2_84).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QUrl&>
    {
    dpf::EventChannelManager m{};
    QUrl sv1_85 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_85).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QUrl&, QVariant>
    {
    dpf::EventChannelManager m{};
    QUrl sv1_86 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_86, QVariant(42)).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QUrl, QVariant>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, QUrl::fromLocalFile("/tmp/sweep"), QVariant(42)).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, QVariant>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, QVariant(42)).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, bool>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, true).isValid());  // unregistered topic -> invalid
    }
}

TEST_F(EventTemplateSweepTest, pushSweep15_Batch6_DocumentedNoSubscriber)
{
    // instantiation: EventChannelManager::push<unsigned long long, bool&>
    {
    dpf::EventChannelManager m{};
    bool sv1_90 {true};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_90).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, bool&, bool&>
    {
    dpf::EventChannelManager m{};
    bool sv1_91 {true};
    bool sv2_91 {true};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_91, sv2_91).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, int>
    {
    dpf::EventChannelManager m{};
    
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, 7).isValid());  // unregistered topic -> invalid
    }

    // instantiation: EventChannelManager::push<unsigned long long, int&>
    {
    dpf::EventChannelManager m{};
    int sv1_93 {7};
    EXPECT_FALSE(m.push(kSweepTopic, 55ull, sv1_93).isValid());  // unregistered topic -> invalid
    }
}

TEST_F(EventTemplateSweepTest, asyncDispatchSweep16_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcher::asyncDispatch<QString>
    {
    dpf::EventDispatcher d{};
    
    auto afut = d.asyncDispatch(QString::fromLatin1("s"));
    afut.waitForFinished();
    EXPECT_TRUE(afut.result());  // no listener -> true
    }
}

TEST_F(EventTemplateSweepTest, dispatchSweep17_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcher::dispatch<QList<QString>>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QList<QString>{ QString::fromLatin1("s") }));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QList<QUrl>>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QList<QUrl>, QList<QUrl>&, QList<QVariant>&, bool, QString>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_2 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QList<QVariant> sv2_2 {QList<QVariant>{ QVariant(42) }};
    EXPECT_TRUE(d.dispatch(QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv1_2, sv2_2, true, QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QList<QUrl>, QList<QUrl>&, bool, QString>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_3 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv1_3, true, QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QList<QUrl>, QList<QUrl>, bool, QString>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true, QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QList<QUrl>, bool, QString>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true, QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QMap<QString, QVariant>>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QString>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QString, QList<QUrl>&>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_8 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(QString::fromLatin1("s"), sv1_8));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QString, QMap<QString, QVariant>&>
    {
    dpf::EventDispatcher d{};
    QMap<QString, QVariant> sv1_9 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_TRUE(d.dispatch(QString::fromLatin1("s"), sv1_9));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QString, QString&>
    {
    dpf::EventDispatcher d{};
    QString sv1_10 {QString::fromLatin1("s")};
    EXPECT_TRUE(d.dispatch(QString::fromLatin1("s"), sv1_10));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QString, QVariant>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QString::fromLatin1("s"), QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QString, QVariant&>
    {
    dpf::EventDispatcher d{};
    QVariant sv1_12 {QVariant(42)};
    EXPECT_TRUE(d.dispatch(QString::fromLatin1("s"), sv1_12));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QUrl>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QUrl::fromLocalFile("/tmp/sweep")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QUrl, QUrl>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep")));  // no listener -> true
    }
}

TEST_F(EventTemplateSweepTest, dispatchSweep18_Batch1_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcher::dispatch<QUrl, bool>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(QUrl::fromLocalFile("/tmp/sweep"), true));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<QUrl, bool&>
    {
    dpf::EventDispatcher d{};
    bool sv1_16 {true};
    EXPECT_TRUE(d.dispatch(QUrl::fromLocalFile("/tmp/sweep"), sv1_16));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<bool>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(true));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<int>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(7));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<int, QList<QUrl>&, QList<QString> >
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_19 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(7, sv1_19, QList<QString>{ QString::fromLatin1("s") }));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<int, QList<QUrl>&, QList<QString>&>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_20 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QList<QString> sv2_20 {QList<QString>{ QString::fromLatin1("s") }};
    EXPECT_TRUE(d.dispatch(7, sv1_20, sv2_20));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<int, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_21 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl sv2_21 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(7, sv1_21, sv2_21, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<int, QList<QUrl>&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_22 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(7, sv1_22, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl> >
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_25 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(55ull, sv1_25));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, QVariant>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_26 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl sv2_26 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_26, sv2_26, dfmbase::AbstractJobHandler::JobFlag::kNoHint, QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_27 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl sv2_27 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_27, sv2_27, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), decltype(nullptr), QVariant, decltype(nullptr)>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_28 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl sv2_28 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_28, sv2_28, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(42), nullptr));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, QUrl, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_29 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(55ull, sv1_29, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // no listener -> true
    }
}

TEST_F(EventTemplateSweepTest, dispatchSweep19_Batch2_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, QUrl, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_30 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariant sv5_30 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> sv6_30 {nullptr};
    EXPECT_TRUE(d.dispatch(55ull, sv1_30, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, sv5_30, sv6_30));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, QVariant, QVariant>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_31 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(55ull, sv1_31, QVariant(42), QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, bool>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_32 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(55ull, sv1_32, true));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, bool&, QString&>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_33 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    bool sv2_33 {true};
    QString sv3_33 {QString::fromLatin1("s")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_33, sv2_33, sv3_33));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, bool, QString>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_34 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(55ull, sv1_34, true, QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, bool, QString&>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_35 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QString sv3_35 {QString::fromLatin1("s")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_35, true, sv3_35));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, bool, char const (&) [21]>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_36 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(55ull, sv1_36, true, "xxxxxxxxxxxxxxxxxxxx"));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, decltype(nullptr), QVariant, QVariant>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_37 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(55ull, sv1_37, nullptr, QVariant(42), QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcher d{};
    QList<QUrl> sv1_38 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(55ull, sv1_38, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>, QList<QString> >
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QList<QString>{ QString::fromLatin1("s") }));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>, QList<QString>, QVariant, QVariant>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QList<QString>{ QString::fromLatin1("s") }, QVariant(42), QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcher d{};
    QUrl sv2_41 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv2_41, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant, QVariant>
    {
    dpf::EventDispatcher d{};
    QUrl sv2_42 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv2_42, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(42), QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>, QVariant>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>, QVariant, QVariant>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QVariant(42), QVariant(42)));  // no listener -> true
    }
}

TEST_F(EventTemplateSweepTest, dispatchSweep20_Batch3_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant, QVariant>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(42), QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QMap<QUrl, QUrl>&, bool&, QString&>
    {
    dpf::EventDispatcher d{};
    QMap<QUrl, QUrl> sv1_47 {QMap<QUrl, QUrl>{}};
    bool sv2_47 {true};
    QString sv3_47 {QString::fromLatin1("s")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_47, sv2_47, sv3_47));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QMap<QUrl, QUrl>&, bool, QString>
    {
    dpf::EventDispatcher d{};
    QMap<QUrl, QUrl> sv1_48 {QMap<QUrl, QUrl>{}};
    EXPECT_TRUE(d.dispatch(55ull, sv1_48, true, QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QMap<QUrl, QUrl>&, bool, QString&>
    {
    dpf::EventDispatcher d{};
    QMap<QUrl, QUrl> sv1_49 {QMap<QUrl, QUrl>{}};
    QString sv3_49 {QString::fromLatin1("s")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_49, true, sv3_49));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QMap<QUrl, QUrl>&, bool, char const (&) [1]>
    {
    dpf::EventDispatcher d{};
    QMap<QUrl, QUrl> sv1_50 {QMap<QUrl, QUrl>{}};
    EXPECT_TRUE(d.dispatch(55ull, sv1_50, true, ""));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QMap<QUrl, QUrl>, bool, QString>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QMap<QUrl, QUrl>{}, true, QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QString>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QString&, QList<QUrl>&>
    {
    dpf::EventDispatcher d{};
    QString sv1_53 {QString::fromLatin1("s")};
    QList<QUrl> sv2_53 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_TRUE(d.dispatch(55ull, sv1_53, sv2_53));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QString, QList<QUrl> >
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QString::fromLatin1("s"), QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QString, QString>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QString::fromLatin1("s"), QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QUrl::fromLocalFile("/tmp/sweep")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_57 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_57));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QFileDevice::Permission, QVariant, QVariant>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_58 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_58, QFileDevice::ReadOwner, QVariant(42), QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QFlags<QFileDevice::Permission>&>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_59 {QUrl::fromLocalFile("/tmp/sweep")};
    QFlags<QFileDevice::Permission> sv2_59 {QFileDevice::Permissions(QFileDevice::ReadOwner)};
    EXPECT_TRUE(d.dispatch(55ull, sv1_59, sv2_59));  // no listener -> true
    }
}

TEST_F(EventTemplateSweepTest, dispatchSweep21_Batch4_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QList<QUrl>, bool>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_60 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_60, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QUrl&, QString>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_61 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_61 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_61, sv2_61, QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QUrl&, QString, QVariant, QVariant>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_62 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_62 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_62, sv2_62, QString::fromLatin1("s"), QVariant(42), QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QUrl&, bool, bool>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_63 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_63 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_63, sv2_63, true, true));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QUrl&, bool, bool, QVariant, QVariant>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_64 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_64 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_64, sv2_64, true, true, QVariant(42), QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QUrl&, dfmbase::AbstractJobHandler::JobFlag>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_65 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_65 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_65, sv2_65, dfmbase::AbstractJobHandler::JobFlag::kNoHint));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, QVariant, QVariant>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_66 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_66 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_66, sv2_66, dfmbase::AbstractJobHandler::JobFlag::kNoHint, QVariant(42), QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QUrl, bool, bool>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_67 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_67, QUrl::fromLocalFile("/tmp/sweep"), true, true));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QUrl, char const (&) [1]>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_68 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_68, QUrl::fromLocalFile("/tmp/sweep"), ""));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, QVariant, QVariant>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_69 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_69, QVariant(42), QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, dfmbase::Global::CreateFileType, QString>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_70 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_70, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s")));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, dfmbase::Global::CreateFileType, QString, QMap<QString, QVariant>&, decltype(nullptr)>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_71 {QUrl::fromLocalFile("/tmp/sweep")};
    QMap<QString, QVariant> sv4_71 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_TRUE(d.dispatch(55ull, sv1_71, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), sv4_71, nullptr));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl&, dfmbase::Global::CreateFileType, QString, QVariant, QVariant>
    {
    dpf::EventDispatcher d{};
    QUrl sv1_72 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_TRUE(d.dispatch(55ull, sv1_72, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), QVariant(42), QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl, QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    dpf::EventDispatcher d{};
    QVariant sv2_73 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> sv3_73 {nullptr};
    EXPECT_TRUE(d.dispatch(55ull, QUrl::fromLocalFile("/tmp/sweep"), sv2_73, sv3_73));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, QUrl, dfmbase::Global::CreateFileType, QString, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    dpf::EventDispatcher d{};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> sv5_74 {nullptr};
    EXPECT_TRUE(d.dispatch(55ull, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), QVariant(42), sv5_74));  // no listener -> true
    }
}

TEST_F(EventTemplateSweepTest, dispatchSweep22_Batch5_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcher::dispatch<unsigned long long, QVariant>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, QVariant(42)));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, bool&>
    {
    dpf::EventDispatcher d{};
    bool sv1_76 {true};
    EXPECT_TRUE(d.dispatch(55ull, sv1_76));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, decltype(nullptr)>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, nullptr));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, int>
    {
    dpf::EventDispatcher d{};
    
    EXPECT_TRUE(d.dispatch(55ull, 7));  // no listener -> true
    }

    // instantiation: EventDispatcher::dispatch<unsigned long long, std::function<void (QSharedPointer<dfmbase::AbstractJobHandler>)>&>
    {
    dpf::EventDispatcher d{};
    std::function<void (QSharedPointer<dfmbase::AbstractJobHandler>)> sv1_79 {nullptr};
    EXPECT_TRUE(d.dispatch(55ull, sv1_79));  // no listener -> true
    }
}

TEST_F(EventTemplateSweepTest, publishSweep23_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcherManager::publish<QList<QString>>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QList<QString>{ QString::fromLatin1("s") }));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QList<QUrl>>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QList<QUrl>, QList<QUrl>&, QList<QVariant>&, bool, QString>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_2 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QList<QVariant> sv2_2 {QList<QVariant>{ QVariant(42) }};
    EXPECT_FALSE(m.publish(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv1_2, sv2_2, true, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QList<QUrl>, QList<QUrl>&, bool, QString>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_3 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv1_3, true, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QList<QUrl>, QList<QUrl>, bool, QString>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QList<QUrl>, bool, QString>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QMap<QString, QVariant>>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QString>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QString, QList<QUrl>&>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_8 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, QString::fromLatin1("s"), sv1_8));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QString, QMap<QString, QVariant>&>
    {
    dpf::EventDispatcherManager m{};
    QMap<QString, QVariant> sv1_9 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(m.publish(kSweepTopic, QString::fromLatin1("s"), sv1_9));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QString, QString&>
    {
    dpf::EventDispatcherManager m{};
    QString sv1_10 {QString::fromLatin1("s")};
    EXPECT_FALSE(m.publish(kSweepTopic, QString::fromLatin1("s"), sv1_10));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QString, QVariant>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QString::fromLatin1("s"), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QString, QVariant&>
    {
    dpf::EventDispatcherManager m{};
    QVariant sv1_12 {QVariant(42)};
    EXPECT_FALSE(m.publish(kSweepTopic, QString::fromLatin1("s"), sv1_12));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QUrl>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QUrl, QUrl>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep")));  // unregistered topic -> false
    }
}

TEST_F(EventTemplateSweepTest, publishSweep24_Batch1_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcherManager::publish<QUrl, bool>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), true));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<QUrl, bool&>
    {
    dpf::EventDispatcherManager m{};
    bool sv1_16 {true};
    EXPECT_FALSE(m.publish(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), sv1_16));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<bool>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, true));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<int>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 7));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<int, QList<QUrl>&, QList<QString> >
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_19 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 7, sv1_19, QList<QString>{ QString::fromLatin1("s") }));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<int, QList<QUrl>&, QList<QString>&>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_20 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QList<QString> sv2_20 {QList<QString>{ QString::fromLatin1("s") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 7, sv1_20, sv2_20));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<int, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_21 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl sv2_21 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 7, sv1_21, sv2_21, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<int, QList<QUrl>&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_22 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 7, sv1_22, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // unregistered topic -> false
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

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_25 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_25));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, QVariant>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_26 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl sv2_26 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_26, sv2_26, dfmbase::AbstractJobHandler::JobFlag::kNoHint, QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_27 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl sv2_27 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_27, sv2_27, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), decltype(nullptr), QVariant, decltype(nullptr)>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_28 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl sv2_28 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_28, sv2_28, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(42), nullptr));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, QUrl, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_29 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_29, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // unregistered topic -> false
    }
}

TEST_F(EventTemplateSweepTest, publishSweep25_Batch2_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, QUrl, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_30 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariant sv5_30 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> sv6_30 {nullptr};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_30, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, sv5_30, sv6_30));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_31 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_31, QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, bool>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_32 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_32, true));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, bool&, QString&>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_33 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    bool sv2_33 {true};
    QString sv3_33 {QString::fromLatin1("s")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_33, sv2_33, sv3_33));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, bool, QString>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_34 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_34, true, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, bool, QString&>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_35 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QString sv3_35 {QString::fromLatin1("s")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_35, true, sv3_35));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, bool, char const (&) [21]>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_36 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_36, true, "xxxxxxxxxxxxxxxxxxxx"));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, decltype(nullptr), QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_37 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_37, nullptr, QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcherManager m{};
    QList<QUrl> sv1_38 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_38, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>, QList<QString> >
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QList<QString>{ QString::fromLatin1("s") }));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>, QList<QString>, QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QList<QString>{ QString::fromLatin1("s") }, QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv2_41 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv2_41, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv2_42 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv2_42, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>, QVariant>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>, QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QVariant(42), QVariant(42)));  // unregistered topic -> false
    }
}

TEST_F(EventTemplateSweepTest, publishSweep26_Batch3_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QMap<QUrl, QUrl>&, bool&, QString&>
    {
    dpf::EventDispatcherManager m{};
    QMap<QUrl, QUrl> sv1_47 {QMap<QUrl, QUrl>{}};
    bool sv2_47 {true};
    QString sv3_47 {QString::fromLatin1("s")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_47, sv2_47, sv3_47));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QMap<QUrl, QUrl>&, bool, QString>
    {
    dpf::EventDispatcherManager m{};
    QMap<QUrl, QUrl> sv1_48 {QMap<QUrl, QUrl>{}};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_48, true, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QMap<QUrl, QUrl>&, bool, QString&>
    {
    dpf::EventDispatcherManager m{};
    QMap<QUrl, QUrl> sv1_49 {QMap<QUrl, QUrl>{}};
    QString sv3_49 {QString::fromLatin1("s")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_49, true, sv3_49));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QMap<QUrl, QUrl>&, bool, char const (&) [1]>
    {
    dpf::EventDispatcherManager m{};
    QMap<QUrl, QUrl> sv1_50 {QMap<QUrl, QUrl>{}};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_50, true, ""));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QMap<QUrl, QUrl>, bool, QString>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QMap<QUrl, QUrl>{}, true, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QString>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QString&, QList<QUrl>&>
    {
    dpf::EventDispatcherManager m{};
    QString sv1_53 {QString::fromLatin1("s")};
    QList<QUrl> sv2_53 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_53, sv2_53));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QString, QList<QUrl> >
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QString::fromLatin1("s"), QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QString, QString>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QString::fromLatin1("s"), QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QUrl::fromLocalFile("/tmp/sweep")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_57 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_57));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QFileDevice::Permission, QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_58 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_58, QFileDevice::ReadOwner, QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QFlags<QFileDevice::Permission>&>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_59 {QUrl::fromLocalFile("/tmp/sweep")};
    QFlags<QFileDevice::Permission> sv2_59 {QFileDevice::Permissions(QFileDevice::ReadOwner)};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_59, sv2_59));  // unregistered topic -> false
    }
}

TEST_F(EventTemplateSweepTest, publishSweep27_Batch4_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QList<QUrl>, bool>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_60 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_60, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QUrl&, QString>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_61 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_61 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_61, sv2_61, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QUrl&, QString, QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_62 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_62 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_62, sv2_62, QString::fromLatin1("s"), QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QUrl&, bool, bool>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_63 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_63 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_63, sv2_63, true, true));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QUrl&, bool, bool, QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_64 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_64 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_64, sv2_64, true, true, QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QUrl&, dfmbase::AbstractJobHandler::JobFlag>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_65 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_65 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_65, sv2_65, dfmbase::AbstractJobHandler::JobFlag::kNoHint));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_66 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_66 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_66, sv2_66, dfmbase::AbstractJobHandler::JobFlag::kNoHint, QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QUrl, bool, bool>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_67 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_67, QUrl::fromLocalFile("/tmp/sweep"), true, true));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QUrl, char const (&) [1]>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_68 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_68, QUrl::fromLocalFile("/tmp/sweep"), ""));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_69 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_69, QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, dfmbase::Global::CreateFileType, QString>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_70 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_70, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, dfmbase::Global::CreateFileType, QString, QMap<QString, QVariant>&, decltype(nullptr)>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_71 {QUrl::fromLocalFile("/tmp/sweep")};
    QMap<QString, QVariant> sv4_71 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_71, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), sv4_71, nullptr));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl&, dfmbase::Global::CreateFileType, QString, QVariant, QVariant>
    {
    dpf::EventDispatcherManager m{};
    QUrl sv1_72 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_72, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), QVariant(42), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl, QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    dpf::EventDispatcherManager m{};
    QVariant sv2_73 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> sv3_73 {nullptr};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QUrl::fromLocalFile("/tmp/sweep"), sv2_73, sv3_73));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, QUrl, dfmbase::Global::CreateFileType, QString, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    dpf::EventDispatcherManager m{};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> sv5_74 {nullptr};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), QVariant(42), sv5_74));  // unregistered topic -> false
    }
}

TEST_F(EventTemplateSweepTest, publishSweep28_Batch5_DocumentedNoSubscriber)
{
    // instantiation: EventDispatcherManager::publish<unsigned long long, QVariant>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, bool&>
    {
    dpf::EventDispatcherManager m{};
    bool sv1_76 {true};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_76));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, decltype(nullptr)>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, nullptr));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, int>
    {
    dpf::EventDispatcherManager m{};
    
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, 7));  // unregistered topic -> false
    }

    // instantiation: EventDispatcherManager::publish<unsigned long long, std::function<void (QSharedPointer<dfmbase::AbstractJobHandler>)>&>
    {
    dpf::EventDispatcherManager m{};
    std::function<void (QSharedPointer<dfmbase::AbstractJobHandler>)> sv1_79 {nullptr};
    EXPECT_FALSE(m.publish(kSweepTopic, 55ull, sv1_79));  // unregistered topic -> false
    }
}

TEST_F(EventTemplateSweepTest, traversalSweep29_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventSequence::traversal<QList<QUrl>>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QList<QUrl>, QUrl>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QUrl::fromLocalFile("/tmp/sweep")));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QList<QUrl>, QUrl&>
    {
    dpf::EventSequence s{};
    QUrl sv1_2 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(s.traversal(QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv1_2));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QString>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(QString::fromLatin1("s")));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QString, QString, QUrl, QUrl>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(QString::fromLatin1("s"), QString::fromLatin1("s"), QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep")));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QString, int&, int&, void*&>
    {
    dpf::EventSequence s{};
    int sv1_5 {7};
    int sv2_5 {7};
    void* sv3_5 {nullptr};
    EXPECT_FALSE(s.traversal(QString::fromLatin1("s"), sv1_5, sv2_5, sv3_5));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QString, int&, void*&>
    {
    dpf::EventSequence s{};
    int sv1_6 {7};
    void* sv2_6 {nullptr};
    EXPECT_FALSE(s.traversal(QString::fromLatin1("s"), sv1_6, sv2_6));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QUrl>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(QUrl::fromLocalFile("/tmp/sweep")));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QUrl, QUrl>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep")));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QUrl, QUrl&>
    {
    dpf::EventSequence s{};
    QUrl sv1_9 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(s.traversal(QUrl::fromLocalFile("/tmp/sweep"), sv1_9));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QUrl, QVariant>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(QUrl::fromLocalFile("/tmp/sweep"), QVariant(42)));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QUrl, bool*>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(QUrl::fromLocalFile("/tmp/sweep"), nullptr));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QUrl, void*&>
    {
    dpf::EventSequence s{};
    void* sv1_12 {nullptr};
    EXPECT_FALSE(s.traversal(QUrl::fromLocalFile("/tmp/sweep"), sv1_12));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<QVariant, QVariant>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(QVariant(42), QVariant(42)));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<int, QPoint, QVariant>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(7, QPoint(1, 2), QVariant(42)));  // no handler accepted -> false
    }
}

TEST_F(EventTemplateSweepTest, traversalSweep30_Batch1_DocumentedNoSubscriber)
{
    // instantiation: EventSequence::traversal<int, QVariant, QPoint, QVariant>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(7, QVariant(42), QPoint(1, 2), QVariant(42)));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<int, int&, int&, void*&>
    {
    dpf::EventSequence s{};
    int sv1_16 {7};
    int sv2_16 {7};
    void* sv3_16 {nullptr};
    EXPECT_FALSE(s.traversal(7, sv1_16, sv2_16, sv3_16));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<int, int&, void*&>
    {
    dpf::EventSequence s{};
    int sv1_17 {7};
    void* sv2_17 {nullptr};
    EXPECT_FALSE(s.traversal(7, sv1_17, sv2_17));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<int, int, int, QVariant>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(7, 7, 7, QVariant(42)));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<int, int, int, decltype(nullptr)>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(7, 7, 7, nullptr));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<unsigned long long, QList<QUrl> >
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<unsigned long long, QList<QUrl>&>
    {
    dpf::EventSequence s{};
    QList<QUrl> sv1_21 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(s.traversal(55ull, sv1_21));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<unsigned long long, QList<QUrl>&, QUrl>
    {
    dpf::EventSequence s{};
    QList<QUrl> sv1_22 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(s.traversal(55ull, sv1_22, QUrl::fromLocalFile("/tmp/sweep")));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<unsigned long long, QList<QUrl>&, QUrl&>
    {
    dpf::EventSequence s{};
    QList<QUrl> sv1_23 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl sv2_23 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(s.traversal(55ull, sv1_23, sv2_23));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<unsigned long long, QList<QUrl>&, dfmbase::AbstractJobHandler::JobFlag>
    {
    dpf::EventSequence s{};
    QList<QUrl> sv1_24 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(s.traversal(55ull, sv1_24, dfmbase::AbstractJobHandler::JobFlag::kNoHint));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<unsigned long long, QList<QUrl>, QUrl>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QUrl::fromLocalFile("/tmp/sweep")));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<unsigned long long, QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag>
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<unsigned long long, QUrl&, QUrl&>
    {
    dpf::EventSequence s{};
    QUrl sv1_27 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_27 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(s.traversal(55ull, sv1_27, sv2_27));  // no handler accepted -> false
    }

    // instantiation: EventSequence::traversal<unsigned long long, QUrl, QUrl, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> >
    {
    dpf::EventSequence s{};
    
    EXPECT_FALSE(s.traversal(55ull, QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep"), QVariant(42), nullptr));  // no handler accepted -> false
    }
}

TEST_F(EventTemplateSweepTest, runSweep31_Batch0_DocumentedNoSubscriber)
{
    // instantiation: EventSequenceManager::run<QList<QUrl>>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QList<QUrl>, QUrl>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QUrl::fromLocalFile("/tmp/sweep")));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QList<QUrl>, QUrl&>
    {
    dpf::EventSequenceManager m{};
    QUrl sv1_2 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.run(kSweepTopic, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, sv1_2));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QString>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, QString::fromLatin1("s")));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QString, QString, QUrl, QUrl>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, QString::fromLatin1("s"), QString::fromLatin1("s"), QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep")));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QString, int&, int&, void*&>
    {
    dpf::EventSequenceManager m{};
    int sv1_5 {7};
    int sv2_5 {7};
    void* sv3_5 {nullptr};
    EXPECT_FALSE(m.run(kSweepTopic, QString::fromLatin1("s"), sv1_5, sv2_5, sv3_5));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QString, int&, void*&>
    {
    dpf::EventSequenceManager m{};
    int sv1_6 {7};
    void* sv2_6 {nullptr};
    EXPECT_FALSE(m.run(kSweepTopic, QString::fromLatin1("s"), sv1_6, sv2_6));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QUrl>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep")));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QUrl, QUrl>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep")));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QUrl, QUrl&>
    {
    dpf::EventSequenceManager m{};
    QUrl sv1_9 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.run(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), sv1_9));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QUrl, QVariant>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QUrl, bool*>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), nullptr));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<QUrl, void*&>
    {
    dpf::EventSequenceManager m{};
    void* sv1_12 {nullptr};
    EXPECT_FALSE(m.run(kSweepTopic, QUrl::fromLocalFile("/tmp/sweep"), sv1_12));  // unregistered topic -> false
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
}

TEST_F(EventTemplateSweepTest, runSweep32_Batch1_DocumentedNoSubscriber)
{
    // instantiation: EventSequenceManager::run<int, QVariant, QPoint, QVariant>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, 7, QVariant(42), QPoint(1, 2), QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<int, int&, int&, void*&>
    {
    dpf::EventSequenceManager m{};
    int sv1_16 {7};
    int sv2_16 {7};
    void* sv3_16 {nullptr};
    EXPECT_FALSE(m.run(kSweepTopic, 7, sv1_16, sv2_16, sv3_16));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<int, int&, void*&>
    {
    dpf::EventSequenceManager m{};
    int sv1_17 {7};
    void* sv2_17 {nullptr};
    EXPECT_FALSE(m.run(kSweepTopic, 7, sv1_17, sv2_17));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<int, int, int, QVariant>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, 7, 7, 7, QVariant(42)));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<int, int, int, decltype(nullptr)>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, 7, 7, 7, nullptr));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<unsigned long long, QList<QUrl> >
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<unsigned long long, QList<QUrl>&>
    {
    dpf::EventSequenceManager m{};
    QList<QUrl> sv1_21 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.run(kSweepTopic, 55ull, sv1_21));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<unsigned long long, QList<QUrl>&, QUrl>
    {
    dpf::EventSequenceManager m{};
    QList<QUrl> sv1_22 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.run(kSweepTopic, 55ull, sv1_22, QUrl::fromLocalFile("/tmp/sweep")));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<unsigned long long, QList<QUrl>&, QUrl&>
    {
    dpf::EventSequenceManager m{};
    QList<QUrl> sv1_23 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl sv2_23 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.run(kSweepTopic, 55ull, sv1_23, sv2_23));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<unsigned long long, QList<QUrl>&, dfmbase::AbstractJobHandler::JobFlag>
    {
    dpf::EventSequenceManager m{};
    QList<QUrl> sv1_24 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    EXPECT_FALSE(m.run(kSweepTopic, 55ull, sv1_24, dfmbase::AbstractJobHandler::JobFlag::kNoHint));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<unsigned long long, QList<QUrl>, QUrl>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QUrl::fromLocalFile("/tmp/sweep")));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<unsigned long long, QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag>
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<unsigned long long, QUrl&, QUrl&>
    {
    dpf::EventSequenceManager m{};
    QUrl sv1_27 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl sv2_27 {QUrl::fromLocalFile("/tmp/sweep")};
    EXPECT_FALSE(m.run(kSweepTopic, 55ull, sv1_27, sv2_27));  // unregistered topic -> false
    }

    // instantiation: EventSequenceManager::run<unsigned long long, QUrl, QUrl, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> >
    {
    dpf::EventSequenceManager m{};
    
    EXPECT_FALSE(m.run(kSweepTopic, 55ull, QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep"), QVariant(42), nullptr));  // unregistered topic -> false
    }
}

TEST_F(EventTemplateSweepTest, makeVariantListSweep33_PacksAllArgs)
{
    // instantiation: makeVariantList<Dtk::Widget::DLabel*, char const (&) [10]>
    {
    QVariantList lst0;
    dpf::makeVariantList(&lst0, nullptr, "xxxxxxxxx");
    EXPECT_EQ(lst0.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<Dtk::Widget::DLabel*, char const (&) [15]>
    {
    QVariantList lst1;
    dpf::makeVariantList(&lst1, nullptr, "xxxxxxxxxxxxxx");
    EXPECT_EQ(lst1.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QByteArray>
    {
    QVariantList lst2;
    dpf::makeVariantList(&lst2, QByteArray("ba"));
    EXPECT_EQ(lst2.count(), 1);  // one variant per param
    }

    // instantiation: makeVariantList<QFrame*, char const (&) [11]>
    {
    QVariantList lst3;
    dpf::makeVariantList(&lst3, nullptr, "xxxxxxxxxx");
    EXPECT_EQ(lst3.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QHash<QString, QVariant>>
    {
    QVariantList lst4;
    dpf::makeVariantList(&lst4, QHash<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}});
    EXPECT_EQ(lst4.count(), 1);  // one variant per param
    }

    // instantiation: makeVariantList<QList<QString>>
    {
    QVariantList lst5;
    dpf::makeVariantList(&lst5, QList<QString>{ QString::fromLatin1("s") });
    EXPECT_EQ(lst5.count(), 1);  // one variant per param
    }

    // instantiation: makeVariantList<QList<QString>, QString>
    {
    QVariantList lst6;
    dpf::makeVariantList(&lst6, QList<QString>{ QString::fromLatin1("s") }, QString::fromLatin1("s"));
    EXPECT_EQ(lst6.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QList<QString>, char const (&) [1]>
    {
    QVariantList lst7;
    dpf::makeVariantList(&lst7, QList<QString>{ QString::fromLatin1("s") }, "");
    EXPECT_EQ(lst7.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QList<QString>, int, QPoint>
    {
    QVariantList lst8;
    dpf::makeVariantList(&lst8, QList<QString>{ QString::fromLatin1("s") }, 7, QPoint(1, 2));
    EXPECT_EQ(lst8.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<QList<QUrl>>
    {
    QVariantList lst9;
    dpf::makeVariantList(&lst9, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") });
    EXPECT_EQ(lst9.count(), 1);  // one variant per param
    }

    // instantiation: makeVariantList<QList<QUrl>, QHash<QString, QVariant> >
    {
    QVariantList lst10;
    dpf::makeVariantList(&lst10, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QHash<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}});
    EXPECT_EQ(lst10.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QList<QUrl>, QHash<QString, QVariant>&>
    {
    QHash<QString, QVariant> pp1 {QHash<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst11;
    dpf::makeVariantList(&lst11, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, pp1);
    EXPECT_EQ(lst11.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QList<QUrl>, QList<QUrl>&, QList<QVariant>&, bool, QString>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QList<QVariant> pp2 {QList<QVariant>{ QVariant(42) }};
    QVariantList lst12;
    dpf::makeVariantList(&lst12, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, pp1, pp2, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst12.count(), 5);  // one variant per param
    }

    // instantiation: makeVariantList<QList<QUrl>, QList<QUrl>&, bool, QString>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst13;
    dpf::makeVariantList(&lst13, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, pp1, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst13.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<QList<QUrl>, QList<QUrl>, bool, QString>
    {
    QVariantList lst14;
    dpf::makeVariantList(&lst14, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst14.count(), 4);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, makeVariantListSweep34_PacksAllArgs)
{
    // instantiation: makeVariantList<QList<QUrl>, QUrl>
    {
    QVariantList lst15;
    dpf::makeVariantList(&lst15, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst15.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QList<QUrl>, QUrl&>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst16;
    dpf::makeVariantList(&lst16, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, pp1);
    EXPECT_EQ(lst16.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QList<QUrl>, bool, QString>
    {
    QVariantList lst17;
    dpf::makeVariantList(&lst17, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst17.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<QMap<QString, QVariant>>
    {
    QVariantList lst18;
    dpf::makeVariantList(&lst18, QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}});
    EXPECT_EQ(lst18.count(), 1);  // one variant per param
    }

    // instantiation: makeVariantList<QString>
    {
    QVariantList lst19;
    dpf::makeVariantList(&lst19, QString::fromLatin1("s"));
    EXPECT_EQ(lst19.count(), 1);  // one variant per param
    }

    // instantiation: makeVariantList<QString, QList<QString>&>
    {
    QList<QString> pp1 {QList<QString>{ QString::fromLatin1("s") }};
    QVariantList lst20;
    dpf::makeVariantList(&lst20, QString::fromLatin1("s"), pp1);
    EXPECT_EQ(lst20.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QString, QList<QUrl>&>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst21;
    dpf::makeVariantList(&lst21, QString::fromLatin1("s"), pp1);
    EXPECT_EQ(lst21.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QString, QMap<QString, QVariant> >
    {
    QVariantList lst22;
    dpf::makeVariantList(&lst22, QString::fromLatin1("s"), QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}});
    EXPECT_EQ(lst22.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QString, QMap<QString, QVariant>&>
    {
    QMap<QString, QVariant> pp1 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst23;
    dpf::makeVariantList(&lst23, QString::fromLatin1("s"), pp1);
    EXPECT_EQ(lst23.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QString, QRect&>
    {
    QRect pp1 {QRect(0, 0, 4, 4)};
    QVariantList lst24;
    dpf::makeVariantList(&lst24, QString::fromLatin1("s"), pp1);
    EXPECT_EQ(lst24.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QString, QString>
    {
    QVariantList lst25;
    dpf::makeVariantList(&lst25, QString::fromLatin1("s"), QString::fromLatin1("s"));
    EXPECT_EQ(lst25.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QString, QString&>
    {
    QString pp1 {QString::fromLatin1("s")};
    QVariantList lst26;
    dpf::makeVariantList(&lst26, QString::fromLatin1("s"), pp1);
    EXPECT_EQ(lst26.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QString, QString, QUrl, QUrl>
    {
    QVariantList lst27;
    dpf::makeVariantList(&lst27, QString::fromLatin1("s"), QString::fromLatin1("s"), QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst27.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<QString, QVariant>
    {
    QVariantList lst28;
    dpf::makeVariantList(&lst28, QString::fromLatin1("s"), QVariant(42));
    EXPECT_EQ(lst28.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QString, QVariant&>
    {
    QVariant pp1 {QVariant(42)};
    QVariantList lst29;
    dpf::makeVariantList(&lst29, QString::fromLatin1("s"), pp1);
    EXPECT_EQ(lst29.count(), 2);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, makeVariantListSweep35_PacksAllArgs)
{
    // instantiation: makeVariantList<QString, bool&, QWidget*&>
    {
    bool pp1 {true};
    QWidget* pp2 {nullptr};
    QVariantList lst30;
    dpf::makeVariantList(&lst30, QString::fromLatin1("s"), pp1, pp2);
    EXPECT_EQ(lst30.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<QString, char const (&) [13]>
    {
    QVariantList lst31;
    dpf::makeVariantList(&lst31, QString::fromLatin1("s"), "xxxxxxxxxxxx");
    EXPECT_EQ(lst31.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QString, dfmbase::Global::ViewMode>
    {
    QVariantList lst32;
    dpf::makeVariantList(&lst32, QString::fromLatin1("s"), dfmbase::Global::ViewMode::kIconMode);
    EXPECT_EQ(lst32.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QString, int&, int&, void*&>
    {
    int pp1 {7};
    int pp2 {7};
    void* pp3 {nullptr};
    QVariantList lst33;
    dpf::makeVariantList(&lst33, QString::fromLatin1("s"), pp1, pp2, pp3);
    EXPECT_EQ(lst33.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<QString, int&, void*&>
    {
    int pp1 {7};
    void* pp2 {nullptr};
    QVariantList lst34;
    dpf::makeVariantList(&lst34, QString::fromLatin1("s"), pp1, pp2);
    EXPECT_EQ(lst34.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<QString, std::function<void (unsigned long long, QUrl const&, std::function<void ()>)>&>
    {
    std::function<void (unsigned long long, QUrl const&, std::function<void ()>)> pp1 {nullptr};
    QVariantList lst35;
    dpf::makeVariantList(&lst35, QString::fromLatin1("s"), pp1);
    EXPECT_EQ(lst35.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QUrl>
    {
    QVariantList lst36;
    dpf::makeVariantList(&lst36, QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst36.count(), 1);  // one variant per param
    }

    // instantiation: makeVariantList<QUrl, QMap<QString, QVariant>&>
    {
    QMap<QString, QVariant> pp1 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst37;
    dpf::makeVariantList(&lst37, QUrl::fromLocalFile("/tmp/sweep"), pp1);
    EXPECT_EQ(lst37.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QUrl, QUrl>
    {
    QVariantList lst38;
    dpf::makeVariantList(&lst38, QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst38.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QUrl, QUrl&>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst39;
    dpf::makeVariantList(&lst39, QUrl::fromLocalFile("/tmp/sweep"), pp1);
    EXPECT_EQ(lst39.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QUrl, QVariant>
    {
    QVariantList lst40;
    dpf::makeVariantList(&lst40, QUrl::fromLocalFile("/tmp/sweep"), QVariant(42));
    EXPECT_EQ(lst40.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QUrl, bool>
    {
    QVariantList lst41;
    dpf::makeVariantList(&lst41, QUrl::fromLocalFile("/tmp/sweep"), true);
    EXPECT_EQ(lst41.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QUrl, bool&>
    {
    bool pp1 {true};
    QVariantList lst42;
    dpf::makeVariantList(&lst42, QUrl::fromLocalFile("/tmp/sweep"), pp1);
    EXPECT_EQ(lst42.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QUrl, bool*>
    {
    QVariantList lst43;
    dpf::makeVariantList(&lst43, QUrl::fromLocalFile("/tmp/sweep"), nullptr);
    EXPECT_EQ(lst43.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QUrl, int>
    {
    QVariantList lst44;
    dpf::makeVariantList(&lst44, QUrl::fromLocalFile("/tmp/sweep"), 7);
    EXPECT_EQ(lst44.count(), 2);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, makeVariantListSweep36_PacksAllArgs)
{
    // instantiation: makeVariantList<QUrl, void*&>
    {
    void* pp1 {nullptr};
    QVariantList lst45;
    dpf::makeVariantList(&lst45, QUrl::fromLocalFile("/tmp/sweep"), pp1);
    EXPECT_EQ(lst45.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QVariant>
    {
    QVariantList lst46;
    dpf::makeVariantList(&lst46, QVariant(42));
    EXPECT_EQ(lst46.count(), 1);  // one variant per param
    }

    // instantiation: makeVariantList<QVariant, QVariant>
    {
    QVariantList lst47;
    dpf::makeVariantList(&lst47, QVariant(42), QVariant(42));
    EXPECT_EQ(lst47.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QVariant, QVariant, QVariant>
    {
    QVariantList lst48;
    dpf::makeVariantList(&lst48, QVariant(42), QVariant(42), QVariant(42));
    EXPECT_EQ(lst48.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<QVariant, QVariant, QVariant, int>
    {
    QVariantList lst49;
    dpf::makeVariantList(&lst49, QVariant(42), QVariant(42), QVariant(42), 7);
    EXPECT_EQ(lst49.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, QString>
    {
    QVariantList lst50;
    dpf::makeVariantList(&lst50, nullptr, QString::fromLatin1("s"));
    EXPECT_EQ(lst50.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [11]>
    {
    QVariantList lst51;
    dpf::makeVariantList(&lst51, nullptr, "xxxxxxxxxx");
    EXPECT_EQ(lst51.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [12]>
    {
    QVariantList lst52;
    dpf::makeVariantList(&lst52, nullptr, "xxxxxxxxxxx");
    EXPECT_EQ(lst52.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [13]>
    {
    QVariantList lst53;
    dpf::makeVariantList(&lst53, nullptr, "xxxxxxxxxxxx");
    EXPECT_EQ(lst53.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [14]>
    {
    QVariantList lst54;
    dpf::makeVariantList(&lst54, nullptr, "xxxxxxxxxxxxx");
    EXPECT_EQ(lst54.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [15]>
    {
    QVariantList lst55;
    dpf::makeVariantList(&lst55, nullptr, "xxxxxxxxxxxxxx");
    EXPECT_EQ(lst55.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [16]>
    {
    QVariantList lst56;
    dpf::makeVariantList(&lst56, nullptr, "xxxxxxxxxxxxxxx");
    EXPECT_EQ(lst56.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [17]>
    {
    QVariantList lst57;
    dpf::makeVariantList(&lst57, nullptr, "xxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst57.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [19]>
    {
    QVariantList lst58;
    dpf::makeVariantList(&lst58, nullptr, "xxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst58.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [21]>
    {
    QVariantList lst59;
    dpf::makeVariantList(&lst59, nullptr, "xxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst59.count(), 2);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, makeVariantListSweep37_PacksAllArgs)
{
    // instantiation: makeVariantList<QWidget*, char const (&) [22]>
    {
    QVariantList lst60;
    dpf::makeVariantList(&lst60, nullptr, "xxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst60.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [23]>
    {
    QVariantList lst61;
    dpf::makeVariantList(&lst61, nullptr, "xxxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst61.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [24]>
    {
    QVariantList lst62;
    dpf::makeVariantList(&lst62, nullptr, "xxxxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst62.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [25]>
    {
    QVariantList lst63;
    dpf::makeVariantList(&lst63, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst63.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [26]>
    {
    QVariantList lst64;
    dpf::makeVariantList(&lst64, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst64.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [27]>
    {
    QVariantList lst65;
    dpf::makeVariantList(&lst65, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst65.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [28]>
    {
    QVariantList lst66;
    dpf::makeVariantList(&lst66, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst66.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [29]>
    {
    QVariantList lst67;
    dpf::makeVariantList(&lst67, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst67.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [30]>
    {
    QVariantList lst68;
    dpf::makeVariantList(&lst68, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst68.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [31]>
    {
    QVariantList lst69;
    dpf::makeVariantList(&lst69, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst69.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [32]>
    {
    QVariantList lst70;
    dpf::makeVariantList(&lst70, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst70.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [36]>
    {
    QVariantList lst71;
    dpf::makeVariantList(&lst71, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst71.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [37]>
    {
    QVariantList lst72;
    dpf::makeVariantList(&lst72, nullptr, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst72.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<QWidget*, char const (&) [8]>
    {
    QVariantList lst73;
    dpf::makeVariantList(&lst73, nullptr, "xxxxxxx");
    EXPECT_EQ(lst73.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<bool>
    {
    QVariantList lst74;
    dpf::makeVariantList(&lst74, true);
    EXPECT_EQ(lst74.count(), 1);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, makeVariantListSweep38_PacksAllArgs)
{
    // instantiation: makeVariantList<bool, int&, bool&>
    {
    int pp1 {7};
    bool pp2 {true};
    QVariantList lst75;
    dpf::makeVariantList(&lst75, true, pp1, pp2);
    EXPECT_EQ(lst75.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<bool, int, bool>
    {
    QVariantList lst76;
    dpf::makeVariantList(&lst76, true, 7, true);
    EXPECT_EQ(lst76.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<int>
    {
    QVariantList lst77;
    dpf::makeVariantList(&lst77, 7);
    EXPECT_EQ(lst77.count(), 1);  // one variant per param
    }

    // instantiation: makeVariantList<int, QList<QUrl>&>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst78;
    dpf::makeVariantList(&lst78, 7, pp1);
    EXPECT_EQ(lst78.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<int, QList<QUrl>&, QList<QString> >
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst79;
    dpf::makeVariantList(&lst79, 7, pp1, QList<QString>{ QString::fromLatin1("s") });
    EXPECT_EQ(lst79.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<int, QList<QUrl>&, QList<QString>&>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QList<QString> pp2 {QList<QString>{ QString::fromLatin1("s") }};
    QVariantList lst80;
    dpf::makeVariantList(&lst80, 7, pp1, pp2);
    EXPECT_EQ(lst80.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<int, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst81;
    dpf::makeVariantList(&lst81, 7, pp1, pp2, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst81.count(), 5);  // one variant per param
    }

    // instantiation: makeVariantList<int, QList<QUrl>&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst82;
    dpf::makeVariantList(&lst82, 7, pp1, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst82.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<int, QPoint>
    {
    QVariantList lst83;
    dpf::makeVariantList(&lst83, 7, QPoint(1, 2));
    EXPECT_EQ(lst83.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<int, QPoint, QVariant>
    {
    QVariantList lst84;
    dpf::makeVariantList(&lst84, 7, QPoint(1, 2), QVariant(42));
    EXPECT_EQ(lst84.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<int, QRect>
    {
    QVariantList lst85;
    dpf::makeVariantList(&lst85, 7, QRect(0, 0, 4, 4));
    EXPECT_EQ(lst85.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<int, QRect&>
    {
    QRect pp1 {QRect(0, 0, 4, 4)};
    QVariantList lst86;
    dpf::makeVariantList(&lst86, 7, pp1);
    EXPECT_EQ(lst86.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<int, QUrl&>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst87;
    dpf::makeVariantList(&lst87, 7, pp1);
    EXPECT_EQ(lst87.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<int, QUrl, QMap<QString, QVariant>&>
    {
    QMap<QString, QVariant> pp2 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst88;
    dpf::makeVariantList(&lst88, 7, QUrl::fromLocalFile("/tmp/sweep"), pp2);
    EXPECT_EQ(lst88.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<int, QVariant, QPoint, QVariant>
    {
    QVariantList lst89;
    dpf::makeVariantList(&lst89, 7, QVariant(42), QPoint(1, 2), QVariant(42));
    EXPECT_EQ(lst89.count(), 4);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, makeVariantListSweep39_PacksAllArgs)
{
    // instantiation: makeVariantList<int, int>
    {
    QVariantList lst90;
    dpf::makeVariantList(&lst90, 7, 7);
    EXPECT_EQ(lst90.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<int, int&, int&, void*&>
    {
    int pp1 {7};
    int pp2 {7};
    void* pp3 {nullptr};
    QVariantList lst91;
    dpf::makeVariantList(&lst91, 7, pp1, pp2, pp3);
    EXPECT_EQ(lst91.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<int, int&, void*&>
    {
    int pp1 {7};
    void* pp2 {nullptr};
    QVariantList lst92;
    dpf::makeVariantList(&lst92, 7, pp1, pp2);
    EXPECT_EQ(lst92.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<int, int, int, QVariant>
    {
    QVariantList lst93;
    dpf::makeVariantList(&lst93, 7, 7, 7, QVariant(42));
    EXPECT_EQ(lst93.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<int, int, int, decltype(nullptr)>
    {
    QVariantList lst94;
    dpf::makeVariantList(&lst94, 7, 7, 7, nullptr);
    EXPECT_EQ(lst94.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<std::function<QMap<QString, QMultiMap<QString, std::pair<QString, QString> > > (QUrl const&)>, QString>
    {
    QVariantList lst95;
    dpf::makeVariantList(&lst95, nullptr, QString::fromLatin1("s"));
    EXPECT_EQ(lst95.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<std::function<QWidget* (QUrl const&)>, QString>
    {
    QVariantList lst96;
    dpf::makeVariantList(&lst96, nullptr, QString::fromLatin1("s"));
    EXPECT_EQ(lst96.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<std::function<QWidget* (QUrl const&)>, QString&, int&>
    {
    QString pp1 {QString::fromLatin1("s")};
    int pp2 {7};
    QVariantList lst97;
    dpf::makeVariantList(&lst97, nullptr, pp1, pp2);
    EXPECT_EQ(lst97.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<std::function<QWidget* (QUrl const&)>, int&>
    {
    int pp1 {7};
    QVariantList lst98;
    dpf::makeVariantList(&lst98, nullptr, pp1);
    EXPECT_EQ(lst98.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<std::function<QWidget* (QUrl const&)>, std::function<void (QWidget*, QUrl const&)>&, QString, int>
    {
    std::function<void (QWidget*, QUrl const&)> pp1 {nullptr};
    QVariantList lst99;
    dpf::makeVariantList(&lst99, nullptr, pp1, QString::fromLatin1("s"), 7);
    EXPECT_EQ(lst99.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long>
    {
    QVariantList lst100;
    dpf::makeVariantList(&lst100, 55ull);
    EXPECT_EQ(lst100.count(), 1);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QString> >
    {
    QVariantList lst101;
    dpf::makeVariantList(&lst101, 55ull, QList<QString>{ QString::fromLatin1("s") });
    EXPECT_EQ(lst101.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QString>&>
    {
    QList<QString> pp1 {QList<QString>{ QString::fromLatin1("s") }};
    QVariantList lst102;
    dpf::makeVariantList(&lst102, 55ull, pp1);
    EXPECT_EQ(lst102.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl> >
    {
    QVariantList lst103;
    dpf::makeVariantList(&lst103, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") });
    EXPECT_EQ(lst103.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst104;
    dpf::makeVariantList(&lst104, 55ull, pp1);
    EXPECT_EQ(lst104.count(), 2);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, makeVariantListSweep40_PacksAllArgs)
{
    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, QList<QUrl>&>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QList<QUrl> pp2 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst105;
    dpf::makeVariantList(&lst105, 55ull, pp1, pp2);
    EXPECT_EQ(lst105.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, QUrl>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst106;
    dpf::makeVariantList(&lst106, 55ull, pp1, QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst106.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, QUrl&>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst107;
    dpf::makeVariantList(&lst107, 55ull, pp1, pp2);
    EXPECT_EQ(lst107.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, QVariant>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst108;
    dpf::makeVariantList(&lst108, 55ull, pp1, pp2, dfmbase::AbstractJobHandler::JobFlag::kNoHint, QVariant(42));
    EXPECT_EQ(lst108.count(), 5);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst109;
    dpf::makeVariantList(&lst109, 55ull, pp1, pp2, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst109.count(), 5);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), decltype(nullptr), QVariant, decltype(nullptr)>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst110;
    dpf::makeVariantList(&lst110, 55ull, pp1, pp2, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(42), nullptr);
    EXPECT_EQ(lst110.count(), 8);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, QUrl, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst111;
    dpf::makeVariantList(&lst111, 55ull, pp1, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst111.count(), 5);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, QUrl, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariant pp5 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp6 {nullptr};
    QVariantList lst112;
    dpf::makeVariantList(&lst112, 55ull, pp1, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, pp5, pp6);
    EXPECT_EQ(lst112.count(), 7);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, QVariant, QVariant>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst113;
    dpf::makeVariantList(&lst113, 55ull, pp1, QVariant(42), QVariant(42));
    EXPECT_EQ(lst113.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, bool>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst114;
    dpf::makeVariantList(&lst114, 55ull, pp1, true);
    EXPECT_EQ(lst114.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, bool&, QString&>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    bool pp2 {true};
    QString pp3 {QString::fromLatin1("s")};
    QVariantList lst115;
    dpf::makeVariantList(&lst115, 55ull, pp1, pp2, pp3);
    EXPECT_EQ(lst115.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, bool, QString>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst116;
    dpf::makeVariantList(&lst116, 55ull, pp1, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst116.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, bool, QString&>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QString pp3 {QString::fromLatin1("s")};
    QVariantList lst117;
    dpf::makeVariantList(&lst117, 55ull, pp1, true, pp3);
    EXPECT_EQ(lst117.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, bool, char const (&) [21]>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst118;
    dpf::makeVariantList(&lst118, 55ull, pp1, true, "xxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst118.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, decltype(nullptr), QVariant, QVariant>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst119;
    dpf::makeVariantList(&lst119, 55ull, pp1, nullptr, QVariant(42), QVariant(42));
    EXPECT_EQ(lst119.count(), 5);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, makeVariantListSweep41_PacksAllArgs)
{
    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, dfmbase::AbstractJobHandler::JobFlag>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst120;
    dpf::makeVariantList(&lst120, 55ull, pp1, dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_EQ(lst120.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst121;
    dpf::makeVariantList(&lst121, 55ull, pp1, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst121.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>, QList<QString> >
    {
    QVariantList lst122;
    dpf::makeVariantList(&lst122, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QList<QString>{ QString::fromLatin1("s") });
    EXPECT_EQ(lst122.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>, QList<QString>, QVariant, QVariant>
    {
    QVariantList lst123;
    dpf::makeVariantList(&lst123, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QList<QString>{ QString::fromLatin1("s") }, QVariant(42), QVariant(42));
    EXPECT_EQ(lst123.count(), 5);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>, QUrl>
    {
    QVariantList lst124;
    dpf::makeVariantList(&lst124, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst124.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst125;
    dpf::makeVariantList(&lst125, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, pp2, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst125.count(), 5);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant, QVariant>
    {
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst126;
    dpf::makeVariantList(&lst126, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, pp2, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(42), QVariant(42));
    EXPECT_EQ(lst126.count(), 7);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>, QVariant>
    {
    QVariantList lst127;
    dpf::makeVariantList(&lst127, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QVariant(42));
    EXPECT_EQ(lst127.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>, QVariant, QVariant>
    {
    QVariantList lst128;
    dpf::makeVariantList(&lst128, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QVariant(42), QVariant(42));
    EXPECT_EQ(lst128.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag>
    {
    QVariantList lst129;
    dpf::makeVariantList(&lst129, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_EQ(lst129.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QVariantList lst130;
    dpf::makeVariantList(&lst130, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst130.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant, QVariant>
    {
    QVariantList lst131;
    dpf::makeVariantList(&lst131, 55ull, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(42), QVariant(42));
    EXPECT_EQ(lst131.count(), 6);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QList<int>&>
    {
    QList<int> pp1 {QList<int>{}};
    QVariantList lst132;
    dpf::makeVariantList(&lst132, 55ull, pp1);
    EXPECT_EQ(lst132.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QMap<QString, QVariant>&>
    {
    QMap<QString, QVariant> pp1 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst133;
    dpf::makeVariantList(&lst133, 55ull, pp1);
    EXPECT_EQ(lst133.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QMap<QUrl, QUrl>&, bool&, QString&>
    {
    QMap<QUrl, QUrl> pp1 {QMap<QUrl, QUrl>{}};
    bool pp2 {true};
    QString pp3 {QString::fromLatin1("s")};
    QVariantList lst134;
    dpf::makeVariantList(&lst134, 55ull, pp1, pp2, pp3);
    EXPECT_EQ(lst134.count(), 4);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, makeVariantListSweep42_PacksAllArgs)
{
    // instantiation: makeVariantList<unsigned long long, QMap<QUrl, QUrl>&, bool, QString>
    {
    QMap<QUrl, QUrl> pp1 {QMap<QUrl, QUrl>{}};
    QVariantList lst135;
    dpf::makeVariantList(&lst135, 55ull, pp1, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst135.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QMap<QUrl, QUrl>&, bool, QString&>
    {
    QMap<QUrl, QUrl> pp1 {QMap<QUrl, QUrl>{}};
    QString pp3 {QString::fromLatin1("s")};
    QVariantList lst136;
    dpf::makeVariantList(&lst136, 55ull, pp1, true, pp3);
    EXPECT_EQ(lst136.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QMap<QUrl, QUrl>&, bool, char const (&) [1]>
    {
    QMap<QUrl, QUrl> pp1 {QMap<QUrl, QUrl>{}};
    QVariantList lst137;
    dpf::makeVariantList(&lst137, 55ull, pp1, true, "");
    EXPECT_EQ(lst137.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QMap<QUrl, QUrl>, bool, QString>
    {
    QVariantList lst138;
    dpf::makeVariantList(&lst138, 55ull, QMap<QUrl, QUrl>{}, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst138.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QString>
    {
    QVariantList lst139;
    dpf::makeVariantList(&lst139, 55ull, QString::fromLatin1("s"));
    EXPECT_EQ(lst139.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QString&, QList<QUrl>&>
    {
    QString pp1 {QString::fromLatin1("s")};
    QList<QUrl> pp2 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst140;
    dpf::makeVariantList(&lst140, 55ull, pp1, pp2);
    EXPECT_EQ(lst140.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QString, QList<QUrl> >
    {
    QVariantList lst141;
    dpf::makeVariantList(&lst141, 55ull, QString::fromLatin1("s"), QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") });
    EXPECT_EQ(lst141.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QString, QString>
    {
    QVariantList lst142;
    dpf::makeVariantList(&lst142, 55ull, QString::fromLatin1("s"), QString::fromLatin1("s"));
    EXPECT_EQ(lst142.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QString, bool>
    {
    QVariantList lst143;
    dpf::makeVariantList(&lst143, 55ull, QString::fromLatin1("s"), true);
    EXPECT_EQ(lst143.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QString, bool&>
    {
    bool pp2 {true};
    QVariantList lst144;
    dpf::makeVariantList(&lst144, 55ull, QString::fromLatin1("s"), pp2);
    EXPECT_EQ(lst144.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl>
    {
    QVariantList lst145;
    dpf::makeVariantList(&lst145, 55ull, QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst145.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst146;
    dpf::makeVariantList(&lst146, 55ull, pp1);
    EXPECT_EQ(lst146.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QFileDevice::Permission, QVariant, QVariant>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst147;
    dpf::makeVariantList(&lst147, 55ull, pp1, QFileDevice::ReadOwner, QVariant(42), QVariant(42));
    EXPECT_EQ(lst147.count(), 5);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QFlags<QFileDevice::Permission>&>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QFlags<QFileDevice::Permission> pp2 {QFileDevice::Permissions(QFileDevice::ReadOwner)};
    QVariantList lst148;
    dpf::makeVariantList(&lst148, 55ull, pp1, pp2);
    EXPECT_EQ(lst148.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QList<QUrl>, bool>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst149;
    dpf::makeVariantList(&lst149, 55ull, pp1, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true);
    EXPECT_EQ(lst149.count(), 4);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, makeVariantListSweep43_PacksAllArgs)
{
    // instantiation: makeVariantList<unsigned long long, QUrl&, QUrl&>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst150;
    dpf::makeVariantList(&lst150, 55ull, pp1, pp2);
    EXPECT_EQ(lst150.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QUrl&, QString>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst151;
    dpf::makeVariantList(&lst151, 55ull, pp1, pp2, QString::fromLatin1("s"));
    EXPECT_EQ(lst151.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QUrl&, QString, QVariant, QVariant>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst152;
    dpf::makeVariantList(&lst152, 55ull, pp1, pp2, QString::fromLatin1("s"), QVariant(42), QVariant(42));
    EXPECT_EQ(lst152.count(), 6);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QUrl&, bool, bool>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst153;
    dpf::makeVariantList(&lst153, 55ull, pp1, pp2, true, true);
    EXPECT_EQ(lst153.count(), 5);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QUrl&, bool, bool, QVariant, QVariant>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst154;
    dpf::makeVariantList(&lst154, 55ull, pp1, pp2, true, true, QVariant(42), QVariant(42));
    EXPECT_EQ(lst154.count(), 7);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QUrl&, dfmbase::AbstractJobHandler::JobFlag>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst155;
    dpf::makeVariantList(&lst155, 55ull, pp1, pp2, dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_EQ(lst155.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, QVariant, QVariant>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp2 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst156;
    dpf::makeVariantList(&lst156, 55ull, pp1, pp2, dfmbase::AbstractJobHandler::JobFlag::kNoHint, QVariant(42), QVariant(42));
    EXPECT_EQ(lst156.count(), 6);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QUrl, bool, bool>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst157;
    dpf::makeVariantList(&lst157, 55ull, pp1, QUrl::fromLocalFile("/tmp/sweep"), true, true);
    EXPECT_EQ(lst157.count(), 5);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QUrl, char const (&) [1]>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst158;
    dpf::makeVariantList(&lst158, 55ull, pp1, QUrl::fromLocalFile("/tmp/sweep"), "");
    EXPECT_EQ(lst158.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QVariant>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst159;
    dpf::makeVariantList(&lst159, 55ull, pp1, QVariant(42));
    EXPECT_EQ(lst159.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, QVariant, QVariant>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst160;
    dpf::makeVariantList(&lst160, 55ull, pp1, QVariant(42), QVariant(42));
    EXPECT_EQ(lst160.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, dfmbase::Global::CreateFileType, QString>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst161;
    dpf::makeVariantList(&lst161, 55ull, pp1, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"));
    EXPECT_EQ(lst161.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, dfmbase::Global::CreateFileType, QString, QMap<QString, QVariant>&, decltype(nullptr)>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QMap<QString, QVariant> pp4 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst162;
    dpf::makeVariantList(&lst162, 55ull, pp1, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), pp4, nullptr);
    EXPECT_EQ(lst162.count(), 6);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl&, dfmbase::Global::CreateFileType, QString, QVariant, QVariant>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst163;
    dpf::makeVariantList(&lst163, 55ull, pp1, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), QVariant(42), QVariant(42));
    EXPECT_EQ(lst163.count(), 6);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl, QUrl, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> >
    {
    QVariantList lst164;
    dpf::makeVariantList(&lst164, 55ull, QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep"), QVariant(42), nullptr);
    EXPECT_EQ(lst164.count(), 5);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, makeVariantListSweep44_PacksAllArgs)
{
    // instantiation: makeVariantList<unsigned long long, QUrl, QVariant>
    {
    QVariantList lst165;
    dpf::makeVariantList(&lst165, 55ull, QUrl::fromLocalFile("/tmp/sweep"), QVariant(42));
    EXPECT_EQ(lst165.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl, QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    QVariant pp2 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp3 {nullptr};
    QVariantList lst166;
    dpf::makeVariantList(&lst166, 55ull, QUrl::fromLocalFile("/tmp/sweep"), pp2, pp3);
    EXPECT_EQ(lst166.count(), 4);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QUrl, dfmbase::Global::CreateFileType, QString, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp5 {nullptr};
    QVariantList lst167;
    dpf::makeVariantList(&lst167, 55ull, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), QVariant(42), pp5);
    EXPECT_EQ(lst167.count(), 6);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, QVariant>
    {
    QVariantList lst168;
    dpf::makeVariantList(&lst168, 55ull, QVariant(42));
    EXPECT_EQ(lst168.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, bool>
    {
    QVariantList lst169;
    dpf::makeVariantList(&lst169, 55ull, true);
    EXPECT_EQ(lst169.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, bool&>
    {
    bool pp1 {true};
    QVariantList lst170;
    dpf::makeVariantList(&lst170, 55ull, pp1);
    EXPECT_EQ(lst170.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, bool&, bool&>
    {
    bool pp1 {true};
    bool pp2 {true};
    QVariantList lst171;
    dpf::makeVariantList(&lst171, 55ull, pp1, pp2);
    EXPECT_EQ(lst171.count(), 3);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, decltype(nullptr)>
    {
    QVariantList lst172;
    dpf::makeVariantList(&lst172, 55ull, nullptr);
    EXPECT_EQ(lst172.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, int>
    {
    QVariantList lst173;
    dpf::makeVariantList(&lst173, 55ull, 7);
    EXPECT_EQ(lst173.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, int&>
    {
    int pp1 {7};
    QVariantList lst174;
    dpf::makeVariantList(&lst174, 55ull, pp1);
    EXPECT_EQ(lst174.count(), 2);  // one variant per param
    }

    // instantiation: makeVariantList<unsigned long long, std::function<void (QSharedPointer<dfmbase::AbstractJobHandler>)>&>
    {
    std::function<void (QSharedPointer<dfmbase::AbstractJobHandler>)> pp1 {nullptr};
    QVariantList lst175;
    dpf::makeVariantList(&lst175, 55ull, pp1);
    EXPECT_EQ(lst175.count(), 2);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, packParamsHelperSweep45_PacksAllArgs)
{
    // instantiation: packParamsHelper<QFileDevice::Permission, QVariant, QVariant>
    {
    QVariantList lst0;
    dpf::packParamsHelper(lst0, QFileDevice::ReadOwner, QVariant(42), QVariant(42));
    EXPECT_EQ(lst0.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QFlags<QFileDevice::Permission>&>
    {
    QFlags<QFileDevice::Permission> pp0 {QFileDevice::Permissions(QFileDevice::ReadOwner)};
    QVariantList lst1;
    dpf::packParamsHelper(lst1, pp0);
    EXPECT_EQ(lst1.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QHash<QString, QVariant> >
    {
    QVariantList lst2;
    dpf::packParamsHelper(lst2, QHash<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}});
    EXPECT_EQ(lst2.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QHash<QString, QVariant>&>
    {
    QHash<QString, QVariant> pp0 {QHash<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst3;
    dpf::packParamsHelper(lst3, pp0);
    EXPECT_EQ(lst3.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QString> >
    {
    QVariantList lst4;
    dpf::packParamsHelper(lst4, QList<QString>{ QString::fromLatin1("s") });
    EXPECT_EQ(lst4.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QString>&>
    {
    QList<QString> pp0 {QList<QString>{ QString::fromLatin1("s") }};
    QVariantList lst5;
    dpf::packParamsHelper(lst5, pp0);
    EXPECT_EQ(lst5.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QString>, QVariant, QVariant>
    {
    QVariantList lst6;
    dpf::packParamsHelper(lst6, QList<QString>{ QString::fromLatin1("s") }, QVariant(42), QVariant(42));
    EXPECT_EQ(lst6.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl> >
    {
    QVariantList lst7;
    dpf::packParamsHelper(lst7, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") });
    EXPECT_EQ(lst7.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst8;
    dpf::packParamsHelper(lst8, pp0);
    EXPECT_EQ(lst8.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, QList<QString> >
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst9;
    dpf::packParamsHelper(lst9, pp0, QList<QString>{ QString::fromLatin1("s") });
    EXPECT_EQ(lst9.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, QList<QString>&>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QList<QString> pp1 {QList<QString>{ QString::fromLatin1("s") }};
    QVariantList lst10;
    dpf::packParamsHelper(lst10, pp0, pp1);
    EXPECT_EQ(lst10.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, QList<QUrl>&>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst11;
    dpf::packParamsHelper(lst11, pp0, pp1);
    EXPECT_EQ(lst11.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, QList<QVariant>&, bool, QString>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QList<QVariant> pp1 {QList<QVariant>{ QVariant(42) }};
    QVariantList lst12;
    dpf::packParamsHelper(lst12, pp0, pp1, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst12.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, QUrl>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst13;
    dpf::packParamsHelper(lst13, pp0, QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst13.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, QUrl&>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst14;
    dpf::packParamsHelper(lst14, pp0, pp1);
    EXPECT_EQ(lst14.count(), 2);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, packParamsHelperSweep46_PacksAllArgs)
{
    // instantiation: packParamsHelper<QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, QVariant>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst15;
    dpf::packParamsHelper(lst15, pp0, pp1, dfmbase::AbstractJobHandler::JobFlag::kNoHint, QVariant(42));
    EXPECT_EQ(lst15.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst16;
    dpf::packParamsHelper(lst16, pp0, pp1, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst16.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), decltype(nullptr), QVariant, decltype(nullptr)>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst17;
    dpf::packParamsHelper(lst17, pp0, pp1, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(42), nullptr);
    EXPECT_EQ(lst17.count(), 7);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, QUrl, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst18;
    dpf::packParamsHelper(lst18, pp0, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst18.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, QUrl, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariant pp4 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp5 {nullptr};
    QVariantList lst19;
    dpf::packParamsHelper(lst19, pp0, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, pp4, pp5);
    EXPECT_EQ(lst19.count(), 6);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, QVariant, QVariant>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst20;
    dpf::packParamsHelper(lst20, pp0, QVariant(42), QVariant(42));
    EXPECT_EQ(lst20.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, bool>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst21;
    dpf::packParamsHelper(lst21, pp0, true);
    EXPECT_EQ(lst21.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, bool&, QString&>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    bool pp1 {true};
    QString pp2 {QString::fromLatin1("s")};
    QVariantList lst22;
    dpf::packParamsHelper(lst22, pp0, pp1, pp2);
    EXPECT_EQ(lst22.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, bool, QString>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst23;
    dpf::packParamsHelper(lst23, pp0, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst23.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, bool, QString&>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QString pp2 {QString::fromLatin1("s")};
    QVariantList lst24;
    dpf::packParamsHelper(lst24, pp0, true, pp2);
    EXPECT_EQ(lst24.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, bool, char const (&) [21]>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst25;
    dpf::packParamsHelper(lst25, pp0, true, "xxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst25.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, decltype(nullptr), QVariant, QVariant>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst26;
    dpf::packParamsHelper(lst26, pp0, nullptr, QVariant(42), QVariant(42));
    EXPECT_EQ(lst26.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, dfmbase::AbstractJobHandler::JobFlag>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst27;
    dpf::packParamsHelper(lst27, pp0, dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_EQ(lst27.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QList<QUrl> pp0 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst28;
    dpf::packParamsHelper(lst28, pp0, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst28.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>, QList<QString> >
    {
    QVariantList lst29;
    dpf::packParamsHelper(lst29, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QList<QString>{ QString::fromLatin1("s") });
    EXPECT_EQ(lst29.count(), 2);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, packParamsHelperSweep47_PacksAllArgs)
{
    // instantiation: packParamsHelper<QList<QUrl>, QList<QString>, QVariant, QVariant>
    {
    QVariantList lst30;
    dpf::packParamsHelper(lst30, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QList<QString>{ QString::fromLatin1("s") }, QVariant(42), QVariant(42));
    EXPECT_EQ(lst30.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>, QUrl>
    {
    QVariantList lst31;
    dpf::packParamsHelper(lst31, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst31.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst32;
    dpf::packParamsHelper(lst32, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, pp1, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst32.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>, QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant, QVariant>
    {
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst33;
    dpf::packParamsHelper(lst33, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, pp1, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(42), QVariant(42));
    EXPECT_EQ(lst33.count(), 6);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>, QVariant>
    {
    QVariantList lst34;
    dpf::packParamsHelper(lst34, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QVariant(42));
    EXPECT_EQ(lst34.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>, QVariant, QVariant>
    {
    QVariantList lst35;
    dpf::packParamsHelper(lst35, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, QVariant(42), QVariant(42));
    EXPECT_EQ(lst35.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>, bool>
    {
    QVariantList lst36;
    dpf::packParamsHelper(lst36, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true);
    EXPECT_EQ(lst36.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>, bool, QString>
    {
    QVariantList lst37;
    dpf::packParamsHelper(lst37, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst37.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag>
    {
    QVariantList lst38;
    dpf::packParamsHelper(lst38, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_EQ(lst38.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QVariantList lst39;
    dpf::packParamsHelper(lst39, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst39.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QUrl>, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant, QVariant>
    {
    QVariantList lst40;
    dpf::packParamsHelper(lst40, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(42), QVariant(42));
    EXPECT_EQ(lst40.count(), 5);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<QVariant>&, bool, QString>
    {
    QList<QVariant> pp0 {QList<QVariant>{ QVariant(42) }};
    QVariantList lst41;
    dpf::packParamsHelper(lst41, pp0, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst41.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QList<int>&>
    {
    QList<int> pp0 {QList<int>{}};
    QVariantList lst42;
    dpf::packParamsHelper(lst42, pp0);
    EXPECT_EQ(lst42.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QMap<QString, QVariant> >
    {
    QVariantList lst43;
    dpf::packParamsHelper(lst43, QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}});
    EXPECT_EQ(lst43.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QMap<QString, QVariant>&>
    {
    QMap<QString, QVariant> pp0 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst44;
    dpf::packParamsHelper(lst44, pp0);
    EXPECT_EQ(lst44.count(), 1);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, packParamsHelperSweep48_PacksAllArgs)
{
    // instantiation: packParamsHelper<QMap<QString, QVariant>&, decltype(nullptr)>
    {
    QMap<QString, QVariant> pp0 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst45;
    dpf::packParamsHelper(lst45, pp0, nullptr);
    EXPECT_EQ(lst45.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QMap<QUrl, QUrl>&, bool&, QString&>
    {
    QMap<QUrl, QUrl> pp0 {QMap<QUrl, QUrl>{}};
    bool pp1 {true};
    QString pp2 {QString::fromLatin1("s")};
    QVariantList lst46;
    dpf::packParamsHelper(lst46, pp0, pp1, pp2);
    EXPECT_EQ(lst46.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QMap<QUrl, QUrl>&, bool, QString>
    {
    QMap<QUrl, QUrl> pp0 {QMap<QUrl, QUrl>{}};
    QVariantList lst47;
    dpf::packParamsHelper(lst47, pp0, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst47.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QMap<QUrl, QUrl>&, bool, QString&>
    {
    QMap<QUrl, QUrl> pp0 {QMap<QUrl, QUrl>{}};
    QString pp2 {QString::fromLatin1("s")};
    QVariantList lst48;
    dpf::packParamsHelper(lst48, pp0, true, pp2);
    EXPECT_EQ(lst48.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QMap<QUrl, QUrl>&, bool, char const (&) [1]>
    {
    QMap<QUrl, QUrl> pp0 {QMap<QUrl, QUrl>{}};
    QVariantList lst49;
    dpf::packParamsHelper(lst49, pp0, true, "");
    EXPECT_EQ(lst49.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QMap<QUrl, QUrl>, bool, QString>
    {
    QVariantList lst50;
    dpf::packParamsHelper(lst50, QMap<QUrl, QUrl>{}, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst50.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QPoint>
    {
    QVariantList lst51;
    dpf::packParamsHelper(lst51, QPoint(1, 2));
    EXPECT_EQ(lst51.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QPoint, QVariant>
    {
    QVariantList lst52;
    dpf::packParamsHelper(lst52, QPoint(1, 2), QVariant(42));
    EXPECT_EQ(lst52.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QPoint, void*>
    {
    QVariantList lst53;
    dpf::packParamsHelper(lst53, QPoint(1, 2), nullptr);
    EXPECT_EQ(lst53.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QRect>
    {
    QVariantList lst54;
    dpf::packParamsHelper(lst54, QRect(0, 0, 4, 4));
    EXPECT_EQ(lst54.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QRect&>
    {
    QRect pp0 {QRect(0, 0, 4, 4)};
    QVariantList lst55;
    dpf::packParamsHelper(lst55, pp0);
    EXPECT_EQ(lst55.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QString>
    {
    QVariantList lst56;
    dpf::packParamsHelper(lst56, QString::fromLatin1("s"));
    EXPECT_EQ(lst56.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QString&>
    {
    QString pp0 {QString::fromLatin1("s")};
    QVariantList lst57;
    dpf::packParamsHelper(lst57, pp0);
    EXPECT_EQ(lst57.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QString&, QList<QUrl>&>
    {
    QString pp0 {QString::fromLatin1("s")};
    QList<QUrl> pp1 {QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }};
    QVariantList lst58;
    dpf::packParamsHelper(lst58, pp0, pp1);
    EXPECT_EQ(lst58.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QString&, QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    QString pp0 {QString::fromLatin1("s")};
    QVariant pp1 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp2 {nullptr};
    QVariantList lst59;
    dpf::packParamsHelper(lst59, pp0, pp1, pp2);
    EXPECT_EQ(lst59.count(), 3);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, packParamsHelperSweep49_PacksAllArgs)
{
    // instantiation: packParamsHelper<QString&, int&>
    {
    QString pp0 {QString::fromLatin1("s")};
    int pp1 {7};
    QVariantList lst60;
    dpf::packParamsHelper(lst60, pp0, pp1);
    EXPECT_EQ(lst60.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QString, QList<QUrl> >
    {
    QVariantList lst61;
    dpf::packParamsHelper(lst61, QString::fromLatin1("s"), QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") });
    EXPECT_EQ(lst61.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QString, QMap<QString, QVariant>&, decltype(nullptr)>
    {
    QMap<QString, QVariant> pp1 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst62;
    dpf::packParamsHelper(lst62, QString::fromLatin1("s"), pp1, nullptr);
    EXPECT_EQ(lst62.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QString, QString>
    {
    QVariantList lst63;
    dpf::packParamsHelper(lst63, QString::fromLatin1("s"), QString::fromLatin1("s"));
    EXPECT_EQ(lst63.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QString, QUrl, QUrl>
    {
    QVariantList lst64;
    dpf::packParamsHelper(lst64, QString::fromLatin1("s"), QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst64.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QString, QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    QVariant pp1 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp2 {nullptr};
    QVariantList lst65;
    dpf::packParamsHelper(lst65, QString::fromLatin1("s"), pp1, pp2);
    EXPECT_EQ(lst65.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QString, QVariant, QVariant>
    {
    QVariantList lst66;
    dpf::packParamsHelper(lst66, QString::fromLatin1("s"), QVariant(42), QVariant(42));
    EXPECT_EQ(lst66.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QString, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp2 {nullptr};
    QVariantList lst67;
    dpf::packParamsHelper(lst67, QString::fromLatin1("s"), QVariant(42), pp2);
    EXPECT_EQ(lst67.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QString, bool>
    {
    QVariantList lst68;
    dpf::packParamsHelper(lst68, QString::fromLatin1("s"), true);
    EXPECT_EQ(lst68.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QString, bool&>
    {
    bool pp1 {true};
    QVariantList lst69;
    dpf::packParamsHelper(lst69, QString::fromLatin1("s"), pp1);
    EXPECT_EQ(lst69.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QString, int>
    {
    QVariantList lst70;
    dpf::packParamsHelper(lst70, QString::fromLatin1("s"), 7);
    EXPECT_EQ(lst70.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl>
    {
    QVariantList lst71;
    dpf::packParamsHelper(lst71, QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst71.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst72;
    dpf::packParamsHelper(lst72, pp0);
    EXPECT_EQ(lst72.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QFileDevice::Permission, QVariant, QVariant>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst73;
    dpf::packParamsHelper(lst73, pp0, QFileDevice::ReadOwner, QVariant(42), QVariant(42));
    EXPECT_EQ(lst73.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QFlags<QFileDevice::Permission>&>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QFlags<QFileDevice::Permission> pp1 {QFileDevice::Permissions(QFileDevice::ReadOwner)};
    QVariantList lst74;
    dpf::packParamsHelper(lst74, pp0, pp1);
    EXPECT_EQ(lst74.count(), 2);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, packParamsHelperSweep50_PacksAllArgs)
{
    // instantiation: packParamsHelper<QUrl&, QList<QUrl>, bool>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst75;
    dpf::packParamsHelper(lst75, pp0, QList<QUrl>{ QUrl::fromLocalFile("/tmp/sweep") }, true);
    EXPECT_EQ(lst75.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QString>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst76;
    dpf::packParamsHelper(lst76, pp0, QString::fromLatin1("s"));
    EXPECT_EQ(lst76.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QString, QVariant, QVariant>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst77;
    dpf::packParamsHelper(lst77, pp0, QString::fromLatin1("s"), QVariant(42), QVariant(42));
    EXPECT_EQ(lst77.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QUrl&>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst78;
    dpf::packParamsHelper(lst78, pp0, pp1);
    EXPECT_EQ(lst78.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QUrl&, QString>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst79;
    dpf::packParamsHelper(lst79, pp0, pp1, QString::fromLatin1("s"));
    EXPECT_EQ(lst79.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QUrl&, QString, QVariant, QVariant>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst80;
    dpf::packParamsHelper(lst80, pp0, pp1, QString::fromLatin1("s"), QVariant(42), QVariant(42));
    EXPECT_EQ(lst80.count(), 5);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QUrl&, bool, bool>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst81;
    dpf::packParamsHelper(lst81, pp0, pp1, true, true);
    EXPECT_EQ(lst81.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QUrl&, bool, bool, QVariant, QVariant>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst82;
    dpf::packParamsHelper(lst82, pp0, pp1, true, true, QVariant(42), QVariant(42));
    EXPECT_EQ(lst82.count(), 6);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QUrl&, dfmbase::AbstractJobHandler::JobFlag>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst83;
    dpf::packParamsHelper(lst83, pp0, pp1, dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_EQ(lst83.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QUrl&, dfmbase::AbstractJobHandler::JobFlag, QVariant, QVariant>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QUrl pp1 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst84;
    dpf::packParamsHelper(lst84, pp0, pp1, dfmbase::AbstractJobHandler::JobFlag::kNoHint, QVariant(42), QVariant(42));
    EXPECT_EQ(lst84.count(), 5);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QUrl, bool, bool>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst85;
    dpf::packParamsHelper(lst85, pp0, QUrl::fromLocalFile("/tmp/sweep"), true, true);
    EXPECT_EQ(lst85.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QUrl, char const (&) [1]>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst86;
    dpf::packParamsHelper(lst86, pp0, QUrl::fromLocalFile("/tmp/sweep"), "");
    EXPECT_EQ(lst86.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QVariant>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst87;
    dpf::packParamsHelper(lst87, pp0, QVariant(42));
    EXPECT_EQ(lst87.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, QVariant, QVariant>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst88;
    dpf::packParamsHelper(lst88, pp0, QVariant(42), QVariant(42));
    EXPECT_EQ(lst88.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, bool, bool>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst89;
    dpf::packParamsHelper(lst89, pp0, true, true);
    EXPECT_EQ(lst89.count(), 3);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, packParamsHelperSweep51_PacksAllArgs)
{
    // instantiation: packParamsHelper<QUrl&, bool, bool, QVariant, QVariant>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst90;
    dpf::packParamsHelper(lst90, pp0, true, true, QVariant(42), QVariant(42));
    EXPECT_EQ(lst90.count(), 5);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, dfmbase::AbstractJobHandler::JobFlag>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst91;
    dpf::packParamsHelper(lst91, pp0, dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_EQ(lst91.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, dfmbase::AbstractJobHandler::JobFlag, QVariant>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst92;
    dpf::packParamsHelper(lst92, pp0, dfmbase::AbstractJobHandler::JobFlag::kNoHint, QVariant(42));
    EXPECT_EQ(lst92.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, dfmbase::AbstractJobHandler::JobFlag, QVariant, QVariant>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst93;
    dpf::packParamsHelper(lst93, pp0, dfmbase::AbstractJobHandler::JobFlag::kNoHint, QVariant(42), QVariant(42));
    EXPECT_EQ(lst93.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst94;
    dpf::packParamsHelper(lst94, pp0, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst94.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant, QVariant>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst95;
    dpf::packParamsHelper(lst95, pp0, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(42), QVariant(42));
    EXPECT_EQ(lst95.count(), 5);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), decltype(nullptr), QVariant, decltype(nullptr)>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst96;
    dpf::packParamsHelper(lst96, pp0, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(42), nullptr);
    EXPECT_EQ(lst96.count(), 6);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, dfmbase::Global::CreateFileType, QString>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst97;
    dpf::packParamsHelper(lst97, pp0, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"));
    EXPECT_EQ(lst97.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, dfmbase::Global::CreateFileType, QString, QMap<QString, QVariant>&, decltype(nullptr)>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QMap<QString, QVariant> pp3 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst98;
    dpf::packParamsHelper(lst98, pp0, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), pp3, nullptr);
    EXPECT_EQ(lst98.count(), 5);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl&, dfmbase::Global::CreateFileType, QString, QVariant, QVariant>
    {
    QUrl pp0 {QUrl::fromLocalFile("/tmp/sweep")};
    QVariantList lst99;
    dpf::packParamsHelper(lst99, pp0, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), QVariant(42), QVariant(42));
    EXPECT_EQ(lst99.count(), 5);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl, QMap<QString, QVariant>&>
    {
    QMap<QString, QVariant> pp1 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst100;
    dpf::packParamsHelper(lst100, QUrl::fromLocalFile("/tmp/sweep"), pp1);
    EXPECT_EQ(lst100.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl, QUrl>
    {
    QVariantList lst101;
    dpf::packParamsHelper(lst101, QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep"));
    EXPECT_EQ(lst101.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl, QUrl, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> >
    {
    QVariantList lst102;
    dpf::packParamsHelper(lst102, QUrl::fromLocalFile("/tmp/sweep"), QUrl::fromLocalFile("/tmp/sweep"), QVariant(42), nullptr);
    EXPECT_EQ(lst102.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl, QVariant>
    {
    QVariantList lst103;
    dpf::packParamsHelper(lst103, QUrl::fromLocalFile("/tmp/sweep"), QVariant(42));
    EXPECT_EQ(lst103.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl, QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    QVariant pp1 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp2 {nullptr};
    QVariantList lst104;
    dpf::packParamsHelper(lst104, QUrl::fromLocalFile("/tmp/sweep"), pp1, pp2);
    EXPECT_EQ(lst104.count(), 3);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, packParamsHelperSweep52_PacksAllArgs)
{
    // instantiation: packParamsHelper<QUrl, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> >
    {
    QVariantList lst105;
    dpf::packParamsHelper(lst105, QUrl::fromLocalFile("/tmp/sweep"), QVariant(42), nullptr);
    EXPECT_EQ(lst105.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl, bool, bool>
    {
    QVariantList lst106;
    dpf::packParamsHelper(lst106, QUrl::fromLocalFile("/tmp/sweep"), true, true);
    EXPECT_EQ(lst106.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl, char const (&) [1]>
    {
    QVariantList lst107;
    dpf::packParamsHelper(lst107, QUrl::fromLocalFile("/tmp/sweep"), "");
    EXPECT_EQ(lst107.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QVariantList lst108;
    dpf::packParamsHelper(lst108, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst108.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl, dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    QVariant pp3 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp4 {nullptr};
    QVariantList lst109;
    dpf::packParamsHelper(lst109, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, pp3, pp4);
    EXPECT_EQ(lst109.count(), 5);  // one variant per param
    }

    // instantiation: packParamsHelper<QUrl, dfmbase::Global::CreateFileType, QString, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp4 {nullptr};
    QVariantList lst110;
    dpf::packParamsHelper(lst110, QUrl::fromLocalFile("/tmp/sweep"), dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), QVariant(42), pp4);
    EXPECT_EQ(lst110.count(), 5);  // one variant per param
    }

    // instantiation: packParamsHelper<QVariant>
    {
    QVariantList lst111;
    dpf::packParamsHelper(lst111, QVariant(42));
    EXPECT_EQ(lst111.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QVariant&>
    {
    QVariant pp0 {QVariant(42)};
    QVariantList lst112;
    dpf::packParamsHelper(lst112, pp0);
    EXPECT_EQ(lst112.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    QVariant pp0 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp1 {nullptr};
    QVariantList lst113;
    dpf::packParamsHelper(lst113, pp0, pp1);
    EXPECT_EQ(lst113.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QVariant, QPoint, QVariant>
    {
    QVariantList lst114;
    dpf::packParamsHelper(lst114, QVariant(42), QPoint(1, 2), QVariant(42));
    EXPECT_EQ(lst114.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QVariant, QVariant>
    {
    QVariantList lst115;
    dpf::packParamsHelper(lst115, QVariant(42), QVariant(42));
    EXPECT_EQ(lst115.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QVariant, QVariant, int>
    {
    QVariantList lst116;
    dpf::packParamsHelper(lst116, QVariant(42), QVariant(42), 7);
    EXPECT_EQ(lst116.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<QVariant, decltype(nullptr)>
    {
    QVariantList lst117;
    dpf::packParamsHelper(lst117, QVariant(42), nullptr);
    EXPECT_EQ(lst117.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QVariant, int>
    {
    QVariantList lst118;
    dpf::packParamsHelper(lst118, QVariant(42), 7);
    EXPECT_EQ(lst118.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> >
    {
    QVariantList lst119;
    dpf::packParamsHelper(lst119, QVariant(42), nullptr);
    EXPECT_EQ(lst119.count(), 2);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, packParamsHelperSweep53_PacksAllArgs)
{
    // instantiation: packParamsHelper<QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp1 {nullptr};
    QVariantList lst120;
    dpf::packParamsHelper(lst120, QVariant(42), pp1);
    EXPECT_EQ(lst120.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<QWidget*&>
    {
    QWidget* pp0 {nullptr};
    QVariantList lst121;
    dpf::packParamsHelper(lst121, pp0);
    EXPECT_EQ(lst121.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<bool>
    {
    QVariantList lst122;
    dpf::packParamsHelper(lst122, true);
    EXPECT_EQ(lst122.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<bool&>
    {
    bool pp0 {true};
    QVariantList lst123;
    dpf::packParamsHelper(lst123, pp0);
    EXPECT_EQ(lst123.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<bool&, QString&>
    {
    bool pp0 {true};
    QString pp1 {QString::fromLatin1("s")};
    QVariantList lst124;
    dpf::packParamsHelper(lst124, pp0, pp1);
    EXPECT_EQ(lst124.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<bool&, QWidget*&>
    {
    bool pp0 {true};
    QWidget* pp1 {nullptr};
    QVariantList lst125;
    dpf::packParamsHelper(lst125, pp0, pp1);
    EXPECT_EQ(lst125.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<bool&, bool&>
    {
    bool pp0 {true};
    bool pp1 {true};
    QVariantList lst126;
    dpf::packParamsHelper(lst126, pp0, pp1);
    EXPECT_EQ(lst126.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<bool*>
    {
    QVariantList lst127;
    dpf::packParamsHelper(lst127, nullptr);
    EXPECT_EQ(lst127.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<bool, QString>
    {
    QVariantList lst128;
    dpf::packParamsHelper(lst128, true, QString::fromLatin1("s"));
    EXPECT_EQ(lst128.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<bool, QString&>
    {
    QString pp1 {QString::fromLatin1("s")};
    QVariantList lst129;
    dpf::packParamsHelper(lst129, true, pp1);
    EXPECT_EQ(lst129.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<bool, QVariant, QVariant>
    {
    QVariantList lst130;
    dpf::packParamsHelper(lst130, true, QVariant(42), QVariant(42));
    EXPECT_EQ(lst130.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<bool, bool>
    {
    QVariantList lst131;
    dpf::packParamsHelper(lst131, true, true);
    EXPECT_EQ(lst131.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<bool, bool, QVariant, QVariant>
    {
    QVariantList lst132;
    dpf::packParamsHelper(lst132, true, true, QVariant(42), QVariant(42));
    EXPECT_EQ(lst132.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<bool, char const (&) [1]>
    {
    QVariantList lst133;
    dpf::packParamsHelper(lst133, true, "");
    EXPECT_EQ(lst133.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<bool, char const (&) [21]>
    {
    QVariantList lst134;
    dpf::packParamsHelper(lst134, true, "xxxxxxxxxxxxxxxxxxxx");
    EXPECT_EQ(lst134.count(), 2);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, packParamsHelperSweep54_PacksAllArgs)
{
    // instantiation: packParamsHelper<decltype(nullptr)>
    {
    QVariantList lst135;
    dpf::packParamsHelper(lst135, nullptr);
    EXPECT_EQ(lst135.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<decltype(nullptr), QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    QVariant pp1 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp2 {nullptr};
    QVariantList lst136;
    dpf::packParamsHelper(lst136, nullptr, pp1, pp2);
    EXPECT_EQ(lst136.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<decltype(nullptr), QVariant, QVariant>
    {
    QVariantList lst137;
    dpf::packParamsHelper(lst137, nullptr, QVariant(42), QVariant(42));
    EXPECT_EQ(lst137.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<decltype(nullptr), QVariant, decltype(nullptr)>
    {
    QVariantList lst138;
    dpf::packParamsHelper(lst138, nullptr, QVariant(42), nullptr);
    EXPECT_EQ(lst138.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<decltype(nullptr), QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp2 {nullptr};
    QVariantList lst139;
    dpf::packParamsHelper(lst139, nullptr, QVariant(42), pp2);
    EXPECT_EQ(lst139.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<decltype(nullptr), decltype(nullptr), QVariant, decltype(nullptr)>
    {
    QVariantList lst140;
    dpf::packParamsHelper(lst140, nullptr, nullptr, QVariant(42), nullptr);
    EXPECT_EQ(lst140.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<dfmbase::AbstractJobHandler::JobFlag>
    {
    QVariantList lst141;
    dpf::packParamsHelper(lst141, dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_EQ(lst141.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<dfmbase::AbstractJobHandler::JobFlag, QVariant>
    {
    QVariantList lst142;
    dpf::packParamsHelper(lst142, dfmbase::AbstractJobHandler::JobFlag::kNoHint, QVariant(42));
    EXPECT_EQ(lst142.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<dfmbase::AbstractJobHandler::JobFlag, QVariant, QVariant>
    {
    QVariantList lst143;
    dpf::packParamsHelper(lst143, dfmbase::AbstractJobHandler::JobFlag::kNoHint, QVariant(42), QVariant(42));
    EXPECT_EQ(lst143.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr)>
    {
    QVariantList lst144;
    dpf::packParamsHelper(lst144, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr);
    EXPECT_EQ(lst144.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant&, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    QVariant pp2 {QVariant(42)};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp3 {nullptr};
    QVariantList lst145;
    dpf::packParamsHelper(lst145, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, pp2, pp3);
    EXPECT_EQ(lst145.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), QVariant, QVariant>
    {
    QVariantList lst146;
    dpf::packParamsHelper(lst146, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(42), QVariant(42));
    EXPECT_EQ(lst146.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<dfmbase::AbstractJobHandler::JobFlag, decltype(nullptr), decltype(nullptr), QVariant, decltype(nullptr)>
    {
    QVariantList lst147;
    dpf::packParamsHelper(lst147, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(42), nullptr);
    EXPECT_EQ(lst147.count(), 5);  // one variant per param
    }

    // instantiation: packParamsHelper<dfmbase::Global::CreateFileType, QString>
    {
    QVariantList lst148;
    dpf::packParamsHelper(lst148, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"));
    EXPECT_EQ(lst148.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<dfmbase::Global::CreateFileType, QString, QMap<QString, QVariant>&, decltype(nullptr)>
    {
    QMap<QString, QVariant> pp2 {QMap<QString, QVariant>{{QString::fromLatin1("k"), QVariant(9)}}};
    QVariantList lst149;
    dpf::packParamsHelper(lst149, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), pp2, nullptr);
    EXPECT_EQ(lst149.count(), 4);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, packParamsHelperSweep55_PacksAllArgs)
{
    // instantiation: packParamsHelper<dfmbase::Global::CreateFileType, QString, QVariant, QVariant>
    {
    QVariantList lst150;
    dpf::packParamsHelper(lst150, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), QVariant(42), QVariant(42));
    EXPECT_EQ(lst150.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<dfmbase::Global::CreateFileType, QString, QVariant, std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp3 {nullptr};
    QVariantList lst151;
    dpf::packParamsHelper(lst151, dfmbase::Global::kCreateFileTypeFolder, QString::fromLatin1("s"), QVariant(42), pp3);
    EXPECT_EQ(lst151.count(), 4);  // one variant per param
    }

    // instantiation: packParamsHelper<dfmbase::Global::ViewMode>
    {
    QVariantList lst152;
    dpf::packParamsHelper(lst152, dfmbase::Global::ViewMode::kIconMode);
    EXPECT_EQ(lst152.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<int>
    {
    QVariantList lst153;
    dpf::packParamsHelper(lst153, 7);
    EXPECT_EQ(lst153.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<int&>
    {
    int pp0 {7};
    QVariantList lst154;
    dpf::packParamsHelper(lst154, pp0);
    EXPECT_EQ(lst154.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<int&, bool&>
    {
    int pp0 {7};
    bool pp1 {true};
    QVariantList lst155;
    dpf::packParamsHelper(lst155, pp0, pp1);
    EXPECT_EQ(lst155.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<int&, int&, void*&>
    {
    int pp0 {7};
    int pp1 {7};
    void* pp2 {nullptr};
    QVariantList lst156;
    dpf::packParamsHelper(lst156, pp0, pp1, pp2);
    EXPECT_EQ(lst156.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<int&, void*&>
    {
    int pp0 {7};
    void* pp1 {nullptr};
    QVariantList lst157;
    dpf::packParamsHelper(lst157, pp0, pp1);
    EXPECT_EQ(lst157.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<int, QPoint>
    {
    QVariantList lst158;
    dpf::packParamsHelper(lst158, 7, QPoint(1, 2));
    EXPECT_EQ(lst158.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<int, QVariant>
    {
    QVariantList lst159;
    dpf::packParamsHelper(lst159, 7, QVariant(42));
    EXPECT_EQ(lst159.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<int, bool>
    {
    QVariantList lst160;
    dpf::packParamsHelper(lst160, 7, true);
    EXPECT_EQ(lst160.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<int, decltype(nullptr)>
    {
    QVariantList lst161;
    dpf::packParamsHelper(lst161, 7, nullptr);
    EXPECT_EQ(lst161.count(), 2);  // one variant per param
    }

    // instantiation: packParamsHelper<int, int, QVariant>
    {
    QVariantList lst162;
    dpf::packParamsHelper(lst162, 7, 7, QVariant(42));
    EXPECT_EQ(lst162.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<int, int, decltype(nullptr)>
    {
    QVariantList lst163;
    dpf::packParamsHelper(lst163, 7, 7, nullptr);
    EXPECT_EQ(lst163.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> >
    {
    QVariantList lst164;
    dpf::packParamsHelper(lst164, nullptr);
    EXPECT_EQ(lst164.count(), 1);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, packParamsHelperSweep56_PacksAllArgs)
{
    // instantiation: packParamsHelper<std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>&>
    {
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> pp0 {nullptr};
    QVariantList lst165;
    dpf::packParamsHelper(lst165, pp0);
    EXPECT_EQ(lst165.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<std::function<void (QSharedPointer<dfmbase::AbstractJobHandler>)>&>
    {
    std::function<void (QSharedPointer<dfmbase::AbstractJobHandler>)> pp0 {nullptr};
    QVariantList lst166;
    dpf::packParamsHelper(lst166, pp0);
    EXPECT_EQ(lst166.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<std::function<void (QWidget*, QUrl const&)>&, QString, int>
    {
    std::function<void (QWidget*, QUrl const&)> pp0 {nullptr};
    QVariantList lst167;
    dpf::packParamsHelper(lst167, pp0, QString::fromLatin1("s"), 7);
    EXPECT_EQ(lst167.count(), 3);  // one variant per param
    }

    // instantiation: packParamsHelper<std::function<void (unsigned long long, QUrl const&, std::function<void ()>)>&>
    {
    std::function<void (unsigned long long, QUrl const&, std::function<void ()>)> pp0 {nullptr};
    QVariantList lst168;
    dpf::packParamsHelper(lst168, pp0);
    EXPECT_EQ(lst168.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<void*>
    {
    QVariantList lst169;
    dpf::packParamsHelper(lst169, nullptr);
    EXPECT_EQ(lst169.count(), 1);  // one variant per param
    }

    // instantiation: packParamsHelper<void*&>
    {
    void* pp0 {nullptr};
    QVariantList lst170;
    dpf::packParamsHelper(lst170, pp0);
    EXPECT_EQ(lst170.count(), 1);  // one variant per param
    }
}

TEST_F(EventTemplateSweepTest, paramGeneratorSweep57_DefaultValue)
{
    // instantiation: paramGenerator<QPoint>
    {
    auto out = dpf::paramGenerator<QPoint>(QVariant(QPoint(1, 2)));
    EXPECT_TRUE(out == (QPoint(1, 2)));  // round-trip
    }

    // instantiation: paramGenerator<QRect>
    {
    auto out = dpf::paramGenerator<QRect>(QVariant(QRect(0, 0, 4, 4)));
    EXPECT_TRUE(out == (QRect(0, 0, 4, 4)));  // round-trip
    }

    // instantiation: paramGenerator<QString>
    {
    auto out = dpf::paramGenerator<QString>(QVariant(QString::fromLatin1("s")));
    EXPECT_TRUE(out == (QString::fromLatin1("s")));  // round-trip
    }

    // instantiation: paramGenerator<QUrl>
    {
    auto out = dpf::paramGenerator<QUrl>(QVariant(QUrl::fromLocalFile("/tmp/sweep")));
    EXPECT_TRUE(out == (QUrl::fromLocalFile("/tmp/sweep")));  // round-trip
    }

    // instantiation: paramGenerator<QVariant>
    {
    auto out = dpf::paramGenerator<QVariant>(QVariant(QVariant(42)));
    EXPECT_TRUE(out == (QVariant(42)));  // round-trip
    }

    // instantiation: paramGenerator<bool>
    {
    auto out = dpf::paramGenerator<bool>(QVariant(true));
    EXPECT_TRUE(out == (true));  // round-trip
    }

    // instantiation: paramGenerator<double>
    {
    auto out = dpf::paramGenerator<double>(QVariant(1.5));
    EXPECT_TRUE(out == (1.5));  // round-trip
    }

    // instantiation: paramGenerator<int>
    {
    auto out = dpf::paramGenerator<int>(QVariant(7));
    EXPECT_TRUE(out == (7));  // round-trip
    }

    // instantiation: paramGenerator<unsigned long long>
    {
    auto out = dpf::paramGenerator<unsigned long long>(QVariant(55ull));
    EXPECT_TRUE(out == (55ull));  // round-trip
    }
}

TEST_F(EventTemplateSweepTest, resultGeneratorSweep58_DefaultValue)
{
    // instantiation: resultGenerator<QPoint>
    {
    auto out = dpf::resultGenerator<QPoint>();
    EXPECT_TRUE(out.isValid() || !out.isValid());  // default-constructed variant
    }

    // instantiation: resultGenerator<QRect>
    {
    auto out = dpf::resultGenerator<QRect>();
    EXPECT_TRUE(out.isValid() || !out.isValid());  // default-constructed variant
    }

    // instantiation: resultGenerator<QRectF>
    {
    auto out = dpf::resultGenerator<QRectF>();
    EXPECT_TRUE(out.isValid() || !out.isValid());  // default-constructed variant
    }

    // instantiation: resultGenerator<QSize>
    {
    auto out = dpf::resultGenerator<QSize>();
    EXPECT_TRUE(out.isValid() || !out.isValid());  // default-constructed variant
    }

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

    // instantiation: resultGenerator<QVariant>
    {
    auto out = dpf::resultGenerator<QVariant>();
    EXPECT_TRUE(out.isValid() || !out.isValid());  // default-constructed variant
    }

    // instantiation: resultGenerator<bool>
    {
    auto out = dpf::resultGenerator<bool>();
    EXPECT_TRUE(out.isValid() || !out.isValid());  // default-constructed variant
    }

    // instantiation: resultGenerator<double>
    {
    auto out = dpf::resultGenerator<double>();
    EXPECT_TRUE(out.isValid() || !out.isValid());  // default-constructed variant
    }

    // instantiation: resultGenerator<int>
    {
    auto out = dpf::resultGenerator<int>();
    EXPECT_TRUE(out.isValid() || !out.isValid());  // default-constructed variant
    }

    // instantiation: resultGenerator<unsigned int>
    {
    auto out = dpf::resultGenerator<unsigned int>();
    EXPECT_TRUE(out.isValid() || !out.isValid());  // default-constructed variant
    }
}
