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

#ifndef CONNECTTOSERVERDIALOG_H
#define CONNECTTOSERVERDIALOG_H

#include "dfmglobal.h"

#include <QObject>
#include <DDialog>

DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DListView;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QComboBox;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class ConnectToServerDialog : public DDialog
{
    Q_OBJECT
public:
    explicit ConnectToServerDialog(QWidget *parent = nullptr);

signals:

public slots:
    void onButtonClicked(const int& index);

private:
    void initUI();
    void initConnect();
    void onAddButtonClicked();
    void onDelButtonClicked();

    enum DialogButton {
        CannelButton,
        ConnectButton
    };

    QComboBox *m_serverComboBox = nullptr;
    DIconButton *m_addButton = nullptr;
    DIconButton *m_delButton = nullptr;
    DListView *m_collectionServerView = nullptr;
};

#endif // CONNECTTOSERVERDIALOG_H
