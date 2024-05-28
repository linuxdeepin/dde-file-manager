// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ALERTHIDEALLDIALOG_H
#define ALERTHIDEALLDIALOG_H

#include <DAbstractDialog>

namespace ddplugin_organizer {

class AlertHideAllDialog : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
public:
    explicit AlertHideAllDialog(QWidget *parent = nullptr);

    void initialize();
    bool isRepeatNoMore() const;

private:
    bool repeatNoMore { false };
};

}   // namespace ddplugin_organizer

#endif   // ALERTHIDEALLDIALOG_H
