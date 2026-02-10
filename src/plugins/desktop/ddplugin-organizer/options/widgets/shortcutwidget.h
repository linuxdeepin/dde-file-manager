// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUTWIDGET_H
#define SHORTCUTWIDGET_H

#include "entrywidget.h"

#include <DKeySequenceEdit>

#include <QLabel>

namespace ddplugin_organizer {

class ShortcutWidget : public EntryWidget
{
    Q_OBJECT
public:
    explicit ShortcutWidget(const QString &title, QWidget *parent = nullptr);
    void setKeySequence(const QKeySequence &sequence);

signals:
    void keySequenceChanged(const QKeySequence &seq);
    void keySequenceUpdateFailed(const QKeySequence &seq);

private:
    bool modifierMatched(const QKeySequence &seq);

protected:
    QLabel *label { nullptr };
    DTK_WIDGET_NAMESPACE::DKeySequenceEdit *keyEdit { nullptr };
};

}   // namespace ddplugin_organizer

#endif   // SHORTCUTWIDGET_H
