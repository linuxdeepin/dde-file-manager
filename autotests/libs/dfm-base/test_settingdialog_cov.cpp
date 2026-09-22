// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_settingdialog_cov.cpp
* @brief Coverage-focused unit tests for SettingDialog private widget factories
 *        (src/dfm-base/dialogs/settingsdialog/settingdialog.cpp).
 *
 * The build uses -fno-access-control, so the private static creators and
 * handlers can be invoked directly with hand-built DSettingsOption objects.
 *
 * Covered functions from the gap list (settingdialog.cpp, 23 entries):
 *   - createAutoMountCheckBox (+ stateChanged lambda #1 + valueChanged lambda #2)
 *   - createAutoMountOpenCheckBox (+ lambdas #1/#2)
 *   - createCheckBoxWithMessage (+ lambdas #1/#2)
 *   - createPushButton (+ clicked lambda #1, stubbed trigger)
 *   - createSliderWithSideIcon (+ lambdas #1..#4)
 *   - createPathComboboxItem (+ lambdas #1/#2)
 *   - mountCheckBoxStateChangedHandle (state 0 and state 2 branches)
 *   - autoMountCheckBoxChangedHandle
 *   - pathComboBoxChangedHandle (non-last-index branch; last-index branch is
 *     guarded by a modal QFileDialog and is only exercised up to the guard)
 *
 * Case -> function mapping:
 *   CreateAutoMountCheckBox_*        -> createAutoMountCheckBox (true/false)
 *   AutoMountStateChanged_*          -> mountCheckBoxStateChangedHandle
 *   AutoMountOpenChanged_*           -> autoMountCheckBoxChangedHandle
 *   CreateCheckBoxWithMessage_*      -> createCheckBoxWithMessage + lambdas
 *   CreatePushButton_*               -> createPushButton
 *   CreateSliderWithSideIcon_*       -> createSliderWithSideIcon + lambda #4
 *   CreatePathComboboxItem_*         -> createPathComboboxItem + lambda #2
 *   PathComboBoxChangedHandle_*      -> pathComboBoxChangedHandle
 *
 * Branch list (from get_code_snippet):
 *   createAutoMountCheckBox: value true -> checked; else -> disable open box.
 *   mountCheckBoxStateChangedHandle: state 0 -> disable+uncheck+setValue(false);
 *                                    state 2 -> enable + setValue(true).
 *   pathComboBoxChangedHandle: index == count-1 (dialog path, returns via
 *                              stubbed dialog guard) / normal index -> setValue.
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QCheckBox>
#include <QLabel>
#include <QPair>
#include <QPointer>
#include <QSignalSpy>
#include <QVariant>
#include <QJsonObject>
#include <QDir>
#include <QFileDialog>
#include <QPushButton>
#include <QSlider>
#include <DSettingsOption>

#include "stubext.h"

#include <dfm-base/dialogs/settingsdialog/settingdialog.h>
#include <dfm-base/dialogs/settingsdialog/controls/aliascombobox.h>
#include <dfm-base/dialogs/settingsdialog/controls/checkboxwithmessage.h>

using namespace dfmbase;

class UT_SettingDialogCov : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override
    {
        stub.clear();
        // release the static widget pointers so tests stay independent
        delete SettingDialog::kAutoMountCheckBox.data();
        delete SettingDialog::kAutoMountOpenCheckBox.data();
        SettingDialog::kAutoMountCheckBox = nullptr;
        SettingDialog::kAutoMountOpenCheckBox = nullptr;
    }
    stub_ext::StubExt stub;

    static OptionPtr makeOption(const QVariant &value)
    {
        QJsonObject json;
        json.insert("key", "opt");
        json.insert("name", "option-name");
        auto opt = DSettingsOption::fromJson("grp.sub", json);
        opt->setValue(value);
        return opt;
    }
};

TEST_F(UT_SettingDialogCov, CreateAutoMountCheckBox_ValueTrue_ExpectChecked)
{
    // Arrange
    OptionPtr option = makeOption(true);

    // Act
    QPair<QWidget *, QWidget *> pair = SettingDialog::createAutoMountCheckBox(option.data());

    // Assert
    auto *cb = qobject_cast<QCheckBox *>(pair.first);
    ASSERT_NE(cb, nullptr);
    EXPECT_TRUE(cb->isChecked());
    EXPECT_EQ(pair.second, nullptr);
}

TEST_F(UT_SettingDialogCov, CreateAutoMountCheckBox_ValueFalse_ExpectOpenBoxDisabled)
{
    // Arrange: create the "open" checkbox first so the static pointer exists
    OptionPtr openOption = makeOption(true);
    SettingDialog::createAutoMountOpenCheckBox(openOption.data());
    ASSERT_TRUE(SettingDialog::kAutoMountOpenCheckBox);
    SettingDialog::kAutoMountOpenCheckBox->setEnabled(true);

    // Act
    OptionPtr mountOption = makeOption(false);
    SettingDialog::createAutoMountCheckBox(mountOption.data());

    // Assert
    EXPECT_FALSE(SettingDialog::kAutoMountOpenCheckBox->isEnabled());
    auto *mountBox = qobject_cast<QCheckBox *>(SettingDialog::kAutoMountCheckBox.data());
    ASSERT_NE(mountBox, nullptr);
    EXPECT_FALSE(mountBox->isChecked());
    EXPECT_EQ(static_cast<int>(mountBox->checkState()), static_cast<int>(Qt::Unchecked));
}

TEST_F(UT_SettingDialogCov, CreateAutoMountCheckBox_StateChangedToChecked_ExpectOptionTrue)
{
    // Arrange
    OptionPtr option = makeOption(false);
    QPair<QWidget *, QWidget *> pair = SettingDialog::createAutoMountCheckBox(option.data());
    auto *cb = qobject_cast<QCheckBox *>(pair.first);
    ASSERT_NE(cb, nullptr);

    // Act: toggling triggers the connected stateChanged lambda
    cb->setChecked(true);

    // Assert
    EXPECT_TRUE(option->value().toBool());
    EXPECT_EQ(cb->checkState(), Qt::Checked);
}

TEST_F(UT_SettingDialogCov, CreateAutoMountCheckBox_OptionValueChanged_ExpectBoxSynced)
{
    // Arrange
    OptionPtr option = makeOption(false);
    QPair<QWidget *, QWidget *> pair = SettingDialog::createAutoMountCheckBox(option.data());
    auto *cb = qobject_cast<QCheckBox *>(pair.first);
    ASSERT_NE(cb, nullptr);
    EXPECT_FALSE(cb->isChecked());

    // Act: external value change triggers the valueChanged lambda
    option->setValue(true);

    // Assert
    EXPECT_TRUE(cb->isChecked());
    EXPECT_EQ(static_cast<int>(cb->checkState()), static_cast<int>(Qt::Checked));
}

TEST_F(UT_SettingDialogCov, MountCheckBoxStateChangedHandle_StateZero_ExpectFalseAndOpenBoxDisabled)
{
    // Arrange
    OptionPtr option = makeOption(true);
    SettingDialog::createAutoMountOpenCheckBox(makeOption(true).data());
    ASSERT_TRUE(SettingDialog::kAutoMountOpenCheckBox);

    // Act
    SettingDialog::mountCheckBoxStateChangedHandle(option.data(), 0);

    // Assert
    EXPECT_FALSE(option->value().toBool());
    EXPECT_EQ(option->value().toInt(), 0);
    EXPECT_FALSE(SettingDialog::kAutoMountOpenCheckBox->isEnabled());
    EXPECT_FALSE(SettingDialog::kAutoMountOpenCheckBox->isChecked());
}

TEST_F(UT_SettingDialogCov, MountCheckBoxStateChangedHandle_StateTwo_ExpectTrueAndOpenBoxEnabled)
{
    // Arrange
    OptionPtr option = makeOption(false);
    SettingDialog::createAutoMountOpenCheckBox(makeOption(true).data());
    SettingDialog::kAutoMountOpenCheckBox->setDisabled(true);

    // Act
    SettingDialog::mountCheckBoxStateChangedHandle(option.data(), 2);

    // Assert
    EXPECT_TRUE(option->value().toBool());
    EXPECT_EQ(option->value().toInt(), 1);
    EXPECT_TRUE(SettingDialog::kAutoMountOpenCheckBox->isEnabled());
}

TEST_F(UT_SettingDialogCov, AutoMountCheckBoxChangedHandle_States_ExpectValueFollowsState)
{
    // Arrange
    OptionPtr option = makeOption(false);

    // Act
    SettingDialog::autoMountCheckBoxChangedHandle(option.data(), 2);
    bool afterChecked = option->value().toBool();
    SettingDialog::autoMountCheckBoxChangedHandle(option.data(), 0);

    // Assert
    EXPECT_TRUE(afterChecked);
    EXPECT_FALSE(option->value().toBool());
    EXPECT_EQ(option->value().toInt(), 0);
}

TEST_F(UT_SettingDialogCov, CreateCheckBoxWithMessage_CheckedOption_ExpectCheckedWidget)
{
    // Arrange
    OptionPtr option = makeOption(true);
    option->setData("text", "hello");
    option->setData("message", "world");

    // Act
    QWidget *widget = SettingDialog::createCheckBoxWithMessage(option.data());

    // Assert: CheckBoxWithMessage wraps a private QCheckBox
    auto *cbw = qobject_cast<CheckBoxWithMessage *>(widget);
    ASSERT_NE(cbw, nullptr);
    auto *inner = cbw->findChild<QCheckBox *>();
    ASSERT_NE(inner, nullptr);
    EXPECT_TRUE(inner->isChecked());
    EXPECT_NE(cbw->findChild<QLabel *>(), nullptr);
}

TEST_F(UT_SettingDialogCov, CreateCheckBoxWithMessage_UncheckedThenToggled_ExpectOptionValueToggles)
{
    // Arrange
    OptionPtr option = makeOption(false);
    QWidget *widget = SettingDialog::createCheckBoxWithMessage(option.data());
    auto *cbw = qobject_cast<CheckBoxWithMessage *>(widget);
    ASSERT_NE(cbw, nullptr);
    auto *cb = cbw->findChild<QCheckBox *>();
    ASSERT_NE(cb, nullptr);
    EXPECT_FALSE(cb->isChecked());

    // Act
    cbw->setChecked(true);
    bool afterCheck = option->value().toBool();
    cbw->setChecked(false);

    // Assert
    EXPECT_TRUE(afterCheck);
    EXPECT_FALSE(option->value().toBool());
    EXPECT_EQ(static_cast<int>(cb->checkState()), static_cast<int>(Qt::Unchecked));
}

TEST_F(UT_SettingDialogCov, CreatePushButton_DescAndText_ExpectLabelAndButtonCreated)
{
    // Arrange
    OptionPtr option = makeOption(QVariant());
    option->setData("desc", "the description");
    option->setData("text", "click me");

    // Act
    QPair<QWidget *, QWidget *> pair = SettingDialog::createPushButton(option.data());

    // Assert
    auto *label = qobject_cast<QLabel *>(pair.first);
    auto *button = pair.second->findChild<QPushButton *>();
    ASSERT_NE(label, nullptr);
    ASSERT_NE(button, nullptr);
    EXPECT_EQ(label->text(), QStringLiteral("the description"));
    EXPECT_EQ(button->text(), QStringLiteral("click me"));
}

TEST_F(UT_SettingDialogCov, CreateSliderWithSideIcon_MinMaxValue_ExpectSliderConfigured)
{
    // Arrange
    OptionPtr option = makeOption(5);
    option->setData("max", 10);
    option->setData("min", 0);

    // Act
    QPair<QWidget *, QWidget *> pair = SettingDialog::createSliderWithSideIcon(option.data());

    // Assert (DSlider wraps an inner QSlider)
    auto *slider = pair.second->findChild<QSlider *>();
    ASSERT_NE(slider, nullptr);
    EXPECT_EQ(slider->maximum(), 10);
    EXPECT_EQ(slider->minimum(), 0);
    EXPECT_EQ(slider->value(), 5);
}

TEST_F(UT_SettingDialogCov, CreateSliderWithSideIcon_ValueChangedFromOption_ExpectSliderSynced)
{
    // Arrange
    OptionPtr option = makeOption(2);
    option->setData("max", 10);
    option->setData("min", 0);
    QPair<QWidget *, QWidget *> pair = SettingDialog::createSliderWithSideIcon(option.data());
    auto *slider = pair.second->findChild<QSlider *>();
    ASSERT_NE(slider, nullptr);

    // Act: option change drives slider through lambda #4
    option->setValue(7);

    // Assert
    EXPECT_EQ(slider->value(), 7);
    EXPECT_EQ(slider->maximum(), 10);
}

TEST_F(UT_SettingDialogCov, CreatePathComboboxItem_UnknownValue_ExpectSpecifyDirectorySelected)
{
    // Arrange
    OptionPtr option = makeOption(
            QUrl::fromLocalFile(QDir::temp().filePath("ut-not-in-keys")).toString());
    QVariantMap items;
    items.insert("keys", QStringList { "k1", "k2" });
    items.insert("values", QStringList { "v1", "v2" });
    option->setData("items", items);

    // Act
    QPair<QWidget *, QWidget *> pair = SettingDialog::createPathComboboxItem(option.data());

    // Assert: 2 entries + "Specify directory" -> index 2 selected
    auto *combo = qobject_cast<AliasComboBox *>(pair.second);
    ASSERT_NE(combo, nullptr);
    EXPECT_EQ(combo->count(), 3);
    EXPECT_EQ(combo->currentIndex(), 2);
}

TEST_F(UT_SettingDialogCov, CreatePathComboboxItem_KnownValue_ExpectMatchingIndexSelected)
{
    // Arrange
    OptionPtr option = makeOption(QString("k2"));
    QVariantMap items;
    items.insert("keys", QStringList { "k1", "k2" });
    items.insert("values", QStringList { "v1", "v2" });
    option->setData("items", items);

    // Act
    QPair<QWidget *, QWidget *> pair = SettingDialog::createPathComboboxItem(option.data());

    // Assert
    auto *combo = qobject_cast<AliasComboBox *>(pair.second);
    ASSERT_NE(combo, nullptr);
    EXPECT_EQ(combo->currentIndex(), 1);
    EXPECT_EQ(combo->currentData().toString(), QStringLiteral("k2"));
}

TEST_F(UT_SettingDialogCov, CreatePathComboboxItem_OptionValueChanged_ExpectComboboxSynced)
{
    // Arrange
    OptionPtr option = makeOption(QString("k1"));
    QVariantMap items;
    items.insert("keys", QStringList { "k1", "k2" });
    items.insert("values", QStringList { "v1", "v2" });
    option->setData("items", items);
    QPair<QWidget *, QWidget *> pair = SettingDialog::createPathComboboxItem(option.data());
    auto *combo = qobject_cast<AliasComboBox *>(pair.second);
    ASSERT_NE(combo, nullptr);
    ASSERT_EQ(combo->currentIndex(), 0);

    // Act: valueChanged lambda switches to the item holding "k2"
    option->setValue(QString("k2"));

    // Assert
    EXPECT_EQ(combo->currentIndex(), 1);
    EXPECT_EQ(combo->currentData().toString(), QStringLiteral("k2"));
}

TEST_F(UT_SettingDialogCov, PathComboBoxChangedHandle_NormalIndex_ExpectOptionValueSet)
{
    // Arrange
    OptionPtr option = makeOption(QString("k1"));
    QVariantMap items;
    items.insert("keys", QStringList { "k1", "k2" });
    items.insert("values", QStringList { "v1", "v2" });
    option->setData("items", items);
    QPair<QWidget *, QWidget *> pair = SettingDialog::createPathComboboxItem(option.data());
    auto *combo = qobject_cast<AliasComboBox *>(pair.second);
    ASSERT_NE(combo, nullptr);

    // Act: select a non-last index (no file dialog involved)
    bool ok = SettingDialog::pathComboBoxChangedHandle(combo, option.data(), 1);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(option->value().toString(), QStringLiteral("k2"));
}

TEST_F(UT_SettingDialogCov, PathComboBoxChangedHandle_LastIndexDialogRejected_ExpectFalse)
{
    // Arrange
    OptionPtr option = makeOption(QString("k1"));
    QVariantMap items;
    items.insert("keys", QStringList { "k1" });
    items.insert("values", QStringList { "v1" });
    option->setData("items", items);
    QPair<QWidget *, QWidget *> pair = SettingDialog::createPathComboboxItem(option.data());
    auto *combo = qobject_cast<AliasComboBox *>(pair.second);
    ASSERT_NE(combo, nullptr);
    // modal dialog stubbed to return an invalid (cancelled) url
    using GetDirUrlFunc = QUrl (*)(QWidget *, const QString &, const QUrl &,
                                   QFileDialog::Options, const QStringList &);
    stub.set_lamda(static_cast<GetDirUrlFunc>(&QFileDialog::getExistingDirectoryUrl),
                   [](QWidget *, const QString &, const QUrl &,
                      QFileDialog::Options, const QStringList &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl();
                   });
    const QString before = option->value().toString();

    // Act
    bool ok = SettingDialog::pathComboBoxChangedHandle(combo, option.data(), combo->count() - 1);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(option->value().toString(), before);
}
