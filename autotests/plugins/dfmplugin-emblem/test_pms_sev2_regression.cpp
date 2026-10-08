// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 regression tests for dfmplugin-emblem (BUG 316919):
//   - GioEmblemWorker::onProduce(nullptr) must not crash (null check after the
//     main-thread Q_ASSERT; runs on a worker thread since Debug builds assert)
//   - EmblemHelper::pending(nullptr) must not emit requestProduce
//   - EmblemHelper::onEmblemChanged with an empty product stays safe (no dpf push)
//   - EmblemHelper::onUrlChanged clears emblems and emits requestClear
// Note: the pendingUrls dedup added by the original fix (d011c4a11) was later
// removed upstream (81b4b8b9a), so only the null guards are contract here.

#include <gtest/gtest.h>

#include <stubext.h>
#include "utils/emblemhelper.h"

#include <dfm-base/interfaces/fileinfo.h>

#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QSignalSpy>
#include <QThread>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_emblem;

class UT_PmsRegressionEmblemHelper : public testing::Test
{
protected:
    void SetUp() override
    {
        if (!qApp) {
            int argc = 1;
            char *argv[] = { const_cast<char *>("test") };
            new QApplication(argc, argv);
        }
        stub.clear();

        // Stub initialize to prevent thread creation in tests
        stub.set_lamda(&EmblemHelper::initialize, [](EmblemHelper *) { });

        // QIcon::fromTheme() returns a null icon without an icon theme engine (offscreen)
        QPixmap pix(16, 16);
        pix.fill(Qt::red);
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme),
                       [pix](const QString &) -> QIcon { return QIcon(pix); });

        helper = new EmblemHelper(nullptr);
    }
    void TearDown() override
    {
        stub.clear();
        delete helper;
        helper = nullptr;
    }
    stub_ext::StubExt stub;
    EmblemHelper *helper = nullptr;
};

// Regression for the 316919 crash: pending(nullptr) dereferenced the null info.
TEST_F(UT_PmsRegressionEmblemHelper, BUG316919_PendingNull_NoSignal)
{
    QSignalSpy spy(helper, &EmblemHelper::requestProduce);
    EXPECT_NO_FATAL_FAILURE(helper->pending(FileInfoPointer()));
    EXPECT_EQ(0, spy.count());
}

// onProduce(nullptr) must early-return without touching fetchEmblems/urlOf.
TEST_F(UT_PmsRegressionEmblemHelper, BUG316919_OnEmblemChanged_EmptyProduct_NoCrash)
{
    const QUrl url("file:///tmp/ut-316919-empty.bin");
    EXPECT_NO_FATAL_FAILURE(helper->onEmblemChanged(url, QList<QIcon> { }));
    EXPECT_TRUE(helper->productQueue.value(url).isEmpty());
}

// Sanity: onUrlChanged always clears and reports "not handled".
TEST_F(UT_PmsRegressionEmblemHelper, BUG316919_OnUrlChanged_ClearsEmblems)
{
    QSignalSpy clearSpy(helper, &EmblemHelper::requestClear);
    EXPECT_FALSE(helper->onUrlChanged(0, QUrl("file:///tmp/ut-316919-urlchange.bin")));
    EXPECT_EQ(1, clearSpy.count());
}

// onProduce(nullptr) asserts on the main thread (Q_ASSERT(qApp->thread() != ...)),
// so exercise it on a dedicated worker thread.
TEST_F(UT_PmsRegressionEmblemHelper, BUG316919_WorkerOnProduce_NullInfo_NoCrash)
{
    GioEmblemWorker worker;
    QThread thread;
    bool ran = false;
    QObject::connect(&thread, &QThread::started, [&]() {
        worker.onProduce(FileInfoPointer());
        ran = true;
        thread.quit();
    });
    thread.start();
    thread.wait(5000);
    EXPECT_TRUE(ran);
}
