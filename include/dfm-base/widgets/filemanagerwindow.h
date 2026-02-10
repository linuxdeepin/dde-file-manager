// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGERWINDOW_H
#define FILEMANAGERWINDOW_H

#include <dfm-base/dfm_base_global.h>

#include <DMainWindow>
#include <QList>
#include <QUrl>
#include <QVariantHash>

DWIDGET_USE_NAMESPACE

namespace dfmbase {

// DetailSpace options keys
namespace DetailSpaceOptions {
inline constexpr char kAnimated[] = "animated";
inline constexpr char kUserAction[] = "userAction";   // Whether this is a user-initiated action
}

class AbstractFrame;
class FileManagerWindowPrivate;
class FileManagerWindow : public DMainWindow
{
    Q_OBJECT
    friend class FileManagerWindowPrivate;

public:
    explicit FileManagerWindow(const QUrl &url, QWidget *parent = nullptr);
    virtual ~FileManagerWindow() override;

    virtual void cd(const QUrl &url);
    virtual bool saveClosedSate() const;

    QUrl currentUrl() const;
    void moveCenter();
    void installTitleBar(AbstractFrame *w);
    void installSideBar(AbstractFrame *w);
    void installWorkSpace(AbstractFrame *w);
    void installDetailView(AbstractFrame *w);

    AbstractFrame *titleBar() const;
    AbstractFrame *sideBar() const;
    AbstractFrame *workSpace() const;
    AbstractFrame *detailView() const;

    // DetailSpace width management
    void setDetailViewWidth(int width);
    int detailViewWidth() const;
    // DetailSpace visibility management with animation support
    void showDetailSpace(const QVariantHash &options = QVariantHash());
    void hideDetailSpace(const QVariantHash &options = QVariantHash());
    bool isDetailSpaceVisible() const;

    void loadState();
    void saveState();

protected:
    void closeEvent(QCloseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override;

Q_SIGNALS:
    void aboutToOpen();
    void aboutToClose();
    void positionChanged(const QPoint &pos);
    void currentUrlChanged(const QUrl &url);
    void currentViewStateChanged();
    void selectUrlChanged(const QList<QUrl> &urlList);
    void aboutToPlaySplitterAnimation(int startValue, int endValue);
    void windowSplitterWidthChanged(const QVariant &value);
    void windowActived();

    // The following signals are used to send shortcut requests
    void reqRefresh();   // F5
    void reqActivateNextTab();   // ctrl + Tab
    void reqActivatePreviousTab();   // ctrl + {shift + } BackTab
    void reqSearchCtrlF();   // ctrl + F
    void reqSearchCtrlL();   // ctrl + L
    void reqBack();   // ctrl / alt + Left
    void reqForward();   // ctrl / alt + Right
    void reqCloseCurrentTab();   // ctrl + W
    void reqCreateTab();   // ctrl + T
    void reqCreateWindow();   // ctrl + N
    void reqTriggerActionByIndex(int index);   // ctrl + [1, 8]
    void reqActivateTabByIndex(int index);   // alt + [1, 8]
    void reqShowHotkeyHelp();   // ctrl + shift + ?

    void titleBarInstallFinished();
    void sideBarInstallFinished();
    void workspaceInstallFinished();
    void detailViewInstallFinished();
    void detailSpaceVisibilityChanged(bool visible);

private:
    void initializeUi();
    void updateUi();

private:
    QScopedPointer<FileManagerWindowPrivate> d;
};

}

#endif   // FILEMANAGERWINDOW_H
