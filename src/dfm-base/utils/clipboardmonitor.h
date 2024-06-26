// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CLIPBOARDMONITOR_H
#define CLIPBOARDMONITOR_H

#include <QObject>
#include <QThread>
#include <xcb/xcb.h>

#include <dfm-base/dfm_base_global.h>
namespace dfmbase {
class ClipboardMonitor : public QThread
{
    Q_OBJECT

public:
    ClipboardMonitor(QObject *parent = nullptr);
    ~ClipboardMonitor() override;

Q_SIGNALS:
    void clipboardChanged(const QStringList &mimeTypes);
private:
    void stop();
protected:
    void run() override;

private:
    xcb_connection_t *connection;
    xcb_screen_t *screen;
    const xcb_query_extension_reply_t *m_queryExtension = nullptr;
    std::atomic_bool stoped{false};
};
}   // namespace dfmbase
#endif // CLIPBOARDMONITOR_H
