// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WAITDIALOG_H
#define WAITDIALOG_H

#include "dfmplugin_vault_global.h"

#include <DDialog>

#include <QTimer>

namespace dfmplugin_vault {
class WaitDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit WaitDialog(QWidget *parent = nullptr);
    ~WaitDialog() override;

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void handleTimeout();

private:
    void initUi();
    void initConnect();

    QTimer timer;
};
}

#endif // WAITDIALOG_H
