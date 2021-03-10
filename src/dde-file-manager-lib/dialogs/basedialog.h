/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#ifndef BASEDIALOG_H
#define BASEDIALOG_H

#include <ddialog.h>

DWIDGET_USE_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DTitlebar;
DWIDGET_END_NAMESPACE

class QShowEvent;
class QWidget;

class BaseDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit BaseDialog(QWidget *parent = nullptr);
    ~BaseDialog();

    void setTitle(const QString &title);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    DTitlebar *m_titlebar = nullptr;
};

#endif // BASEDIALOG_H
