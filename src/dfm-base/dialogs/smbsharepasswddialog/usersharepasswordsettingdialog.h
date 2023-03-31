// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USERSHAREPASSWORDSETTINGDIALOG_H
#define USERSHAREPASSWORDSETTINGDIALOG_H

#include <dfm-base/dfm_base_global.h>
#include <DDialog>
#include <DPasswordEdit>

#include <QAbstractButton>
#include <QWindow>

namespace dfmbase {
class UserSharePasswordSettingDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit UserSharePasswordSettingDialog(QWidget *parent = nullptr);
    void initializeUi();

Q_SIGNALS:
    void inputPassword(const QString &password);

public Q_SLOTS:
    void onButtonClicked(const int &index);

private:
    DTK_WIDGET_NAMESPACE::DPasswordEdit *passwordEdit;
    QWidget *content;
};

}

#endif   // USERSHAREPASSWORDSETTINGDIALOG_H
