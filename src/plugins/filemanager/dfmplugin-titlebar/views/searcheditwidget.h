// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHEDITWIDGET_H
#define SEARCHEDITWIDGET_H

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

enum class SearchMode {
    kUnknown,
    kCollapsed,  // Collapsed mode, only show search button
    kExpanded,    // Expanded mode, show search box
    kExtraLarge,  // Extra large mode, show search box
};

class SearchEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SearchEditWidget(QWidget *parent = nullptr);
    ~SearchEditWidget();

    void activateEdit(bool setAdvanceBtn = true);
    void deactivateEdit();

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

    DTK_WIDGET_NAMESPACE::DIconButton *searchButton { nullptr };   // 搜索栏按钮
    DTK_WIDGET_NAMESPACE::DToolButton *advancedButton { nullptr };   // 高级搜索按钮
    DTK_WIDGET_NAMESPACE::DSearchEdit *searchEdit { nullptr };

    int selectPosStart { 0 };
    int selectLength { 0 };
    QString lastEditedString {};
    QString pendingSearchText {};
    int currentCursorPos { 0 };

    SearchMode currentMode { SearchMode::kUnknown };
    QTimer *searchTimer { nullptr };
};

} // namespace dfmplugin_titlebar

#endif // SEARCHEDITWIDGET_H
