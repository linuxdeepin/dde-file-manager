/*
* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
*
* Author: Liu Zhangjian<liuzhangjian@uniontech.com>
*
* Maintainer: Liu Zhangjian<liuzhangjian@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CHANGEDISKPASSWORDDIALOG_H
#define CHANGEDISKPASSWORDDIALOG_H

#include <DDialog>

class DiskInterface;
DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DFloatingWidget;
class DToolTip;
class DPictureSequenceView;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class WaitWork : public QWidget
{
    Q_OBJECT
public:
    explicit WaitWork(QWidget *parent = nullptr);
    ~WaitWork();

private:
    void initUI();
    void showAction();

    DPictureSequenceView *m_loadingIndicator = nullptr;
};

class ChangeDiskPasswordDialog : public DDialog
{
    Q_OBJECT
public:
    explicit ChangeDiskPasswordDialog(QWidget *parent = nullptr);
    virtual ~ChangeDiskPasswordDialog();

public slots:
    void onButtonClicked(const int& index);

private slots:
    void checkPassword(const QString &pwd);
    bool checkRepeatPassword();
    bool checkNewPassword();
    void onConfirmed(bool state);
    void onFinished(int code);

private:
    void initUI();
    void initConnect();
    void showToolTip(const QString &msg, QWidget *w);

    enum DialogButton {
        CancelButton,
        SaveButton
    };

private:
    DPasswordEdit *m_oldPwdEdit = nullptr;
    DPasswordEdit *m_newPwdEdit = nullptr;
    DPasswordEdit *m_repeatPwdEdit = nullptr;
    DFloatingWidget *m_toolTipFrame = nullptr;
    DToolTip *m_toolTip = nullptr;
    DiskInterface *m_diskInterface = nullptr;
    WaitWork *m_wait = nullptr;
};

#endif // CHANGEDISKPASSWORDDIALOG_H
