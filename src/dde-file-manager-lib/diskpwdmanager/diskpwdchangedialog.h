// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKPWDCHANGEDIALOG_H
#define DISKPWDCHANGEDIALOG_H

#include <DDialog>

class QStackedWidget;
class ChangeResultWidget;
class ProgressWidget;
class PwdConfirmWidget;

class DiskPwdChangeDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit DiskPwdChangeDialog(QWidget *parent = nullptr);

public slots:
    void pwdConConfirmed();
    void pwdChangeFinished(bool success, const QString &msg);

private:
    void initUI();
    void initConnect();
    void displayNextPage();

private:
    ChangeResultWidget *m_resultWidget { nullptr };
    ProgressWidget *m_progressWidget { nullptr };
    PwdConfirmWidget *m_confirmWidget { nullptr };
    QStackedWidget *m_switchPageWidget { nullptr };
};

#endif   // DISKPWDCHANGEDIALOG_H
