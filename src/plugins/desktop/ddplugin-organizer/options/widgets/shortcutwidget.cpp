// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcutwidget.h"
#include "ddplugin_organizer_global.h"

DWIDGET_USE_NAMESPACE

namespace ddplugin_organizer {

ShortcutWidget::ShortcutWidget(const QString &title, QWidget *parent)
    : EntryWidget(new QLabel(title), new DKeySequenceEdit(), parent)
{
    label = qobject_cast<QLabel *>(leftWidget);
    label->setParent(this);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    keyEdit = qobject_cast<DKeySequenceEdit *>(rightWidget);
    keyEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto p = keyEdit->palette();
    p.setColor(QPalette::Base, Qt::transparent);
    p.setColor(QPalette::Button, Qt::transparent);
    keyEdit->setPalette(p);
    keyEdit->setAutoFillBackground(true);

    connect(keyEdit, &DKeySequenceEdit::editingFinished, this, [this](const QKeySequence &seq) {
        keyEdit->clearFocus();
        if (!modifierMatched(seq)) {
            emit keySequenceUpdateFailed(seq);
        } else {
            emit keySequenceChanged(seq);
        }
    });
}

void ShortcutWidget::setKeySequence(const QKeySequence &sequence)
{
    keyEdit->setKeySequence(sequence);
}

bool ShortcutWidget::modifierMatched(const QKeySequence &seq)
{
    QStringList modifiers { "Meta",
                            "Shift",
                            "Ctrl",
                            "Alt" };
    return std::any_of(modifiers.begin(), modifiers.end(), [seq](const QString &key) {
        return seq.toString().startsWith(key);
    });
}

}   // namespace ddplugin_organizer
