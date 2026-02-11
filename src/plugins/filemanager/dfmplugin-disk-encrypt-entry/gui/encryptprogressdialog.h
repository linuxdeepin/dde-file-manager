// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENCRYPTPROGRESSDIALOG_H
#define ENCRYPTPROGRESSDIALOG_H

#include <DDialog>
#include <DWaterProgress>

DWIDGET_USE_NAMESPACE

class QStackedLayout;

namespace dfmplugin_diskenc {

class EncryptProgressDialog : public DDialog
{
    Q_OBJECT
public:
    explicit EncryptProgressDialog(QWidget *parent = nullptr);
    void setText(const QString &title, const QString &message);
    void updateProgress(double progress);
    void showResultPage(bool success, const QString &title, const QString &message);
    void showExportPage();
    inline void setRecoveryKey(const QString &key, const QString &device)
    {
        recKey = key;
        this->device = device;
    }

protected Q_SLOTS:
    void onCicked(int idx, const QString &btnTxt);

protected:
    void initUI();
    bool validateExportPath(const QString &path, QString *msg);
    void saveRecKey(const QString &path);

private:
    DWaterProgress *progress { nullptr };
    QLabel *message { nullptr };
    QStackedLayout *mainLay { nullptr };
    QLabel *iconLabel { nullptr };
    QLabel *resultMsg { nullptr };
    QLabel *warningLabel { nullptr };

    QString recKey;
    QString device;
};
}

#endif   // ENCRYPTPROGRESSDIALOG_H
