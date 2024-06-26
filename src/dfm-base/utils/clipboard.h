// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
class QMimeData;
class QUrl;
namespace dfmbase {
class ClipboardMonitor;
class ClipBoard : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ClipBoard)

public:
    enum ClipboardAction : uint8_t {
        kCutAction,
        kCopyAction,
        kDeleteAction,
        kRemoteAction,
        kRemoteCopiedAction,
        kUnknownAction = 255
    };

public:
    static ClipBoard *instance();
    static QList<QUrl> getRemoteUrls();

    static void clearClipboard();
    static void setUrlsToClipboard(const QList<QUrl> &list, ClipBoard::ClipboardAction action, QMimeData *mimeData = nullptr);
    static void setCurUrlToClipboardForRemote(const QUrl &curUrl);
    static void setDataToClipboard(QMimeData *mimeData);
    static bool supportCut();

    QList<QUrl> clipboardFileUrlList() const;
    ClipboardAction clipboardAction() const;
    void removeUrls(const QList<QUrl> &urls);
    void replaceClipboardUrl(const QUrl &oldUrl, const QUrl &newUrl);
    void readFirstClipboard();

private:
    explicit ClipBoard(QObject *parent = nullptr);
    void init();
    virtual ~ClipBoard() = default;
    static QList<QUrl> getUrlsByX11();
    QStringList getFirstMimeTypesByX11();

Q_SIGNALS:
    void clipboardDataChanged();

public Q_SLOTS:
    void onClipboardDataChanged(const QStringList & mimeTypes);
};
}   // namespace dfmbase
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::ClipBoard::ClipboardAction)
#endif   // CLIPBOARD_H
