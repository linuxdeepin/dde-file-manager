// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PWDCHANGEPROGRESSVIEW_H
#define PWDCHANGEPROGRESSVIEW_H

#include <DWidget>

DWIDGET_BEGIN_NAMESPACE
class DWaterProgress;
class DLabel;
DWIDGET_END_NAMESPACE

class QStackedWidget;
class PwdConfirmWidget;

class ProgressWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit ProgressWidget(QWidget *parent = nullptr);

private:
    void initUI();

private:
    DTK_WIDGET_NAMESPACE::DWaterProgress *m_progress { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *m_titleLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *m_msgLabel { nullptr };
};

class ChangeResultWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit ChangeResultWidget(QWidget *parent = nullptr);

private:
    void initUI();

private:
    DTK_WIDGET_NAMESPACE::DLabel *m_resultIcon { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *m_titleLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *m_msgLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *m_closeBtn { nullptr };
};

class SwitchPageWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    enum PageType {
        ConfirmPage = 0,
        ProgressPage,
        SuccessedPage,
        FailedPage
    };

    explicit SwitchPageWidget(QWidget *parent = nullptr);

    void switchPage(PageType type);
    void setMessage(const QString &msg);
    bool checkPassword();

signals:
    void changePasswordFinished(bool success, const QString &msg);

private:
    void initUI();
    void initConnect();

    void switchToConfirmPage();
    void switchToProgressPage();
    void switchToSuccessedPage();
    void switchToFailedPage();

private:
    DTK_WIDGET_NAMESPACE::DWaterProgress *m_progress { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *m_resultIcon { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *m_msgLabel { nullptr };
    QStackedWidget *m_switchWidget { nullptr };
    PwdConfirmWidget *m_confirmWidget { nullptr };
};

#endif   // PWDCHANGEPROGRESSVIEW_H
