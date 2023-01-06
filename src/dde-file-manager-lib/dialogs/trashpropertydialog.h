// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHPROPERTYDIALOG_H
#define TRASHPROPERTYDIALOG_H

#include "durl.h"
#include "propertydialog.h"
#include <QLabel>
#include <ddialog.h>

DWIDGET_USE_NAMESPACE

class TrashPropertyDialog : public DDialog
{
    Q_OBJECT
public:
    explicit TrashPropertyDialog(const DUrl& url, QWidget *parent = nullptr);
    ~TrashPropertyDialog();

    void initUI();
    void startComputerFolderSize(const DUrl& url);

public slots:
    void updateFolderSize(qint64 size);

private:
    DUrl m_url;
    QLabel* m_iconLabel;
    QLabel* m_nameLable;
    QLabel* m_countLabel;
    QLabel* m_sizeLabel;
};

#endif // TRASHPROPERTYDIALOG_H
