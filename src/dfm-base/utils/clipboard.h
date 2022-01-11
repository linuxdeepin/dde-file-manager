/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
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
#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>
class QMimeData;
class QUrl;
DFMBASE_BEGIN_NAMESPACE
class ClipBoard : public QObject
{
    Q_OBJECT
public:
    enum ClipboardAction : uint8_t {
        kCutAction,
        kCopyAction,
        kDeleteAction,
        kRemoteAction,
        kUnknowAction = 255
    };

public:
    virtual ~ClipBoard() = default;
    static ClipBoard *instance();
    static void setUrlsToClipboard(const QList<QUrl> &list, ClipBoard::ClipboardAction action, QMimeData *mimeData = nullptr);
    static const QList<QUrl> fetchUrlsFromClipboard();
    static ClipBoard::ClipboardAction fetchClipboardAction();
    static void clearClipboard();
    static QList<QUrl> getRemoteUrls();

private:
    explicit ClipBoard(QObject *parent = nullptr);
    static QList<QUrl> getUrlsByX11();

signals:
    void clipboardDataChanged();
public slots:
    void onClipboardDataChanged();

private:
    static ClipBoard *self;
};
DFMBASE_END_NAMESPACE
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::ClipBoard::ClipboardAction)
#endif   // CLIPBOARD_H
