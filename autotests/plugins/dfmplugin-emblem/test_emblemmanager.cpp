// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>
#include <QPainter>
#include <QImage>

#define private public
#define protected public
#include "utils/emblemmanager.h"
#include "utils/emblemhelper.h"
#include "events/emblemeventsequence.h"
#undef private
#undef protected

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <QApplication>

using namespace dfmplugin_emblem;
DFMBASE_USE_NAMESPACE;
DFMGLOBAL_USE_NAMESPACE

class UT_EmblemManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        if (!qApp) {
            int argc = 1;
            char *argv[] = { const_cast<char *>("test") };
            new QApplication(argc, argv);
        }

        manager = EmblemManager::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

    EmblemManager *manager { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_EmblemManager, Instance_ReturnsSingleton)
{
    auto instance1 = EmblemManager::instance();
    auto instance2 = EmblemManager::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(UT_EmblemManager, PaintEmblems_ReturnsFalseForInvalidRole)
{
    QImage image(100, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    QRectF paintArea(0, 0, 100, 100);
    FileInfoPointer info;

    int invalidRole = 999;  // Not kItemIconRole
    bool result = manager->paintEmblems(invalidRole, info, &painter, &paintArea);

    EXPECT_FALSE(result);
}

TEST_F(UT_EmblemManager, PaintEmblems_ReturnsFalseForNullInfo)
{
    QImage image(100, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    QRectF paintArea(0, 0, 100, 100);
    FileInfoPointer info;  // null pointer

    bool result = manager->paintEmblems(kItemIconRole, info, &painter, &paintArea);

    EXPECT_FALSE(result);
}

TEST_F(UT_EmblemManager, PaintEmblems_SetsRenderHints)
{
    QImage image(100, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    QRectF paintArea(0, 0, 100, 100);

    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    stub.set_lamda(&EmblemHelper::systemEmblems, [](EmblemHelper *, const FileInfoPointer &) {
        __DBG_STUB_INVOKE__
        return QList<QIcon>();
    });

    stub.set_lamda(&EmblemHelper::isExtEmblemProhibited, [](EmblemHelper *, const FileInfoPointer &, const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;  // Prohibit to simplify test
    });

    manager->paintEmblems(kItemIconRole, mockInfo, &painter, &paintArea);

    EXPECT_TRUE(painter.renderHints() & QPainter::SmoothPixmapTransform);
}

TEST_F(UT_EmblemManager, PaintEmblems_ReturnsFalseWhenNoEmblems)
{
    QImage image(100, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    QRectF paintArea(0, 0, 100, 100);

    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    stub.set_lamda(&EmblemHelper::systemEmblems, [](EmblemHelper *, const FileInfoPointer &) {
        __DBG_STUB_INVOKE__
        return QList<QIcon>();
    });

    stub.set_lamda(&EmblemHelper::isExtEmblemProhibited, [](EmblemHelper *, const FileInfoPointer &, const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = manager->paintEmblems(kItemIconRole, mockInfo, &painter, &paintArea);

    EXPECT_FALSE(result);
}

TEST_F(UT_EmblemManager, PaintEmblems_PaintsSystemEmblems)
{
    QImage image(100, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    QRectF paintArea(0, 0, 100, 100);

    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    QList<QIcon> systemEmblems;
    systemEmblems << QIcon::fromTheme("emblem-symbolic-link");

    stub.set_lamda(&EmblemHelper::systemEmblems, [&](EmblemHelper *, const FileInfoPointer &) {
        __DBG_STUB_INVOKE__
        return systemEmblems;
    });

    stub.set_lamda(&EmblemHelper::isExtEmblemProhibited, [](EmblemHelper *, const FileInfoPointer &, const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&EmblemHelper::emblemRects, [](EmblemHelper *, const QRectF &) {
        __DBG_STUB_INVOKE__
        QList<QRectF> rects;
        rects << QRectF(10, 10, 20, 20);
        return rects;
    });

    bool result = manager->paintEmblems(kItemIconRole, mockInfo, &painter, &paintArea);

    EXPECT_TRUE(result);
}

TEST_F(UT_EmblemManager, PaintEmblems_CallsPendingForGioEmblems)
{
    QImage image(100, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    QRectF paintArea(0, 0, 100, 100);

    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    bool pendingCalled = false;

    stub.set_lamda(&EmblemHelper::systemEmblems, [](EmblemHelper *, const FileInfoPointer &) {
        __DBG_STUB_INVOKE__
        return QList<QIcon>();
    });

    stub.set_lamda(&EmblemHelper::isExtEmblemProhibited, [](EmblemHelper *, const FileInfoPointer &, const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&EmblemHelper::pending, [&](EmblemHelper *, const FileInfoPointer &) {
        __DBG_STUB_INVOKE__
        pendingCalled = true;
    });

    stub.set_lamda(&EmblemHelper::gioEmblemIcons, [](EmblemHelper *, const QUrl &) {
        __DBG_STUB_INVOKE__
        return QList<QIcon>();
    });

    stub.set_lamda(&EmblemEventSequence::doFetchCustomEmblems, [](EmblemEventSequence *, const QUrl &, QList<QIcon> *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&EmblemEventSequence::doFetchExtendEmblems, [](EmblemEventSequence *, const QUrl &, QList<QIcon> *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    manager->paintEmblems(kItemIconRole, mockInfo, &painter, &paintArea);

    EXPECT_TRUE(pendingCalled);
}

TEST_F(UT_EmblemManager, PaintEmblems_SkipsNullIcons)
{
    QImage image(100, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    QRectF paintArea(0, 0, 100, 100);

    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    QList<QIcon> emblems;
    emblems << QIcon();  // Null icon
    emblems << QIcon::fromTheme("emblem-symbolic-link");

    stub.set_lamda(&EmblemHelper::systemEmblems, [&](EmblemHelper *, const FileInfoPointer &) {
        __DBG_STUB_INVOKE__
        return emblems;
    });

    stub.set_lamda(&EmblemHelper::isExtEmblemProhibited, [](EmblemHelper *, const FileInfoPointer &, const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&EmblemHelper::emblemRects, [](EmblemHelper *, const QRectF &) {
        __DBG_STUB_INVOKE__
        QList<QRectF> rects;
        rects << QRectF(10, 10, 20, 20) << QRectF(30, 30, 20, 20);
        return rects;
    });

    bool result = manager->paintEmblems(kItemIconRole, mockInfo, &painter, &paintArea);

    EXPECT_TRUE(result);
}
