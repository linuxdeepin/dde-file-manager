// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUNTSECRETDISKASKPASSWORDDIALOG_H
#define MOUNTSECRETDISKASKPASSWORDDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonObject>
#include <DDialog>
#include <DPasswordEdit>

DWIDGET_USE_NAMESPACE

class MountSecretDiskAskPasswordDialog : public DDialog
{
    Q_OBJECT
public:
    explicit MountSecretDiskAskPasswordDialog(const QString& tipMessage, QWidget *parent = nullptr);
    ~MountSecretDiskAskPasswordDialog();

    void initUI();
    void initConnect();

    QString password() const;

    int passwordSaveMode() const;

public slots:
    void handleRadioButtonClicked(int index);
    void handleButtonClicked(int index, QString text);

protected:
    void showEvent(QShowEvent* event);

private:
    QString m_descriptionMessage = "";
    QLabel* m_titleLabel = nullptr;
    QLabel* m_descriptionLabel = nullptr;
    DPasswordEdit* m_passwordLineEdit = nullptr;

    QRadioButton* m_neverRadioCheckBox = nullptr;
    QRadioButton* m_sessionRadioCheckBox = nullptr;
    QRadioButton* m_forerverRadioCheckBox = nullptr;

    QButtonGroup* m_passwordButtonGroup = nullptr;


    QString m_password = "";
    int m_passwordSaveMode = 0;
};

#endif // MOUNTSECRETDISKASKPASSWORDDIALOG_H
