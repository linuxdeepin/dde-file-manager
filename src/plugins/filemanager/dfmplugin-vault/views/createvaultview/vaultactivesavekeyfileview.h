// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTACTIVESAVEKEYFILEVIEW_H
#define VAULTACTIVESAVEKEYFILEVIEW_H

#include "dfmplugin_vault_global.h"
#include "vaultbaseview.h"

#include <dtkwidget_global.h>
#include <DSuggestButton>
#include <DSpinner>

#include <QFrame>
#include <QPushButton>
#include <QRadioButton>
#include <QFutureWatcher>

DWIDGET_BEGIN_NAMESPACE
class DFileDialog;
class DLabel;
class DFileChooserEdit;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {

class VaultActiveSaveKeyFileView : public VaultBaseView
{
    Q_OBJECT
public:
    explicit VaultActiveSaveKeyFileView(QWidget *parent = nullptr);
    void setEncryptInfo(EncryptInfo &info) override;

    void setNextButtonText(const QString &text);
    void setOldPasswordSchemeMigrationMode(bool enabled);

Q_SIGNALS:
    void sigAccepted();

private:
    void initUI();
    void initUiForSizeMode();
    void initConnect();

public slots:
    /*!
     * /brief slotChangeEdit 处理保存密钥路径权限问题,并在UI上进行提示
     * /param fileName      选择保存密钥的路径
     */
    void slotChangeEdit(const QString &fileName);

    void slotSelectCurrentFile(const QString &file);
    void slotNextBtnClicked();

private slots:
    void onOldPasswordSchemeMigrationFinished();

protected:
    void showEvent(QShowEvent *event) override;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    //! 保存密钥文件页面标题
    DTK_WIDGET_NAMESPACE::DLabel *titleLabel { nullptr };
    //! 密钥文件安全提示
    DTK_WIDGET_NAMESPACE::DLabel *hintMsg { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *otherPathLabel { Q_NULLPTR };
    //! 用户自定义保存路径编辑框
    DTK_WIDGET_NAMESPACE::DFileChooserEdit *selectfileSavePathEdit { nullptr };
    //! 保存密钥文件并进行下一步操作的按钮
    DTK_WIDGET_NAMESPACE::DSuggestButton *nextBtn { nullptr };
    //! 保存密钥权限提示
    DTK_WIDGET_NAMESPACE::DLabel *otherRadioBtnHitMsg { nullptr };
    DTK_WIDGET_NAMESPACE::DFileDialog *filedialog { nullptr };

    bool isOldPasswordSchemeMigrationMode { false };
    DTK_WIDGET_NAMESPACE::DSpinner *spinner { nullptr };
    struct OldPasswordSchemeMigrationResult {
        bool success;
        bool unlocked;
    };
    QFutureWatcher<OldPasswordSchemeMigrationResult> *oldPasswordSchemeMigrationWatcher { nullptr };
};
}
#endif   // VAULTACTIVESAVEKEYFILEVIEW_H
