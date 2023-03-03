// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTACTIVESAVEKEYFILEVIEW_H
#define VAULTACTIVESAVEKEYFILEVIEW_H

#include "dfmplugin_vault_global.h"

#include <DLabel>
#include <DFileChooserEdit>
#include <dtkwidget_global.h>

#include <QWidget>
#include <QPushButton>
#include <QRadioButton>

DWIDGET_USE_NAMESPACE
namespace dfmplugin_vault {
class RadioFrame : public QFrame
{
    Q_OBJECT
public:
    explicit RadioFrame(QFrame *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class VaultActiveSaveKeyFileView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultActiveSaveKeyFileView(QWidget *parent = nullptr);

    void initUI();

signals:
    /*!
     * /brief sigAccepted   下一步的信号
     */
    void sigAccepted();

public slots:
    /*!
     * /brief slotNextBtnClicked 保存密钥文件
     */
    void slotNextBtnClicked();

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
    DLabel *titleLabel { nullptr };
    //! 密钥文件安全提示
    DLabel *hintMsg { nullptr };
    //! 默认保存单选框按钮
    QRadioButton *defaultPathRadioBtn { nullptr };
    //! 用户自定义保存单选框按钮
    QRadioButton *otherPathRadioBtn { nullptr };
    //! 用户自定义保存路径编辑框
    DFileChooserEdit *selectfileSavePathEdit { nullptr };
    //! 保存密钥文件并进行下一步操作的按钮
    QPushButton *nextBtn { nullptr };
    //! 保存密钥权限提示
    DLabel *otherRadioBtnHitMsg { nullptr };
};
}
#endif   // VAULTACTIVESAVEKEYFILEVIEW_H
