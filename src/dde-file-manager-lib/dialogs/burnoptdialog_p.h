// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BURNOPTDIALOG_P_H
#define BURNOPTDIALOG_P_H
#include "burnoptdialog.h"

class BurnOptDialogPrivate
{
public:
    explicit BurnOptDialogPrivate(BurnOptDialog *q);
    ~BurnOptDialogPrivate();
    void setupUi();
    void setDevice(const QString &device);
private:
    BurnOptDialog *q_ptr = nullptr;
    QWidget *w_content = nullptr;
    QLabel *lb_volname = nullptr;
    QLineEdit *le_volname = nullptr;
    QLabel *lb_writespeed = nullptr;
    QComboBox *cb_writespeed = nullptr;
    QLabel *lb_fs = nullptr;
    QComboBox *cb_fs = nullptr;
    QCheckBox *cb_donotclose = nullptr;
    QLabel *lb_postburn = nullptr;
    QCheckBox *cb_checkdisc = nullptr;
    QCheckBox *cb_eject = nullptr;
    QString dev{""};
    QHash<QString, int> speedmap;
    DUrl image_file;
    int window_id = 0;

    QString lastVolName;

    Q_DECLARE_PUBLIC(BurnOptDialog)
};


#endif // BURNOPTDIALOG_P_H
