// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "textpreview.h"
#include "textpreviewplugin.h"
#include "textcontextwidget.h"
#include "textbrowseredit.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/interfaces/abstractbasepreview.h>
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

#include <gtest/gtest.h>

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QUrl>

#include <DTextEncoding>

#include <QByteArray>
#include <QStringList>

DFMBASE_USE_NAMESPACE
using namespace plugin_filepreview;

namespace {
constexpr int kPreviewSizeLimit { 1024 * 1024 * 5 };
}

class UT_TextPreview : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        preview = new TextPreview();
    }

    virtual void TearDown() override
    {
        delete preview;
        preview = nullptr;
        if (qApp)
            qApp->processEvents();
        for (const QString &path : tempFiles)
            QFile::remove(path);
        tempFiles.clear();
        stub.clear();
    }

    QString createTempFile(const QByteArray &content)
    {
        QTemporaryFile file;
        file.setAutoRemove(false);
        if (!file.open())
            return QString();
        file.write(content);
        file.close();
        tempFiles << file.fileName();
        return file.fileName();
    }

    QByteArray invalidUtf8Content()
    {
        return QByteArray::fromHex("d6d0cec4") + QByteArrayLiteral(" tail");
    }

protected:
    TextPreview *preview { nullptr };
    stub_ext::StubExt stub;
    QStringList tempFiles;
};

TEST_F(UT_TextPreview, Constructor_InitialState_AllAccessorsEmpty)
{
    EXPECT_EQ(preview->fileUrl(), QUrl());
    EXPECT_EQ(preview->contentWidget(), nullptr);
    EXPECT_TRUE(preview->title().isEmpty());
    EXPECT_FALSE(preview->showStatusBarSeparator());
}

TEST_F(UT_TextPreview, SetFileUrl_RemoteUrl_ReturnsFalse)
{
    QSignalSpy spy(preview, &AbstractBasePreview::titleChanged);

    EXPECT_FALSE(preview->setFileUrl(QUrl("http://example.com/readme.txt")));
    EXPECT_EQ(preview->contentWidget(), nullptr);
    EXPECT_EQ(preview->fileUrl(), QUrl());
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UT_TextPreview, SetFileUrl_NonExistentLocalFile_ReturnsFalse)
{
    EXPECT_FALSE(preview->setFileUrl(QUrl::fromLocalFile("/tmp/ut_text_not_exist_1234.txt")));
    EXPECT_EQ(preview->contentWidget(), nullptr);
    EXPECT_EQ(preview->fileUrl(), QUrl());
}

TEST_F(UT_TextPreview, SetFileUrl_EmptyLocalFile_ReturnsFalse)
{
    const QString path = createTempFile(QByteArray());

    EXPECT_FALSE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_NE(preview->contentWidget(), nullptr);
}

TEST_F(UT_TextPreview, SetFileUrl_ValidUtf8File_ReturnsTrueAndEmitsTitleChanged)
{
    QSignalSpy spy(preview, &AbstractBasePreview::titleChanged);
    const QString path = createTempFile(QByteArrayLiteral("hello text preview\nsecond line"));

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_EQ(preview->title(), QFileInfo(path).fileName());
    EXPECT_EQ(preview->fileUrl(), QUrl::fromLocalFile(path));
    EXPECT_EQ(spy.count(), 1);

    auto *context = dynamic_cast<TextContextWidget *>(preview->contentWidget());
    ASSERT_TRUE(context);
    EXPECT_TRUE(context->textBrowserEdit()->toPlainText().contains("hello text preview"));
}

TEST_F(UT_TextPreview, SetFileUrl_SameUrlCalledTwice_ReturnsTrueWithoutReload)
{
    QSignalSpy spy(preview, &AbstractBasePreview::titleChanged);
    const QString path = createTempFile(QByteArrayLiteral("once only"));
    const QUrl url = QUrl::fromLocalFile(path);

    ASSERT_TRUE(preview->setFileUrl(url));
    ASSERT_EQ(spy.count(), 1);

    EXPECT_TRUE(preview->setFileUrl(url));
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_TextPreview, SetFileUrl_Gb18030EncodedFile_ReturnsTrueAfterConversion)
{
    QSignalSpy spy(preview, &AbstractBasePreview::titleChanged);
    const QString path = createTempFile(QByteArray::fromHex("d6d0cec4"));

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_EQ(preview->title(), QFileInfo(path).fileName());
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_TextPreview, SetFileUrl_AllConversionsFailWithDetectedEncoding_ReturnsTrue)
{
    stub.set_lamda(ADDR(DTK_CORE_NAMESPACE::DTextEncoding, convertTextEncoding),
                   [](QByteArray &content, QByteArray &outContent, const QByteArray &toEncoding,
                      const QByteArray &fromEncoding, QString *errString) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });
    stub.set_lamda(ADDR(DTK_CORE_NAMESPACE::DTextEncoding, detectFileEncoding),
                   [](const QString &fileName, bool *isOk) -> QByteArray {
                       __DBG_STUB_INVOKE__
                       if (isOk)
                           *isOk = true;
                       return QByteArrayLiteral("GB18030");
                   });

    const QString path = createTempFile(invalidUtf8Content());

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_EQ(preview->title(), QFileInfo(path).fileName());
}

TEST_F(UT_TextPreview, SetFileUrl_EncodingDetectionFails_ReturnsTrueWithRawData)
{
    stub.set_lamda(ADDR(DTK_CORE_NAMESPACE::DTextEncoding, convertTextEncoding),
                   [](QByteArray &content, QByteArray &outContent, const QByteArray &toEncoding,
                      const QByteArray &fromEncoding, QString *errString) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });
    stub.set_lamda(ADDR(DTK_CORE_NAMESPACE::DTextEncoding, detectFileEncoding),
                   [](const QString &fileName, bool *isOk) -> QByteArray {
                       __DBG_STUB_INVOKE__
                       if (isOk)
                           *isOk = false;
                       return QByteArray();
                   });

    const QString path = createTempFile(invalidUtf8Content());

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_EQ(preview->title(), QFileInfo(path).fileName());
}

TEST_F(UT_TextPreview, SetFileUrl_DetectedEncodingIsUtf8_SkipsSecondConversion)
{
    stub.set_lamda(ADDR(DTK_CORE_NAMESPACE::DTextEncoding, convertTextEncoding),
                   [](QByteArray &content, QByteArray &outContent, const QByteArray &toEncoding,
                      const QByteArray &fromEncoding, QString *errString) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });
    stub.set_lamda(ADDR(DTK_CORE_NAMESPACE::DTextEncoding, detectFileEncoding),
                   [](const QString &fileName, bool *isOk) -> QByteArray {
                       __DBG_STUB_INVOKE__
                       if (isOk)
                           *isOk = true;
                       return QByteArrayLiteral("UTF-8");
                   });

    const QString path = createTempFile(invalidUtf8Content());

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_EQ(preview->title(), QFileInfo(path).fileName());
}

TEST_F(UT_TextPreview, SetFileUrl_FileLargerThanSizeLimit_TruncatesAndReturnsTrue)
{
    QByteArray big;
    big.reserve(kPreviewSizeLimit + 10);
    while (big.size() < kPreviewSizeLimit + 10)
        big += QByteArrayLiteral("0123456789abcdefghijklmnopqrstuvwxyz\n");
    big.resize(kPreviewSizeLimit + 10);

    const QString path = createTempFile(big);

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_EQ(preview->title(), QFileInfo(path).fileName());

    auto *context = dynamic_cast<TextContextWidget *>(preview->contentWidget());
    ASSERT_TRUE(context);
    EXPECT_EQ(context->textBrowserEdit()->toPlainText().size(), kPreviewSizeLimit);
}

TEST_F(UT_TextPreview, FileUrl_AfterSuccessfulLoad_ReturnsSelectedUrl)
{
    const QString path = createTempFile(QByteArrayLiteral("url check"));
    const QUrl url = QUrl::fromLocalFile(path);

    ASSERT_TRUE(preview->setFileUrl(url));
    EXPECT_EQ(preview->fileUrl(), url);
}

TEST_F(UT_TextPreview, Title_AfterSuccessfulLoad_ReturnsFileName)
{
    const QString path = createTempFile(QByteArrayLiteral("title check"));

    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_EQ(preview->title(), QFileInfo(path).fileName());
}

TEST_F(UT_TextPreview, ContentWidget_AfterSuccessfulLoad_ReturnsTextContextWidget)
{
    const QString path = createTempFile(QByteArrayLiteral("widget check"));

    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));

    auto *context = dynamic_cast<TextContextWidget *>(preview->contentWidget());
    EXPECT_TRUE(context);
    EXPECT_TRUE(context->textBrowserEdit());
}

TEST_F(UT_TextPreview, ShowStatusBarSeparator_AnyState_ReturnsFalse)
{
    EXPECT_FALSE(preview->showStatusBarSeparator());

    const QString path = createTempFile(QByteArrayLiteral("separator check"));
    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_FALSE(preview->showStatusBarSeparator());
}

TEST_F(UT_TextPreview, Destructor_AfterSuccessfulLoad_DestroysBrowserWithoutCrash)
{
    const QString path = createTempFile(QByteArrayLiteral("destroy check"));
    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));

    delete preview;
    preview = nullptr;

    EXPECT_NO_FATAL_FAILURE(qApp->processEvents());
}

class UT_TextPreviewPlugin : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        plugin = new TextPreviewPlugin();
    }

    virtual void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
    }

protected:
    TextPreviewPlugin *plugin { nullptr };
};

TEST_F(UT_TextPreviewPlugin, Create_WithArbitraryKey_ReturnsTextPreviewInstance)
{
    AbstractBasePreview *result = plugin->create("text");

    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(dynamic_cast<TextPreview *>(result));
    delete result;
}

TEST_F(UT_TextPreviewPlugin, Create_WithEmptyKey_ReturnsInstance)
{
    AbstractBasePreview *result = plugin->create("");

    ASSERT_NE(result, nullptr);
    delete result;
}

TEST_F(UT_TextPreviewPlugin, Create_RepeatedCalls_ReturnsDistinctInstances)
{
    AbstractBasePreview *first = plugin->create("text");
    AbstractBasePreview *second = plugin->create("text");

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_NE(first, second);

    delete first;
    delete second;
}
