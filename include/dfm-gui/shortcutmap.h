// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUTMAP_H
#define SHORTCUTMAP_H

#include <dfm-gui/dfm_gui_global.h>
#include <dfm-gui/quickutils.h>

#include <QKeySequence>
#include <QObject>

class QKeyEvent;

DFMGUI_BEGIN_NAMESPACE

class ShortcutMapData;
class ShortcutMap : public QObject
{
    Q_OBJECT

public:
    enum SpecialType {
        kType_Unknown = 0,   // 特殊快捷键类型，0 被视为未知类型
    };

    // 特殊键值，定义范围参考 Qt::Key
    enum SpecialKey {
        kKey_NumRange = 0x01111000,   // 映射 Qt::Key_1 ~ Qt::Key_8
    };

    // 按键绑定信息(后期按需添加 name / description 等属性)
    struct KeyBinding
    {
        int type { kType_Unknown };   // 快捷键类型标记
        QKeyCombination shortcutKeys;   // 快捷键设置

        KeyBinding() = default;
        KeyBinding(int t, const QKeyCombination &k)
            : type(t), shortcutKeys(k) { }
    };

    explicit ShortcutMap(QObject *parent = nullptr);
    ~ShortcutMap() override;

    bool addShortcut(const KeyBinding &shortcut);
    bool addShortcutList(const QList<KeyBinding> &shortcutList);

    bool replaceShortcut(int type, const QKeyCombination &keys);
    bool replaceShortcut(const KeyBinding &shortcut);
    bool replaceShortcutList(const QList<KeyBinding> &shortcutList);

    bool removeShortcut(int type);

    int detectShortcut(QKeyEvent *keyEvent) const;

private:
    QScopedPointer<ShortcutMapData> d;
    Q_DISABLE_COPY(ShortcutMap)
};

using ShortcutMapPtr = QSharedPointer<ShortcutMap>;

DFMGUI_END_NAMESPACE

#endif   // SHORTCUTMAP_H
