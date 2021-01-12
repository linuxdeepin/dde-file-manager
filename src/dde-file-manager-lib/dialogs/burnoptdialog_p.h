/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     xushitong <xushitong@uniontech.com>
 *
 * Maintainer: xushitong <xushitong@uniontech.com>
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
