// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUNTASKPASSWORDDIALOG_H
#define MOUNTASKPASSWORDDIALOG_H

#include <dfm-base/dfm_base_global.h>

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

namespace dfmbase {

class MountAskPasswordDialog : public DTK_WIDGET_NAMESPACE::DDialog
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

public Q_SLOTS:
    void handleConnect();
    void handleButtonClicked(int index, QString text);

private:
    QLabel *messageLabel { nullptr };
    QLabel *domainLabel { nullptr };
    QFrame *passwordFrame { nullptr };
    QLineEdit *usernameLineEdit { nullptr };
    QLineEdit *domainLineEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *passwordLineEdit { nullptr };
    QCheckBox *passwordCheckBox { nullptr };

    QButtonGroup *passwordButtonGroup { nullptr };
    DTK_WIDGET_NAMESPACE::DButtonBoxButton *anonymousButton { nullptr };
    DTK_WIDGET_NAMESPACE::DButtonBoxButton *registerButton { nullptr };

    QJsonObject loginObj;

    bool domainLineVisible { true };
};

}

#endif   // MOUNTASKPASSWORDDIALOG_H
