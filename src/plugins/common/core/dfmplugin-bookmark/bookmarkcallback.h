#ifndef BOOKMARK_CALLBACK_H
#define BOOKMARK_CALLBACK_H

#include <QObject>

namespace BookmarkCallBack {
void contextMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);
void renameCallBack(quint64 windowId, const QUrl &url, const QString &name);
void cdBookMarkUrlCallBack(quint64 windowId, const QUrl &url);
void cdDefaultItemUrlCallBack(quint64 windowId, const QUrl &url);
void bookMarkActionClickedCallBack(bool isNormal, const QUrl &currentUrl, const QUrl &focusFile, const QList<QUrl> &selected);
}   // namespace BookmarkCallBack

#endif   // BOOKMARK_CALLBACK_H
