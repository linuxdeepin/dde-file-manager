// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEPREVIEWINTERFACE_H
#define FILEPREVIEWINTERFACE_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QUrl>

namespace dfmbase {
class AbstractBasePreview : public QObject
{
    Q_OBJECT
public:
    explicit AbstractBasePreview(QObject *parent = nullptr);

    virtual void initialize(QWidget *window, QWidget *statusBar);
    virtual bool setFileUrl(const QUrl &url) = 0;
    virtual QUrl fileUrl() const = 0;

    virtual QWidget *contentWidget() const = 0;
    virtual QWidget *statusBarWidget() const;
    virtual Qt::Alignment statusBarWidgetAlignment() const;

    virtual QString title() const;
    virtual bool showStatusBarSeparator() const;

    virtual void play(); /*play media if file is music or video*/
    virtual void pause(); /*pause playing if file is music or video*/
    virtual void stop(); /*stop playing if file is music or video*/

    virtual void handleBeforDestroy();

Q_SIGNALS:
    void titleChanged();
};
}
#endif   // FILEPREVIEWINTERFACE_H
