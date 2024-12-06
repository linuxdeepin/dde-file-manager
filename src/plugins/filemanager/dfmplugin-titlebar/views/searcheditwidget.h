// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHEDITWIDGET_H
#define SEARCHEDITWIDGET_H

#include "dfmplugin_titlebar_global.h"

#include <dtkwidget_global.h>

#include <QWidget>
#include <QCompleter>

DWIDGET_BEGIN_NAMESPACE
class DSearchEdit;
class DToolButton;
class DSpinner;
class DIconButton;
DWIDGET_END_NAMESPACE

namespace dfmplugin_titlebar {

namespace DConfigSearch {
inline constexpr char kSearchCfgPath[] { "org.deepin.dde.file-manager.search" };
inline constexpr char kDisplaySearchHistory[] = "displaySearchHistory";
}

enum class SearchMode {
    kUnknown,
    kCollapsed,  // Collapsed mode, only show search button
    kExpanded,    // Expanded mode, show search box
    kExtraLarge,  // Extra large mode, show search box
};

class CompleterViewModel;
class CompleterView;
class CompleterViewDelegate;
class SearchEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SearchEditWidget(QWidget *parent = nullptr);

    void activateEdit(bool setAdvanceBtn = true);
    void deactivateEdit();

    void startSpinner();
    void stopSpinner();

    void setAdvancedButtonVisible(bool visible);
    bool completerViewVisible();

    void updateSearchEditWidget(int parentWidth);
    void setSearchMode(SearchMode mode);

public Q_SLOTS:
    void onUrlChanged(const QUrl &url);
    void onPauseButtonClicked();
    void onAdvancedButtonClicked();
    void onReturnPressed();
    void onTextEdited(const QString &text);
    void onClearSearchHistory(quint64 winId);
    void onDConfigValueChanged(const QString &config, const QString &key);

    void insertCompletion(const QString &completion);
    void onCompletionHighlighted(const QString &highlightedCompletion);
    void onCompletionModelCountChanged();
    void appendToCompleterModel(const QStringList &stringList);

    void expandSearchEdit();

Q_SIGNALS:
    void searchQuit();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void initUI();
    void initConnect();
    void initData();
    void updateHistory();

    int showClearSearchHistory();
    void clearSearchHistory();

    QString text() const;

    void setCompleter(QCompleter *c);
    void clearCompleterModel();
    void updateCompletionState(const QString &text);
    void doComplete();
    void completeSearchHistory(const QString &text);
    void filterHistory(const QString &text);

    bool handleKeyPress(QKeyEvent *keyEvent);
    void handleFocusInEvent(QFocusEvent *e);
    void handleFocusOutEvent(QFocusEvent *e);
    void handleInputMethodEvent(QInputMethodEvent *e);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    void handleEnterEvent(QEvent *e);
#else
    void handleEnterEvent(QEnterEvent *e);
#endif
    void handleLeaveEvent(QEvent *e);

    void updateSearchWidgetLayout();
    void quitSearch();

    DTK_WIDGET_NAMESPACE::DIconButton *searchButton { nullptr };   // 搜索栏按钮
    DTK_WIDGET_NAMESPACE::DToolButton *advancedButton { nullptr };   // 高级搜索按钮
    DTK_WIDGET_NAMESPACE::DSearchEdit *searchEdit { nullptr };

    QStringList historyList {};
    QStringList showHistoryList {};
    bool isHistoryInCompleterModel { false };
    bool isClearSearch { false };
    int lastPressedKey { Qt::Key_D };   // just an init value
    int lastPreviousKey { Qt::Key_Control };   //记录上前一个按钮
    int selectPosStart { 0 };
    int selectLength { 0 };
    bool isKeyPressed { false };
    QString completerBaseString {};
    QString completionPrefix {};
    QString lastEditedString {};

    DTK_WIDGET_NAMESPACE::DSpinner *spinner { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *pauseButton { nullptr };

    CompleterViewModel *completerModel { nullptr };
    CompleterView *completerView { nullptr };
    QCompleter *urlCompleter { nullptr };
    CompleterViewDelegate *cpItemDelegate { nullptr };

    SearchMode currentMode { SearchMode::kUnknown };
};

} // namespace dfmplugin_titlebar

#endif // SEARCHEDITWIDGET_H
