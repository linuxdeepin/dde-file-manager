/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef AddressBar_P_H
#define AddressBar_P_H

#include "dfmplugin_titlebar_global.h"
#include "views/addressbar.h"
#include "views/completerview.h"

#include "dfm-base/base/urlroute.h"

#include <DSpinner>
#include <DAnchors>
#include <DIconButton>

#include <QFileInfo>
#include <QDir>
#include <QEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QHideEvent>
#include <QApplication>
#include <QToolButton>
#include <QTimer>
#include <QVariantAnimation>
#include <QPalette>
#include <QAction>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

DPTITLEBAR_BEGIN_NAMESPACE
class CrumbInterface;
class AddressBarPrivate : public QObject
{
    Q_OBJECT
    friend class AddressBar;
    AddressBar *const q;
    QStringList historyList;
    QTimer timer;
    DSpinner spinner;
    DIconButton *pauseButton;
    QVariantAnimation animation;
    QString placeholderText { tr("Search or enter address") };
    QAction indicatorAction;
    QAction clearAction;
    QString completerBaseString;
    QString lastEditedString;
    AddressBar::IndicatorType indicatorType { AddressBar::IndicatorType::Search };
    bool isHistoryInCompleterModel { false };
    int lastPressedKey { Qt::Key_D };   // just an init value
    int lastPreviousKey { Qt::Key_Control };   //记录上前一个按钮
    bool isKeyPressed { false };
    CrumbInterface *crumbController { nullptr };
    QStringListModel completerModel;
    CompleterView *completerView { nullptr };
    QCompleter *urlCompleter { nullptr };
    // inputMethodEvent中获取不到选中的内容，故缓存光标开始位置以及选中长度
    int selectPosStart { 0 };
    int selectLength { 0 };

public:
    explicit AddressBarPrivate(AddressBar *qq);
    void initializeUi();
    void initConnect();
    void initData();
    void setIndicator(enum AddressBar::IndicatorType type);
    void setCompleter(QCompleter *c);
    void clearCompleterModel();
    void updateCompletionState(const QString &text);
    void doComplete();
    void requestCompleteByUrl(const QUrl &url);

public Q_SLOTS:
    void startSpinner();
    void stopSpinner();
    void onTextEdited(const QString &text);
    void onReturnPressed();
    void insertCompletion(const QString &completion);
    void onCompletionHighlighted(const QString &highlightedCompletion);
    void updateIndicatorIcon();
    void onCompletionModelCountChanged();
    void appendToCompleterModel(const QStringList &stringList);
    void onTravelCompletionListFinished();
    void onIndicatorTriggerd();

protected:
    virtual bool eventFilterResize(AddressBar *addressbar, QResizeEvent *event);
    virtual bool eventFilterHide(AddressBar *addressbar, QHideEvent *event);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

DPTITLEBAR_END_NAMESPACE

#endif   //AddressBar_P_H
