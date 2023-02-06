/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef USERSHAREPASSWORDSETTINGDIALOG_H
#define USERSHAREPASSWORDSETTINGDIALOG_H

#include "dfm-base/dfm_base_global.h"
#include <DDialog>
#include <DPasswordEdit>

#include <QAbstractButton>
#include <QWindow>
#include <QLabel>

namespace dfmbase {
class UserSharePasswordSettingDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit UserSharePasswordSettingDialog(QWidget *parent = nullptr);
    void initializeUi();
Q_SIGNALS:
    void inputPassword(const QString &password);
public Q_SLOTS:
    void onButtonClicked(const int &index);

private:
    DTK_WIDGET_NAMESPACE::DPasswordEdit *passwordEdit;
    QWidget *content;
};

}

#endif   // USERSHAREPASSWORDSETTINGDIALOG_H
