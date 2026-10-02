// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OFDSIDEBAR_H
#define OFDSIDEBAR_H

#include <QHash>
#include <QSharedPointer>
#include <QWidget>

#include <atomic>

class QListWidget;

namespace plugin_filepreview {

/**
 * @brief Left-hand page thumbnail panel. Renders every page of the document
 * into small preview icons in one background job; clicking an icon asks the
 * preview to jump to that page.
 */
class OfdSidebar : public QWidget
{
    Q_OBJECT
public:
    explicit OfdSidebar(QWidget *parent = nullptr);
    ~OfdSidebar() override;

    void init(int pageCount, const QString &filePath);
    void setCurrentPage(int index);

Q_SIGNALS:
    void pageClicked(int index);

private:
    void startThumbnailJob();
    void applyItemIcon(int index);

    QListWidget *list = nullptr;
    QString filePath;
    QHash<int, QImage> pageThumbs;
    int pages = 0;
    int currentPageIdx = 0;
    int thumbGeneration = 0;
    QSharedPointer<std::atomic_bool> cancelFlag;
};
}

#endif   // OFDSIDEBAR_H
