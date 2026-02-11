// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHEDITWIDGET_H
#define SEARCHEDITWIDGET_H

#include <dtkwidget_global.h>

#include <QWidget>
#include <QCompleter>
#include <QSpacerItem>

DWIDGET_BEGIN_NAMESPACE
class DSearchEdit;
class DToolButton;
class DSpinner;
class DIconButton;
DWIDGET_END_NAMESPACE

namespace dfmplugin_titlebar {

enum class SearchMode {
    kUnknown,
    kCollapsed,   // Collapsed mode, only show search button
    kExpanded,   // Expanded mode, show search box
    kExtraLarge,   // Extra large mode, show search box
};

class SearchEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SearchEditWidget(QWidget *parent = nullptr);
    ~SearchEditWidget();

    void activateEdit(bool setAdvanceBtn = true);
    void deactivateEdit();

    bool isAdvancedButtonVisible() const;
    bool isAdvancedButtonChecked() const;
    void setAdvancedButtonChecked(bool checked);
    void setAdvancedButtonVisible(bool visible);

    void updateSearchEditWidget(int parentWidth);
    void setSearchMode(SearchMode mode);

    QString text() const;
    void setText(const QString &text);

public Q_SLOTS:
    void onUrlChanged(const QUrl &url);
    void onAdvancedButtonClicked();
    void onTextEdited(const QString &text);
    void expandSearchEdit();
    void performSearch();

Q_SIGNALS:
    void searchQuit();
    void searchStop();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initUI();
    void initConnect();

    void handleFocusInEvent(QFocusEvent *e);
    void handleFocusOutEvent(QFocusEvent *e);
    void handleInputMethodEvent(QInputMethodEvent *e);

    void updateSearchWidgetLayout();
    void quitSearch();
    void stopSearch();

    /**
     * @brief Update spacing between searchEdit and advancedButton
     * @param showAdvancedButton Whether the advanced button should be visible
     *
     * This method ensures consistent 10px right margin by adjusting the internal
     * spacing based on advanced button visibility. When the button is visible,
     * 10px spacing is applied; when hidden, spacing is removed.
     */
    void updateSpacing(bool showAdvancedButton);

    int determineSearchDelay(const QString &inputText);
    bool shouldDelaySearch(const QString &inputText);

    DTK_WIDGET_NAMESPACE::DIconButton *searchButton { nullptr };   // 搜索栏按钮
    DTK_WIDGET_NAMESPACE::DToolButton *advancedButton { nullptr };   // 高级搜索按钮
    DTK_WIDGET_NAMESPACE::DSearchEdit *searchEdit { nullptr };
    QSpacerItem *spacingItem { nullptr };   // Dynamic spacing between searchEdit and advancedButton

    int selectPosStart { 0 };
    int selectLength { 0 };
    QString lastEditedString {};
    QString pendingSearchText {};
    QString lastExecutedSearchText {};
    int currentCursorPos { 0 };

    SearchMode currentMode { SearchMode::kUnknown };
    QTimer *delayTimer { nullptr };
    qint64 lastSearchTime { 0 };
};

}   // namespace dfmplugin_titlebar

#endif   // SEARCHEDITWIDGET_H
