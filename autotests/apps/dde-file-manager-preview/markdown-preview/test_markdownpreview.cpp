// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "markdownpreview.h"
#include "markdownpreviewplugin.h"
#include "markdowncontextwidget.h"
#include "markdownbrowser.h"

#include <dfm-base/interfaces/abstractbasepreview.h>

#include <gtest/gtest.h>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QSignalSpy>
#include <QTemporaryFile>

DFMBASE_USE_NAMESPACE
using namespace plugin_filepreview;

class UT_MarkdownPreview : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        preview = new MarkdownPreview();
    }

    virtual void TearDown() override
    {
        for (const QString &path : tempFiles)
            QFile::remove(path);
        delete preview;
        preview = nullptr;
        stub.clear();
        qApp->processEvents();
    }

    QString createTempFile(const QString &suffix, const QByteArray &content)
    {
        QTemporaryFile file;
        file.setAutoRemove(false);
        file.setFileTemplate(QDir::temp().absoluteFilePath("ut-markdown-XXXXXX" + suffix));
        if (!file.open())
            return QString();
        file.write(content);
        file.close();
        tempFiles.append(file.fileName());
        return file.fileName();
    }

protected:
    MarkdownPreview *preview { nullptr };
    stub_ext::StubExt stub;
    QStringList tempFiles;
};

class UT_MarkdownPreviewPlugin : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        plugin = new MarkdownPreviewPlugin();
    }

    virtual void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

protected:
    MarkdownPreviewPlugin *plugin { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_MarkdownPreview, Constructor_FreshInstance_AllAccessorsEmpty)
{
    EXPECT_EQ(QUrl(), preview->fileUrl());
    EXPECT_TRUE(preview->title().isEmpty());
    EXPECT_EQ(nullptr, preview->contentWidget());
    EXPECT_EQ(nullptr, qobject_cast<MarkdownContextWidget *>(preview->contentWidget()));
}

TEST_F(UT_MarkdownPreview, FileUrl_BeforeAnyLoad_ReturnsEmptyUrl)
{
    EXPECT_EQ(QUrl(), preview->fileUrl());
    EXPECT_TRUE(preview->fileUrl().isEmpty());
}

TEST_F(UT_MarkdownPreview, Title_BeforeAnyLoad_ReturnsEmptyString)
{
    EXPECT_EQ(QString(), preview->title());
    EXPECT_TRUE(preview->title().isEmpty());
}

TEST_F(UT_MarkdownPreview, ContentWidget_BeforeAnyLoad_ReturnsNullPointer)
{
    EXPECT_EQ(nullptr, preview->contentWidget());
    EXPECT_TRUE(preview->title().isEmpty());
}

TEST_F(UT_MarkdownPreview, ShowStatusBarSeparator_AnyCall_ReturnsFalse)
{
    EXPECT_FALSE(preview->showStatusBarSeparator());
    EXPECT_FALSE(preview->showStatusBarSeparator());
}

TEST_F(UT_MarkdownPreview, SetFileUrl_EmptyUrlOnFreshInstance_ReturnsTrueWithoutLoading)
{
    QSignalSpy spy(preview, &AbstractBasePreview::titleChanged);

    EXPECT_TRUE(preview->setFileUrl(QUrl()));

    EXPECT_EQ(0, spy.count());
    EXPECT_EQ(nullptr, preview->contentWidget());
    EXPECT_TRUE(preview->title().isEmpty());
}

TEST_F(UT_MarkdownPreview, SetFileUrl_RemoteUrl_ReturnsFalse)
{
    QSignalSpy spy(preview, &AbstractBasePreview::titleChanged);

    EXPECT_FALSE(preview->setFileUrl(QUrl("https://example.com/readme.md")));

    EXPECT_EQ(0, spy.count());
    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_EQ(nullptr, preview->contentWidget());
}

TEST_F(UT_MarkdownPreview, SetFileUrl_NonExistentLocalFile_ReturnsFalse)
{
    QUrl url = QUrl::fromLocalFile(QDir::temp().absoluteFilePath("ut-markdown-ghost-not-exist.md"));
    ASSERT_FALSE(QFileInfo::exists(url.toLocalFile()));

    EXPECT_FALSE(preview->setFileUrl(url));

    EXPECT_NE(url, preview->fileUrl());
    EXPECT_EQ(nullptr, preview->contentWidget());
}

TEST_F(UT_MarkdownPreview, SetFileUrl_EmptyLocalFile_ReturnsFalse)
{
    QString path = createTempFile(".md", QByteArray());
    ASSERT_FALSE(path.isEmpty());
    ASSERT_EQ(QFileInfo(path).size(), 0);

    EXPECT_FALSE(preview->setFileUrl(QUrl::fromLocalFile(path)));

    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_EQ(nullptr, preview->contentWidget());
}

TEST_F(UT_MarkdownPreview, SetFileUrl_FileOpenFailure_ReturnsFalse)
{
    QString path = createTempFile(".md", "# OpenFail\n\nbody");
    ASSERT_FALSE(path.isEmpty());
    QFile permissionHolder(path);
    ASSERT_TRUE(permissionHolder.setPermissions(QFileDevice::Permissions()));
    ASSERT_TRUE(QFileInfo::exists(path));
    ASSERT_GT(QFileInfo(path).size(), 0);

    EXPECT_FALSE(preview->setFileUrl(QUrl::fromLocalFile(path)));

    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_EQ(nullptr, preview->contentWidget());
    permissionHolder.setPermissions(QFileDevice::ReadOwner);
}

TEST_F(UT_MarkdownPreview, SetFileUrl_FileReadFailure_ReturnsFalse)
{
    QString path = createTempFile(".md", "# ReadFail\n\nbody");
    ASSERT_FALSE(path.isEmpty());

    stub.set_lamda(static_cast<QByteArray (QIODevice::*)(qint64)>(&QIODevice::read),
                   [](QIODevice *, qint64) -> QByteArray {
                       __DBG_STUB_INVOKE__
                       return QByteArray();
                   });

    EXPECT_FALSE(preview->setFileUrl(QUrl::fromLocalFile(path)));

    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_EQ(nullptr, preview->contentWidget());
}

TEST_F(UT_MarkdownPreview, SetFileUrl_ValidMarkdownFile_ReturnsTrueAndUpdatesState)
{
    QString path = createTempFile(".md", "# Preview Heading\n\nMarkdown body content");
    ASSERT_FALSE(path.isEmpty());
    QUrl url = QUrl::fromLocalFile(path);
    QSignalSpy spy(preview, &AbstractBasePreview::titleChanged);

    EXPECT_TRUE(preview->setFileUrl(url));

    EXPECT_EQ(url, preview->fileUrl());
    EXPECT_EQ(QFileInfo(path).fileName(), preview->title());
    EXPECT_EQ(1, spy.count());

    auto context = qobject_cast<MarkdownContextWidget *>(preview->contentWidget());
    ASSERT_NE(nullptr, context);
    EXPECT_TRUE(context->markdownBrowser()->document()->toPlainText().contains("Preview Heading"));
    EXPECT_EQ(context->markdownBrowser()->searchPaths(), QStringList { QFileInfo(path).absolutePath() });
}

TEST_F(UT_MarkdownPreview, SetFileUrl_SameUrlRepeated_ReturnsTrueWithoutReloading)
{
    QString path = createTempFile(".md", "# Stable content");
    ASSERT_FALSE(path.isEmpty());
    QUrl url = QUrl::fromLocalFile(path);
    ASSERT_TRUE(preview->setFileUrl(url));

    QSignalSpy spy(preview, &AbstractBasePreview::titleChanged);
    QWidget *widgetBefore = preview->contentWidget();

    EXPECT_TRUE(preview->setFileUrl(url));

    EXPECT_EQ(0, spy.count());
    EXPECT_EQ(widgetBefore, preview->contentWidget());
    EXPECT_EQ(url, preview->fileUrl());
}

TEST_F(UT_MarkdownPreview, SetFileUrl_SecondFile_ReusesSameContentWidget)
{
    QString firstPath = createTempFile(".md", "# First document");
    QString secondPath = createTempFile(".md", "# Second document");
    ASSERT_FALSE(firstPath.isEmpty());
    ASSERT_FALSE(secondPath.isEmpty());

    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(firstPath)));
    QWidget *widgetBefore = preview->contentWidget();

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(secondPath)));

    EXPECT_EQ(widgetBefore, preview->contentWidget());
    EXPECT_EQ(QUrl::fromLocalFile(secondPath), preview->fileUrl());
    EXPECT_EQ(QFileInfo(secondPath).fileName(), preview->title());

    auto context = qobject_cast<MarkdownContextWidget *>(preview->contentWidget());
    ASSERT_NE(nullptr, context);
    EXPECT_TRUE(context->markdownBrowser()->document()->toPlainText().contains("Second document"));
    EXPECT_FALSE(context->markdownBrowser()->document()->toPlainText().contains("First document"));
}

TEST_F(UT_MarkdownPreview, SetFileUrl_FileOverSizeLimit_TruncatesContentToFiveMB)
{
    const QByteArray oversized(5 * 1024 * 1024 + 1, 'a');
    QString path = createTempFile(".md", oversized);
    ASSERT_FALSE(path.isEmpty());

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));

    auto context = qobject_cast<MarkdownContextWidget *>(preview->contentWidget());
    ASSERT_NE(nullptr, context);
    EXPECT_EQ(5 * 1024 * 1024, context->markdownBrowser()->document()->toPlainText().size());
    EXPECT_EQ(QFileInfo(path).fileName(), preview->title());
}

TEST_F(UT_MarkdownPreview, SetFileUrl_TextSuffixFile_StillLoadsAsMarkdown)
{
    QString path = createTempFile(".txt", "# Txt as markdown");
    ASSERT_FALSE(path.isEmpty());

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));

    auto context = qobject_cast<MarkdownContextWidget *>(preview->contentWidget());
    ASSERT_NE(nullptr, context);
    EXPECT_TRUE(context->markdownBrowser()->document()->toPlainText().contains("Txt as markdown"));
    EXPECT_EQ(QFileInfo(path).fileName(), preview->title());
}

TEST_F(UT_MarkdownPreview, Destructor_AfterLoad_DefersWidgetDeletionToEventLoop)
{
    QString path = createTempFile(".md", "# Deferred delete");
    ASSERT_FALSE(path.isEmpty());
    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));

    QPointer<MarkdownContextWidget> guard = qobject_cast<MarkdownContextWidget *>(preview->contentWidget());
    ASSERT_FALSE(guard.isNull());

    delete preview;
    preview = nullptr;

    EXPECT_FALSE(guard.isNull());
    qApp->processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    EXPECT_TRUE(guard.isNull());
}

TEST_F(UT_MarkdownPreviewPlugin, Create_WithMarkdownKey_ReturnsMarkdownPreviewInstance)
{
    AbstractBasePreview *created = plugin->create("markdown");
    EXPECT_NE(nullptr, created);
    EXPECT_NE(nullptr, qobject_cast<MarkdownPreview *>(created));
    delete created;
}

TEST_F(UT_MarkdownPreviewPlugin, Create_WithEmptyKey_ReturnsMarkdownPreviewInstance)
{
    AbstractBasePreview *created = plugin->create("");
    EXPECT_NE(nullptr, created);
    EXPECT_NE(nullptr, qobject_cast<MarkdownPreview *>(created));
    delete created;
}

TEST_F(UT_MarkdownPreviewPlugin, Create_RepeatedCalls_ReturnDistinctInstances)
{
    AbstractBasePreview *first = plugin->create("markdown");
    AbstractBasePreview *second = plugin->create("other-key");
    EXPECT_NE(nullptr, first);
    EXPECT_NE(nullptr, second);
    EXPECT_NE(first, second);
    delete first;
    delete second;
}
