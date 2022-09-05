// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTOOLBAR_H
#define DTOOLBAR_H

#include <QFrame>
#include <QStackedWidget>

#include "dfileview.h"

class DFMEvent;
class HistoryStack;

QT_BEGIN_NAMESPACE
class QPushButton;
class QHBoxLayout;
class QToolButton;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DButtonBoxButton;
DWIDGET_END_NAMESPACE

DFM_BEGIN_NAMESPACE
class DFMCrumbBar;
class DFMAddressBar;
class DFMIconButton;
DFM_END_NAMESPACE

DFM_USE_NAMESPACE

class DToolBar : public QFrame
{
    Q_OBJECT
public:
    explicit DToolBar(QWidget *parent = nullptr);
    ~DToolBar();
    static const int ButtonWidth;
    static const int ButtonHeight;
    void initData();
    void initUI();
    void initAddressToolBar();
    void initContollerToolBar();
    void initConnect();
    DFMCrumbBar *getCrumbWidget();
    void addHistoryStack();

    int navStackCount() const;
    void updateBackForwardButtonsState();

    void setCustomActionList(const QList<QAction *> &list);
    void triggerActionByIndex(int index);
    void setSearchButtonVisible(bool visble);

signals:
    void refreshButtonClicked();
    void toolbarUrlChanged(const DUrl &url);
    void detailButtonClicked();

public slots:
    void searchBarActivated();
    void searchBarDeactivated();
    void searchBarTextEntered(const QString textEntered);
    void onSearchButtonClicked();
    void currentUrlChanged(const DFMEvent &event);

    void back();
    void forward();

    void handleHotkeyCtrlF(quint64 winId);
    void handleHotkeyCtrlL(quint64 winId);

    void moveNavStacks(int from, int to);
    void removeNavStackAt(int index);
    void switchHistoryStack(const int index );

private:
    void toggleSearchButtonState(bool asb = true);
    void pushUrlToHistoryStack(DUrl url);
    void onBackButtonClicked();
    void onForwardButtonClicked();
    void setEnterText(QString &text);

    QFrame *m_addressToolBar;
    DButtonBoxButton *m_backButton = nullptr;
    DButtonBoxButton *m_forwardButton = nullptr;
    QPushButton *m_searchButton = nullptr;
    QToolButton *m_detailButton = nullptr;
    QFrame *m_contollerToolBar;
    QHBoxLayout *m_contollerToolBarContentLayout;
    QList<QAction *> m_actionList;

    bool m_switchState = false;
    bool m_searchButtonAsbState = false;
    DFMCrumbBar *m_crumbWidget = nullptr;
    std::shared_ptr<HistoryStack> m_navStack = nullptr;
    QList<std::shared_ptr<HistoryStack>> m_navStacks;
};

#endif // DTOOLBAR_H
