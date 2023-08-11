// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screen/screenproxyqt.h"

#include "dfm-framework/dpf.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QSharedPointer>
#include <QMap>
#include <QVector>
#include <QScreen>
#include <QTimer>

DDPCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

Q_DECLARE_METATYPE(QStringList*)

class ProxyNullScreen : public AbstractScreen
{
public:
    ProxyNullScreen(const QString &n = "") : AbstractScreen(),scname(n) {}
    virtual QString name() const {return scname;}
    QRect geometry() const {return QRect(0, 0, 200, 200);}
    QRect availableGeometry() const {return QRect(0, 0, 200, 180);}
    QRect handleGeometry() const {return QRect(0, 0, 250, 250);}
    QString scname;
};

TEST(ScreenProxyQt, reset)
{
    ScreenProxyQt sp;
    auto qscs = qApp->screens();

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ScreenProxyQt, displayMode),
                   [](){
        return kDuplicate;
    });

    sp.reset();

    EXPECT_EQ(qscs.size(), sp.screenMap.size());
    EXPECT_EQ(sp.lastMode, kDuplicate);
}

TEST(ScreenProxyQt, primaryScreen)
{
    ScreenProxyQt sp;
    stub_ext::StubExt stub;
    QScreen *fake = (QScreen *)0x22;
    stub.set_lamda(&QGuiApplication::primaryScreen,
                   [fake](){
        return fake;
    });
    ScreenPointer sc(new ProxyNullScreen);

    sp.screenMap.insert(fake, sc);

    EXPECT_EQ(sp.primaryScreen(), sc);
    sp.screenMap.clear();
}


TEST(ScreenProxyQt, screens)
{
    ScreenProxyQt sp;
    auto qscs = qApp->screens();

    for (QScreen *sc : qscs) {
        sp.screenMap.insert(sc, ScreenPointer(new ProxyNullScreen));
    }

    auto ret = sp.screens();
    ASSERT_EQ(ret.size(), qscs.size());
    sp.screenMap.clear();
}

TEST(ScreenProxyQt, logicScreens)
{
    ScreenProxyQt sp;
    auto qscs = qApp->screens();

    for (QScreen *sc : qscs) {
        sp.screenMap.insert(sc, ScreenPointer(new ProxyNullScreen));
    }

    auto ret = sp.logicScreens();
    ASSERT_EQ(ret.size(), qscs.size());
    if (!ret.isEmpty()) {
        QScreen *primary = qApp->primaryScreen();
        auto pri = sp.screenMap.value(primary);
        EXPECT_EQ(ret.first(), pri);
    }

    sp.screenMap.clear();
}

TEST(ScreenProxyQt, devicePixelRatio)
{
    ScreenProxyQt sp;
    if (auto pr = qApp->primaryScreen()) {
        EXPECT_EQ(pr->devicePixelRatio(), sp.devicePixelRatio());
    }
}

TEST(ScreenProxyQt, screen)
{
    ScreenProxyQt sp;
    auto qscs = qApp->screens();

    for (QScreen *sc : qscs) {
        sp.screenMap.insert(sc, ScreenPointer(new ProxyNullScreen(sc->name())));
    }

    if (!qscs.isEmpty()) {
        auto sc = qscs.first();
        EXPECT_EQ(sp.screen(sc->name()).get(), sp.screenMap.value(sc).get());
    }
}

TEST(ScreenProxyQt, onScreenAdded)
{
    ScreenProxyQt sp;
    if (auto pr = qApp->primaryScreen()) {
        sp.onScreenAdded(pr);
        EXPECT_EQ(sp.screenMap.size(), 1);
        EXPECT_NE(sp.screenMap.value(pr), nullptr);
        EXPECT_EQ(sp.events.size(), 1);
        EXPECT_TRUE(sp.eventShot->isActive());
    }
}

TEST(ScreenProxyQt, onScreenRemoved)
{
    ScreenProxyQt sp;
    if (auto pr = qApp->primaryScreen()) {
        sp.screenMap.insert(pr, ScreenPointer(new ProxyNullScreen(pr->name())));

        sp.onScreenRemoved(pr);
        EXPECT_EQ(sp.screenMap.size(), 0);
        EXPECT_EQ(sp.events.size(), 1);
        EXPECT_TRUE(sp.eventShot->isActive());
    }
}

TEST(ScreenProxyQt, processEvent)
{
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ScreenProxyQt, displayMode),
                   [](){
        return kDuplicate;
    });

    stub.set_lamda(&ScreenProxyQt::checkUsedScreens, [](){
        return true;
    });

    ScreenProxyQt sp;
    sp.lastMode = kDuplicate;

    int sig = -1;
    QObject::connect(&sp, &ScreenProxyQt::displayModeChanged, &sp, [&sig](){
        sig = 0;
    });
    QObject::connect(&sp, &ScreenProxyQt::screenChanged, &sp, [&sig](){
        sig = 1;
    });
    QObject::connect(&sp, &ScreenProxyQt::screenGeometryChanged, &sp, [&sig](){
        sig = 2;
    });    QObject::connect(&sp, &ScreenProxyQt::screenAvailableGeometryChanged, &sp, [&sig](){
        sig = 3;
    });

    // no changed
    {
        sp.processEvent();
        EXPECT_EQ(sig, -1);
        sp.events.clear();
    }

    // display change
    {
        sig = -1;
        sp.lastMode = kExtend;
        sp.processEvent();
        EXPECT_EQ(sig, 0);
        EXPECT_EQ(sp.lastMode, kDuplicate);
        sp.events.clear();

        sig = -1;
        sp.lastMode = kDuplicate;
        sp.events.insert(AbstractScreenProxy::kMode, 0);
        sp.events.insert(AbstractScreenProxy::kScreen, 0);
        sp.events.insert(AbstractScreenProxy::kGeometry, 0);
        sp.events.insert(AbstractScreenProxy::kAvailableGeometry, 0);
        sp.processEvent();
        EXPECT_EQ(sig, 0);
        sp.events.clear();
    }

    // screen
    {
        sig = -1;
        sp.lastMode = kDuplicate;
        sp.events.insert(AbstractScreenProxy::kScreen, 0);
        sp.events.insert(AbstractScreenProxy::kGeometry, 0);
        sp.events.insert(AbstractScreenProxy::kAvailableGeometry, 0);
        sp.processEvent();
        EXPECT_EQ(sig, 1);
        sp.events.clear();

        sig = -1;
        sp.events.insert(AbstractScreenProxy::kScreen, 0);
        sp.processEvent();
        EXPECT_EQ(sig, 1);
        sp.events.clear();
    }

    // geometry
    {
        sig = -1;
        sp.lastMode = kDuplicate;
        sp.events.insert(AbstractScreenProxy::kGeometry, 0);
        sp.events.insert(AbstractScreenProxy::kAvailableGeometry, 0);
        sp.processEvent();
        EXPECT_EQ(sig, 2);
        sp.events.clear();

        sig = -1;
        sp.events.insert(AbstractScreenProxy::kGeometry, 0);
        sp.processEvent();
        EXPECT_EQ(sig, 2);
        sp.events.clear();
    }

    // availableGeometry
    {
        sig = -1;
        sp.lastMode = kDuplicate;
        sp.events.insert(AbstractScreenProxy::kAvailableGeometry, 0);
        sp.processEvent();
        EXPECT_EQ(sig, 3);
        sp.events.clear();
    }
}

TEST(ScreenProxyQt, displayMode)
{
    stub_ext::StubExt stub;
    ScreenProxyQt sp;
    QList<ScreenPointer> scs;
    stub.set_lamda(VADDR(ScreenProxyQt, screens), [&scs](){
        return scs;
    });

    // empty
    EXPECT_EQ(sp.displayMode(), kCustom);

    scs.append(ScreenPointer(new ProxyNullScreen()));
    EXPECT_EQ(sp.displayMode(), kShowonly);

    scs.append(ScreenPointer(new ProxyNullScreen()));
    EXPECT_EQ(sp.displayMode(), kDuplicate);

    class ProxyNullScreen2 : public ProxyNullScreen
    {
    public:
        QRect geometry() const {return QRect(0, 200, 200, 200);}
    };

    scs.removeAt(1);
    scs.append(ScreenPointer(new ProxyNullScreen2()));
    EXPECT_EQ(sp.displayMode(), kExtend);
}


TEST(ScreenProxyQt, checkUsedScreens)
{
    ScreenProxyQt sp;
    stub_ext::StubExt stub;
    QStringList inuse;
    stub.set_lamda(((bool (EventSequenceManager::*)(const QString &, const QString &, QStringList *out))
                   &EventSequenceManager::run), [&inuse]
                   (EventSequenceManager *, const QString &t1, const QString &t2, QStringList *out) {
        if (t1 == "ddplugin_core") {
            if (t2 == "hook_ScreenProxy_ScreensInUse") {
                *out = inuse;
                return false;
            }
        }
        return false;
    });

    auto pri = qApp->primaryScreen();
    if (pri) {
        sp.screenMap.insert(pri, nullptr);
    }

    inuse << "fake" << pri->name();
    EXPECT_FALSE(sp.checkUsedScreens());

    inuse.clear();
    inuse << "fake";
    EXPECT_FALSE(sp.checkUsedScreens());

    inuse.clear();
    inuse << pri->name();
    EXPECT_TRUE(sp.checkUsedScreens());

    inuse.clear();
    EXPECT_TRUE(sp.checkUsedScreens());
}


TEST(ScreenProxyQt, bug_214195_fakescreen)
{
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ScreenProxyQt, displayMode),
                   [](){
        return kDuplicate;
    });

    bool cc = false;
    bool valid = false;
    stub.set_lamda(&ScreenProxyQt::checkUsedScreens, [&cc, &valid](){
        cc = true;
        return valid;
    });

    ScreenProxyQt sp;
    sp.lastMode = kDuplicate;

    int sig = -1;
    QObject::connect(&sp, &ScreenProxyQt::displayModeChanged, &sp, [&sig](){
        sig = 0;
    });
    QObject::connect(&sp, &ScreenProxyQt::screenChanged, &sp, [&sig](){
        sig = 1;
    });
    QObject::connect(&sp, &ScreenProxyQt::screenGeometryChanged, &sp, [&sig](){
        sig = 2;
    });    QObject::connect(&sp, &ScreenProxyQt::screenAvailableGeometryChanged, &sp, [&sig](){
        sig = 3;
    });

    // display mode
    {
        sig = -1;
        cc = false;
        sp.events.insert(AbstractScreenProxy::kMode, 0);
        sp.processEvent();
        EXPECT_FALSE(cc);
        EXPECT_EQ(sig, 0);
        sp.events.clear();
    }

    // screen
    {
        sig = -1;
        cc = false;
        sp.events.insert(AbstractScreenProxy::kScreen, 0);
        sp.processEvent();
        EXPECT_FALSE(cc);
        EXPECT_EQ(sig, 1);
        sp.events.clear();
    }

    // geometry and valid
    {
        sig = -1;
        cc = false;
        valid = true;
        sp.events.insert(AbstractScreenProxy::kGeometry, 0);
        sp.processEvent();
        EXPECT_TRUE(cc);
        EXPECT_EQ(sig, 2);
        sp.events.clear();
    }

    // geometry and invalid
    {
        sig = -1;
        cc = false;
        valid = false;
        sp.events.insert(AbstractScreenProxy::kGeometry, 0);
        sp.processEvent();
        EXPECT_TRUE(cc);
        EXPECT_EQ(sig, 1);
        sp.events.clear();
    }

    // available geometry and valid
    {
        sig = -1;
        cc = false;
        valid = true;
        sp.events.insert(AbstractScreenProxy::kAvailableGeometry, 0);
        sp.processEvent();
        EXPECT_TRUE(cc);
        EXPECT_EQ(sig, 3);
        sp.events.clear();
    }

    // available geometry and invalid
    {
        sig = -1;
        cc = false;
        valid = false;
        sp.events.insert(AbstractScreenProxy::kAvailableGeometry, 0);
        sp.processEvent();
        EXPECT_TRUE(cc);
        EXPECT_EQ(sig, 1);
        sp.events.clear();
    }
}
