/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#ifndef BURNOPTDIALOG_H
#define BURNOPTDIALOG_H
#include "ddialog.h"
#include "durl.h"

#include <QCheckBox>
#include <DLineEdit>
#include <QComboBox>

class BurnOptDialogPrivate;
class BurnOptDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    BurnOptDialog(QString device, QWidget *parent = nullptr);
    ~BurnOptDialog();

    void setISOImage(DUrl image);
    void setJobWindowId(int wid);
    void setDefaultVolName(const QString &volName);
    void setDiscAndFsInfo(int type, QString filesystem, QString version);
    static bool isSupportedUDVersion(const QString &version);
    static bool isSupportedUDMedium(int type);

private:
    QScopedPointer<BurnOptDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE(BurnOptDialog)

};

#endif // BURNOPTDIALOG_H
