/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#include "dfmsettingdialog.h"

#include <QCheckBox>
#include <QFrame>
#include <QVBoxLayout>
#include <QDebug>
#include <QProcessEnvironment>
#include <QWindow>

#include <dtkcore_global.h>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <dsettingsbackend.h>
#ifdef DISABLE_QUICK_SEARCH
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#endif
#ifndef FULLTEXTSEARCH_ENABLE
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#else
#include "fulltextsearch/fulltextsearch.h"
#endif

#include "dfmglobal.h"
#include "app/define.h"
#include "singleton.h"
#include "dfmapplication.h"
#include "app/filesignalmanager.h"
#include "dfmsettings.h"
#include "private/dfmsettingdialog_p.h"

#define FULLTEXT_KEY      "IndexFullTextSearch"
#define FULLTEXT_GROUP             "GenericAttribute"

SettingBackend::SettingBackend(QObject *parent)
    : DSettingsBackend(parent)
{
    Q_ASSERT(DFMApplication::instance());

    connect(DFMApplication::instance(), &DFMApplication::appAttributeEdited, this, &SettingBackend::onValueChanged);
    connect(DFMApplication::instance(), &DFMApplication::genericAttributeEdited, this, &SettingBackend::onValueChanged);
}

QStringList SettingBackend::keys() const
{
    return keyToAA.keys() + keyToGA.keys();
}

QVariant SettingBackend::getOption(const QString &key) const
{
    int attribute = keyToAA.value(key, static_cast<DFMApplication::ApplicationAttribute>(-1));

    if (attribute >= 0) {
        return DFMApplication::instance()->appAttribute(static_cast<DFMApplication::ApplicationAttribute>(attribute));
    }

    attribute = keyToGA.value(key, static_cast<DFMApplication::GenericAttribute>(-1));

    Q_ASSERT(attribute >= 0);

    return DFMApplication::instance()->genericAttribute(static_cast<DFMApplication::GenericAttribute>(attribute));
}

void SettingBackend::doSync()
{

}

void SettingBackend::doSetOption(const QString &key, const QVariant &value)
{
    QSignalBlocker blocker(this);
    Q_UNUSED(blocker)

    int attribute = keyToAA.value(key, static_cast<DFMApplication::ApplicationAttribute>(-1));

    if (attribute >= 0) {
        return DFMApplication::instance()->setAppAttribute(static_cast<DFMApplication::ApplicationAttribute>(attribute), value);
    }

    attribute = keyToGA.value(key, static_cast<DFMApplication::GenericAttribute>(-1));

    Q_ASSERT(attribute >= 0);

    DFMApplication::instance()->setGenericAttribute(static_cast<DFMApplication::GenericAttribute>(attribute), value);

    //fix bug 39785 【专业版 sp3】【文件管理器】【5.2.0.8-1】文管菜单设置隐藏系统盘，关闭所有文管窗口，再打开新文管窗口，系统盘没有隐藏
    if (key == QString("advance.other.hide_system_partition")) {
        fileSignalManager->requestHideSystemPartition(value.toBool());
    }

    // fix bug 81014
#ifdef FULLTEXTSEARCH_ENABLE
    if (key == QString("advance.index.index_search") && value.toBool())
        DFMFullTextSearchManager::getInstance()->fulltextIndex("/");/*全文搜索建立索引*/
#endif
}

void SettingBackend::onValueChanged(int attribute, const QVariant &value)
{
    QString key = keyToAA.key(static_cast<DFMApplication::ApplicationAttribute>(attribute));

    if (key.isEmpty()) {
        key = keyToGA.key(static_cast<DFMApplication::GenericAttribute>(attribute));
    }

    Q_ASSERT(!key.isEmpty());

    emit optionChanged(key, value);
}

static auto fromJsJson(const QString &fileName) -> decltype(DSettings::fromJson(QByteArray()))
{
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly)) {
        return nullptr;
    }

    QByteArray data = file.readAll();

    file.close();

    auto indexOfChar = [](const QByteArray & data, char ch, int from) {
        for (; from < data.size(); ++from) {
            if (data.at(from) == '\\') {
                continue;
            }

            if (data.at(from) == ch) {
                return from;
            }
        }

        return from;
    };

    auto clean_qsTr = [indexOfChar](QByteArray & data, int &from) {
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

            // 寻找qsTranslate的第一个参数
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

        // 保留需要翻译的字符串
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
    };

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
            clean_qsTr(data, i);
            break;
        default:
            break;
        }
    }

#ifndef FULLTEXTSEARCH_ENABLE
    auto const &jdoc = QJsonDocument::fromJson(data);
    QJsonObject RootObject = jdoc.object();
    QJsonValueRef ArrayRef = RootObject.find("groups").value();
    QJsonArray Array = ArrayRef.toArray();
    QJsonArray::iterator ArrayIterator = Array.begin();
    QJsonValueRef ElementOneValueRef = ArrayIterator[1];
    QJsonObject ElementOneObject = ElementOneValueRef.toObject();
    QJsonValueRef ArrayRef2 = ElementOneObject.find("groups").value();
    QJsonArray Array2 = ArrayRef2.toArray();
    Array2.removeFirst();
    ArrayRef2 = Array2;
    ElementOneValueRef = ElementOneObject;
    ArrayRef = Array;
    qDebug() << RootObject;
    QByteArray arr = QJsonDocument(RootObject).toJson();

    return DSettings::fromJson(arr);
#else
#ifdef DISABLE_QUICK_SEARCH
    /*fix task 22667,针对ARM下不能使用anything,所以去掉整个索引配置项
    */
    //解析
    auto const &jdoc = QJsonDocument::fromJson(data);
    QJsonObject RootObject = jdoc.object();
    QJsonValueRef ArrayRef = RootObject.find("groups").value();
    QJsonArray Array = ArrayRef.toArray();
    QJsonArray::iterator ArrayIterator = Array.begin();
    QJsonValueRef ElementOneValueRef = ArrayIterator[1];
    QJsonObject ElementOneObject = ElementOneValueRef.toObject();
    QJsonValueRef ArrayRef2 = ElementOneObject.find("groups").value();
    QJsonArray Array2 = ArrayRef2.toArray();
    QJsonArray::iterator ArrayIterator2 = Array2.begin();
    QJsonValueRef ElementOneValueRef2 = ArrayIterator2[0];
    QJsonObject ElementOneObject2 = ElementOneValueRef2.toObject();
    /*使能全文搜索在ARM下能正常工作*/
    QJsonValueRef indexArrayRef = ElementOneObject2.find("options").value();
    QJsonArray indexArray = indexArrayRef.toArray();
    indexArray.removeFirst();
    indexArray.removeFirst();
    indexArrayRef = indexArray;

    ElementOneValueRef2 = ElementOneObject2;
    ArrayRef2 = Array2;
    ElementOneValueRef = ElementOneObject;
    ArrayRef = Array;
    qDebug() << RootObject;
    QByteArray arr = QJsonDocument(RootObject).toJson();

    return DSettings::fromJson(arr);
#else
    return DSettings::fromJson(data);
#endif
#endif
}

QPointer<QCheckBox> DFMSettingDialog::AutoMountCheckBox = nullptr;
QPointer<QCheckBox> DFMSettingDialog::AutoMountOpenCheckBox = nullptr;

DFMSettingDialog::DFMSettingDialog(QWidget *parent):
    DSettingsDialog(parent)
{
    widgetFactory()->registerWidget("mountCheckBox", &DFMSettingDialog::createAutoMountCheckBox);
    widgetFactory()->registerWidget("openCheckBox", &DFMSettingDialog::createAutoMountOpenCheckBox);

#ifdef DISABLE_COMPRESS_PREIVEW
    //load temlate
    m_settings = fromJsJson(":/configure/global-setting-template-pro.js").data();
#else
#ifdef DISABLE_FFMEPG
    m_settings = fromJsJson(":/configure/global-setting-template-fedora.js").data();
#else
    m_settings = fromJsJson(":/configure/global-setting-template.js").data();
#endif
#endif

    if(DFMGlobal::isWayLand())
    {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
        this->setFixedSize(QSize(700, 700));
    }


    //load conf value
    auto backen = new SettingBackend(this);
#ifdef FULLTEXTSEARCH_ENABLE
    //fulltext fix 42500 配置文件无FULLTEXT_KEY 导致第一次索引失败
    QVariant var = DFMApplication::genericSetting()->value(FULLTEXT_GROUP, FULLTEXT_KEY);
    if (!var.isValid()) {
        DFMApplication::genericSetting()->setValue(FULLTEXT_GROUP, FULLTEXT_KEY, QVariant(false));

    }
#endif
    m_settings->setParent(this);
    m_settings->setBackend(backen);
    updateSettings("GenerateSettingTranslate", m_settings);
}

QPair<QWidget *, QWidget *> DFMSettingDialog::createAutoMountCheckBox(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
    QCheckBox *mountCheckBox = new QCheckBox(QObject::tr("Auto mount"));
    DFMSettingDialog::AutoMountCheckBox = mountCheckBox;

    if (option->value().toBool()) {
        mountCheckBox->setChecked(true);
    } else if (AutoMountOpenCheckBox) {
        AutoMountOpenCheckBox->setDisabled(true);
    }

    QObject::connect(mountCheckBox,
                     &QCheckBox::stateChanged,
                     option,
    [ = ](int state) {
        if (state == 0) {
            if (DFMSettingDialog::AutoMountOpenCheckBox) {
                DFMSettingDialog::AutoMountOpenCheckBox->setDisabled(true);
                DFMSettingDialog::AutoMountOpenCheckBox->setChecked(false);
            }
            option->setValue(false);
        } else if (state == 2) {
            if (DFMSettingDialog::AutoMountOpenCheckBox) {
                DFMSettingDialog::AutoMountOpenCheckBox->setDisabled(false);
            }

            option->setValue(true);
        }
    });

    QObject::connect(option, &DSettingsOption::valueChanged, mountCheckBox, [ = ](QVariant value) {
        mountCheckBox->setChecked(value.toBool());
    });

    return qMakePair(mountCheckBox, nullptr);
}

QPair<QWidget *, QWidget *> DFMSettingDialog::createAutoMountOpenCheckBox(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
    QCheckBox *openCheckBox = new QCheckBox(QObject::tr("Open after auto mount"));
    DFMSettingDialog::AutoMountOpenCheckBox = openCheckBox;

    if (option->value().toBool()) {
        openCheckBox->setChecked(true);
        openCheckBox->setDisabled(false);
    } else {
        if (AutoMountCheckBox && !AutoMountCheckBox->isChecked()) {
            openCheckBox->setDisabled(true);
        }
    }

    QObject::connect(openCheckBox,
                     &QCheckBox::stateChanged,
                     option,
    [ = ](int state) {
        if (state == 0) {
            option->setValue(false);
        } else if (state == 2) {
            option->setValue(true);
        }
    });

    QObject::connect(option, &DSettingsOption::valueChanged, openCheckBox, [ = ](QVariant value) {
        openCheckBox->setChecked(value.toBool());
    });

    return qMakePair(openCheckBox, nullptr);
}
