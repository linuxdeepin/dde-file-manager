/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "settingdialog.h"
#include "settingdialog_p.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/utils/windowutils.h"

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

#ifndef ENABLE_QUICK_SEARCH
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#endif

DFMBASE_USE_NAMESPACE

#ifndef ENABLE_QUICK_SEARCH
namespace {
const char *const kGroupsName { "groups" };
const char *const kOptionsName { "options" };
}
#endif

SettingBackend::SettingBackend(QObject *parent)
    : DSettingsBackend(parent)
{
    Q_ASSERT(Application::instance());

    connect(Application::instance(), &Application::appAttributeEdited, this, &SettingBackend::onValueChanged);
    connect(Application::instance(), &Application::genericAttributeEdited, this, &SettingBackend::onValueChanged);
}

QStringList SettingBackend::keys() const
{
    return keyToAA.keys() + keyToGA.keys();
}

QVariant SettingBackend::getOption(const QString &key) const
{
    int attribute = keyToAA.value(key, static_cast<Application::ApplicationAttribute>(-1));

    if (attribute >= 0) {
        return Application::instance()->appAttribute(static_cast<Application::ApplicationAttribute>(attribute));
    }

    attribute = keyToGA.value(key, static_cast<Application::GenericAttribute>(-1));

    Q_ASSERT(attribute >= 0);

    return Application::instance()->genericAttribute(static_cast<Application::GenericAttribute>(attribute));
}

void SettingBackend::doSync()
{
}

void SettingBackend::doSetOption(const QString &key, const QVariant &value)
{
    QSignalBlocker blocker(this);
    Q_UNUSED(blocker)

    int attribute = keyToAA.value(key, static_cast<Application::ApplicationAttribute>(-1));

    if (attribute >= 0) {
        return Application::instance()->setAppAttribute(static_cast<Application::ApplicationAttribute>(attribute), value);
    }

    attribute = keyToGA.value(key, static_cast<Application::GenericAttribute>(-1));

    Q_ASSERT(attribute >= 0);

    Application::instance()->setGenericAttribute(static_cast<Application::GenericAttribute>(attribute), value);
}

void SettingBackend::onValueChanged(int attribute, const QVariant &value)
{
    QString key = keyToAA.key(static_cast<Application::ApplicationAttribute>(attribute));

    if (key.isEmpty()) {
        key = keyToGA.key(static_cast<Application::GenericAttribute>(attribute));
    }

    Q_ASSERT(!key.isEmpty());

    emit optionChanged(key, value);
}

#ifndef ENABLE_QUICK_SEARCH
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
#endif

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

void cleanQsTr(QByteArray &data, int &from)
{
    const QByteArray &qsTr = QByteArrayLiteral("qsTr");
    const QByteArray &qsTranslate = QByteArrayLiteral("anslate(");

    if (qsTr != QByteArray(data.data() + from, qsTr.size())) {
        return;
    }

    int index = from + qsTr.size();

    if (data.at(index) == '(') {
        data.remove(from, index - from + 1);
    } else if (qsTranslate == QByteArray(data.data() + index, qsTranslate.size())) {
        index += qsTranslate.size();

        // find the first parameter of qsTranslate
        if (data.at(index) == '"' || data.at(index) == '\'') {
            index = indexOfChar(data, data.at(index), index + 1);

            if (index >= data.size()) {
                return;
            }
        } else {
            return;
        }

        int quote1_index = data.indexOf('"', index + 1);
        int quote2_index = data.indexOf('\'', index + 1);

        if (quote1_index > 0) {
            index = quote1_index;
        }

        if (quote2_index > 0 && quote2_index < index) {
            index = quote2_index;
        }

        data.remove(from, index - from);
    } else {
        return;
    }

    // keep the strings that need to be translated
    if (data.at(from) == '"' || data.at(from) == '\'') {
        from = indexOfChar(data, data.at(from), from + 1);

        if (from >= data.size()) {
            return;
        }
    } else {
        return;
    }

    from = indexOfChar(data, ')', from + 1);

    if (from < data.size()) {
        data.remove(from, 1);
        from -= 1;
    }
}

static auto fromJsJson(const QString &fileName) -> decltype(DSettings::fromJson(QByteArray()))
{
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly)) {
        return nullptr;
    }

    QByteArray data = file.readAll();

    file.close();

    for (int i = 0; i < data.size(); ++i) {
        char ch = data.at(i);

        switch (ch) {
        case '\\':
            break;
        case '\'':
        case '"':
            i = indexOfChar(data, ch, i + 1);
            break;
        case 'q':
            cleanQsTr(data, i);
            break;
        default:
            break;
        }
    }
#ifndef ENABLE_QUICK_SEARCH
    auto const &byteArray = removeQuickSearchIndex(data);
    return DSettings::fromJson(byteArray);
#else
    return DSettings::fromJson(data);
#endif
}

QPointer<QCheckBox> SettingDialog::kAutoMountCheckBox = nullptr;
QPointer<QCheckBox> SettingDialog::kAutoMountOpenCheckBox = nullptr;

SettingDialog::SettingDialog(QWidget *parent)
    : DSettingsDialog(parent)
{
    widgetFactory()->registerWidget("mountCheckBox", &SettingDialog::createAutoMountCheckBox);
    widgetFactory()->registerWidget("openCheckBox", &SettingDialog::createAutoMountOpenCheckBox);

#ifdef DISABLE_COMPRESS_PREIVEW
    // load temlate
    m_settings = fromJsJson(":/configure/global-setting-template-pro.js").data();
#else
#    ifdef DISABLE_FFMEPG
    m_settings = fromJsJson(":/configure/global-setting-template-fedora.js").data();
#    else
    dtkSettings = fromJsJson(":/configure/global-setting-template.js").data();
#    endif
#endif

    if (WindowUtils::isWayLand()) {
        setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        setAttribute(Qt::WA_NativeWindow);
        windowHandle()->setProperty("_d_dwayland_minimizable", false);
        windowHandle()->setProperty("_d_dwayland_maximizable", false);
        windowHandle()->setProperty("_d_dwayland_resizable", false);
        setFixedSize(QSize(700, 700));
    }

    // load conf value
    auto backen = new SettingBackend(this);
    if (dtkSettings) {
        dtkSettings->setParent(this);
        dtkSettings->setBackend(backen);
        updateSettings("GenerateSettingTranslate", dtkSettings);
    }
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
