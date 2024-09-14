// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AddressBar_P_H
#define AddressBar_P_H

#include "dfmplugin_titlebar_global.h"
#include "views/addressbar.h"
#include "views/completerview.h"
#include "views/completerviewdelegate.h"
#include "models/completerviewmodel.h"

#include <dfm-base/base/urlroute.h>

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

namespace DConfigSearch {
inline constexpr char kSearchCfgPath[] { "org.deepin.dde.file-manager.search" };
inline constexpr char kDisplaySearchHistory[] = "displaySearchHistory";
}

namespace dfmplugin_titlebar {
class CrumbInterface;
class AddressBarPrivate : public QObject
{
    Q_OBJECT
    friend class AddressBar;
    AddressBar *const q;
    QStringList historyList;
    QStringList showHistoryList;
    QList<IPHistroyData> ipHistroyList;
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
    int lastPressedKey { Qt::Key_D };   // just an init value
    int lastPreviousKey { Qt::Key_Control };   //记录上前一个按钮
    int selectPosStart { 0 };
    CrumbInterface *crumbController { nullptr };
    CompleterViewModel completerModel;
    CompleterView *completerView { nullptr };
    QCompleter *urlCompleter { nullptr };
    CompleterViewDelegate *cpItemDelegate { nullptr };
    // inputMethodEvent中获取不到选中的内容，故缓存光标开始位置以及选中长度
    int selectLength { 0 };
    bool isKeepVisible { false };
    bool isClearSearch { false };
    bool isKeyPressed { false };
    bool isHistoryInCompleterModel { false };
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QRegExp ipRegExp;   // 0.0.0.0-255.255.255.255
    QRegExp protocolIPRegExp;   // smb://ip, ftp://ip, sftp://ip
#else
    QRegularExpression ipRegExp;   // 0.0.0.0-255.255.255.255
    QRegularExpression protocolIPRegExp;   // smb://ip, ftp://ip, sftp://ip
#endif
    QString completionPrefix;
    bool inputIsIpAddress { false };

public:
    explicit AddressBarPrivate(AddressBar *qq);
    void initializeUi();
    void initConnect();
    void initUiForSizeMode();
    void initData();
    void updateHistory();
    void setIndicator(enum AddressBar::IndicatorType type);
    void setCompleter(QCompleter *c);
    void clearCompleterModel();
    void updateCompletionState(const QString &text);
    void doComplete();
    void requestCompleteByUrl(const QUrl &url);

    void completeSearchHistory(const QString &text);
    void completeIpAddress(const QString &text);
    void completeLocalPath(const QString &text, const QUrl &url, int slashIndex);

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
    void onDConfigValueChanged(const QString &config, const QString &key);
    void filterHistory(const QString &text);
    int showClearSearchHistory();
    void onClearSearchHistory();

protected:
    virtual bool eventFilterResize(AddressBar *addressbar, QResizeEvent *event);
    virtual bool eventFilterHide(AddressBar *addressbar, QHideEvent *event);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

}

#endif   //AddressBar_P_H
