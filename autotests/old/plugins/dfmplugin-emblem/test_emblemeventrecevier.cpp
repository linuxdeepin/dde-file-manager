// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>
#include <QPainter>
#include <QImage>

#define private public
#define protected public
#include "events/emblemeventrecevier.h"
#include "utils/emblemmanager.h"
#undef private
#undef protected

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-framework/event/event.h>
#include <QApplication>

using namespace dfmplugin_emblem;
DFMBASE_USE_NAMESPACE;
DPF_USE_NAMESPACE;

class UT_EmblemEventRecevier : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        if (!qApp) {
            int argc = 1;
            char *argv[] = { const_cast<char *>("test") };
            new QApplication(argc, argv);
        }

        receiver = EmblemEventRecevier::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

    EmblemEventRecevier *receiver { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_EmblemEventRecevier, Instance_ReturnsSingleton)
{
    auto instance1 = EmblemEventRecevier::instance();
    auto instance2 = EmblemEventRecevier::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(UT_EmblemEventRecevier, HandlePaintEmblems_CallsEmblemManager)
{
    QImage image(100, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    QRectF paintArea(0, 0, 100, 100);

    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    bool paintCalled = false;
    stub.set_lamda(&EmblemManager::paintEmblems, [&](EmblemManager *, int role, const FileInfoPointer &info, QPainter *p, QRectF *area) {
        __DBG_STUB_INVOKE__
        paintCalled = true;
        return true;
    });

    bool result = receiver->handlePaintEmblems(&painter, paintArea, mockInfo);

    EXPECT_TRUE(paintCalled);
    EXPECT_TRUE(result);
}

TEST_F(UT_EmblemEventRecevier, InitializeConnections_ConnectsSlot)
{
    // This test mainly ensures the function doesn't crash
    // The actual connection is handled by DPF framework
    EXPECT_NO_FATAL_FAILURE(receiver->initializeConnections());
}
