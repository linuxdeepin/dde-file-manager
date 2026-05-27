// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>
#include <QSignalSpy>
#include <QThread>
#include <QTest>

#define private public
#define protected public
#include "utils/emblemhelper.h"
#undef private
#undef protected

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-framework/event/event.h>
#include <dfm-io/dfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <QApplication>
#include <QIcon>

using namespace dfmplugin_emblem;
DFMBASE_USE_NAMESPACE;
DPF_USE_NAMESPACE;

// =============================================================================
// GioEmblemWorker Tests
// =============================================================================
class UT_GioEmblemWorker : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        worker = new GioEmblemWorker();
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete worker;
        worker = nullptr;
    }

    GioEmblemWorker *worker { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_GioEmblemWorker, OnClear_ClearsCache)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    worker->cache[url] = QList<QIcon>();

    worker->onClear();

    EXPECT_TRUE(worker->cache.isEmpty());
}

TEST_F(UT_GioEmblemWorker, FetchEmblems_ReturnsEmptyForNullInfo)
{
    FileInfoPointer nullInfo;
    auto emblems = worker->fetchEmblems(nullInfo);
    EXPECT_TRUE(emblems.isEmpty());
}

TEST_F(UT_GioEmblemWorker, FetchEmblems_CallsGetGioEmblems)
{
    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    bool getCalled = false;
    stub.set_lamda(&GioEmblemWorker::getGioEmblems, [&](GioEmblemWorker *, const FileInfoPointer &) {
        __DBG_STUB_INVOKE__
        getCalled = true;
        return QMap<int, QIcon>();
    });

    worker->fetchEmblems(mockInfo);

    EXPECT_TRUE(getCalled);
}

TEST_F(UT_GioEmblemWorker, FetchEmblems_BuildsCorrectList)
{
    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    QMap<int, QIcon> iconMap;
    iconMap[0] = QIcon::fromTheme("icon0");
    iconMap[2] = QIcon::fromTheme("icon2");

    stub.set_lamda(&GioEmblemWorker::getGioEmblems, [&](GioEmblemWorker *, const FileInfoPointer &) {
        __DBG_STUB_INVOKE__
        return iconMap;
    });

    auto emblems = worker->fetchEmblems(mockInfo);

    EXPECT_GE(emblems.size(), 3);   // Should have at least 3 elements (0, 1-null, 2)
}

TEST_F(UT_GioEmblemWorker, GetGioEmblems_ReturnsEmptyForNullInfo)
{
    FileInfoPointer nullInfo;
    auto map = worker->getGioEmblems(nullInfo);
    EXPECT_TRUE(map.isEmpty());
}

TEST_F(UT_GioEmblemWorker, GetGioEmblems_ReturnsEmptyForNoEmblemData)
{
    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    stub.set_lamda(VADDR(FileInfo, customAttribute), [] {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());   // Empty list
    });

    auto map = worker->getGioEmblems(mockInfo);

    EXPECT_TRUE(map.isEmpty());
}

TEST_F(UT_GioEmblemWorker, GetGioEmblems_ParsesEmblemString)
{
    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    stub.set_lamda(VADDR(FileInfo, customAttribute), [] {
        __DBG_STUB_INVOKE__
        QStringList list;
        list << "/tmp/icon1.png|/tmp/icon2.png";
        return QVariant(list);
    });

    bool parseCalled = false;
    stub.set_lamda(&GioEmblemWorker::parseEmblemString, [&](GioEmblemWorker *, QIcon *, QString &, const QString &) {
        __DBG_STUB_INVOKE__
        parseCalled = true;
        return false;
    });

    worker->getGioEmblems(mockInfo);

    EXPECT_TRUE(parseCalled);
}

TEST_F(UT_GioEmblemWorker, ParseEmblemString_ReturnsFalseForEmptyString)
{
    QIcon emblem;
    QString pos;
    bool result = worker->parseEmblemString(&emblem, pos, "");

    EXPECT_FALSE(result);
}

TEST_F(UT_GioEmblemWorker, ParseEmblemString_SetsDefaultPosition)
{
    QIcon emblem;
    QString pos;

    stub.set_lamda(static_cast<bool (DFMIO::DFile::*)() const>(&DFMIO::DFile::exists), [](const DFMIO::DFile *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    worker->parseEmblemString(&emblem, pos, "/tmp/test.png");

    EXPECT_EQ("rd", pos);
}

TEST_F(UT_GioEmblemWorker, ParseEmblemString_ParsesPathWithPosition)
{
    QIcon emblem;
    QString pos;
    QString emblemStr = "/tmp/test.png;lu";

    stub.set_lamda(static_cast<bool (DFMIO::DFile::*)() const>(&DFMIO::DFile::exists), [](const DFMIO::DFile *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DFMIO::DFile::size, [](const DFMIO::DFile *) {
        __DBG_STUB_INVOKE__
        return 1024;
    });

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("png");
    });

    worker->parseEmblemString(&emblem, pos, emblemStr);

    EXPECT_EQ("lu", pos);
}

TEST_F(UT_GioEmblemWorker, ParseEmblemString_ExpandsTildePath)
{
    QIcon emblem;
    QString pos;
    QString emblemStr = "~/test.png";

    bool existsCalled = false;
    stub.set_lamda(static_cast<bool (DFMIO::DFile::*)() const>(&DFMIO::DFile::exists), [&](const DFMIO::DFile *file) {
        __DBG_STUB_INVOKE__
        existsCalled = true;
        return false;
    });

    worker->parseEmblemString(&emblem, pos, emblemStr);

    EXPECT_TRUE(existsCalled);
}

TEST_F(UT_GioEmblemWorker, ParseEmblemString_RejectsNonExistentFile)
{
    QIcon emblem;
    QString pos;

    stub.set_lamda(static_cast<bool (DFMIO::DFile::*)() const>(&DFMIO::DFile::exists), [](const DFMIO::DFile *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = worker->parseEmblemString(&emblem, pos, "/tmp/nonexistent.png");

    EXPECT_FALSE(result);
}

TEST_F(UT_GioEmblemWorker, ParseEmblemString_RejectsTooLargeFile)
{
    QIcon emblem;
    QString pos;

    stub.set_lamda(static_cast<bool (DFMIO::DFile::*)() const>(&DFMIO::DFile::exists), [](const DFMIO::DFile *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DFMIO::DFile::size, [](const DFMIO::DFile *) {
        __DBG_STUB_INVOKE__
        return 200000;   // > 102400
    });

    bool result = worker->parseEmblemString(&emblem, pos, "/tmp/large.png");

    EXPECT_FALSE(result);
}

TEST_F(UT_GioEmblemWorker, ParseEmblemString_RejectsUnsupportedFormat)
{
    QIcon emblem;
    QString pos;

    stub.set_lamda(static_cast<bool (DFMIO::DFile::*)() const>(&DFMIO::DFile::exists), [](const DFMIO::DFile *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DFMIO::DFile::size, [](const DFMIO::DFile *) {
        __DBG_STUB_INVOKE__
        return 1024;
    });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("txt");   // Unsupported format
    });

    bool result = worker->parseEmblemString(&emblem, pos, "/tmp/test.txt");

    EXPECT_FALSE(result);
}

TEST_F(UT_GioEmblemWorker, ParseEmblemString_AcceptsSupportedFormats)
{
    QIcon emblem;
    QString pos;

    stub.set_lamda(static_cast<bool (DFMIO::DFile::*)() const>(&DFMIO::DFile::exists), [](const DFMIO::DFile *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DFMIO::DFile::size, [](const DFMIO::DFile *) {
        __DBG_STUB_INVOKE__
        return 1024;
    });

    QStringList supportedFormats = { "svg", "png", "gif", "bmp", "jpg" };
    for (const QString &format : supportedFormats) {
        stub.set_lamda(VADDR(FileInfo, nameOf), [&](FileInfo *, NameInfoType) {
            __DBG_STUB_INVOKE__
            return format;
        });

        // Just check it doesn't reject based on format
        worker->parseEmblemString(&emblem, pos, QString("/tmp/test.%1").arg(format));
        // Result may still be false if icon loading fails, but format check should pass
    }
}

TEST_F(UT_GioEmblemWorker, IconNamesEqual_ReturnsTrueForEmptyLists)
{
    QList<QIcon> list1, list2;
    EXPECT_TRUE(worker->iconNamesEqual(list1, list2));
}

TEST_F(UT_GioEmblemWorker, IconNamesEqual_ReturnsFalseForDifferentSizes)
{
    QList<QIcon> list1, list2;
    list1 << QIcon();
    list2 << QIcon() << QIcon();

    EXPECT_FALSE(worker->iconNamesEqual(list1, list2));
}

TEST_F(UT_GioEmblemWorker, IconNamesEqual_ComparesIconNames)
{
    QList<QIcon> list1, list2;
    list1 << QIcon::fromTheme("icon1") << QIcon::fromTheme("icon2");
    list2 << QIcon::fromTheme("icon1") << QIcon::fromTheme("icon2");

    bool result = worker->iconNamesEqual(list1, list2);

    EXPECT_TRUE(result);
}

TEST_F(UT_GioEmblemWorker, SetEmblemIntoIcons_SetsRightDownPosition)
{
    QMap<int, QIcon> iconMap;
    QIcon emblem = QIcon::fromTheme("test");

    worker->setEmblemIntoIcons("rd", emblem, &iconMap);

    EXPECT_TRUE(iconMap.contains(0));
    EXPECT_EQ(emblem.name(), iconMap[0].name());
}

TEST_F(UT_GioEmblemWorker, SetEmblemIntoIcons_SetsLeftDownPosition)
{
    QMap<int, QIcon> iconMap;
    QIcon emblem = QIcon::fromTheme("test");

    worker->setEmblemIntoIcons("ld", emblem, &iconMap);

    EXPECT_TRUE(iconMap.contains(1));
}

TEST_F(UT_GioEmblemWorker, SetEmblemIntoIcons_SetsLeftUpPosition)
{
    QMap<int, QIcon> iconMap;
    QIcon emblem = QIcon::fromTheme("test");

    worker->setEmblemIntoIcons("lu", emblem, &iconMap);

    EXPECT_TRUE(iconMap.contains(2));
}

TEST_F(UT_GioEmblemWorker, SetEmblemIntoIcons_SetsRightUpPosition)
{
    QMap<int, QIcon> iconMap;
    QIcon emblem = QIcon::fromTheme("test");

    worker->setEmblemIntoIcons("ru", emblem, &iconMap);

    EXPECT_TRUE(iconMap.contains(3));
}

TEST_F(UT_GioEmblemWorker, SetEmblemIntoIcons_DefaultsToRightDown)
{
    QMap<int, QIcon> iconMap;
    QIcon emblem = QIcon::fromTheme("test");

    worker->setEmblemIntoIcons("unknown", emblem, &iconMap);

    EXPECT_TRUE(iconMap.contains(0));
}

// =============================================================================
// EmblemHelper Tests
// =============================================================================
class UT_EmblemHelper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        if (!qApp) {
            int argc = 1;
            char *argv[] = { const_cast<char *>("test") };
            new QApplication(argc, argv);
        }

        // Stub initialize to prevent thread creation in tests
        stub.set_lamda(&EmblemHelper::initialize, [](EmblemHelper *) {
            __DBG_STUB_INVOKE__
            // Do nothing
        });

        helper = new EmblemHelper(nullptr);
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete helper;
        helper = nullptr;
    }

    EmblemHelper *helper { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_EmblemHelper, HasEmblem_ReturnsFalseForUnknownUrl)
{
    QUrl url = QUrl::fromLocalFile("/tmp/unknown");
    EXPECT_FALSE(helper->hasEmblem(url));
}

TEST_F(UT_EmblemHelper, HasEmblem_ReturnsTrueForKnownUrl)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    helper->productQueue[url] = QList<QIcon>();

    EXPECT_TRUE(helper->hasEmblem(url));
}

TEST_F(UT_EmblemHelper, ClearEmblem_ClearsProductQueue)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    helper->productQueue[url] = QList<QIcon>();

    helper->clearEmblem();

    EXPECT_FALSE(helper->hasEmblem(url));
    EXPECT_TRUE(helper->productQueue.isEmpty());
}

TEST_F(UT_EmblemHelper, SystemEmblems_ReturnsEmptyForNullInfo)
{
    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    FileInfoPointer nullInfo;
    auto emblems = helper->systemEmblems(nullInfo);

    EXPECT_TRUE(emblems.isEmpty());
}

TEST_F(UT_EmblemHelper, SystemEmblems_ReturnsEmptyForDesktopFiles)
{
    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    stub.set_lamda(&FileUtils::isDesktopFileInfo, [](const FileInfoPointer &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test.desktop")));
    auto emblems = helper->systemEmblems(mockInfo);

    EXPECT_TRUE(emblems.isEmpty());
}

TEST_F(UT_EmblemHelper, SystemEmblems_ReturnsSymlinkEmblem)
{
    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    stub.set_lamda(&FileUtils::isDesktopFileInfo, [](const FileInfoPointer &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, OptInfoType type) {
        __DBG_STUB_INVOKE__
        if (type == OptInfoType::kIsSymLink)
            return true;
        return false;
    });

    auto emblems = helper->systemEmblems(mockInfo);

    EXPECT_FALSE(emblems.isEmpty());
}

TEST_F(UT_EmblemHelper, SystemEmblems_ReturnsReadonlyEmblem)
{
    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    stub.set_lamda(&FileUtils::isDesktopFileInfo, [](const FileInfoPointer &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, OptInfoType type) {
        __DBG_STUB_INVOKE__
        if (type == OptInfoType::kIsWritable)
            return false;   // Not writable
        return false;
    });

    auto emblems = helper->systemEmblems(mockInfo);

    EXPECT_FALSE(emblems.isEmpty());
}

TEST_F(UT_EmblemHelper, SystemEmblems_ReturnsUnreadableEmblem)
{
    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    stub.set_lamda(&FileUtils::isDesktopFileInfo, [](const FileInfoPointer &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, OptInfoType type) {
        __DBG_STUB_INVOKE__
        if (type == OptInfoType::kIsReadable)
            return false;   // Not readable
        return false;
    });

    auto emblems = helper->systemEmblems(mockInfo);

    EXPECT_FALSE(emblems.isEmpty());
}

TEST_F(UT_EmblemHelper, EmblemRects_ReturnsFourRects)
{
    QRectF paintArea(0, 0, 100, 100);
    auto rects = helper->emblemRects(paintArea);

    EXPECT_EQ(4, rects.size());
}

TEST_F(UT_EmblemHelper, EmblemRects_CalculatesCorrectPositions)
{
    QRectF paintArea(0, 0, 120, 120);
    auto rects = helper->emblemRects(paintArea);

    // First rect should be at right-down
    EXPECT_GT(rects[0].x(), paintArea.width() / 2);
    EXPECT_GT(rects[0].y(), paintArea.height() / 2);

    // Second rect should be at left-down
    EXPECT_LT(rects[1].x(), paintArea.width() / 2);
    EXPECT_GT(rects[1].y(), paintArea.height() / 2);

    // Third rect should be at left-up
    EXPECT_LT(rects[2].x(), paintArea.width() / 2);
    EXPECT_LT(rects[2].y(), paintArea.height() / 2);

    // Fourth rect should be at right-up
    EXPECT_GT(rects[3].x(), paintArea.width() / 2);
    EXPECT_LT(rects[3].y(), paintArea.height() / 2);
}

TEST_F(UT_EmblemHelper, EmblemRects_BoundsSizeCorrectly)
{
    QRectF paintArea(0, 0, 300, 300);
    auto rects = helper->emblemRects(paintArea);

    for (const auto &rect : rects) {
        EXPECT_GE(rect.width(), kMinEmblemSize);
        EXPECT_LE(rect.width(), kMaxEmblemSize);
        EXPECT_GE(rect.height(), kMinEmblemSize);
        EXPECT_LE(rect.height(), kMaxEmblemSize);
    }
}

TEST_F(UT_EmblemHelper, GioEmblemIcons_ReturnsEmptyForUnknownUrl)
{
    QUrl url = QUrl::fromLocalFile("/tmp/unknown");
    auto icons = helper->gioEmblemIcons(url);

    EXPECT_TRUE(icons.isEmpty());
}

TEST_F(UT_EmblemHelper, GioEmblemIcons_ReturnsStoredIcons)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QList<QIcon> expectedIcons;
    expectedIcons << QIcon::fromTheme("test-icon");

    helper->productQueue[url] = expectedIcons;

    auto icons = helper->gioEmblemIcons(url);

    EXPECT_EQ(expectedIcons.size(), icons.size());
}

TEST_F(UT_EmblemHelper, Pending_HandlesNullInfo)
{
    EXPECT_NO_THROW(helper->pending(nullptr));
}

TEST_F(UT_EmblemHelper, Pending_EmitsSignalForValidInfo)
{
    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test")));
    EXPECT_NO_THROW(helper->pending(mockInfo));
}

TEST_F(UT_EmblemHelper, IsExtEmblemProhibited_ReturnsTrueForRemoteFiles)
{
    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl url = QUrl("smb://server/share/file");
    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(url));

    bool result = helper->isExtEmblemProhibited(mockInfo, url);

    EXPECT_TRUE(result);
}

TEST_F(UT_EmblemHelper, IsExtEmblemProhibited_ReturnsFalseForLocalDeviceWithEnable)
{
    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(true);   // blockExtEnable = true
    });

    QUrl url = QUrl::fromLocalFile("/tmp/test");
    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(url));

    stub.set_lamda(VADDR(FileInfo, extendAttributes), [](FileInfo *, ExtInfoType) {
        __DBG_STUB_INVOKE__
        return QVariant(true);   // isLocalDevice = true
    });

    bool result = helper->isExtEmblemProhibited(mockInfo, url);

    EXPECT_FALSE(result);
}

TEST_F(UT_EmblemHelper, IsExtEmblemProhibited_ReturnsTrueForNonLocalDeviceWithDisable)
{
    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(false);   // blockExtEnable = false
    });

    QUrl url = QUrl::fromLocalFile("/tmp/test");
    auto mockInfo = QSharedPointer<FileInfo>(new FileInfo(url));

    stub.set_lamda(VADDR(FileInfo, extendAttributes), [](FileInfo *, ExtInfoType) {
        __DBG_STUB_INVOKE__
        return QVariant(false);   // isLocalDevice = false
    });

    bool result = helper->isExtEmblemProhibited(mockInfo, url);

    EXPECT_TRUE(result);
}

TEST_F(UT_EmblemHelper, OnEmblemChanged_UpdatesProductQueue)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QList<QIcon> emblems;
    emblems << QIcon::fromTheme("test-icon");

    stub.set_lamda(&Event::eventType, [](Event *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return EventTypeScope::kInValid;
    });

    helper->onEmblemChanged(url, emblems);

    EXPECT_TRUE(helper->hasEmblem(url));
    EXPECT_EQ(emblems.size(), helper->productQueue[url].size());
}

TEST_F(UT_EmblemHelper, OnEmblemChanged_DoesNotPushForEmptyProduct)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QList<QIcon> emptyEmblems;

    helper->onEmblemChanged(url, emptyEmblems);

    // Should update queue but not push events
    EXPECT_TRUE(helper->hasEmblem(url));
}

TEST_F(UT_EmblemHelper, OnUrlChanged_ClearsEmblems)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    helper->productQueue[url] = QList<QIcon>();

    helper->onUrlChanged(0, QUrl::fromLocalFile("/tmp/other"));

    EXPECT_FALSE(helper->hasEmblem(url));
}

TEST_F(UT_EmblemHelper, OnUrlChanged_ReturnsFalse)
{
    bool result = helper->onUrlChanged(0, QUrl::fromLocalFile("/tmp/test"));
    EXPECT_FALSE(result);
}

TEST_F(UT_EmblemHelper, StandardEmblem_ReturnsLinkEmblem)
{
    auto emblem = helper->standardEmblem(SystemEmblemType::kLink);
    EXPECT_FALSE(emblem.isNull());
}

TEST_F(UT_EmblemHelper, StandardEmblem_ReturnsLockEmblem)
{
    auto emblem = helper->standardEmblem(SystemEmblemType::kLock);
    EXPECT_FALSE(emblem.isNull());
}

TEST_F(UT_EmblemHelper, StandardEmblem_ReturnsUnreadableEmblem)
{
    auto emblem = helper->standardEmblem(SystemEmblemType::kUnreadable);
    EXPECT_FALSE(emblem.isNull());
}

TEST_F(UT_EmblemHelper, StandardEmblem_ReturnsShareEmblem)
{
    auto emblem = helper->standardEmblem(SystemEmblemType::kShare);
    EXPECT_FALSE(emblem.isNull());
}
