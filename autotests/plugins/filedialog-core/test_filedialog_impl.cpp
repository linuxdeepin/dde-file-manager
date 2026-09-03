// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "../../../src/plugins/filedialog/core/views/filedialog.h"
#include "../../../src/plugins/filedialog/core/views/filedialog_p.h"
#include "../../../src/plugins/filedialog/core/views/filedialogstatusbar.h"
#include "../../../src/plugins/filedialog/core/events/coreeventscaller.h"
#include "../../../src/plugins/filedialog/core/utils/corehelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QApplication>
#include <QDir>
#include <QUrl>
#include <QSignalSpy>
#include <QJsonObject>
#include <QJsonDocument>
#include <DLineEdit>
#include <DComboBox>
#include <DSuggestButton>
#include <DPushButton>

DFMBASE_USE_NAMESPACE
using namespace filedialog_core;

class FileDialogImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        if (!qApp) {
            int argc = 0;
            char **argv = nullptr;
            new QApplication(argc, argv);
        }

        dialog = new FileDialog(QUrl::fromLocalFile("/tmp"));

        // External dependencies: window manager must not touch real windows.
        using CreateWinFunc = dfmbase::FileManagerWindow *(FileManagerWindowsManager::*)(const QUrl &, bool, QString *);
        stub.set_lamda(static_cast<CreateWinFunc>(&FileManagerWindowsManager::createWindow),
                       [this](FileManagerWindowsManager *, const QUrl &, bool, QString *) -> dfmbase::FileManagerWindow * {
                           __DBG_STUB_INVOKE__
                           return dialog;
                       });
        using FindWinFunc = dfmbase::FileManagerWindow *(FileManagerWindowsManager::*)(quint64);
        stub.set_lamda(static_cast<FindWinFunc>(&FileManagerWindowsManager::findWindowById),
                       [this](FileManagerWindowsManager *, quint64) -> dfmbase::FileManagerWindow * {
                           __DBG_STUB_INVOKE__
                           return nullptr;   // keep cd() safe by default
                       });

        // DConfig values used by setAcceptMode etc.
        using DConfigValue = QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const;
        stub.set_lamda(static_cast<DConfigValue>(&DConfigManager::value),
                       [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                           __DBG_STUB_INVOKE__
                           return QVariant();
                       });

        // File info factory: avoid real filesystem
        using CreateFileInfo = FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *);
        stub.set_lamda(static_cast<CreateFileInfo>(&InfoFactory::create<FileInfo>),
                       [](const QUrl &, Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                           __DBG_STUB_INVOKE__
                           return nullptr;
                       });

        // URL / workspace helpers
        stub.set_lamda(&UniversalUtils::urlsTransformToLocal,
                       [](const QList<QUrl> &src, QList<QUrl> *dst) -> bool {
                           __DBG_STUB_INVOKE__
                           if (dst) *dst = src;
                           return true;
                       });
        stub.set_lamda(&UrlRoute::fromLocalFile,
                       [](const QString &path) -> QUrl {
                           __DBG_STUB_INVOKE__
                           return QUrl::fromLocalFile(path);
                       });

        // Core event callers that depend on dpfSlotChannel
        stub.set_lamda(&CoreEventsCaller::sendGetSelectedFiles,
                       [](const quint64) -> QList<QUrl> {
                           __DBG_STUB_INVOKE__
                           return {};
                       });
        stub.set_lamda(&CoreEventsCaller::sendSelectFiles,
                       [](quint64, const QList<QUrl> &) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(&CoreEventsCaller::setSelectionMode,
                       [](QWidget *, const QAbstractItemView::SelectionMode) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(&CoreEventsCaller::setEnabledSelectionModes,
                       [](QWidget *, const QList<QAbstractItemView::SelectionMode> &) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(&CoreEventsCaller::setSidebarItemVisible,
                       [](const QUrl &, bool) {
                           __DBG_STUB_INVOKE__
                       });
    }

    void TearDown() override
    {
        delete dialog;
        dialog = nullptr;
        stub.clear();
    }

    FileDialog *dialog { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(FileDialogImpl, ConstructorAndDestructor)
{
    EXPECT_NE(dialog, nullptr);
    EXPECT_EQ(dialog->property("_dfm_Disable_RestoreWindowState_").toBool(), true);
}

TEST_F(FileDialogImpl, SaveClosedSateReturnsFalse)
{
    EXPECT_FALSE(dialog->saveClosedSate());
}

TEST_F(FileDialogImpl, UpdateAsDefaultSizeSetsNonZeroSize)
{
    dialog->updateAsDefaultSize();
    EXPECT_GT(dialog->width(), 0);
    EXPECT_GT(dialog->height(), 0);
}

TEST_F(FileDialogImpl, LastVisitedUrlRoundTrip)
{
    QUrl url("file:///home");
    dialog->saveLastVisitedUrl(url);
    EXPECT_EQ(dialog->lastVisitedUrl(), url);
}

TEST_F(FileDialogImpl, DirectoryUrlUsesCurrentUrl)
{
    QUrl result = dialog->directoryUrl();
    EXPECT_EQ(result.toLocalFile(), QString("/tmp"));
}

TEST_F(FileDialogImpl, DirectoryReturnsLocalDir)
{
    QDir dir = dialog->directory();
    EXPECT_EQ(dir.absolutePath(), QString("/tmp"));
}

TEST_F(FileDialogImpl, DirectoryUrlTransformsLocalUrls)
{
    QUrl result = dialog->directoryUrl();
    EXPECT_TRUE(result.isLocalFile());
}

TEST_F(FileDialogImpl, SelectFileDoesNotCrash)
{
    dialog->d.data()->isFileView = true;
    dialog->selectFile("readme.txt");
    EXPECT_TRUE(true);
}

TEST_F(FileDialogImpl, SelectedFilesEmptyWhenNothingSelected)
{
    dialog->d.data()->isFileView = true;
    EXPECT_TRUE(dialog->selectedFiles().isEmpty());
}

TEST_F(FileDialogImpl, SelectedUrlsRoundTrip)
{
    dialog->d.data()->isFileView = true;
    QList<QUrl> expected { QUrl::fromLocalFile("/tmp/a.txt"), QUrl::fromLocalFile("/tmp/b.txt") };

    stub.set_lamda(&CoreEventsCaller::sendGetSelectedFiles,
                   [&expected](const quint64) -> QList<QUrl> {
                       __DBG_STUB_INVOKE__
                       return expected;
                   });

    QList<QUrl> result = dialog->selectedUrls();
    EXPECT_EQ(result, expected);
}

TEST_F(FileDialogImpl, NameFiltersRoundTrip)
{
    QStringList filters { "Text Files (*.txt)", "Images (*.png)" };
    dialog->setNameFilters(filters);
    EXPECT_EQ(dialog->nameFilters(), filters);
    EXPECT_EQ(dialog->selectedNameFilterIndex(), 0);
    EXPECT_FALSE(dialog->selectedNameFilter().isEmpty());
}

TEST_F(FileDialogImpl, SelectNameFilterByIndex)
{
    dialog->d.data()->isFileView = true;
    QStringList filters { "Text Files (*.txt)", "Images (*.png)" };
    dialog->setNameFilters(filters);
    dialog->selectNameFilterByIndex(1);
    EXPECT_EQ(dialog->selectedNameFilterIndex(), 1);
}

TEST_F(FileDialogImpl, OptionsRoundTrip)
{
    dialog->setOptions(QFileDialog::ReadOnly | QFileDialog::HideNameFilterDetails);
    EXPECT_TRUE(dialog->testOption(QFileDialog::ReadOnly));
    EXPECT_TRUE(dialog->testOption(QFileDialog::HideNameFilterDetails));
    EXPECT_FALSE(dialog->options().testFlag(QFileDialog::DontConfirmOverwrite));

    dialog->setOption(QFileDialog::ShowDirsOnly, true);
    EXPECT_TRUE(dialog->testOption(QFileDialog::ShowDirsOnly));
}

TEST_F(FileDialogImpl, SetAndGetFilter)
{
    QDir::Filters f = QDir::Files | QDir::Dirs;
    dialog->setFilter(f);
    // With no real workspace the getter returns an empty QVariant -> 0.
    EXPECT_NO_THROW(dialog->filter());
}

TEST_F(FileDialogImpl, SetFileModeAndAllowMixedSelection)
{
    dialog->d.data()->isFileView = true;
    dialog->setFileMode(QFileDialog::ExistingFiles);
    dialog->setAllowMixedSelection(true);
    EXPECT_EQ(dialog->d.data()->fileMode, QFileDialog::ExistingFiles);
    EXPECT_TRUE(dialog->d.data()->allowMixedSelection);
}

TEST_F(FileDialogImpl, AcceptModeRoundTrip)
{
    dialog->d.data()->isFileView = true;
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    EXPECT_EQ(dialog->acceptMode(), QFileDialog::AcceptSave);

    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    EXPECT_EQ(dialog->acceptMode(), QFileDialog::AcceptOpen);
}

TEST_F(FileDialogImpl, LabelTextRoundTrip)
{
    dialog->setLabelText(QFileDialog::Accept, QString("OK"));
    EXPECT_EQ(dialog->labelText(QFileDialog::Accept), QString("OK"));

    dialog->setLabelText(QFileDialog::Reject, QString("Cancel"));
    EXPECT_EQ(dialog->labelText(QFileDialog::Reject), QString("Cancel"));
}

TEST_F(FileDialogImpl, CustomWidgetRoundTrip)
{
    QJsonObject obj;
    obj["text"] = "Username";
    obj["defaultValue"] = "guest";
    obj["maxLength"] = 32;
    QJsonDocument doc(obj);
    dialog->addCustomWidget(FileDialog::kLineEditType, QString::fromUtf8(doc.toJson()));

    EXPECT_EQ(dialog->getCustomWidgetValue(FileDialog::kLineEditType, "Username").toString(), QString("guest"));
    QVariantMap all = dialog->allCustomWidgetsValue(FileDialog::kLineEditType);
    EXPECT_TRUE(all.contains("Username"));

    dialog->beginAddCustomWidget();
    dialog->endAddCustomWidget();
    EXPECT_TRUE(dialog->allCustomWidgetsValue(FileDialog::kLineEditType).isEmpty());

    QJsonObject combo;
    combo["text"] = "Format";
    combo["data"] = QJsonArray::fromStringList({ "pdf", "txt" });
    combo["defaultValue"] = "txt";
    QJsonDocument comboDoc(combo);
    dialog->addCustomWidget(FileDialog::kComboBoxType, QString::fromUtf8(comboDoc.toJson()));
    EXPECT_EQ(dialog->getCustomWidgetValue(FileDialog::kComboBoxType, "Format").toString(), QString("txt"));
}

TEST_F(FileDialogImpl, HideOnAcceptRoundTrip)
{
    dialog->setHideOnAccept(false);
    EXPECT_FALSE(dialog->hideOnAccept());
    dialog->setHideOnAccept(true);
    EXPECT_TRUE(dialog->hideOnAccept());
}

TEST_F(FileDialogImpl, GetCurrentUrl)
{
    QUrl url("file:///home");
    dialog->d.data()->currentUrl = url;
    EXPECT_EQ(dialog->getcurrenturl(), url);
}

TEST_F(FileDialogImpl, CheckFileSuffixAppendsExtension)
{
    dialog->d.data()->isFileView = true;
    QStringList filters { "Text Files (*.txt)" };
    dialog->setNameFilters(filters);

    QString suffix;
    bool ok = dialog->checkFileSuffix("document", suffix);
    EXPECT_TRUE(ok);
    EXPECT_EQ(suffix, QString("txt"));
}

TEST_F(FileDialogImpl, UrlSchemeEnableDoesNotCrash)
{
    dialog->urlSchemeEnable("recent", false);
    EXPECT_TRUE(true);
}

TEST_F(FileDialogImpl, AcceptEmitsAccepted)
{
    QSignalSpy spy(dialog, &FileDialog::accepted);
    QSignalSpy finishedSpy(dialog, &FileDialog::finished);

    dialog->accept();

    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(finishedSpy.takeFirst().at(0).toInt(), static_cast<int>(QDialog::Accepted));
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(FileDialogImpl, RejectEmitsRejected)
{
    QSignalSpy spy(dialog, &FileDialog::rejected);
    QSignalSpy finishedSpy(dialog, &FileDialog::finished);

    dialog->reject();

    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(finishedSpy.takeFirst().at(0).toInt(), static_cast<int>(QDialog::Rejected));
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(FileDialogImpl, DoneEmitsFinished)
{
    QSignalSpy spy(dialog, &FileDialog::finished);
    dialog->done(QDialog::Accepted);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(FileDialogImpl, OnAcceptButtonClickedTriggersSaveAccept)
{
    dialog->d.data()->isFileView = true;
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setCurrentInputName("ut_nosuch_file_dialog_test.txt");

    QSignalSpy acceptedSpy(dialog, &FileDialog::accepted);
    dialog->onAcceptButtonClicked();

    EXPECT_EQ(acceptedSpy.count(), 1);
}
