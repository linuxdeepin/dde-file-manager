/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
