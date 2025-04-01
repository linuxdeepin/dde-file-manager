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
    if (searchTimer) {
        searchTimer->stop();
    }
}

void SearchEditWidget::activateEdit(bool setAdvanceBtn)
{
    if (!searchEdit || !advancedButton || !searchButton)
        return;

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
    if (!searchEdit || !advancedButton)
        return;

    advancedButton->setChecked(false);
    advancedButton->setVisible(false);

    searchEdit->clearEdit();
    searchEdit->clearFocus();
    if (parentWidget())
        updateSearchEditWidget(parentWidget()->width());
}

bool SearchEditWidget::isAdvancedButtonChecked() const
{
    return advancedButton->isChecked();
}

void SearchEditWidget::setAdvancedButtonChecked(bool checked)
{
    advancedButton->setVisible(checked);
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
    if (advancedButton->isChecked() || searchEdit->hasFocus())
        return;

    currentMode = mode;
    updateSearchWidgetLayout();
}

void SearchEditWidget::setText(const QString &text)
{
    searchEdit->setText(text);
    pendingSearchText = text;

    // 如果文本不为空，触发搜索
    if (!text.isEmpty())
        searchTimer->start();
}

void SearchEditWidget::onUrlChanged(const QUrl &url)
{
    if (TitleBarHelper::checkKeepTitleStatus(url)) {
        QUrlQuery query { url.query() };
        QString searchKey { query.queryItemValue("keyword", QUrl::FullyDecoded) };
        if (!searchKey.isEmpty()) {
            activateEdit(false);
            searchEdit->setText(searchKey);
        }
        return;
    }

    // deactivateEdit();
}

void SearchEditWidget::onAdvancedButtonClicked()
{
    TitleBarEventCaller::sendShowFilterView(this, advancedButton->isChecked());
}

void SearchEditWidget::onTextEdited(const QString &text)
{
    lastEditedString = text;
    pendingSearchText = text;

    // 重置计时器，合并连续输入
    searchTimer->start();

    // 如果文本为空，可以立即触发清除搜索
    if (text.isEmpty()) {
        searchTimer->stop();
        quitSearch();
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
    if (pendingSearchText.isEmpty())
        return;

    // 执行搜索
    TitleBarHelper::handleSearch(this, pendingSearchText);
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
    searchEdit->setFocusPolicy(Qt::StrongFocus);
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
    layout->addSpacing(10);
    layout->addWidget(advancedButton);

    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(500); // 设置 500ms 的延迟
}

void SearchEditWidget::initConnect()
{
    connect(searchButton, &DIconButton::clicked, this, &SearchEditWidget::expandSearchEdit);
    connect(searchEdit, &DSearchEdit::textEdited, this, &SearchEditWidget::onTextEdited, Qt::QueuedConnection);
    connect(searchEdit, &DSearchEdit::searchAborted, this, &SearchEditWidget::quitSearch);
    connect(advancedButton, &DToolButton::clicked, this, &SearchEditWidget::onAdvancedButtonClicked);
    connect(searchTimer, &QTimer::timeout, this, &SearchEditWidget::performSearch); // 连接计时器超时信号

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
}

void SearchEditWidget::handleFocusOutEvent(QFocusEvent *e)
{
    if (e->reason() == Qt::PopupFocusReason
        || e->reason() == Qt::ActiveWindowFocusReason
        || e->reason() == Qt::OtherFocusReason) {
        e->accept();
        if (!searchEdit->text().isEmpty())
            searchEdit->lineEdit()->setFocus(Qt::MouseFocusReason);
        return;
    }

    if (searchEdit->lineEdit()->text().isEmpty() && !advancedButton->isChecked()) {
        advancedButton->setVisible(false);
    }

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
    } else {
        int width = kSearchEditMediumWidth;
        if (currentMode == SearchMode::kExtraLarge)
            width = (parentWidget()->width() - kWidthThresholdExpand) + kSearchEditMediumWidth;
        setFixedWidth(qMin(width, kSearchEditMaxWidth));
        searchEdit->setVisible(true);
        searchButton->setVisible(false);
        advancedButton->setVisible(searchEdit->hasFocus() || !searchEdit->text().isEmpty());
    }
}

void SearchEditWidget::quitSearch()
{
    searchTimer->stop();
    // deactivateEdit();
    Q_EMIT searchQuit();
}
