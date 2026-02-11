// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HEADERVIEW_H
#define HEADERVIEW_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QHeaderView>

namespace dfmplugin_workspace {

class FileView;
class FileViewModel;
class HeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit HeaderView(Qt::Orientation orientation, FileView *parent = nullptr);
    QSize sizeHint() const override;
    using QHeaderView::updateGeometries;

    int sectionsTotalWidth() const;
    void updateColumnWidth();
    void doFileNameColumnResize(const int totalWidth);

public slots:
    void onActionClicked(const int column, QAction *action);
    void syncOffset(int value);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *e) override;
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    bool event(QEvent *e) override;

Q_SIGNALS:
    void mousePressed();
    void mouseReleased();
    void viewResized();
    void hiddenSectionChanged(const QString &roleName, const bool checked);

private:
    FileViewModel *viewModel() const;
    QString sectionElidedName(int logicalIndex, int availableWidth) const;
    QString sectionName(int logicalIndex) const;

    FileView *view { nullptr };
    int firstVisibleColumn = -1;
    int lastVisibleColumn = -1;
    bool isChangeCursorState = false;
};

}

#endif   // DFMHEADERVIEW_H
