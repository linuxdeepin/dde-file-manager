// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILSPACEWIDGET_H
#define DETAILSPACEWIDGET_H

#include "dfmplugin_detailspace_global.h"
#include <dfm-base/interfaces/abstractframe.h>

#include <QPoint>
#include <QUrl>

class QMouseEvent;
class QEvent;

namespace dfmplugin_detailspace {

class DetailView;
class DetailSpaceWidget : public DFMBASE_NAMESPACE::AbstractFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(DetailSpaceWidget)

public:
    explicit DetailSpaceWidget(QFrame *parent = nullptr);
    void setCurrentUrl(const QUrl &url) override;
    void setCurrentUrl(const QUrl &url, int widgetFilter);
    QUrl currentUrl() const override;
    int detailWidth() const;

    bool insterExpandControl(const int &index, QWidget *widget);

    void removeControls();

    void prepareForAnimation(bool show);
    void handleAnimationFinished(bool show);
    void resetWidthRange();

private slots:
    void initUiForSizeMode();

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

    bool handleMousePress(QMouseEvent *event, const QPoint &localPos);
    bool handleMouseMove(QMouseEvent *event, const QPoint &localPos);
    bool handleMouseRelease(QMouseEvent *event, const QPoint &localPos);

    void initializeUi();
    void initConnect();
    void applyPreferredWidth();
    void setPreferredWidth(int width, bool fromUser);
    int defaultDetailWidth() const;
    int minDetailWidth() const;
    int maxDetailWidth() const;
    int clampWidth(int width) const;
    bool isOnResizeArea(const QPoint &pos) const;
    void updateCursorShape(bool inResizeArea);
    void notifyWorkspaceWidthDelta(int delta);
    QPoint mapFromChild(const QMouseEvent *event) const;
    int eventGlobalX(const QMouseEvent *event) const;

private:
    QUrl detailSpaceUrl;
    DetailView *detailView { nullptr };
    bool resizing { false };
    bool animationInProgress { false };
    int resizeStartGlobalX { 0 };
    int resizeStartWidth { 0 };
    int preferredWidth { 0 };
    bool userResized { false };

    static constexpr int kResizeHandleWidth { 8 };
};

}

#endif   // DETAILSPACEWIDGET_H
