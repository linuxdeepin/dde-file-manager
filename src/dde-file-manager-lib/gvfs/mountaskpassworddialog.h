// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

signals:

public slots:
    void setLoginData(const QJsonObject& obj);
    void handleConnect();
    void handleButtonClicked(int index, QString text);

private:
    QLabel* m_messageLabel;
    QLabel* m_domainLabel;

    QFrame* m_passwordFrame;

    DButtonBoxButton *m_anonymousButton;
    DButtonBoxButton *m_registerButton;

    QLineEdit* m_usernameLineEdit;
    QLineEdit* m_domainLineEdit;
    DPasswordEdit* m_passwordLineEdit;

    QCheckBox* m_passwordCheckBox;

    QButtonGroup* m_passwordButtonGroup;


    QJsonObject m_loginObj;

    bool m_domainLineVisible = true;
};

#endif // MOUNTASKPASSWORDDIALOG_H
