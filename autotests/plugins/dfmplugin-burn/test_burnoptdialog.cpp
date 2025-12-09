// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/dialogs/burnoptdialog.h"
#include "plugins/common/dfmplugin-burn/utils/burnjobmanager.h"
#include "plugins/common/dfmplugin-burn/utils/burnhelper.h"

#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <dfm-burn/dopticaldiscmanager.h>
#include <dfm-burn/dopticaldiscinfo.h>

#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QFile>
#include <QTemporaryFile>

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_BurnOptDialog : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(VADDR(DDialog, exec), [&] { __DBG_STUB_INVOKE__ return QDialog::Accepted; });
        stub.set_lamda(ADDR(QWidget, show), [&] {
            __DBG_STUB_INVOKE__
        });

        // Mock WindowUtils to avoid Wayland-specific code
        stub.set_lamda(ADDR(WindowUtils, isWayLand), [] {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(ADDR(Settings, sync), [](Settings *) {
            __DBG_STUB_INVOKE__
            return true;
        });

        dialog = new BurnOptDialog("/dev/sr0");
    }

    virtual void TearDown() override
    {
        delete dialog;
        dialog = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    BurnOptDialog *dialog = nullptr;
};

TEST_F(UT_BurnOptDialog, Constructor)
{
    EXPECT_EQ(dialog->curDev, "/dev/sr0");
    EXPECT_TRUE(dialog->isModal());
}

TEST_F(UT_BurnOptDialog, Constructor_Wayland)
{
    stub.set_lamda(ADDR(WindowUtils, isWayLand), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    BurnOptDialog waylandDialog("/dev/sr1");
    EXPECT_EQ(waylandDialog.curDev, "/dev/sr1");
}

TEST_F(UT_BurnOptDialog, setUDFSupported_Supported)
{
    dialog->setUDFSupported(true, false);

    EXPECT_TRUE(dialog->isSupportedUDF);
}

TEST_F(UT_BurnOptDialog, setUDFSupported_NotSupported)
{
    dialog->setUDFSupported(false, false);

    EXPECT_FALSE(dialog->isSupportedUDF);
}

TEST_F(UT_BurnOptDialog, setUDFSupported_DisableISOOpts)
{
    dialog->setUDFSupported(true, true);

    EXPECT_TRUE(dialog->isSupportedUDF);
    // When ISO options are disabled, UDF should be selected by default
    EXPECT_EQ(dialog->fsComb->currentIndex(), 3);
}

TEST_F(UT_BurnOptDialog, setISOImage_ValidImage)
{
    QTemporaryFile tempFile;
    ASSERT_TRUE(tempFile.open());
    QUrl imageUrl = QUrl::fromLocalFile(tempFile.fileName());

    // Mock DOpticalDiscManager
    stub.set_lamda(ADDR(DFMBURN::DOpticalDiscManager, createOpticalInfo), [] {
        __DBG_STUB_INVOKE__
        auto info = new DFMBURN::DOpticalDiscInfo();
        // Mock volumeName method
        return info;
    });

    EXPECT_NO_THROW(dialog->setISOImage(imageUrl));

    EXPECT_EQ(dialog->imageFile, imageUrl);
    EXPECT_FALSE(dialog->finalizeDiscCheckbox->isVisible());
    EXPECT_FALSE(dialog->fsComb->isVisible());
    EXPECT_FALSE(dialog->volnameEdit->isEnabled());
}

TEST_F(UT_BurnOptDialog, setISOImage_InvalidImage)
{
    QUrl invalidUrl = QUrl::fromLocalFile("/non/existent/file.iso");

    stub.set_lamda(ADDR(DFMBURN::DOpticalDiscManager, createOpticalInfo), [] {
        __DBG_STUB_INVOKE__
        return nullptr;   // Return null for invalid image
    });

    dialog->setISOImage(invalidUrl);

    EXPECT_EQ(dialog->imageFile, invalidUrl);
}

TEST_F(UT_BurnOptDialog, setDefaultVolName)
{
    QString testVolName = "TestVolume";

    dialog->setDefaultVolName(testVolName);

    EXPECT_EQ(dialog->volnameEdit->text(), testVolName);
    EXPECT_EQ(dialog->lastVolName, testVolName);
    EXPECT_TRUE(dialog->volnameEdit->hasSelectedText());
}

TEST_F(UT_BurnOptDialog, setDefaultVolName_Empty)
{
    QString emptyVolName = "";

    dialog->setDefaultVolName(emptyVolName);

    EXPECT_TRUE(dialog->volnameEdit->text().isEmpty());
    EXPECT_TRUE(dialog->lastVolName.isEmpty());
}

TEST_F(UT_BurnOptDialog, setWriteSpeedInfo)
{
    QStringList speedInfo;
    speedInfo << "1\t1.0"
              << "2\t2.0"
              << "4\t4.0"
              << "8\t8.0";

    dialog->setWriteSpeedInfo(speedInfo);

    EXPECT_GT(dialog->writespeedComb->count(), 1);   // Should have more than just "Maximum"
    EXPECT_TRUE(dialog->speedMap.contains("1.0x"));
    EXPECT_TRUE(dialog->speedMap.contains("2.0x"));
    EXPECT_TRUE(dialog->speedMap.contains("4.0x"));
    EXPECT_TRUE(dialog->speedMap.contains("8.0x"));
    EXPECT_EQ(dialog->speedMap["1.0x"], 1);
    EXPECT_EQ(dialog->speedMap["4.0x"], 4);
}

TEST_F(UT_BurnOptDialog, setWriteSpeedInfo_EmptyList)
{
    QStringList emptySpeedInfo;

    dialog->setWriteSpeedInfo(emptySpeedInfo);

    EXPECT_EQ(dialog->writespeedComb->count(), 1);   // Should only have "Maximum"
}

TEST_F(UT_BurnOptDialog, currentBurnOptions_DefaultOptions)
{
    DFMBURN::BurnOptions opts = dialog->currentBurnOptions();

    // Default options should include KeepAppendable and EjectDisc
    EXPECT_TRUE(opts & DFMBURN::BurnOption::kKeepAppendable);
    EXPECT_TRUE(opts & DFMBURN::BurnOption::kEjectDisc);
    EXPECT_TRUE(opts & DFMBURN::BurnOption::kJolietSupport);   // Default filesystem
}

TEST_F(UT_BurnOptDialog, currentBurnOptions_VerifyData)
{
    dialog->checkdiscCheckbox->setChecked(true);

    DFMBURN::BurnOptions opts = dialog->currentBurnOptions();

    EXPECT_TRUE(opts & DFMBURN::BurnOption::kVerifyDatas);
}

TEST_F(UT_BurnOptDialog, currentBurnOptions_NoEject)
{
    dialog->ejectCheckbox->setChecked(false);

    DFMBURN::BurnOptions opts = dialog->currentBurnOptions();

    EXPECT_FALSE(opts & DFMBURN::BurnOption::kEjectDisc);
}

TEST_F(UT_BurnOptDialog, currentBurnOptions_NoAppendable)
{
    dialog->finalizeDiscCheckbox->setChecked(false);

    DFMBURN::BurnOptions opts = dialog->currentBurnOptions();

    EXPECT_FALSE(opts & DFMBURN::BurnOption::kKeepAppendable);
}

TEST_F(UT_BurnOptDialog, currentBurnOptions_ISO9660Only)
{
    dialog->fsComb->setCurrentIndex(0);

    DFMBURN::BurnOptions opts = dialog->currentBurnOptions();

    EXPECT_TRUE(opts & DFMBURN::BurnOption::kISO9660Only);
}

TEST_F(UT_BurnOptDialog, currentBurnOptions_RockRidge)
{
    dialog->fsComb->setCurrentIndex(2);

    DFMBURN::BurnOptions opts = dialog->currentBurnOptions();

    EXPECT_TRUE(opts & DFMBURN::BurnOption::kRockRidgeSupport);
}

TEST_F(UT_BurnOptDialog, currentBurnOptions_UDF)
{
    dialog->fsComb->setCurrentIndex(3);

    DFMBURN::BurnOptions opts = dialog->currentBurnOptions();

    EXPECT_TRUE(opts & DFMBURN::BurnOption::kUDF102Supported);
}

TEST_F(UT_BurnOptDialog, startDataBurn_ISOFiles)
{
    bool startBurnISOFilesCalled = false;

    stub.set_lamda(ADDR(BurnJobManager, startBurnISOFiles), [&startBurnISOFilesCalled] {
        __DBG_STUB_INVOKE__
        startBurnISOFilesCalled = true;
    });

    using LocalStagingFileFunc = QUrl (*)(QString);
    stub.set_lamda(static_cast<LocalStagingFileFunc>(&BurnHelper::localStagingFile), [] {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/staging");
    });

    dialog->volnameEdit->setText("TestVolume");
    dialog->fsComb->setCurrentIndex(1);   // Joliet

    dialog->startDataBurn();

    EXPECT_TRUE(startBurnISOFilesCalled);
}

TEST_F(UT_BurnOptDialog, startDataBurn_UDFFiles)
{
    bool startBurnUDFFilesCalled = false;

    stub.set_lamda(ADDR(BurnJobManager, startBurnUDFFiles), [&startBurnUDFFilesCalled] {
        __DBG_STUB_INVOKE__
        startBurnUDFFilesCalled = true;
    });

    using LocalStagingFileFunc = QUrl (*)(QString);
    stub.set_lamda(static_cast<LocalStagingFileFunc>(&BurnHelper::localStagingFile), [] {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/staging");
    });

    dialog->volnameEdit->setText("TestVolume");
    dialog->fsComb->setCurrentIndex(3);   // UDF

    dialog->startDataBurn();

    EXPECT_TRUE(startBurnUDFFilesCalled);
}

TEST_F(UT_BurnOptDialog, startDataBurn_EmptyVolumeName)
{
    bool startBurnISOFilesCalled = false;

    stub.set_lamda(ADDR(BurnJobManager, startBurnISOFiles), [&startBurnISOFilesCalled] {
        __DBG_STUB_INVOKE__
        startBurnISOFilesCalled = true;
    });

    using LocalStagingFileFunc = QUrl (*)(QString);
    stub.set_lamda(static_cast<LocalStagingFileFunc>(&BurnHelper::localStagingFile), [] {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/staging");
    });

    stub.set_lamda(ADDR(Settings, sync), [](Settings *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    dialog->volnameEdit->clear();
    dialog->lastVolName = "DefaultVolume";

    dialog->startDataBurn();

    EXPECT_TRUE(startBurnISOFilesCalled);
}

TEST_F(UT_BurnOptDialog, startImageBurn)
{
    bool startBurnISOImageCalled = false;

    stub.set_lamda(ADDR(BurnJobManager, startBurnISOImage), [&startBurnISOImageCalled] {
        __DBG_STUB_INVOKE__
        startBurnISOImageCalled = true;
    });

    QTemporaryFile tempFile;
    ASSERT_TRUE(tempFile.open());
    dialog->imageFile = QUrl::fromLocalFile(tempFile.fileName());

    dialog->startImageBurn();

    EXPECT_TRUE(startBurnISOImageCalled);
}

TEST_F(UT_BurnOptDialog, onIndexChanged_UDFSelected)
{
    dialog->onIndexChanged(3);   // UDF index

    EXPECT_FALSE(dialog->checkdiscCheckbox->isChecked());
    EXPECT_FALSE(dialog->checkdiscCheckbox->isEnabled());
    EXPECT_TRUE(dialog->finalizeDiscCheckbox->isChecked());
    EXPECT_FALSE(dialog->finalizeDiscCheckbox->isEnabled());
    EXPECT_EQ(dialog->writespeedComb->currentIndex(), 0);
    EXPECT_FALSE(dialog->writespeedComb->isEnabled());
}

TEST_F(UT_BurnOptDialog, onIndexChanged_NonUDFSelected)
{
    // First set UDF to disable controls
    dialog->onIndexChanged(3);

    // Then select non-UDF to re-enable controls
    dialog->onIndexChanged(1);   // Joliet

    EXPECT_TRUE(dialog->checkdiscCheckbox->isEnabled());
    EXPECT_TRUE(dialog->finalizeDiscCheckbox->isEnabled());
    EXPECT_TRUE(dialog->writespeedComb->isEnabled());
}

TEST_F(UT_BurnOptDialog, onButnBtnClicked_Burn_DeviceExists)
{
    bool startDataBurnCalled = false;

    stub.set_lamda(ADDR(BurnOptDialog, startDataBurn), [&startDataBurnCalled] {
        __DBG_STUB_INVOKE__
        startDataBurnCalled = true;
    });

    // Mock QFile::exists to return true (static version)
    using QFileExistsFunc = bool (*)(const QString &);
    stub.set_lamda(static_cast<QFileExistsFunc>(&QFile::exists), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    dialog->imageFile = QUrl();   // Empty image file means data burn

    dialog->onButnBtnClicked(1, "Burn");

    EXPECT_TRUE(startDataBurnCalled);
}

TEST_F(UT_BurnOptDialog, onButnBtnClicked_Burn_ImageBurn)
{
    bool startImageBurnCalled = false;

    stub.set_lamda(ADDR(BurnOptDialog, startImageBurn), [&startImageBurnCalled] {
        __DBG_STUB_INVOKE__
        startImageBurnCalled = true;
    });

    using QFileExistsFunc = bool (*)(const QString &);
    stub.set_lamda(static_cast<QFileExistsFunc>(&QFile::exists), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    QTemporaryFile tempFile;
    ASSERT_TRUE(tempFile.open());
    dialog->imageFile = QUrl::fromLocalFile(tempFile.fileName());

    dialog->onButnBtnClicked(1, "Burn");

    EXPECT_TRUE(startImageBurnCalled);
}

TEST_F(UT_BurnOptDialog, onButnBtnClicked_Burn_DeviceNotExists)
{
    bool errorDialogShown = false;

    stub.set_lamda(ADDR(DialogManager, showErrorDialog), [&errorDialogShown] {
        __DBG_STUB_INVOKE__
        errorDialogShown = true;
    });

    using QFileExistsFunc = bool (*)(const QString &);
    stub.set_lamda(static_cast<QFileExistsFunc>(&QFile::exists), [] {
        __DBG_STUB_INVOKE__
        return false;   // Device doesn't exist
    });

    dialog->onButnBtnClicked(1, "Burn");

    EXPECT_TRUE(errorDialogShown);
}

TEST_F(UT_BurnOptDialog, volnameEdit_TextChanged_ValidLength)
{
    stub.set_lamda(ADDR(Settings, sync), [](Settings *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    QString validText = "ValidVolumeName";

    dialog->volnameEdit->setText(validText);

    EXPECT_EQ(dialog->volnameEdit->text(), validText);
}

TEST_F(UT_BurnOptDialog, volnameEdit_TextChanged_TooLong)
{
    // Create a string longer than 30 characters
    QString longText = "This_is_a_very_long_volume_name_that_exceeds_the_limit";

    dialog->volnameEdit->setText(longText);

    // The text should be truncated to fit within the limit
    EXPECT_LE(dialog->volnameEdit->text().toUtf8().length(), 30);
}

TEST_F(UT_BurnOptDialog, advanceBtn_Toggle)
{
    // Initially advanced settings should be hidden
    EXPECT_TRUE(dialog->advancedSettings->isHidden());

    // Simulate clicking the advance button
    emit dialog->advanceBtn->clicked();

    // Advanced settings should now be visible
    EXPECT_FALSE(dialog->advancedSettings->isHidden());

    // Click again to hide
    emit dialog->advanceBtn->clicked();

    // Should be hidden again
    EXPECT_TRUE(dialog->advancedSettings->isHidden());
}

TEST_F(UT_BurnOptDialog, UI_Components_Existence)
{
    // Test that all UI components are properly created
    EXPECT_TRUE(dialog->volnameLabel != nullptr);
    EXPECT_TRUE(dialog->volnameEdit != nullptr);
    EXPECT_TRUE(dialog->advanceBtn != nullptr);
    EXPECT_TRUE(dialog->advancedSettings != nullptr);
    EXPECT_TRUE(dialog->fsLabel != nullptr);
    EXPECT_TRUE(dialog->fsComb != nullptr);
    EXPECT_TRUE(dialog->writespeedLabel != nullptr);
    EXPECT_TRUE(dialog->writespeedComb != nullptr);
    EXPECT_TRUE(dialog->finalizeDiscCheckbox != nullptr);
    EXPECT_TRUE(dialog->checkdiscCheckbox != nullptr);
    EXPECT_TRUE(dialog->ejectCheckbox != nullptr);
}

TEST_F(UT_BurnOptDialog, UI_DefaultValues)
{
    // Test default values
    EXPECT_EQ(dialog->fsComb->currentIndex(), 1);   // Default to Joliet
    EXPECT_TRUE(dialog->finalizeDiscCheckbox->isChecked());
    EXPECT_TRUE(dialog->ejectCheckbox->isChecked());
    EXPECT_FALSE(dialog->checkdiscCheckbox->isChecked());
    EXPECT_EQ(dialog->writespeedComb->itemText(0), QObject::tr("Maximum"));
}

TEST_F(UT_BurnOptDialog, speedMap_DefaultMaximum)
{
    EXPECT_TRUE(dialog->speedMap.contains(QObject::tr("Maximum")));
    EXPECT_EQ(dialog->speedMap[QObject::tr("Maximum")], 0);
}
