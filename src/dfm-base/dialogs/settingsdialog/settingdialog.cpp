// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingdialog.h"
#include "controls/checkboxwithmessage.h"

#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/settingdialog/customsettingitemregister.h>
#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/dialogs/settingsdialog/controls/aliascombobox.h>

#include <dtkcore_global.h>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <dsettingsbackend.h>
#include <DPushButton>
#include <DSlider>
#include <DLabel>

#include <QFileDialog>
#include <QToolTip>
#include <QWindow>
#include <QFile>
#include <QFrame>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QDebug>
#include <QLabel>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QDBusConnectionInterface>

using namespace dfmbase;

namespace {
inline constexpr char kGroupsName[] { "groups" };
inline constexpr char kItemKey[] { "key" };
inline constexpr char kOptionsName[] { "options" };
}

int indexOfChar(const QByteArray &data, char ch, int from)
{
    for (; from < data.size(); ++from) {
        if (data.at(from) == '\\') {
            continue;
        }

        if (data.at(from) == ch) {
            return from;
        }
    }

    return from;
}

void SettingDialog::settingFilter(QByteArray &data)
{
    QJsonParseError err;
    QJsonDocument doc { QJsonDocument::fromJson(data, &err) };
    if (err.error != QJsonParseError::NoError) {
        qCWarning(logDFMBase) << "config template json is not valid!" << err.errorString();
        return;
    }

    if (!doc.object().contains(kGroupsName)) {
        qCWarning(logDFMBase) << "config template is not valid, no group item";
        return;
    }

    bool updated = false;

    auto &&primaryGroups = doc.object().value(kGroupsName).toArray();
    for (int i = 0; i < primaryGroups.count(); ++i) {
        auto &&primaryGroup = primaryGroups.at(i).toObject();
        if (primaryGroup.isEmpty())
            continue;

        QString &&priGroupKey = primaryGroup.value(kItemKey).toString();

        auto &&secondaryGroups = primaryGroup.value(kGroupsName).toArray();
        for (int j = 0; j < secondaryGroups.count(); ++j) {
            auto &&secondaryGroup = secondaryGroups.at(j).toObject();
            if (secondaryGroup.isEmpty())
                continue;

            QString &&secGroupKey = secondaryGroup.value(kItemKey).toString();

            auto &&options = secondaryGroup.value(kOptionsName).toArray();
            for (int k = 0; k < options.count(); ++k) {
                auto option = options.at(k).toObject();
                if (option.isEmpty())
                    continue;

                QString &&optKey = option.value(kItemKey).toString();
                QString key = QString("%1.%2.%3").arg(priGroupKey).arg(secGroupKey).arg(optKey);
                if (SettingDialog::needHide(key)) {
                    option.insert("hide", true);
                    options.replace(k, option);
                    updated = true;
                }
            }

            if (updated) {
                secondaryGroup.insert(kOptionsName, options);
                secondaryGroups.replace(j, secondaryGroup);
            }
        }

        if (updated) {
            primaryGroup.insert(kGroupsName, secondaryGroups);
            primaryGroups.replace(i, primaryGroup);
        }
    }

    if (updated) {
        QJsonObject obj;
        obj.insert(kGroupsName, primaryGroups);
        QJsonDocument newDoc;
        newDoc.setObject(obj);
        data = newDoc.toJson();
    }
}

void SettingDialog::loadSettings(const QString & /*templateFile*/)
{
    QByteArray configJson = SettingJsonGenerator::instance()->genSettingJson();

    settingFilter(configJson);
    dtkSettings = DSettings::fromJson(configJson);
}

QPointer<QCheckBox> SettingDialog::kAutoMountCheckBox = nullptr;
QPointer<QCheckBox> SettingDialog::kAutoMountOpenCheckBox = nullptr;
QSet<QString> SettingDialog::kHiddenSettingItems {};
quint64 SettingDialog::parentWid { 0 };

SettingDialog::SettingDialog(QWidget *parent)
    : DSettingsDialog(parent)
{
    parentWid = FMWindowsIns.findWindowId(parent);
}

void SettingDialog::initialze()
{
    // TODO(xust): move to server plugin.
    widgetFactory()->registerWidget("mountCheckBox", &SettingDialog::createAutoMountCheckBox);
    widgetFactory()->registerWidget("openCheckBox", &SettingDialog::createAutoMountOpenCheckBox);

    widgetFactory()->registerWidget("checkBoxWithMessage", &SettingDialog::createCheckBoxWithMessage);
    widgetFactory()->registerWidget("pushButton", &SettingDialog::createPushButton);
    widgetFactory()->registerWidget("sliderWithSideIcon", &SettingDialog::createSliderWithSideIcon);
    widgetFactory()->registerWidget("pathcombobox", &SettingDialog::createPathComboboxItem);

    auto creators = CustomSettingItemRegister::instance()->getCreators();
    auto iter = creators.cbegin();
    while (iter != creators.cend()) {
        widgetFactory()->registerWidget(iter.key(), iter.value());
        iter++;
    }

    if (WindowUtils::isWayLand()) {
        setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        setAttribute(Qt::WA_NativeWindow);
        windowHandle()->setProperty("_d_dwayland_minimizable", false);
        windowHandle()->setProperty("_d_dwayland_maximizable", false);
        windowHandle()->setProperty("_d_dwayland_resizable", false);
        setFixedSize(QSize(700, 700));
    }

    // TODO(xust): no need these files anymore.
    QString settingTemplate =
#ifdef DISABLE_COMPRESS_PREIVEW
            ":/configure/global-setting-template-pro.js";
#else
#    ifdef DISABLE_FFMEPG
            ":/configure/global-setting-template-fedora.js";
#    else
            ":/configure/global-setting-template.js";
#    endif
#endif

    loadSettings(settingTemplate);

    // load conf value
    if (dtkSettings) {
        dtkSettings->setParent(this);
        SettingBackend::instance()->setToSettings(dtkSettings);
        updateSettings(dtkSettings);
    }

    setIcon(QIcon::fromTheme("dde-file-manager"));
}

void SettingDialog::setItemVisiable(const QString &key, bool visiable)
{
    if (visiable)
        kHiddenSettingItems.remove(key);
    else
        kHiddenSettingItems.insert(key);
}

bool SettingDialog::needHide(const QString &key)
{
    return kHiddenSettingItems.contains(key);
}

QPair<QWidget *, QWidget *> SettingDialog::createAutoMountCheckBox(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
    QCheckBox *mountCheckBox = new QCheckBox(QObject::tr("Auto mount"));
    SettingDialog::kAutoMountCheckBox = mountCheckBox;

    if (option->value().toBool()) {
        mountCheckBox->setChecked(true);
    } else if (kAutoMountOpenCheckBox) {
        kAutoMountOpenCheckBox->setDisabled(true);
    }

    QObject::connect(mountCheckBox,
                     &QCheckBox::stateChanged,
                     option,
                     [=](int state) {
                         mountCheckBoxStateChangedHandle(option, state);
                     });

    QObject::connect(option, &DSettingsOption::valueChanged, mountCheckBox, [=](QVariant value) {
        mountCheckBox->setChecked(value.toBool());
    });

    return qMakePair(mountCheckBox, nullptr);
}

QPair<QWidget *, QWidget *> SettingDialog::createAutoMountOpenCheckBox(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
    QCheckBox *openCheckBox = new QCheckBox(QObject::tr("Open after auto mount"));
    SettingDialog::kAutoMountOpenCheckBox = openCheckBox;

    if (option->value().toBool()) {
        openCheckBox->setChecked(true);
        openCheckBox->setDisabled(false);
    } else {
        if (kAutoMountCheckBox && !kAutoMountCheckBox->isChecked()) {
            openCheckBox->setDisabled(true);
        }
    }

    QObject::connect(openCheckBox,
                     &QCheckBox::stateChanged,
                     option,
                     [=](int state) {
                         autoMountCheckBoxChangedHandle(option, state);
                     });

    QObject::connect(option, &DSettingsOption::valueChanged, openCheckBox, [=](QVariant value) {
        openCheckBox->setChecked(value.toBool());
    });

    return qMakePair(openCheckBox, nullptr);
}

QWidget *SettingDialog::createCheckBoxWithMessage(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
    const QString &text = option->data("text").toString();
    const QString &message = option->data("message").toString();

    CheckBoxWithMessage *cb = new CheckBoxWithMessage;
    cb->setDisplayText(qApp->translate("QObject", text.toStdString().c_str()),
                       qApp->translate("QObject", message.toStdString().c_str()));

    cb->setChecked(option->value().toBool());

    QObject::connect(cb, &CheckBoxWithMessage::stateChanged, option, [=](int state) {
        if (state == 0)
            option->setValue(false);
        else if (state == 2)
            option->setValue(true);
    });

    QObject::connect(option, &DSettingsOption::valueChanged, cb, [=](QVariant value) { cb->setChecked(value.toBool()); });

    return cb;
}

QPair<QWidget *, QWidget *> SettingDialog::createPushButton(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
    const QString &desc = option->data("desc").toString();
    const QString &text = option->data("text").toString();
    auto attributeType = option->data("trigger").toInt();

    auto rightWidget = new QWidget;
    rightWidget->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *layout = new QHBoxLayout(rightWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    rightWidget->setLayout(layout);

    layout->addStretch(0);

    DPushButton *button = new DPushButton;
    button->setText(text);

    layout->addWidget(button, 0, Qt::AlignRight);

    connect(button, &DPushButton::clicked, option, [=] {
        Application::appAttributeTrigger(static_cast<Application::TriggerAttribute>(attributeType), parentWid);
    });

    return qMakePair(new QLabel(desc), rightWidget);
}

QPair<QWidget *, QWidget *> SettingDialog::createSliderWithSideIcon(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);

    const QString &text = option->name();
    DLabel *label = new DLabel(text);

    DSlider *slider = new DSlider;
    slider->setObjectName("OptionQSlider");
    slider->setAccessibleName("OptionQSlider");
    slider->slider()->setOrientation(Qt::Horizontal);
    slider->setMaximum(option->data("max").toInt());
    slider->setMinimum(option->data("min").toInt());
    const QString &leftIcon = option->data("left-icon").toString();
    const QString &rightIcon = option->data("right-icon").toString();
    if (!leftIcon.isEmpty()) {
        slider->setLeftIcon(QIcon::fromTheme(leftIcon));
    }
    if (!rightIcon.isEmpty()) {
        slider->setRightIcon(QIcon::fromTheme(rightIcon));
    }
    slider->setValue(option->value().toInt());
    slider->setIconSize(QSize(20, 20));

    QVariantList valList = option->data("values").toList();
    if (!valList.isEmpty()) {
        QObject::connect(slider, &DSlider::sliderMoved, slider, [=](int position) {
            if (position >= valList.count())
                return;
            int stepLength = (slider->slider()->width() - 28) / option->data("max").toInt();
            QPoint pos = slider->slider()->mapToGlobal(QPoint(4 + position * stepLength, -48));
            QToolTip::showText(pos, valList.at(position).toString(), slider);
        });
        QObject::connect(slider, &DSlider::sliderPressed, slider, [=] {
            int position = slider->slider()->sliderPosition();
            if (position >= valList.count())
                return;
            int stepLength = (slider->slider()->width() - 28) / option->data("max").toInt();
            QPoint pos = slider->slider()->mapToGlobal(QPoint(4 + position * stepLength, -48));
            QToolTip::showText(pos, valList.at(position).toString(), slider);
        });
    }

    option->connect(slider, &DSlider::valueChanged,
                    option, [=](int value) {
                        slider->blockSignals(true);
                        option->setValue(value);
                        slider->blockSignals(false);
                    });
    option->connect(option, &DTK_CORE_NAMESPACE::DSettingsOption::valueChanged,
                    slider, [=](const QVariant &value) {
                        slider->setValue(value.toInt());
                        slider->update();
                    });

    return qMakePair(label, slider);
}

QPair<QWidget *, QWidget *> SettingDialog::createPathComboboxItem(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
    const auto &name = option->name();
    const auto &itemMap = option->data("items").toMap();
    const auto &keyList = itemMap.value("keys").toStringList();
    const auto &valueList = itemMap.value("values").toStringList();
    Q_ASSERT(keyList.size() == valueList.size());

    AliasComboBox *combobox = new AliasComboBox;
    for (int i = 0; i < valueList.size(); ++i) {
        combobox->addItem(valueList[i], keyList[i]);
    }
    combobox->addItem(QObject::tr("Specify directory"));

    auto defItem = option->defaultValue().toString();
    if (option->value().isValid())
        defItem = option->value().toString();

    static int lastIndex = 0;
    lastIndex = keyList.indexOf(defItem);
    if (lastIndex == -1) {
        lastIndex = combobox->count() - 1;
        combobox->setItemAlias(lastIndex, QObject::tr("Specify directory %1").arg(QUrl(defItem).path()));
    }
    combobox->setCurrentIndex(lastIndex);

    combobox->connect(combobox, &AliasComboBox::activated, option,
                      [=](int index) {
                          if (lastIndex == index && index != combobox->count() - 1)
                              return;

                          if (pathComboBoxChangedHandle(combobox, option, index))
                              lastIndex = index;
                          else
                              combobox->setCurrentIndex(lastIndex);
                      });
    option->connect(option, &DTK_CORE_NAMESPACE::DSettingsOption::valueChanged,
                    combobox, [=](const QVariant &value) {
                        const auto &url = value.toString();
                        int index = combobox->findData(url);
                        if (index == -1) {
                            index = combobox->count() - 1;
                            combobox->setItemData(index, url);
                            combobox->setItemAlias(index, QObject::tr("Specify directory %1").arg(QUrl(url).path()));
                        }
                        lastIndex = index;
                        QSignalBlocker blk(combobox);
                        combobox->setCurrentIndex(index);
                    });

    return qMakePair(new QLabel(name), combobox);
}

void SettingDialog::mountCheckBoxStateChangedHandle(DSettingsOption *option, int state)
{
    if (state == 0) {
        if (SettingDialog::kAutoMountOpenCheckBox) {
            SettingDialog::kAutoMountOpenCheckBox->setDisabled(true);
            SettingDialog::kAutoMountOpenCheckBox->setChecked(false);
        }
        option->setValue(false);
    } else if (state == 2) {
        if (SettingDialog::kAutoMountOpenCheckBox) {
            SettingDialog::kAutoMountOpenCheckBox->setDisabled(false);
        }

        option->setValue(true);
    }
}

void SettingDialog::autoMountCheckBoxChangedHandle(DSettingsOption *option, int state)
{
    if (state == 0) {
        option->setValue(false);
    } else if (state == 2) {
        option->setValue(true);
    }
}

bool SettingDialog::pathComboBoxChangedHandle(AliasComboBox *acb, DSettingsOption *option, int index)
{
    QSignalBlocker blk(acb);
    if (index == acb->count() - 1) {
        const auto &url = QFileDialog::getExistingDirectoryUrl(acb);
        if (!url.isValid())
            return false;

        if (!ProtocolUtils::isLocalFile(url)) {
            DialogManagerInstance->showErrorDialog(QObject::tr("Invalid Directory"),
                                                   QObject::tr("This directory does not support pinning"));
            return false;
        }
        acb->setItemData(index, url.toString());
        acb->setItemAlias(index, QObject::tr("Specify directory %1").arg(url.path()));
    }

    option->setValue(acb->itemData(index).toString());
    return true;
}
