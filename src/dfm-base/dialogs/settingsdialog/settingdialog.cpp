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
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dtkcore_global.h>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <dsettingsbackend.h>
#include <DPushButton>

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
    // TODO(xust): move to server plugin.
    widgetFactory()->registerWidget("mountCheckBox", &SettingDialog::createAutoMountCheckBox);
    widgetFactory()->registerWidget("openCheckBox", &SettingDialog::createAutoMountOpenCheckBox);

    widgetFactory()->registerWidget("checkBoxWithMessage", &SettingDialog::createCheckBoxWithMessage);
    widgetFactory()->registerWidget("pushButton", &SettingDialog::createPushButton);

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

QPair<QWidget *, QWidget *> SettingDialog::createCheckBoxWithMessage(QObject *opt)
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

    return qMakePair(cb, nullptr);
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
