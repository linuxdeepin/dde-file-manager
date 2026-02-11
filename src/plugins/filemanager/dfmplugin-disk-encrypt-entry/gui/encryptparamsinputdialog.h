// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENCRYPTPARAMSINPUTDIALOG_H
#define ENCRYPTPARAMSINPUTDIALOG_H

#include "dfmplugin_disk_encrypt_global.h"

#include <dtkwidget_global.h>
#include <DDialog>

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DFileChooserEdit;
class DComboBox;
class DLineEdit;
DWIDGET_END_NAMESPACE

class QLabel;
class QStackedLayout;
class QLayout;

namespace dfmplugin_diskenc {

class EncryptParamsInputDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit EncryptParamsInputDialog(const QVariantMap &args, QWidget *parent = nullptr);
    disk_encrypt::DeviceEncryptParam getInputs();

protected:
    void initUi();
    void initConn();
    QWidget *createPasswordPage();
    QWidget *createExportPage();
    bool validatePassword();
    bool validateExportPath(const QString &path, QString *msg);
    void setPasswordInputVisible(bool visible);

protected Q_SLOTS:
    void onButtonClicked(int idx);
    void onPageChanged(int page);
    void onEncTypeChanged(int type);
    void onExpPathChanged(const QString &path, bool silent);

private:
    bool encryptByTpm(const QString &deviceName);
    void confirmEncrypt();

private:
    DTK_WIDGET_NAMESPACE::DComboBox *encType { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *encKeyEdit1 { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *encKeyEdit2 { nullptr };
    DTK_WIDGET_NAMESPACE::DFileChooserEdit *keyExportInput { nullptr };

    QLabel *keyHint1 { nullptr };
    QLabel *keyHint2 { nullptr };
    QLabel *unlockTypeHint { nullptr };
    QStackedLayout *pagesLay { nullptr };

private:
    bool expPathValid { false };
    bool exportRecKeyEnabled { false };
    QString tpmPassword;
    QVariantMap args;
};

}
#endif   // ENCRYPTPARAMSINPUTDIALOG_H
