// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUNTSECRETDISKASKPASSWORDDIALOG_H
#define MOUNTSECRETDISKASKPASSWORDDIALOG_H

#include <DDialog>

class QLineEdit;
class QLabel;

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DCommandLinkButton;
DWIDGET_END_NAMESPACE

namespace dfmplugin_diskenc {
class UnlockPartitionDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    enum UnlockType {
        kPwd,
        kPin,
        kRec
    };
    explicit UnlockPartitionDialog(UnlockType type, QWidget *parent = nullptr);
    ~UnlockPartitionDialog();

    QPair<UnlockType, QString> getUnlockKey() const;

protected Q_SLOTS:
    void handleButtonClicked(int index, QString text);
    void switchUnlockType();
    void updateUserHint();

protected:
    void showEvent(QShowEvent *event);
    void initUI();
    void initConnect();

private:
    DTK_WIDGET_NAMESPACE::DPasswordEdit *passwordLineEdit = nullptr;
    Dtk::Widget::DCommandLinkButton *chgUnlockType = nullptr;
    QString key = "";
    UnlockType currType { kPin };
    UnlockType initType { kPin };
};
}

#endif   // MOUNTSECRETDISKASKPASSWORDDIALOG_H
