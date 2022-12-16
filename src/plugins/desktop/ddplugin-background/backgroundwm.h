/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangtingwei<wangtingwei@uniontech.com>
 *
 * Maintainer: wangtingwei<wangtingwei@uniontech.com>
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
#ifndef BACKGROUNDWM_H
#define BACKGROUNDWM_H

#include "backgroundservice.h"

#include <DWindowManagerHelper>

#include <QObject>

class QGSettings;

DDP_BACKGROUND_BEGIN_NAMESPACE

class BackgroundWM : public BackgroundService
{
    Q_OBJECT
public:
    explicit BackgroundWM(QObject *parent = nullptr);
    ~BackgroundWM() override;
public:
    QString background(const QString &screen) override;
    QString getDefaultBackground() override;

protected:
    QString getBackgroundFromWm(const QString &screen);
    QString getBackgroundFromConfig(const QString &screen);

private slots:
    void onAppearanceValueChanged(const QString& key);

public:
    QGSettings *gsettings = nullptr;
};

DDP_BACKGROUND_END_NAMESPACE

#endif // BACKGROUNDWM_H
