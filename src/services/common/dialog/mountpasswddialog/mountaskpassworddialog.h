/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef MOUNTASKPASSWORDDIALOG_H
#define MOUNTASKPASSWORDDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QJsonObject>
#include <DDialog>
#include <DPasswordEdit>
#include <DButtonBox>

DWIDGET_USE_NAMESPACE

class MountAskPasswordDialog : public DDialog
{
    Q_OBJECT
public:
    explicit MountAskPasswordDialog(QWidget *parent = nullptr);
    ~MountAskPasswordDialog();

    void initUI();
    void initConnect();
    QJsonObject getLoginData();

    bool getDomainLineVisible() const;
    void setDomainLineVisible(bool domainLineVisible);

    void setDomain(const QString &domain);
    void setUser(const QString &user);

    enum {
        kNeverSave,
        kSaveBeforeLogout,
        kSavePermanently,
    };

public slots:
    void handleConnect();
    void handleButtonClicked(int index, QString text);

private:
    QLabel *messageLabel { nullptr };
    QLabel *domainLabel { nullptr };
    QFrame *passwordFrame { nullptr };
    QLineEdit *usernameLineEdit { nullptr };
    QLineEdit *domainLineEdit { nullptr };
    DPasswordEdit *passwordLineEdit { nullptr };
    QCheckBox *passwordCheckBox { nullptr };

    QButtonGroup *passwordButtonGroup { nullptr };
    DButtonBoxButton *anonymousButton { nullptr };
    DButtonBoxButton *registerButton { nullptr };

    QJsonObject loginObj;

    bool domainLineVisible { true };
};

#endif   // MOUNTASKPASSWORDDIALOG_H
