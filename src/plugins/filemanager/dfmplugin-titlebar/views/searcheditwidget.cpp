// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searcheditwidget.h"
#include "utils/titlebarhelper.h"
#include "events/titlebareventcaller.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/dfmcustombuttons/customiconbutton.h>
#include <dfm-base/widgets/dfmcustombuttons/customdtoolbutton.h>

#include <DToolButton>
#include <DIconButton>
#include <DSearchEdit>
#include <DSpinner>
#include <DDialog>
#include <DGuiApplicationHelper>
#include <DPalette>

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QKeyEvent>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPTITLEBAR_USE_NAMESPACE

inline constexpr int kSearchEditMaxWidth { 240 };   // Maximum width of search box
inline constexpr int kSearchEditMediumWidth { 200 };   // Medium width of search box
inline constexpr int kWidthThresholdCollapse { 900 };   // Threshold width to collapse search box
inline constexpr int kWidthThresholdExpand { 1100 };   // Threshold width to expand search box

SearchEditWidget::SearchEditWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnect();
    searchEdit->lineEdit()->installEventFilter(this);
    searchEdit->installEventFilter(this);
    advancedButton->installEventFilter(this);
}

SearchEditWidget::~SearchEditWidget()
{
    if (delayTimer) {
        delayTimer->stop();
    }
}

void SearchEditWidget::activateEdit(bool setAdvanceBtn)
{
    if (!searchEdit || !advancedButton || !searchButton) {
        fmWarning() << "Cannot activate edit - one or more widgets are null";
        return;
    }

    if (parentWidget() && parentWidget()->width() >= kWidthThresholdExpand)
        setSearchMode(SearchMode::kExtraLarge);
    else
        setSearchMode(SearchMode::kExpanded);

    if (searchEdit->hasFocus() && setAdvanceBtn) {
        advancedButton->setChecked(!advancedButton->isChecked());
        TitleBarEventCaller::sendShowFilterView(this, advancedButton->isChecked());
    } else {
        searchEdit->lineEdit()->setFocus();
    }
}

void SearchEditWidget::deactivateEdit()
{
    if (!searchEdit || !advancedButton) {
        fmWarning() << "Cannot deactivate edit - searchEdit or advancedButton is null";
        return;
    }

    advancedButton->setChecked(false);
    advancedButton->setVisible(false);

    searchEdit->clearEdit();
    searchEdit->clearFocus();
    if (parentWidget())
        updateSearchEditWidget(parentWidget()->width());
}

bool SearchEditWidget::isAdvancedButtonVisible() const
{
    return advancedButton->isVisible();
}

bool SearchEditWidget::isAdvancedButtonChecked() const
{
    return advancedButton->isChecked();
}

void SearchEditWidget::setAdvancedButtonChecked(bool checked)
{
    advancedButton->setChecked(checked);
}

void SearchEditWidget::setAdvancedButtonVisible(bool visible)
{
    advancedButton->setVisible(visible);
}

void SearchEditWidget::updateSearchEditWidget(int parentWidth)
{
    if (parentWidth >= kWidthThresholdExpand) {
        setSearchMode(SearchMode::kExtraLarge);
    } else if (parentWidth > kWidthThresholdCollapse) {
        setSearchMode(SearchMode::kExpanded);
    } else {
        setSearchMode(SearchMode::kCollapsed);
    }
}

void SearchEditWidget::setSearchMode(SearchMode mode)
{
    if (advancedButton->isChecked() || searchEdit->hasFocus()) {
        fmDebug() << "Cannot change search mode - advanced button checked or search edit has focus";
        return;
    }

    currentMode = mode;
    updateSearchWidgetLayout();
}

void SearchEditWidget::setText(const QString &text)
{
    searchEdit->setText(text);
    pendingSearchText = text;

    // 如果文本不为空，触发搜索
    if (!text.isEmpty())
        delayTimer->start(determineSearchDelay(text));
}

void SearchEditWidget::onUrlChanged(const QUrl &url)
{
    if (TitleBarHelper::checkKeepTitleStatus(url)) {
        QUrlQuery query { url.query() };
        QString searchKey { query.queryItemValue("keyword", QUrl::FullyDecoded) };
        if (!searchKey.isEmpty()) {
            fmDebug() << "Found search keyword in URL:" << searchKey;
            activateEdit(false);
            if (searchKey != lastExecutedSearchText && searchKey != searchEdit->text()) {
                fmDebug() << "Setting search text from URL";
                searchEdit->setText(searchKey);
            }
        }
        return;
    }

    fmDebug() << "URL changed to non-search view, cleaning up search edit state";
    lastSearchTime = 0;
    lastExecutedSearchText.clear();
    searchEdit->clearEdit();
    if (delayTimer && delayTimer->isActive())
        delayTimer->stop();
    advancedButton->setVisible(false);
    advancedButton->setChecked(false);

    // Clear focus to allow mode change
    searchEdit->clearFocus();

    // Force update layout to collapse search edit
    if (parentWidget()) {
        updateSearchEditWidget(parentWidget()->width());
    }
}

void SearchEditWidget::onAdvancedButtonClicked()
{
    TitleBarEventCaller::sendShowFilterView(this, advancedButton->isChecked());
}

void SearchEditWidget::onTextEdited(const QString &text)
{
    lastEditedString = text;
    pendingSearchText = text;

    if (text.isEmpty()) {
        fmDebug() << "Search text is empty, stopping timer and stopping search";
        stopSearch();
        return;
    }

    delayTimer->stop();

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    bool isFirstInputAfterLongGap = (lastSearchTime != 0) && (currentTime - lastSearchTime) > 2000;   // 2秒的毫秒数
    if (isFirstInputAfterLongGap && !shouldDelaySearch(text)) {
        // 超过2秒后的第一次输入，立即触发搜索
        performSearch();
    } else {
        delayTimer->start(determineSearchDelay(text));
    }
}

void SearchEditWidget::expandSearchEdit()
{
    setSearchMode(SearchMode::kExpanded);
    searchEdit->lineEdit()->setFocus();
}

void SearchEditWidget::performSearch()
{
    currentCursorPos = searchEdit->lineEdit()->cursorPosition();
    if (pendingSearchText.isEmpty()) {
        fmDebug() << "Pending search text is empty, skipping search";
        return;
    }

    if (!TitleBarHelper::searchEnabled) {
        fmWarning() << "Search is disabled, cannot perform search";
        return;
    }

    // Trim whitespace from the search string
    QString trimmedSearchText = pendingSearchText.trimmed();
    if (trimmedSearchText.isEmpty()) {
        fmDebug() << "Trimmed search text is empty, skipping search";
        return;
    }

    // Check if this is the same as the last executed search
    if (trimmedSearchText == lastExecutedSearchText) {
        fmDebug() << "Search text unchanged from last search, skipping";
        return;
    }

    lastExecutedSearchText = trimmedSearchText;
    lastSearchTime = QDateTime::currentMSecsSinceEpoch();

    // 执行搜索
    TitleBarHelper::handleSearch(this, trimmedSearchText);
}

bool SearchEditWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == searchEdit->lineEdit()) {
        if (event->type() == QEvent::FocusOut) {
            handleFocusOutEvent(static_cast<QFocusEvent *>(event));
        } else if (event->type() == QEvent::FocusIn) {
            handleFocusInEvent(static_cast<QFocusEvent *>(event));
        } else if (event->type() == QEvent::InputMethod) {
            handleInputMethodEvent(static_cast<QInputMethodEvent *>(event));
        } else if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                fmDebug() << "ESC key pressed in search edit, quitting search";
                quitSearch();
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void SearchEditWidget::initUI()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // search button
    searchButton = new CustomDIconButton(this);
    searchButton->setIcon(QIcon::fromTheme("dfm_search_button"));
    searchButton->setFixedSize(kToolButtonSize, kToolButtonSize);
    searchButton->setIconSize(QSize(kToolButtonIconSize, kToolButtonIconSize));
    searchButton->setFocusPolicy(Qt::NoFocus);
    searchButton->setToolTip(QObject::tr("search"));
    searchButton->setFlat(true);
    searchButton->setVisible(false);

    // search edit
    searchEdit = new DSearchEdit(this);
    searchEdit->setVisible(true);
    // searchEdit->setFocusPolicy(Qt::StrongFocus);
    searchEdit->lineEdit()->setFocusPolicy(Qt::ClickFocus);

    // advanced search button
    advancedButton = new CustomDToolButton(this);
    advancedButton->setIcon(QIcon::fromTheme("dfm_view_filter"));
    advancedButton->setFixedSize(kToolButtonSize, kToolButtonSize);
    advancedButton->setFocusPolicy(Qt::NoFocus);
    advancedButton->setToolTip(QObject::tr("advanced search"));
    advancedButton->setCheckable(true);
    advancedButton->setVisible(false);

    layout->addWidget(searchButton);
    layout->addWidget(searchEdit);
    // Create a spacer item to control spacing dynamically
    spacingItem = new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
    layout->addItem(spacingItem);
    layout->addWidget(advancedButton);

    delayTimer = new QTimer(this);
    delayTimer->setSingleShot(true);
}

void SearchEditWidget::initConnect()
{
    connect(searchButton, &DIconButton::clicked, this, &SearchEditWidget::expandSearchEdit);
    connect(searchEdit, &DSearchEdit::textEdited, this, &SearchEditWidget::onTextEdited, Qt::QueuedConnection);
    connect(searchEdit, &DSearchEdit::searchAborted, this, &SearchEditWidget::quitSearch);
    connect(advancedButton, &DToolButton::clicked, this, &SearchEditWidget::onAdvancedButtonClicked);
    connect(delayTimer, &QTimer::timeout, this, &SearchEditWidget::performSearch);   // 连接计时器超时信号

    // fix bug#31692 搜索框输入中文后,全选已输入的,再次输入未覆盖之前的内容
    // 选中内容时，记录光标开始位置以及选中的长度
    connect(searchEdit, &DSearchEdit::selectionChanged, this, [=] {
        int posStart = searchEdit->lineEdit()->selectionStart();
        int posEnd = searchEdit->lineEdit()->selectionEnd();
        selectPosStart = posStart < posEnd ? posStart : posEnd;
        selectLength = searchEdit->lineEdit()->selectionLength();
    });
}

QString SearchEditWidget::text() const
{
    return searchEdit->text();
}

void SearchEditWidget::handleFocusInEvent(QFocusEvent *e)
{
    advancedButton->setVisible(true);
    updateSpacing(true);   // Advanced button is now visible
}

void SearchEditWidget::handleFocusOutEvent(QFocusEvent *e)
{
    fmDebug() << "Focus out event, reason:" << e->reason()
              << "hasText:" << !searchEdit->text().isEmpty()
              << "focusWidget:" << (QApplication::focusWidget() ? QApplication::focusWidget()->metaObject()->className() : "nullptr");

    if (searchEdit->lineEdit()->text().isEmpty() && !advancedButton->isChecked()) {
        advancedButton->setVisible(false);
        updateSpacing(false);   // Advanced button is now hidden
    }

    // Helper lambda to restore focus if needed
    auto restoreFocusIfNeeded = [this]() {
        if (!searchEdit->text().isEmpty()) {
            // Use QTimer to defer the focus restoration to ensure proper cursor blink
            QTimer::singleShot(0, this, [this]() {
                searchEdit->lineEdit()->setFocus(Qt::OtherFocusReason);
            });
        }
    };

    // Handle special focus reasons that should not trigger collapse
    if (e->reason() == Qt::PopupFocusReason || e->reason() == Qt::ActiveWindowFocusReason) {
        e->accept();
        restoreFocusIfNeeded();
        return;
    }

    // For Qt::OtherFocusReason, delay check to see if focus really moved away
    if (e->reason() == Qt::OtherFocusReason) {
        QTimer::singleShot(0, this, [this]() {
            if (!searchEdit->hasFocus() && !advancedButton->hasFocus() && parentWidget()) {
                updateSearchEditWidget(parentWidget()->width());
            }
        });
        e->accept();
        restoreFocusIfNeeded();
        return;
    }

    // Normal focus out - allow collapse
    if (parentWidget()) {
        updateSearchEditWidget(parentWidget()->width());
    }
}

void SearchEditWidget::handleInputMethodEvent(QInputMethodEvent *e)
{
    if (searchEdit->lineEdit()->hasSelectedText()) {
        // fix bug#31692 搜索框输入中文后,全选已输入的,再次输入未覆盖之前的内容
        int pos = selectPosStart;
        searchEdit->setText(lastEditedString.remove(selectPosStart, selectLength));
        // 设置光标到修改处
        searchEdit->lineEdit()->setCursorPosition(pos);
    }
}

void SearchEditWidget::updateSearchWidgetLayout()
{
    if (currentMode == SearchMode::kCollapsed && searchEdit->text().isEmpty()) {
        setFixedWidth(searchButton->width());
        searchEdit->setVisible(false);
        searchButton->setVisible(true);
        advancedButton->setVisible(false);
        updateSpacing(false);   // No advanced button in collapsed mode
    } else {
        int width = kSearchEditMediumWidth;
        if (currentMode == SearchMode::kExtraLarge)
            width = (parentWidget()->width() - kWidthThresholdExpand) + kSearchEditMediumWidth;
        setFixedWidth(qMin(width, kSearchEditMaxWidth));
        searchEdit->setVisible(true);
        searchButton->setVisible(false);

        bool shouldShowAdvancedButton = searchEdit->hasFocus() || !searchEdit->text().isEmpty();
        advancedButton->setVisible(shouldShowAdvancedButton);
        updateSpacing(shouldShowAdvancedButton);
    }
}

void SearchEditWidget::quitSearch()
{
    lastSearchTime = 0;
    delayTimer->stop();
    // deactivateEdit();
    Q_EMIT searchQuit();
}

void SearchEditWidget::stopSearch()
{
    lastSearchTime = 0;
    delayTimer->stop();
    Q_EMIT searchStop();
}

void SearchEditWidget::updateSpacing(bool showAdvancedButton)
{
    if (!spacingItem) {
        fmWarning() << "Cannot update spacing - spacingItem is null";
        return;
    }

    // Apply spacing based on advanced button visibility
    // 10px spacing when button is visible, 0px when hidden for consistent right margin
    int spacing = showAdvancedButton ? 10 : 0;
    spacingItem->changeSize(spacing, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);

    // Force layout update to ensure immediate visual effect
    if (layout()) {
        layout()->invalidate();
    }

    fmDebug() << "Updated spacing to" << spacing << "px, advancedButton visible:" << showAdvancedButton;
}

int SearchEditWidget::determineSearchDelay(const QString &inputText)
{
    // 基础等待时间
    int delay = 200;   // 毫秒

    // 获取输入文本的字节数
    int byteCount = inputText.toUtf8().size();

    // 针对短输入增加延迟
    if (byteCount <= 2) {
        delay += 150;

        if (inputText == ".")
            delay += 1000;
    } else if (byteCount >= 6) {   // 通常中文2字为6字节
        delay = 100;
    }

    return delay;
}

bool SearchEditWidget::shouldDelaySearch(const QString &inputText)
{
    // 对于过短或者通配符搜索，应该延迟
    return inputText.length() < 2 || inputText == "." || inputText == "*";
}
