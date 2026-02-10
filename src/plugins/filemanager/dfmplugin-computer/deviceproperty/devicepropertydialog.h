// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEPROPERTYVIEW_H
#define DEVICEPROPERTYVIEW_H

#include "dfmplugin_computer_global.h"
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/interfaces/fileinfo.h>
#include "devicebasicwidget.h"

#include <DDialog>
#include <DColoredProgressBar>

#include <QScrollArea>

const int EXTEND_FRAME_MAXHEIGHT = 160;

DWIDGET_BEGIN_NAMESPACE
class DArrowLineDrawer;
DWIDGET_END_NAMESPACE

namespace dfmplugin_computer {

class DFMRoundBackground : public QObject
{
    Q_OBJECT
public:
    DFMRoundBackground(QWidget *parent, int radius);
    ~DFMRoundBackground();

    virtual bool eventFilter(QObject *watched, QEvent *event);
};

class DevicePropertyDialog : public DDialog
{
    Q_OBJECT
public:
    explicit DevicePropertyDialog(QWidget *parent = nullptr);
    virtual ~DevicePropertyDialog() override;

private:
    void iniUI();

    int contentHeight() const;

    void handleHeight(int height);

    void setProgressBar(qint64 totalSize, qint64 freeSize, bool mounted);

    void setFileName(const QString &filename);

public slots:
    void setSelectDeviceInfo(const DeviceInfo &info);

    void insertExtendedControl(int index, QWidget *widget);

    void addExtendedControl(QWidget *widget);

signals:
    void closed(const QUrl &url);

protected:
    void showEvent(QShowEvent *event) override;

    void closeEvent(QCloseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

private:
    DTK_WIDGET_NAMESPACE::DLabel *deviceIcon { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *basicInfo { nullptr };
    DTK_WIDGET_NAMESPACE::DColoredProgressBar *devicesProgressBar { nullptr };
    QFrame *deviceNameFrame { nullptr };
    QVBoxLayout *deviceNameLayout { nullptr };
    DeviceBasicWidget *deviceBasicWidget { nullptr };
    QScrollArea *scrollArea { nullptr };
    QList<QWidget *> extendedControl {};
    QUrl currentFileUrl {};
};
}
#endif   // DEVICEPROPERTYVIEW_H
