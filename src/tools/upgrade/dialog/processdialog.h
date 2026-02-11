// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSDIALOG_H
#define PROCESSDIALOG_H

#include <DDialog>

namespace dfm_upgrade {
class ProcessDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit ProcessDialog(QWidget *parent = nullptr);
    void initialize(bool desktop);
    bool execDialog();
    void restart();
protected:
    QList<int> queryProcess(const QString &exec);
    void killAll(const QList<int> &pids);
private:
    QString targetExe(const QString &proc);
    int targetUid(const QString &proc);
    bool isEqual(const QString &link, QString match) const;
private:
    int accept = -1;
    bool onDesktop = false;
    bool killed = false;
};
}
#endif // PROCESSDIALOG_H
