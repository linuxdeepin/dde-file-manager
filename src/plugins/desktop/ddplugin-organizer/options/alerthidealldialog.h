// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ALERTHIDEALLDIALOG_H
#define ALERTHIDEALLDIALOG_H

#include <DDialog>

namespace ddplugin_organizer {

class AlertHideAllDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit AlertHideAllDialog(QWidget *parent = nullptr);

    void initialize();
    bool isRepeatNoMore() const;
    int confirmBtnIndex() const;

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    bool repeatNoMore { false };
    int btnIndex { -1 };
};

}   // namespace ddplugin_organizer

#endif   // ALERTHIDEALLDIALOG_H
