// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEITEM_H
#define DEVICEITEM_H

#include "device/typedefines.h"

#include <QFrame>
#include <QMouseEvent>

class QLabel;
class QProgressBar;
class DeviceItem : public QFrame
{
    Q_OBJECT
public:
    explicit DeviceItem(const DockItemData &item, QWidget *parent = nullptr);
    static QFrame *createSeparateLine(int width);

protected:
    void mouseReleaseEvent(QMouseEvent *) override;
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;

public Q_SLOTS:
    void updateUsage(quint64 usedSize);

Q_SIGNALS:
    void requestEject(const QString &id);

private:
    void initUI();
    void openDevice();
    void updateDeviceName();
    static void setTextColor(QWidget *obj, int themeType, double alpha);

private:
    DockItemData data;

    QLabel *sizeLabel { nullptr };
    QLabel *nameLabel { nullptr };
    QProgressBar *sizeProgress { nullptr };
};

#endif   // DEVICEITEM_H
