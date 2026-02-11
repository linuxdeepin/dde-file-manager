// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include "dfmplugin_utils_global.h"

#include <DDialog>

#include <QStackedWidget>

DWIDGET_BEGIN_NAMESPACE
class DWaterProgress;
class DLabel;
DWIDGET_END_NAMESPACE

namespace dfmplugin_utils {

class ProgressWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressWidget(QWidget *parent = Q_NULLPTR);
    void setValue(int value, const QString &msg);
    void stopProgress();

private:
    DTK_WIDGET_NAMESPACE::DWaterProgress *progress { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *infoLable { Q_NULLPTR };
};

class ShredFailedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ShredFailedWidget(QWidget *parent = Q_NULLPTR);

    void setMessage(const QString &msg);

private:
    DTK_WIDGET_NAMESPACE::DLabel *infoLabel { Q_NULLPTR };
};

class ProgressDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = Q_NULLPTR);

public Q_SLOTS:
    void updateProgressValue(int value, const QString &fileName);
    void handleShredResult(bool result, const QString &reason);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private Q_SLOTS:
    void handleButtonClicked(int index, const QString &text);

private:
    void initUi();
    void initConnect();

    ProgressWidget *proWidget { Q_NULLPTR };
    ShredFailedWidget *failedWidget { Q_NULLPTR };
    QStackedWidget *stackedWidget { Q_NULLPTR };
};
}

#endif   // PROGRESSDIALOG_H
