// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CHGPASSPHRASEDIALOG_H
#define CHGPASSPHRASEDIALOG_H

#include "dfmplugin_disk_encrypt_global.h"

#include <ddialog.h>
#include <dpasswordedit.h>
#include <dcommandlinkbutton.h>

namespace dfmplugin_diskenc {

class ChgPassphraseDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    explicit ChgPassphraseDialog(const QString &device, QWidget *parent = nullptr);
    QPair<QString, QString> getPassphrase();
    bool validateByRecKey();

protected:
    void initUI();
    bool validatePasswd();

protected Q_SLOTS:
    void onButtonClicked(int idx);
    void onRecSwitchClicked();
    void onOldKeyChanged(const QString &inputs);

private:
    QString device;
    QString encType;
    bool usingRecKey { false };

    QLabel *oldKeyHint { nullptr };
    Dtk::Widget::DPasswordEdit *oldPass { nullptr };
    Dtk::Widget::DPasswordEdit *newPass1 { nullptr };
    Dtk::Widget::DPasswordEdit *newPass2 { nullptr };
    Dtk::Widget::DCommandLinkButton *recSwitch { nullptr };
};

}
#endif   // CHGPASSPHRASEDIALOG_H
