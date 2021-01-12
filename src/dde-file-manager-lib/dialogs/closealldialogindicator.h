/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef CLOSEALLDIALOGINDICATOR_H
#define CLOSEALLDIALOGINDICATOR_H

#include <QLabel>
#include <QPushButton>

#include <dabstractdialog.h>

DWIDGET_USE_NAMESPACE

class CloseAllDialogIndicator : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit CloseAllDialogIndicator(QWidget *parent = nullptr);
    ~CloseAllDialogIndicator() override;

    void initUI();
    void initConnect();

signals:
    void allClosed();

public slots:
    void setTotalMessage(qint64 size, int count);

protected:
//    void showEvent(QShowEvent* event);
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QLabel *m_messageLabel;
    QPushButton *m_closeButton;
};

#endif // CLOSEALLDIALOGINDICATOR_H
