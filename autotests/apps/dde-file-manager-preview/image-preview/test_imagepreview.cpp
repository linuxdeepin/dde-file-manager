// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "imagepreview.h"
#include "imagepreviewplugin.h"
#include "imageview.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <gtest/gtest.h>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QLabel>
#include <QPointer>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>
#include <QWidget>

#include <mutex>

DFMBASE_USE_NAMESPACE
using namespace plugin_filepreview;

class UT_ImagePreview : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }

    void SetUp() override
    {
        preview = new ImagePreview();
        window = new QWidget();
        statusBar = new QWidget();
    }

    void TearDown() override
    {
        delete preview;
        delete statusBar;
        delete window;
        qApp->processEvents();
        stub.clear();
    }

    QString makeImageFile(const QString &name, int width, int height, const char *format)
    {
        QImage image(width, height, QImage::Format_ARGB32);
        image.fill(Qt::red);
        const QString &path = dir.filePath(name);
        if (!image.save(path, format))
            return QString();
        return path;
    }

    QString makeTextFile(const QString &name, const QByteArray &content)
    {
        const QString &path = dir.filePath(name);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly))
            return QString();
        file.write(content);
        file.close();
        return path;
    }

protected:
    ImagePreview *preview { nullptr };
    QWidget *window { nullptr };
    QWidget *statusBar { nullptr };
    QTemporaryDir dir;
    stub_ext::StubExt stub;
    static std::once_flag flag;
};

std::once_flag UT_ImagePreview::flag;

TEST_F(UT_ImagePreview, Construct_FreshInstance_AllAccessorsReturnDefaults)
{
    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_EQ(preview->contentWidget(), nullptr);
    EXPECT_TRUE(preview->title().isEmpty());
    EXPECT_EQ(preview->statusBarWidget(), nullptr);
    EXPECT_EQ(static_cast<int>(preview->statusBarWidgetAlignment()), 0);
}

TEST_F(UT_ImagePreview, Initialize_WithStatusBar_CreatesConfiguredLabel)
{
    preview->initialize(window, statusBar);

    QWidget *widget = preview->statusBarWidget();
    ASSERT_NE(widget, nullptr);
    auto *label = qobject_cast<QLabel *>(widget);
    ASSERT_NE(label, nullptr);
    EXPECT_EQ(label->parentWidget(), statusBar);
    EXPECT_EQ(label->font().pixelSize(), 12);
    EXPECT_EQ(label->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    EXPECT_EQ(label->sizePolicy().verticalPolicy(), QSizePolicy::Preferred);
    EXPECT_EQ(static_cast<int>(label->alignment()), static_cast<int>(Qt::AlignCenter));
    EXPECT_FALSE(label->isHidden());
}

TEST_F(UT_ImagePreview, CanPreview_ValidPngFile_ReturnsTrueAndFillsFormat)
{
    const QString &path = makeImageFile("sample.png", 64, 48, "PNG");
    ASSERT_FALSE(path.isEmpty());

    QByteArray format;
    EXPECT_TRUE(preview->canPreview(QUrl::fromLocalFile(path), &format));
    EXPECT_EQ(format, QByteArray("png"));
}

TEST_F(UT_ImagePreview, CanPreview_ValidJpegFile_ReturnsTrueAndFillsFormat)
{
    const QString &path = makeImageFile("photo.jpg", 32, 32, "JPG");
    ASSERT_FALSE(path.isEmpty());

    QByteArray format;
    EXPECT_TRUE(preview->canPreview(QUrl::fromLocalFile(path), &format));
    EXPECT_EQ(format, QByteArray("jpeg"));
}

TEST_F(UT_ImagePreview, CanPreview_NonImageFile_ReturnsFalseWithMimeSuffix)
{
    const QString &path = makeTextFile("notes.txt", "plain text content");
    ASSERT_FALSE(path.isEmpty());

    QByteArray format;
    EXPECT_FALSE(preview->canPreview(QUrl::fromLocalFile(path), &format));
    EXPECT_EQ(format, QByteArray("txt"));
}

TEST_F(UT_ImagePreview, CanPreview_NonExistentFile_ReturnsFalseWithEmptyFormat)
{
    QByteArray format("sentinel");
    EXPECT_FALSE(preview->canPreview(QUrl::fromLocalFile(dir.filePath("missing.png")), &format));
    EXPECT_TRUE(format.isEmpty());
}

TEST_F(UT_ImagePreview, CanPreview_NullFormatPointer_ReturnsTrueWithoutCrash)
{
    const QString &path = makeImageFile("nopointer.png", 16, 16, "PNG");
    ASSERT_FALSE(path.isEmpty());

    EXPECT_TRUE(preview->canPreview(QUrl::fromLocalFile(path)));
}

TEST_F(UT_ImagePreview, SetFileUrl_UnknownScheme_ReturnsFalseAndKeepsState)
{
    QSignalSpy spy(preview, SIGNAL(titleChanged()));

    EXPECT_FALSE(preview->setFileUrl(QUrl("fake://somehost/image.png")));
    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_EQ(preview->contentWidget(), nullptr);
    EXPECT_TRUE(preview->title().isEmpty());
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UT_ImagePreview, SetFileUrl_NonExistentLocalFile_ReturnsFalse)
{
    QSignalSpy spy(preview, SIGNAL(titleChanged()));
    preview->initialize(window, statusBar);

    EXPECT_FALSE(preview->setFileUrl(QUrl::fromLocalFile(dir.filePath("missing.png"))));
    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UT_ImagePreview, SetFileUrl_NonImageLocalFile_ReturnsFalse)
{
    const QString &path = makeTextFile("document.txt", "not an image at all");
    ASSERT_FALSE(path.isEmpty());
    preview->initialize(window, statusBar);

    EXPECT_FALSE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_TRUE(preview->title().isEmpty());
}

TEST_F(UT_ImagePreview, SetFileUrl_RedirectedToNonLocalUrl_ReturnsFalse)
{
    const QString &path = makeImageFile("redirected.png", 64, 48, "PNG");
    ASSERT_FALSE(path.isEmpty());
    preview->initialize(window, statusBar);

    stub.set_lamda(VADDR(SyncFileInfo, canAttributes),
                   [](SyncFileInfo *, CanableInfoType) -> bool { return true; });
    stub.set_lamda(VADDR(SyncFileInfo, urlOf),
                   [](SyncFileInfo *, UrlInfoType) -> QUrl { return QUrl("http://example.com/remote.png"); });

    EXPECT_FALSE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_TRUE(preview->fileUrl().isEmpty());
}

TEST_F(UT_ImagePreview, SetFileUrl_ValidPng_ReturnsTrueAndUpdatesAllState)
{
    const QString &path = makeImageFile("valid.png", 64, 48, "PNG");
    ASSERT_FALSE(path.isEmpty());
    preview->initialize(window, statusBar);
    QSignalSpy spy(preview, SIGNAL(titleChanged()));

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_EQ(preview->fileUrl(), QUrl::fromLocalFile(path));
    EXPECT_EQ(preview->title(), QString("valid.png"));
    ASSERT_NE(preview->contentWidget(), nullptr);
    EXPECT_NE(qobject_cast<ImageView *>(preview->contentWidget()), nullptr);
    EXPECT_EQ(qobject_cast<QLabel *>(preview->statusBarWidget())->text(), QString("64x48"));
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_ImagePreview, SetFileUrl_SameUrlTwice_ReturnsTrueWithoutReloading)
{
    const QString &path = makeImageFile("same.png", 64, 48, "PNG");
    ASSERT_FALSE(path.isEmpty());
    preview->initialize(window, statusBar);
    QSignalSpy spy(preview, SIGNAL(titleChanged()));

    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_EQ(spy.count(), 1);

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(preview->title(), QString("same.png"));
}

TEST_F(UT_ImagePreview, SetFileUrl_DifferentValidFile_ReusesImageViewAndUpdatesState)
{
    const QString &firstPath = makeImageFile("first.png", 64, 48, "PNG");
    const QString &secondPath = makeImageFile("second.png", 32, 24, "PNG");
    ASSERT_FALSE(firstPath.isEmpty());
    ASSERT_FALSE(secondPath.isEmpty());
    preview->initialize(window, statusBar);

    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(firstPath)));
    QWidget *firstWidget = preview->contentWidget();
    ASSERT_NE(firstWidget, nullptr);

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(secondPath)));
    EXPECT_EQ(preview->contentWidget(), firstWidget);
    EXPECT_EQ(preview->fileUrl(), QUrl::fromLocalFile(secondPath));
    EXPECT_EQ(preview->title(), QString("second.png"));
    EXPECT_EQ(qobject_cast<QLabel *>(preview->statusBarWidget())->text(), QString("32x24"));
}

TEST_F(UT_ImagePreview, Destructor_AfterInitializeAndSetFile_SchedulesWidgetDeletion)
{
    const QString &path = makeImageFile("destroy.png", 16, 16, "PNG");
    ASSERT_FALSE(path.isEmpty());
    preview->initialize(window, statusBar);
    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));

    QPointer<QWidget> content(preview->contentWidget());
    QPointer<QWidget> status(preview->statusBarWidget());

    delete preview;
    preview = nullptr;
    qApp->processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    EXPECT_TRUE(content.isNull());
    EXPECT_TRUE(status.isNull());
}

TEST_F(UT_ImagePreview, Destructor_WithoutInitialize_NoCrash)
{
    delete preview;
    preview = nullptr;
    qApp->processEvents();
    SUCCEED();
}

class UT_ImagePreviewPlugin : public testing::Test
{
protected:
    void SetUp() override
    {
        plugin = new ImagePreviewPlugin();
    }

    void TearDown() override
    {
        delete plugin;
    }

protected:
    ImagePreviewPlugin *plugin { nullptr };
};

TEST_F(UT_ImagePreviewPlugin, Create_AnyKey_ReturnsImagePreviewInstance)
{
    AbstractBasePreview *result = plugin->create("image");
    ASSERT_NE(result, nullptr);
    EXPECT_NE(qobject_cast<ImagePreview *>(result), nullptr);
    delete result;
}

TEST_F(UT_ImagePreviewPlugin, Create_EmptyKey_ReturnsImagePreviewInstance)
{
    AbstractBasePreview *result = plugin->create("");
    ASSERT_NE(result, nullptr);
    EXPECT_NE(qobject_cast<ImagePreview *>(result), nullptr);
    delete result;
}

TEST_F(UT_ImagePreviewPlugin, Create_MultipleCalls_ReturnsDistinctInstances)
{
    AbstractBasePreview *first = plugin->create("image");
    AbstractBasePreview *second = plugin->create("image");
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_NE(first, second);
    delete first;
    delete second;
}

TEST_F(UT_ImagePreviewPlugin, MetaObject_PluginInstance_ReportsPluginClassName)
{
    EXPECT_EQ(QString(plugin->metaObject()->className()), QString("plugin_filepreview::ImagePreviewPlugin"));
}
