// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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

public Q_SLOTS:
    void updateUsage(quint64 usedSize);

Q_SIGNALS:
    void requestEject(const QString &id);

private:
    void initUI();
    void openDevice();
    static void setTextColor(QWidget *obj, int themeType, double alpha);
    static void setTextFont(QWidget *widget, int pixelSize, int weight);

private:
    DockItemData data;

    QLabel *sizeLabel { nullptr };
    QProgressBar *sizeProgress { nullptr };
};

#endif   // DEVICEITEM_H
