/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#ifndef DFMVAULTACTIVESAVEKEYVIEW_H
#define DFMVAULTACTIVESAVEKEYVIEW_H

#include <dtkwidget_global.h>

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QGridLayout;
class QPlainTextEdit;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DButtonBoxButton;
class DLabel;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE

/**
 * @brief The DFMVaultActiveSaveKeyView class 由于新需求UI界面改动很大，所以此类弃用
 */
class DFMVaultActiveSaveKeyView : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultActiveSaveKeyView(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

public slots:

private slots:
    void slotNextBtnClicked();
    void slotKeyBtnClicked();
    void slotQRCodeBtnClicked();

signals:
    void sigAccepted();

private:

private:
    DLabel              *m_pTipsLabel;
    DButtonBoxButton    *m_pKeyBtn;
    QPlainTextEdit      *m_pKeyText;
    DButtonBoxButton    *m_pQRCodeBtn;
    DLabel              *m_pQRCodeImage;
    DLabel              *m_pScanTipsLabel;
    QPushButton         *m_pNext;
    QGridLayout         *play1;
};

#endif // DFMVAULTACTIVESAVEKEYVIEW_H
