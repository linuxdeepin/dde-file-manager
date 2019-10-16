/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef PROGRESSBOX_H
#define PROGRESSBOX_H

#include <DWaterProgress>
#include <QTimer>
#include <QVariantAnimation>

DWIDGET_USE_NAMESPACE

class ProgressBox : public DWaterProgress
{
    Q_OBJECT
public:
    explicit ProgressBox(QWidget *parent = nullptr);
    void initConnections();
    void initUI();

signals:
    void finished(const bool& successful);

public slots:
    void updateAnimation();
    void taskTimeOut();
    void startTask();
    void finishedTask(const bool result);

private:
    QTimer* m_taskTimer = nullptr;
    QTimer* m_updateTimer = nullptr;
    QVariantAnimation* m_taskAni = nullptr;
};

#endif // PROGRESSBOX_H
