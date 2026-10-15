// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventfire2_cov.cpp - explicit EventChannel send / EventChannelManager push
// template specialization sweep.
//
// Production code calls dpf::EventChannelManager::push / EventChannel::send
// with a handful of very specific template argument packs (pointer receivers,
// reference out-params, char array literals, std::function payloads) whose
// instantiations never run under tests. Each case below exercises one such
// pack on a fresh topic id inside a local manager/channel instance. Where a
// channel is required the receiver is a null pointer, so the framework's
// `if (s)` guard keeps the production slot body from running while the
// push/send/makeVariantList/packParamsHelper instantiation executes.

#include <plugins/filemanager/dfmplugin-core/events/coreeventreceiver.h>
#include <plugins/common/dfmplugin-menu/menu.h>
#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QWidget>
#include <QPoint>
#include <QModelIndex>
#include <QMimeData>
#include <QLabel>
#include <DLabel>

#include <functional>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFire2SweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFire2SweepTest, PushSend_AbstractSceneCreatorRef_1_ArityGuardNoBody)
{
    // Arrange
    dpf::EventChannelManager mgr;
    auto *receiver = static_cast<dfmplugin_core::CoreEventReceiver *>(nullptr);
    dfmbase::AbstractSceneCreator *creator = nullptr;
    // Act
    const bool connected = mgr.connect(62551, receiver,
                                       static_cast<void (dfmplugin_core::CoreEventReceiver::*)(quint64)>(&dfmplugin_core::CoreEventReceiver::handleShowSettingDialog));
    const QVariant ret = mgr.push<QString, dfmbase::AbstractSceneCreator *&>(62551, QString::fromLatin1("scene"), creator);
    // Assert
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // arity guard on void slot, reference out-param packed untouched
}

TEST_F(EventFire2SweepTest, PushSend_DLabel_2_ArityGuardNoBody)
{
    // Arrange
    dpf::EventChannel ch;
    auto *label = static_cast<Dtk::Widget::DLabel *>(nullptr);
    // Act
    const QVariant ret15 = ch.send(label, "Sending file 1");  // 14 chars + NUL => char const(&)[15]
    const QVariant ret10 = ch.send(label, "Sending 1");   // 9 chars + NUL => char const(&)[10]
    // Assert
    EXPECT_FALSE(ret15.isValid());  // no receiver bound: send packs args and returns invalid
    EXPECT_FALSE(ret10.isValid());
}

TEST_F(EventFire2SweepTest, PushSend_FunctionStringInt_3_ArityGuardNoBody)
{
    // Arrange
    dpf::EventChannelManager mgr;
    std::function<QWidget *(const QUrl &)> fn = [](const QUrl &) -> QWidget * { return nullptr; };
    // Act (QString const&& / int const&& parameters: pass prvalues so they bind)
    const QVariant byName = mgr.push<std::function<QWidget *(const QUrl &)>, QString const, int const>(
            "dfm.ut.core", "slot_ut_viewext", fn, QString::fromLatin1("ut"), 0);
    const QVariant byId = mgr.push<std::function<QWidget *(const QUrl &)>, QString const, int const>(
            62552, fn, QString::fromLatin1("ut"), 0);
    // Assert
    EXPECT_FALSE(byName.isValid());  // converter unregistered: name-based push never dispatches
    EXPECT_FALSE(byId.isValid());  // no channel bound on this id: push returns invalid
    EXPECT_EQ(dpf::EventConverter::convert("dfm.ut.core", "slot_ut_viewext"), dpf::EventTypeScope::kInValid);
}

TEST_F(EventFire2SweepTest, PushSend_U64UrlBoolRef_4_ArityGuardNoBody)
{
    // Arrange
    dpf::EventChannel ch;
    const QUrl url = QUrl::fromLocalFile("/tmp");
    bool checked = false;
    // Act
    const QVariant ret = ch.send(static_cast<unsigned long long>(1), url, checked);
    // Assert
    EXPECT_FALSE(ret.isValid());  // no receiver bound: invalid return, out-param untouched
    EXPECT_FALSE(checked);
}

TEST_F(EventFire2SweepTest, PushMisc_ScalarRefPacks_5_ArityGuardNoBody)
{
    // Arrange
    dpf::EventChannelManager mgr;
    const QString str = QString::fromLatin1("ut");
    int number = 0;
    const QUrl url = QUrl::fromLocalFile("/tmp");
    const QModelIndex index;
    // Act + Assert
    EXPECT_FALSE((mgr.push<QString const &>("dfm.ut.core", "slot_ut_str", str).isValid()));
    EXPECT_FALSE((mgr.push<int &>("dfm.ut.core", "slot_ut_int", number).isValid()));
    EXPECT_FALSE((mgr.push<QUrl const &>("dfm.ut.core", "slot_ut_url", url).isValid()));
    EXPECT_FALSE((mgr.push<QModelIndex const &>("dfm.ut.core", "slot_ut_mindex", index).isValid()));
    EXPECT_FALSE((mgr.push<QString const &, QString const &>("dfm.ut.core", "slot_ut_str2", str, str).isValid()));
    EXPECT_EQ(number, 0);  // reference packs passed through without side effects (converter unregistered)
}

TEST_F(EventFire2SweepTest, PushMisc_MapAndListPacks_6_ArityGuardNoBody)
{
    // Arrange
    dpf::EventChannelManager mgr;
    const QMap<QString, QVariant> constMap { { QString::fromLatin1("k"), QVariant(1) } };
    QMap<QString, QVariant> mutableMap;
    const QList<QString> nameList { QString::fromLatin1("a") };
    const QList<QUrl> urlList { QUrl::fromLocalFile("/tmp") };
    const QUrl url = QUrl::fromLocalFile("/tmp");
    const QString str = QString::fromLatin1("ut");
    const unsigned long long winId = 1;
    dfmbase::Global::ItemRoles role = dfmbase::Global::ItemRoles::kItemFileDisplayNameRole;
    bool checked = false;
    // Act + Assert
    EXPECT_FALSE((mgr.push<QMap<QString, QVariant> const &>("dfm.ut.core", "slot_ut_map", constMap).isValid()));
    EXPECT_FALSE((mgr.push<QMap<QString, QVariant> &>("dfm.ut.core", "slot_ut_map2", mutableMap).isValid()));
    EXPECT_FALSE((mgr.push<QString const &, QMap<QString, QVariant> const &>("dfm.ut.core", "slot_ut_strmap", str, constMap).isValid()));
    EXPECT_FALSE((mgr.push<QUrl const &, QMap<QString, QVariant> const &>("dfm.ut.core", "slot_ut_urlmap", url, constMap).isValid()));
    EXPECT_FALSE((mgr.push<QList<QString> const &, QString const &>("dfm.ut.core", "slot_ut_liststr", nameList, str).isValid()));
    EXPECT_FALSE((mgr.push<unsigned long long const &, dfmbase::Global::ItemRoles &>("dfm.ut.core", "slot_ut_role", winId, role).isValid()));
    EXPECT_FALSE((mgr.push<QList<QUrl> const &, QUrl const &, bool &>("dfm.ut.core", "slot_ut_urlsbool", urlList, url, checked).isValid()));
    EXPECT_EQ(dpf::EventConverter::convert("dfm.ut.core", "slot_ut_map"), dpf::EventTypeScope::kInValid);  // all name pushes short-circuit identically
}

TEST_F(EventFire2SweepTest, PushMisc_PointAndUrlListPacks_7_ArityGuardNoBody)
{
    // Arrange
    dpf::EventChannelManager mgr;
    int number = 0;
    const QPoint point(1, 2);
    QList<QUrl> urlList { QUrl::fromLocalFile("/tmp") };
    // Act + Assert
    EXPECT_FALSE((mgr.push<int &, QPoint const &>("dfm.ut.core", "slot_ut_intpt", number, point).isValid()));
    EXPECT_FALSE((mgr.push<int, QList<QUrl> &>("dfm.ut.core", "slot_ut_inturls", 0, urlList).isValid()));
    EXPECT_EQ(point, QPoint(1, 2));  // packs untouched, no dispatch happened
    EXPECT_EQ(urlList.size(), 1);
}

TEST_F(EventFire2SweepTest, SeqRun_UrlListPack_8_ArityGuardNoBody)
{
    // Arrange
    dpf::EventSequenceManager mgr;
    const QList<QUrl> urlList { QUrl::fromLocalFile("/tmp") };
    const QUrl url = QUrl::fromLocalFile("/tmp");
    // Act
    const bool ran = mgr.run<QList<QUrl> const &, QUrl const &>("dfm.ut.core", "hook_ut_urls", urlList, url);
    // Assert
    EXPECT_FALSE(ran);  // converter unregistered: run never reaches a sequence
    EXPECT_EQ(dpf::EventConverter::convert("dfm.ut.core", "hook_ut_urls"), dpf::EventTypeScope::kInValid);
}

TEST_F(EventFire2SweepTest, SeqTraversal_MimeDataPack_9_ArityGuardNoBody)
{
    // Arrange
    dpf::EventSequence seq;
    const QUrl url = QUrl::fromLocalFile("/tmp");
    Qt::DropAction action = Qt::IgnoreAction;
    void *userData = nullptr;
    // Act
    const bool first = seq.traversal(static_cast<const QMimeData *>(nullptr), url, action, userData);
    const bool second = seq.traversal(static_cast<const QMimeData *>(nullptr), url, action, userData);
    // Assert
    EXPECT_FALSE(first);  // empty sequence: no handler accepts, traversal returns false
    EXPECT_FALSE(second);
    EXPECT_EQ(action, Qt::IgnoreAction);  // reference out-params untouched
}
