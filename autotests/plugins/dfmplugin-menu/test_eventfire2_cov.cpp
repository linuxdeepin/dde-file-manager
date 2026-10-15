// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventfire2_cov.cpp - value-marshalling helper sweep for dpf templates.
//
// dpf::paramGenerator<T>, dpf::ApplyReturnValue<R>, dpf::operator,<T,T> and
// resultGenerator<R> only execute when a registered handler runs with a
// NON-NULL receiver and MATCHING arity; the arity-guard sweep elsewhere never
// reaches them. These helpers are keyed purely by TYPE, so a local dummy
// receiver whose members mirror the exact production parameter/return types
// instantiates the identical mangled symbols. Every slot body below is a
// trivial, side-effect-free stub owned by this test; firing it through a
// local manager exercises the full pack/unpack round trip.

#include <plugins/common/dfmplugin-menu/menu.h>
#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

#include <QUrl>
#include <QVariant>
#include <QWidget>
#include <QItemSelection>
#include <QRectF>
#include <QHash>
#include <QMap>
#include <QMultiMap>
#include <QStringList>

#include <functional>

#include <QObject>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

namespace utfire {
// dpf registration templates require the receiver to derive QObject.
struct MarshallingReceiver : QObject {
    // return-type sweep slots (one QString param: arity matches a 1-arg push)
    dfmbase::AbstractMenuScene *retMenuScene(const QString &) { return nullptr; }
    dfmbase::AbstractSceneCreator *retSceneCreator(const QString &) { return nullptr; }
    QHash<QString, QVariant> retHash(const QString &) { return { { QString::fromLatin1("k"), QVariant(7) } }; }
    QList<QMap<QString, QVariant>> retListMap(const QString &) { return { QMap<QString, QVariant> { { QString::fromLatin1("m"), QVariant(3) } } }; }
    QList<QWidget *> retListWidget(const QString &) { return QList<QWidget *> {}; }
    QMap<QString, QVariant> retMap(const QString &) { return { { QString::fromLatin1("z"), QVariant(5) } }; }
    unsigned int retUInt(const QString &) { return 42u; }

    // parameter-type sweep slots (write through pointer/out params so the
    // unpacked values can be asserted after the push)
    bool paramSceneCreator(dfmbase::AbstractSceneCreator *) { return true; }
    bool paramHash(const QHash<QString, QVariant> &) { return true; }
    bool paramItemSelection(const QItemSelection &) { return true; }
    bool paramListStrPtr(QList<QString> *out) { *out = QStringList { QString::fromLatin1("ut") }; return true; }
    bool paramRectF(const QRectF &) { return true; }
    bool paramUrlPtr(QUrl *out) { *out = QUrl::fromLocalFile("/tmp/ut-fire"); return true; }
    bool paramVariantPtr(QVariant *out) { *out = QVariant(11); return true; }
    bool paramFnBoolUrl(std::function<bool(const QUrl &)>) { return true; }
    bool paramFnMapMm(std::function<QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(const QUrl &)>) { return true; }
    bool paramFnWidget(std::function<QWidget *(QWidget *)>) { return true; }
    bool paramFnVoidBoolStr(std::function<void(bool, const QString &)>) { return true; }
};
}   // namespace utfire

class EventFire2SweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFire2SweepTest, Marshal_ReturnTypePack_0_RoundTrip)
{
    // Arrange
    dpf::EventChannelManager mgr;
    utfire::MarshallingReceiver receiver;
    const QString arg = QString::fromLatin1("ut");
    // Act + Assert: each non-void return marshals into a valid QVariant via
    // ApplyReturnValue<R> / operator,<R,R> / resultGenerator<R>
    EXPECT_TRUE(mgr.connect(62100, &receiver, &utfire::MarshallingReceiver::retMenuScene));
    EXPECT_TRUE(mgr.push<QString const &>(62100, arg).isValid());   // null scene pointer still yields a typed, valid QVariant
    EXPECT_TRUE(mgr.connect(62102, &receiver, &utfire::MarshallingReceiver::retSceneCreator));
    EXPECT_TRUE(mgr.push<QString const &>(62102, arg).canConvert<dfmbase::AbstractSceneCreator *>());

    EXPECT_TRUE(mgr.connect(62104, &receiver, &utfire::MarshallingReceiver::retHash));
    EXPECT_EQ(mgr.push<QString const &>(62104, arg).toHash().value(QString::fromLatin1("k")).toInt(), 7);
    EXPECT_TRUE(mgr.connect(62106, &receiver, &utfire::MarshallingReceiver::retMap));
    EXPECT_EQ(mgr.push<QString const &>(62106, arg).toMap().value(QString::fromLatin1("z")).toInt(), 5);
    EXPECT_TRUE(mgr.connect(62108, &receiver, &utfire::MarshallingReceiver::retListMap));
    EXPECT_EQ(mgr.push<QString const &>(62108, arg).toList().size(), 1);
    EXPECT_TRUE(mgr.connect(62110, &receiver, &utfire::MarshallingReceiver::retListWidget));
    EXPECT_TRUE((mgr.push<QString const &>(62110, arg).canConvert<QList<QWidget *>>()));
    EXPECT_TRUE(mgr.connect(62112, &receiver, &utfire::MarshallingReceiver::retUInt));
    EXPECT_EQ(mgr.push<QString const &>(62112, arg).toUInt(), 42u);
}

TEST_F(EventFire2SweepTest, Marshal_ParamTypePack_1_RoundTrip)
{
    // Arrange
    dpf::EventChannelManager mgr;
    utfire::MarshallingReceiver receiver;
    QUrl url;
    QVariant variant;
    QList<QString> outList;
    const QUrl pushUrl = QUrl::fromLocalFile("/tmp");
    const QRectF rect(1.0, 2.0, 3.0, 4.0);
    const QItemSelection selection;
    const QHash<QString, QVariant> hash { { QString::fromLatin1("h"), QVariant(1) } };
    // Act
    EXPECT_TRUE(mgr.connect(62120, &receiver, &utfire::MarshallingReceiver::paramUrlPtr));
    const QVariant urlRet = mgr.push<QUrl *>(62120, &url);
    EXPECT_TRUE(mgr.connect(62122, &receiver, &utfire::MarshallingReceiver::paramVariantPtr));
    const QVariant varRet = mgr.push<QVariant *>(62122, &variant);
    EXPECT_TRUE(mgr.connect(62124, &receiver, &utfire::MarshallingReceiver::paramListStrPtr));
    const QVariant listRet = mgr.push<QList<QString> *>(62124, &outList);
    // Assert: out-params written by the stub body prove paramGenerator<T> unpacked a usable value
    EXPECT_EQ(url, QUrl::fromLocalFile("/tmp/ut-fire"));
    EXPECT_EQ(variant.toInt(), 11);
    EXPECT_EQ(outList, QStringList { QString::fromLatin1("ut") });
    EXPECT_TRUE(urlRet.toBool());
    EXPECT_TRUE(varRet.toBool());
    EXPECT_TRUE(listRet.toBool());
}

TEST_F(EventFire2SweepTest, Marshal_ParamValuePack_2_RoundTrip)
{
    // Arrange
    dpf::EventChannelManager mgr;
    utfire::MarshallingReceiver receiver;
    const QUrl url = QUrl::fromLocalFile("/tmp");
    const QRectF rect(1.0, 2.0, 3.0, 4.0);
    const QItemSelection selection;
    const QHash<QString, QVariant> hash { { QString::fromLatin1("h"), QVariant(1) } };
    std::function<bool(const QUrl &)> fnBoolUrl = [](const QUrl &) { return true; };
    std::function<QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(const QUrl &)> fnMapMm =
            [](const QUrl &) { return QMap<QString, QMultiMap<QString, QPair<QString, QString>>> {}; };
    std::function<QWidget *(QWidget *)> fnWidget = [](QWidget *w) { return w; };
    std::function<void(bool, const QString &)> fnVoid = [](bool, const QString &) {};
    // Act + Assert: by-value / const-ref / std::function params all unpack and the stub returns true
    EXPECT_TRUE(mgr.connect(62130, &receiver, &utfire::MarshallingReceiver::paramSceneCreator));
    EXPECT_TRUE(mgr.push<dfmbase::AbstractSceneCreator *>(62130, nullptr).toBool());
    EXPECT_TRUE(mgr.connect(62132, &receiver, &utfire::MarshallingReceiver::paramHash));
    EXPECT_TRUE((mgr.push<QHash<QString, QVariant> const &>(62132, hash).toBool()));
    EXPECT_TRUE(mgr.connect(62134, &receiver, &utfire::MarshallingReceiver::paramRectF));
    EXPECT_TRUE(mgr.push<QRectF const &>(62134, rect).toBool());
    EXPECT_TRUE(mgr.connect(62136, &receiver, &utfire::MarshallingReceiver::paramItemSelection));
    EXPECT_TRUE((mgr.push<QItemSelection const &>(62136, selection).toBool()));
    EXPECT_TRUE(mgr.connect(62138, &receiver, &utfire::MarshallingReceiver::paramFnBoolUrl));
    EXPECT_TRUE((mgr.push<std::function<bool(const QUrl &)>>(62138, fnBoolUrl).toBool()));
    EXPECT_TRUE(mgr.connect(62140, &receiver, &utfire::MarshallingReceiver::paramFnMapMm));
    EXPECT_TRUE((mgr.push<std::function<QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(const QUrl &)>>(62140, fnMapMm).toBool()));
    EXPECT_TRUE(mgr.connect(62142, &receiver, &utfire::MarshallingReceiver::paramFnWidget));
    EXPECT_TRUE((mgr.push<std::function<QWidget *(QWidget *)>>(62142, fnWidget).toBool()));
    EXPECT_TRUE(mgr.connect(62144, &receiver, &utfire::MarshallingReceiver::paramFnVoidBoolStr));
    EXPECT_TRUE((mgr.push<std::function<void(bool, const QString &)>>(62144, fnVoid).toBool()));
}
