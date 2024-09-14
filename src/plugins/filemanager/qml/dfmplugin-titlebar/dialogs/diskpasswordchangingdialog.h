// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    void initUI();
    void initConnect();

private:
    DPCResultWidget *resultWidget { nullptr };
    DPCProgressWidget *progressWidget { nullptr };
    DPCConfirmWidget *confirmWidget { nullptr };
    QStackedWidget *switchPageWidget { nullptr };
};

}

#endif   // DISKPASSWORDCHANGINGDIALOG_H
