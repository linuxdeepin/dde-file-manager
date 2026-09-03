// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QCloseEvent>
#include <QDBusInterface>

#include "stubext.h"
#include "dfmplugin_propertydialog_global.h"
#include "views/basicwidget.h"
#include "events/propertyeventcall.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/filescanner.h>
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-io/dfileinfo.h>

DPPROPERTYDIALOG_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE

namespace {


}   // namespace

class BasicWidgetImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        tempDir.reset(new QTemporaryDir);
        tempFile.reset(new QTemporaryFile(tempDir->path() + "/basicXXXXXX.txt"));
        tempFile->open();
        url = QUrl::fromLocalFile(tempFile->fileName());
        parentUrl = QUrl::fromLocalFile(tempDir->path());
    }

    void TearDown() override
    {
        stub.clear();
        tempFile.reset();
        tempDir.reset();
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    std::unique_ptr<QTemporaryFile> tempFile;
    QUrl url;
    QUrl parentUrl;
};

TEST_F(BasicWidgetImpl, ConstructDestruct)
{
    BasicWidget *widget = new BasicWidget();
    EXPECT_NE(widget, nullptr);
    delete widget;
}

TEST_F(BasicWidgetImpl, SelectFileUrlRegularFile)
{

    BasicWidget widget;
    EXPECT_NO_THROW(widget.selectFileUrl(url));
    EXPECT_GE(widget.getFileSize(), 0);
    EXPECT_GE(widget.getFileCount(), 0);
    EXPECT_GE(widget.expansionPreditHeight(), 0);
}

TEST_F(BasicWidgetImpl, UpdateFileUrl)
{

    BasicWidget widget;
    widget.selectFileUrl(url);

    QUrl newUrl = QUrl::fromLocalFile(tempDir->path() + "/new.txt");
    EXPECT_NO_THROW(widget.updateFileUrl(newUrl));
}

TEST_F(BasicWidgetImpl, SlotFileCountAndSizeChange)
{
    // In a real run a plain file keeps no fileCount label (it is removed in
    // basicFill), and setRightValue would dereference it. Stub the non-virtual
    // KeyValueLabel::setRightValue so the slot's bookkeeping is verified
    // without touching the removed label.
    auto setRightValue = static_cast<void (KeyValueLabel::*)(QString, Qt::TextElideMode,
                                                             Qt::Alignment, bool, int)>(
            &KeyValueLabel::setRightValue);
    stub.set_lamda(setRightValue, [](KeyValueLabel *, QString, Qt::TextElideMode,
                                     Qt::Alignment, bool, int) {
        __DBG_STUB_INVOKE__
    });

    BasicWidget widget;
    widget.selectFileUrl(url);

    FileScanner::ScanResult result;
    result.totalSize = 4096;
    result.fileCount = 5;
    result.directoryCount = 2;
    EXPECT_NO_THROW(widget.slotFileCountAndSizeChange(result));
    EXPECT_EQ(widget.getFileSize(), 4096);
    EXPECT_EQ(widget.getFileCount(), 7);
}

TEST_F(BasicWidgetImpl, SlotFileHide)
{

    bool sent = false;
    stub.set_lamda(&PropertyEventCall::sendFileHide, [&sent](quint64, const QList<QUrl> &) {
        Q_UNUSED(sent)
        sent = true;
    });

    BasicWidget widget;
    widget.selectFileUrl(url);
    widget.slotFileHide(Qt::Checked);
    EXPECT_TRUE(sent);
}

TEST_F(BasicWidgetImpl, SlotOpenFileLocation)
{

    auto isValid = static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid);
    stub.set_lamda(isValid, [](QDBusAbstractInterface *) -> bool { return false; });

    BasicWidget widget;
    widget.selectFileUrl(url);
    EXPECT_NO_THROW(widget.slotOpenFileLocation());
}

TEST_F(BasicWidgetImpl, ImageExtenInfo)
{

    BasicWidget widget;
    widget.selectFileUrl(url);

    QMap<DFileInfo::AttributeExtendID, QVariant> props;
    props.insert(DFileInfo::AttributeExtendID::kExtendMediaWidth, 1920);
    props.insert(DFileInfo::AttributeExtendID::kExtendMediaHeight, 1080);
    EXPECT_NO_THROW(widget.imageExtenInfo(url, props));
}

TEST_F(BasicWidgetImpl, VideoExtenInfo)
{

    BasicWidget widget;
    widget.selectFileUrl(url);

    QMap<DFileInfo::AttributeExtendID, QVariant> props;
    props.insert(DFileInfo::AttributeExtendID::kExtendMediaWidth, 1920);
    props.insert(DFileInfo::AttributeExtendID::kExtendMediaHeight, 1080);
    props.insert(DFileInfo::AttributeExtendID::kExtendMediaDuration, 65000);
    EXPECT_NO_THROW(widget.videoExtenInfo(url, props));
}

TEST_F(BasicWidgetImpl, AudioExtenInfo)
{

    BasicWidget widget;
    widget.selectFileUrl(url);

    QMap<DFileInfo::AttributeExtendID, QVariant> props;
    props.insert(DFileInfo::AttributeExtendID::kExtendMediaDuration, 125000);
    EXPECT_NO_THROW(widget.audioExtenInfo(url, props));
}

TEST_F(BasicWidgetImpl, CloseEvent)
{
    BasicWidget widget;
    QCloseEvent event;
    EXPECT_NO_THROW(QApplication::sendEvent(&widget, &event));
}
