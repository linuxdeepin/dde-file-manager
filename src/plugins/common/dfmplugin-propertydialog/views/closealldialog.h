/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef CLOSEALLDIALOG_H
#define CLOSEALLDIALOG_H

#include "dfmplugin_propertydialog_global.h"

#include <DAbstractDialog>

#include <QPushButton>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
DWIDGET_END_NAMESPACE

namespace dfmplugin_propertydialog {
class CloseAllDialog : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
public:
    explicit CloseAllDialog(QWidget *parent = nullptr);
    ~CloseAllDialog() override;

    void initUI();
    void initConnect();

signals:
    void allClosed();

public slots:
    void setTotalMessage(qint64 size, int count);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

private:
    DTK_WIDGET_NAMESPACE::DLabel *messageLabel { nullptr };
    QPushButton *closeButton { nullptr };
};
}
#endif   // CLOSEALLDIALOG_H
