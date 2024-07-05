// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/shortcutmap.h>

#include <QKeyEvent>
#include <QMultiMap>

DFMGUI_BEGIN_NAMESPACE

class ShortcutMapData
{
public:
    ShortcutMapData() = default;
    ~ShortcutMapData();

    QMap<int, ShortcutMap::KeyBinding> shortcutMap;   // 映射<类型，按键绑定>
    QMap<int, int> shortcutToType;   // 映射<快捷键，类型>
};

ShortcutMapData::~ShortcutMapData()
{
}

/*!
 * \class WindowShortcutMap
 * \brief 用于窗口快捷映射设置和获取
 *  一个动作可以对应多个快捷键，但快捷键对应的动作是唯一的，不能出现冲突。
 *  kType_Unknown (0) 被视为未知的快捷类型
 */
ShortcutMap::ShortcutMap(QObject *parent)
    : QObject { parent }, d { new ShortcutMapData }
{
}

ShortcutMap::~ShortcutMap()
{
}

/*!
 * \brief 追加新的快捷键信息 \a shortcut
 * \return 是否正常添加，若快捷键冲突，返回 false
 */
bool ShortcutMap::addShortcut(const KeyBinding &shortcut)
{
    int keys = shortcut.shortcutKeys.toCombined();
    if (d->shortcutToType.contains(keys))
        return false;

    d->shortcutMap.insert(shortcut.type, shortcut);
    d->shortcutToType.insert(keys, shortcut.type);
    return true;
}

/*!
 * \brief 追加新的快捷键信息 \a shortcutList
 * \return 是否正常添加，若快捷键冲突，返回 false
 */
bool ShortcutMap::addShortcutList(const QList<KeyBinding> &shortcutList)
{
    for (const KeyBinding &bind : shortcutList) {
        if (!addShortcut(bind))
            return false;
    }

    return true;
}

/*!
 * \brief 替换快捷键类型 \a type 的快捷键为 \a keys
 * \return 是否正常替换，若快捷键冲突，返回 false
 */
bool ShortcutMap::replaceShortcut(int type, const QKeyCombination &keys)
{
    if (!d->shortcutMap.contains(type))
        return false;

    auto &bind = d->shortcutMap[type];
    int oldShortcut = bind.shortcutKeys.toCombined();
    int newShortcut = keys.toCombined();
    if (oldShortcut != newShortcut) {
        // 识别到快捷键冲突
        if (d->shortcutToType.contains(newShortcut))
            return false;

        bind.shortcutKeys = keys;
        d->shortcutToType.remove(oldShortcut);
        d->shortcutToType.insert(newShortcut, bind.type);
    }

    return true;
}

/*!
 * \brief 替换现有类型和 \a shortcut 一致的快捷键绑定
 * \return 是否替换成功，若快捷键冲突，返回 false
 */
bool ShortcutMap::replaceShortcut(const KeyBinding &shortcut)
{
    return replaceShortcut(shortcut.type, shortcut.shortcutKeys);
}

/*!
 * \brief 替换快捷键映射，缓存中相同类型的快捷键绑定会被替换为 \a shortcutList 中的快捷键设置
 */
bool ShortcutMap::replaceShortcutList(const QList<KeyBinding> &shortcutList)
{
    if (shortcutList.isEmpty())
        return false;

    for (const KeyBinding &bind : shortcutList) {
        if (!replaceShortcut(bind.type, bind.shortcutKeys))
            return false;
    }

    return true;
}

/*!
 * \brief 移除快捷键类型为 \a type 的按键绑定
 */
bool ShortcutMap::removeShortcut(int type)
{
    if (!d->shortcutMap.contains(type))
        return false;

    KeyBinding bind = d->shortcutMap.take(type);
    d->shortcutToType.remove(bind.shortcutKeys.toCombined());
    return true;
}

/*!
 * \brief 按键事件 \a keyEvent 是否匹配当前记录的按键绑定
 * \return 是否检测到匹配的快捷键类型，未检测成功返回 kType_Unknown
 */
int ShortcutMap::detectShortcut(QKeyEvent *keyEvent) const
{
    if (keyEvent) {
        const QKeyCombination combin = keyEvent->keyCombination();
        const int key = combin.key();
        if (Qt::Key_1 <= key && key <= Qt::Key_8) {
            // 特殊处理：将特定区域的按键(Key_1 ~ Key_8)转换为 kKey_NumRange 处理
            auto specialKey = QKeyCombination::fromCombined(combin.keyboardModifiers() | ShortcutMap::kKey_NumRange);
            const int specialType = d->shortcutToType.value(specialKey.toCombined());
            if (kType_Unknown != specialType) {
                return specialType;
            }
        }

        return d->shortcutToType.value(combin.toCombined());
    }

    return kType_Unknown;
}

DFMGUI_END_NAMESPACE
