// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingdialog.h"
#include "controls/checkboxwithmessage.h"

#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/windowutils.h>

#include <dtkcore_global.h>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <dsettingsbackend.h>

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

static QByteArray removeQuickSearchIndex(const QByteArray &data)
{
    auto const &jdoc = QJsonDocument::fromJson(data);
    QJsonObject RootObject = jdoc.object();
    QJsonValueRef ArrayRef = RootObject.find(kGroupsName).value();
    QJsonArray Array = ArrayRef.toArray();
    QJsonArray::iterator ArrayIterator = Array.begin();
    QJsonValueRef ElementOneValueRef = ArrayIterator[1];
    QJsonObject ElementOneObject = ElementOneValueRef.toObject();
    QJsonValueRef ArrayRef2 = ElementOneObject.find(kGroupsName).value();
    QJsonArray Array2 = ArrayRef2.toArray();
    QJsonArray::iterator ArrayIterator2 = Array2.begin();
    QJsonValueRef ElementOneValueRef2 = ArrayIterator2[0];
    QJsonObject ElementOneObject2 = ElementOneValueRef2.toObject();

    QJsonValueRef indexArrayRef = ElementOneObject2.find(kOptionsName).value();
    QJsonArray indexArray = indexArrayRef.toArray();
    indexArray.removeFirst();
    indexArray.removeFirst();
    indexArrayRef = indexArray;

    ElementOneValueRef2 = ElementOneObject2;
    ArrayRef2 = Array2;
    ElementOneValueRef = ElementOneObject;
    ArrayRef = Array;
    QByteArray arr = QJsonDocument(RootObject).toJson();
    return arr;
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
        qWarning() << "config template json is not valid!" << err.errorString();
        return;
    }

    if (!doc.object().contains(kGroupsName)) {
        qWarning() << "config template is not valid, no group item";
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

void SettingDialog::loadSettings(const QString &templateFile)
{
    QFile file(templateFile);
    if (!file.open(QFile::ReadOnly))
        return;
    QByteArray data = file.readAll();
    file.close();

    if (!QDBusConnection::systemBus().interface()->isServiceRegistered("com.deepin.anything"))
        data = removeQuickSearchIndex(data);

    settingFilter(data);
    dtkSettings = DSettings::fromJson(data);
}

QPointer<QCheckBox> SettingDialog::kAutoMountCheckBox = nullptr;
QPointer<QCheckBox> SettingDialog::kAutoMountOpenCheckBox = nullptr;
QSet<QString> SettingDialog::kHiddenSettingItems {};

SettingDialog::SettingDialog(QWidget *parent)
    : DSettingsDialog(parent)
{
    widgetFactory()->registerWidget("mountCheckBox", &SettingDialog::createAutoMountCheckBox);
    widgetFactory()->registerWidget("openCheckBox", &SettingDialog::createAutoMountOpenCheckBox);
    widgetFactory()->registerWidget("splitter", &SettingDialog::createSplitter);
    widgetFactory()->registerWidget("checkBoxWithMessage", &SettingDialog::createCheckBoxWithMessage);

    if (WindowUtils::isWayLand()) {
        setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        setAttribute(Qt::WA_NativeWindow);
        windowHandle()->setProperty("_d_dwayland_minimizable", false);
        windowHandle()->setProperty("_d_dwayland_maximizable", false);
        windowHandle()->setProperty("_d_dwayland_resizable", false);
        setFixedSize(QSize(700, 700));
    }

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
        updateSettings("QObject", dtkSettings);
    }
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

QPair<QWidget *, QWidget *> SettingDialog::createSplitter(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
    auto lab = new QLabel(qApp->translate("QObject", option->name().toStdString().c_str()));
    return qMakePair(lab, nullptr);
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
