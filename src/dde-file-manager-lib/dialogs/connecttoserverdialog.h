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
#include <QPushButton>
DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DListView;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QComboBox;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class CollectionDelegate;
class ConnectToServerDialog : public DDialog
{
    Q_OBJECT
public:
    explicit ConnectToServerDialog(QWidget *parent = nullptr);
    ~ConnectToServerDialog();
signals:

public slots:
    void onButtonClicked(const int& index);
private slots:
    void collectionOperate();
    void doDeleteCollection(const QString& text, int row = -1);
protected:
    bool eventFilter(QObject *o, QEvent *e);

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
    QComboBox *m_schemeComboBox = nullptr;
    QPushButton *m_addButton = nullptr;
    DIconButton *m_delButton = nullptr;
    DListView *m_collectionServerView = nullptr;
    bool m_isAddState = true;
    CollectionDelegate* m_delegate = nullptr;
};

#endif // CONNECTTOSERVERDIALOG_H
