/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include <dtkcore_global.h>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <dsettingsbackend.h>
#ifdef DISABLE_QUICK_SEARCH
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#endif

#include "dfmglobal.h"
#include "app/define.h"
#include "singleton.h"
#include "dfmapplication.h"
#include "app/filesignalmanager.h"

DFM_USE_NAMESPACE

template<typename Key, typename T>
class BidirectionHash
{
public:
#ifdef Q_COMPILER_INITIALIZER_LISTS
    inline BidirectionHash(std::initializer_list<std::pair<Key, T> > list)
    {
        k2v.reserve(int(list.size()));
        v2k.reserve(int(list.size()));

        for (auto it = list.begin(); it != list.end(); ++it) {
            k2v.insert(it->first, it->second);
            v2k.insert(it->second, it->first);
        }
    }
#endif

    bool containsKey(const Key &key) const
    {
        return k2v.contains(key);
    }
    bool containsValue(const T &value) const
    {
        return v2k.contains(value);
    }
    const Key key(const T &value) const
    {
        return v2k.value(value);
    }
    const Key key(const T &value, const Key &defaultKey) const
    {
        return v2k.value(value, defaultKey);
    }
    const T value(const Key &key) const
    {
        return k2v.value(key);
    }
    const T value(const Key &key, const T &defaultValue) const
    {
        return k2v.value(key, defaultValue);
    }
    QList<Key> keys() const
    {
        return k2v.keys();
    }

private:
    QHash<Key, T> k2v;
    QHash<T, Key> v2k;
};

class SettingBackend : public DSettingsBackend
{
public:
    explicit SettingBackend(QObject *parent = nullptr);

    QStringList keys() const;
    QVariant getOption(const QString &key) const;

    void doSync();

protected:
    void doSetOption(const QString &key, const QVariant &value);
    void onValueChanged(int attribute, const QVariant &value);

private:
    BidirectionHash<QString, DFMApplication::ApplicationAttribute> keyToAA {
        {"base.open_action.allways_open_on_new_window", DFMApplication::AA_AllwayOpenOnNewWindow},
        {"base.open_action.open_file_action", DFMApplication::AA_OpenFileMode},
        {"base.new_tab_windows.default_window_path", DFMApplication::AA_UrlOfNewWindow},
        {"base.new_tab_windows.new_tab_path", DFMApplication::AA_UrlOfNewTab},
        {"base.default_view.icon_size", DFMApplication::AA_IconSizeLevel},
        {"base.default_view.view_mode", DFMApplication::AA_ViewMode},
        {"base.default_view.view_size_adjustable", DFMApplication::AA_ViewSizeAdjustable},
    };
    BidirectionHash<QString, DFMApplication::GenericAttribute> keyToGA {
        {"base.hidden_files.show_hidden", DFMApplication::GA_ShowedHiddenFiles},
        {"base.hidden_files.hide_suffix", DFMApplication::GA_ShowedFileSuffixOnRename},
        {"base.hidden_files.show_recent", DFMApplication::GA_ShowRecentFileEntry},
        {"advance.index.index_internal", DFMApplication::GA_IndexInternal},
        {"advance.index.index_external", DFMApplication::GA_IndexExternal},
        {"advance.index.index_search", DFMApplication::GA_IndexSearch},
        {"advance.search.show_hidden", DFMApplication::GA_ShowedHiddenOnSearch},
        {"advance.preview.compress_file_preview", DFMApplication::GA_PreviewCompressFile},
        {"advance.preview.text_file_preview", DFMApplication::GA_PreviewTextFile},
        {"advance.preview.document_file_preview", DFMApplication::GA_PreviewDocumentFile},
        {"advance.preview.image_file_preview", DFMApplication::GA_PreviewImage},
        {"advance.preview.video_file_preview", DFMApplication::GA_PreviewVideo},
        {"advance.mount.auto_mount", DFMApplication::GA_AutoMount},
        {"advance.mount.auto_mount_and_open", DFMApplication::GA_AutoMountAndOpen},
        {"advance.dialog.default_chooser_dialog", DFMApplication::GA_OverrideFileChooserDialog},
        {"advance.other.hide_system_partition", DFMApplication::GA_HiddenSystemPartition},
        {"advance.other.show_crumbbar_clickable_area", DFMApplication::GA_ShowCsdCrumbBarClickableArea}
    };
};

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
    ElementOneObject2.remove("key");
    ElementOneObject2.remove("name");
    ElementOneObject2.remove("options");
    ElementOneValueRef2 = ElementOneObject2;
    ArrayRef2 = Array2;
    ElementOneValueRef = ElementOneObject;
    ArrayRef = Array;
    qDebug() << RootObject;
    QByteArray arr = QJsonDocument(RootObject).toJson();

    return DSettings::fromJson(arr);
#endif

    return DSettings::fromJson(data);
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

    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") ||
            WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        this->setFixedSize(QSize(700, 700));
    }


    //load conf value
    auto backen = new SettingBackend(this);

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
