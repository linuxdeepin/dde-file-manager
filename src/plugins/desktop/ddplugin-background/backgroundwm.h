// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    bool isWMActive() const;

private slots:
    void onAppearanceValueChanged(const QString& key);

public:
    QGSettings *gsettings = nullptr;
};

DDP_BACKGROUND_END_NAMESPACE

#endif // BACKGROUNDWM_H
