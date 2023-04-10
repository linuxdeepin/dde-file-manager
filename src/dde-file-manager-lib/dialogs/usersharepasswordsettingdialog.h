// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USERSHAREPASSWORDSETTINGDIALOG_H
#define USERSHAREPASSWORDSETTINGDIALOG_H

#include <QObject>
#include <DDialog>
#include <DPasswordEdit>

DWIDGET_USE_NAMESPACE

class UserSharePasswordSettingDialog : public DDialog
{
    Q_OBJECT
public:
    explicit UserSharePasswordSettingDialog(QWidget *parent = nullptr);
    void initUI();

signals:

protected:
    virtual void showEvent(QShowEvent *event) override;

public slots:
    void onButtonClicked(const int& index);

private:
    DPasswordEdit *m_passwordEdit;
    QWidget *m_content;
};

#endif // USERSHAREPASSWORDSETTINGDIALOG_H
