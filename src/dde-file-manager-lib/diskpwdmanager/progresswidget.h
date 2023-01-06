// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include <DWidget>

DWIDGET_BEGIN_NAMESPACE
class DWaterProgress;
class DLabel;
DWIDGET_END_NAMESPACE

class QTimer;
class DiskInterface;

class ProgressWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit ProgressWidget(QWidget *parent = nullptr);

    void start();

Q_SIGNALS:
    void sigChangeFinished(bool success, const QString &msg);

private Q_SLOTS:
    void changeProgressValue();
    void onFinished(int result);

private:
    void initUI();
    void initConnect();

private:
    DTK_WIDGET_NAMESPACE::DWaterProgress *m_progress { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *m_titleLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *m_msgLabel { nullptr };

    QTimer *m_progressTimer { nullptr };
    DiskInterface *m_diskInterface { nullptr };
};

#endif   // PROGRESSWIDGET_H
