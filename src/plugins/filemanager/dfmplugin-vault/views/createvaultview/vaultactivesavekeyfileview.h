// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTACTIVESAVEKEYFILEVIEW_H
#define VAULTACTIVESAVEKEYFILEVIEW_H

#include "dfmplugin_vault_global.h"
#include "vaultbaseview.h"

#include <dtkwidget_global.h>
#include <DSuggestButton>

#include <QFrame>
#include <QPushButton>
#include <QRadioButton>

DWIDGET_BEGIN_NAMESPACE
class DFileDialog;
class DLabel;
class DFileChooserEdit;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {
class RadioFrame : public QFrame
{
    Q_OBJECT
public:
    explicit RadioFrame(QFrame *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class VaultActiveSaveKeyFileView : public VaultBaseView
{
    Q_OBJECT
public:
    explicit VaultActiveSaveKeyFileView(QWidget *parent = nullptr);
    void setEncryptInfo(EncryptInfo &info) override;

private:
    void initUI();
    void initUiForSizeMode();
    void initConnect();

public slots:
    /*!
     * /brief slotSelectRadioBtn  单选框选中槽函数
     * /param btn   选中的单选框按钮
     */
    void slotSelectRadioBtn(QAbstractButton *btn);

    /*!
     * /brief slotChangeEdit 处理保存密钥路径权限问题,并在UI上进行提示
     * /param fileName      选择保存密钥的路径
     */
    void slotChangeEdit(const QString &fileName);

    void slotSelectCurrentFile(const QString &file);

protected:
    void showEvent(QShowEvent *event) override;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    //! 保存密钥文件页面标题
    DTK_WIDGET_NAMESPACE::DLabel *titleLabel { nullptr };
    //! 密钥文件安全提示
    DTK_WIDGET_NAMESPACE::DLabel *hintMsg { nullptr };
    //! 默认保存单选框按钮
    QRadioButton *defaultPathRadioBtn { nullptr };
    //! 用户自定义保存单选框按钮
    QRadioButton *otherPathRadioBtn { nullptr };
    //! 用户自定义保存路径编辑框
    DTK_WIDGET_NAMESPACE::DFileChooserEdit *selectfileSavePathEdit { nullptr };
    //! 保存密钥文件并进行下一步操作的按钮
    DTK_WIDGET_NAMESPACE::DSuggestButton *nextBtn { nullptr };
    //! 保存密钥权限提示
    DTK_WIDGET_NAMESPACE::DLabel *otherRadioBtnHitMsg { nullptr };
    QButtonGroup *group { nullptr };
    DTK_WIDGET_NAMESPACE::DFileDialog *filedialog { nullptr };
};
}
#endif   // VAULTACTIVESAVEKEYFILEVIEW_H
