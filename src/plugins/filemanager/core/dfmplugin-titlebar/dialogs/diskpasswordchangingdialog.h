/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef DISKPASSWORDCHANGINGDIALOG_H
#define DISKPASSWORDCHANGINGDIALOG_H

#include "dfmplugin_titlebar_global.h"

#include <DDialog>

class QStackedWidget;

namespace dfmplugin_titlebar {

class DPCResultWidget;
class DPCProgressWidget;
class DPCConfirmWidget;

class DiskPasswordChangingDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit DiskPasswordChangingDialog(QWidget *parent = nullptr);

public Q_SLOTS:
    void onConfirmed();
    void onChangeCompleted(bool success, const QString &msg);

private:
    void initUI();
    void initConnect();
    void displayNextPage();

private:
    DPCResultWidget *resultWidget { nullptr };
    DPCProgressWidget *progressWidget { nullptr };
    DPCConfirmWidget *confirmWidget { nullptr };
    QStackedWidget *switchPageWidget { nullptr };
};

}

#endif   // DISKPASSWORDCHANGINGDIALOG_H
