// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DECRYPTPARAMSINPUTDIALOG_H
#define DECRYPTPARAMSINPUTDIALOG_H

#include <ddialog.h>
#include <dpasswordedit.h>
#include <dcommandlinkbutton.h>

namespace dfmplugin_diskenc {

class DecryptParamsInputDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    explicit DecryptParamsInputDialog(const QString &device, QWidget *parent = nullptr);
    QString getKey();
    void setInputPIN(bool);
    bool usingRecKey();

protected:
    void onRecSwitchClicked();
    void onKeyChanged(const QString &key);
    void onButtonClicked(int idx);
    void updateUserHints();

protected:
    void initUI();

private:
    QString devDesc;
    QString passphrase;
    bool useRecKey { false };
    bool requestPIN { false };

    Dtk::Widget::DPasswordEdit *editor { nullptr };
    Dtk::Widget::DCommandLinkButton *recSwitch { nullptr };
};

}
#endif   // DECRYPTPARAMSINPUTDIALOG_H
