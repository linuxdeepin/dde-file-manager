/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DFMVAULTACTIVESAVEKEYFILEVIEW_H
#define DFMVAULTACTIVESAVEKEYFILEVIEW_H

#include <DLabel>
#include <DFileChooserEdit>

#include <QWidget>
#include <QPushButton>
#include <QRadioButton>

DWIDGET_USE_NAMESPACE

class RadioFrame : public QFrame
{
    Q_OBJECT
public:
    explicit RadioFrame(QFrame *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class DFMVaultActiveSaveKeyFileView : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultActiveSaveKeyFileView(QWidget *parent = nullptr);

    void initUI();

signals:
    /**
     * @brief sigAccepted   下一步的信号
     */
    void sigAccepted();

public slots:
    /**
     * @brief slotNextBtnClicked 保存密钥文件
     */
    void slotNextBtnClicked();

    /**
     * @brief slotSelectRadioBtn  单选框选中槽函数
     * @param btn   选中的单选框按钮
     */
    void slotSelectRadioBtn(QAbstractButton *btn);

    /**
     * @brief slotChangeEdit 处理保存密钥路径权限问题,并在UI上进行提示
     * @param fileName      选择保存密钥的路径
     */
    void slotChangeEdit(const QString &fileName);

    void slotSelectCurrentFile(const QString &file);

protected:
    void showEvent(QShowEvent *event) override;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    //! 保存密钥文件页面标题
    DLabel *m_title = nullptr;
    //! 密钥文件安全提示
    DLabel *m_hintMsg = nullptr;
    //! 默认保存单选框按钮
    QRadioButton *m_defaultPathRadioBtn = nullptr;
    //! 用户自定义保存单选框按钮
    QRadioButton *m_otherPathRadioBtn = nullptr;
    //! 用户自定义保存路径编辑框
    DFileChooserEdit *m_SelectfileSavePathEdit = nullptr;
    //! 保存密钥文件并进行下一步操作的按钮
    QPushButton *m_pNext = nullptr;
    //! 保存密钥权限提示
    DLabel *m_otherRadioBtnHitMsg = nullptr;
};

#endif   // DFMVAULTACTIVESAVEKEYFILEVIEW_H
