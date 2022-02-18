/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef MOUNTSECRETDISKASKPASSWORDDIALOG_H
#define MOUNTSECRETDISKASKPASSWORDDIALOG_H

#include "dfm-base/dfm_base_global.h"

#include <DDialog>

class QLineEdit;
class QLabel;

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
DWIDGET_END_NAMESPACE

DFMBASE_BEGIN_NAMESPACE
class MountSecretDiskAskPasswordDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit MountSecretDiskAskPasswordDialog(const QString &tipMessage, QWidget *parent = nullptr);
    ~MountSecretDiskAskPasswordDialog();

    QString getUerInputedPassword() const;

public slots:
    void handleButtonClicked(int index, QString text);

protected:
    void showEvent(QShowEvent *event);
    void initUI();
    void initConnect();

private:
    QString descriptionMessage = "";
    QLabel *titleLabel = nullptr;
    QLabel *descriptionLabel = nullptr;
    DTK_WIDGET_NAMESPACE::DPasswordEdit *passwordLineEdit = nullptr;
    QString password = "";
};
DFMBASE_END_NAMESPACE

#endif   // MOUNTSECRETDISKASKPASSWORDDIALOG_H
