// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/private/addressbar_p.h"
#include "views/addressbar.h"
#include "utils/crumbmanager.h"
#include "utils/crumbinterface.h"
#include "utils/searchhistroymanager.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QCompleter>

using namespace dfmplugin_titlebar;

/*!
 * \class AddressBarPrivate
 * \brief parent
 */

AddressBarPrivate::AddressBarPrivate(AddressBar *qq)
    : QObject(qq),
      q(qq)
{
    initializeUi();
    initConnect();
    initData();
    qq->installEventFilter(this);
}

void AddressBarPrivate::initializeUi()
{
    // pause button
    pauseButton = new DIconButton(q);
    pauseButton->setIcon(QIcon::fromTheme("dfm_search_pause"));
    pauseButton->setFocusPolicy(Qt::NoFocus);
    pauseButton->setCursor({ Qt::ArrowCursor });
    pauseButton->setFlat(true);
    pauseButton->setVisible(false);

    // 左侧Action按钮 设置
    q->addAction(&indicatorAction, QLineEdit::LeadingPosition);

    // Clear text button
    q->setClearButtonEnabled(true);

    // Completer List
    completerView = new CompleterView(q);

    timer.setInterval(200);
    timer.setSingleShot(true);

    animation.setParent(q);
    animation.setDuration(616);
    animation.setEasingCurve(QEasingCurve::OutQuad);
    animation.setStartValue(QVariant(1.0f));
    animation.setEndValue(QVariant(0.0f));

    spinner.setParent(q);
    spinner.setAttribute(Qt::WA_TransparentForMouseEvents);
    spinner.setFocusPolicy(Qt::NoFocus);
    spinner.hide();

    q->setMinimumWidth(1);
    q->setAlignment(Qt::AlignHCenter);
    q->setAlignment(Qt::AlignLeft);
    q->setFocusPolicy(Qt::ClickFocus);

    initUiForSizeMode();
}

void AddressBarPrivate::initConnect()
{
    connect(&indicatorAction, &QAction::triggered, this, &AddressBarPrivate::onIndicatorTriggerd);

    connect(&animation, &QVariantAnimation::valueChanged,
            q, QOverload<>::of(&AddressBar::update));

    connect(&timer, &QTimer::timeout, &animation, [=]() {
        animation.start();
        //    q->update();
    });

    connect(q, &QLineEdit::textEdited,
            this, &AddressBarPrivate::onTextEdited,
            Qt::ConnectionType::DirectConnection);

    connect(q, &QLineEdit::returnPressed,
            this, &AddressBarPrivate::onReturnPressed);

    QAction *clearAction = q->findChild<QAction *>("_q_qlineeditclearaction");
    if (clearAction)
        connect(clearAction, &QAction::triggered, q, &AddressBar::clearButtonPressed);

    // fix bug#31692 搜索框输入中文后,全选已输入的,再次输入未覆盖之前的内容
    // 选中内容时，记录光标开始位置以及选中的长度
    connect(q, &AddressBar::selectionChanged, this, [=] {
        int posStart = q->selectionStart();
        int posEnd = q->selectionEnd();
        selectPosStart = posStart < posEnd ? posStart : posEnd;
        selectLength = q->selectionLength();
    });

    connect(pauseButton, &DIconButton::clicked, q, &AddressBar::pauseButtonClicked);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        initUiForSizeMode();
    });
#endif
}

void AddressBarPrivate::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    pauseButton->setFixedSize(DSizeModeHelper::element(QSize(16, 16), QSize(24, 24)));
    pauseButton->setIconSize(DSizeModeHelper::element(QSize(16, 16), QSize(24, 24)));
#else
    pauseButton->setFixedSize(QSize(24, 24));
    pauseButton->setIconSize(QSize(24, 24));
#endif
}

void AddressBarPrivate::initData()
{
    ipRegExp.setPattern(R"(((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3})");
    protocolIPRegExp.setPattern(R"(((smb)|(ftp)|(sftp))(://)((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3})");
    protocolIPRegExp.setCaseSensitivity(Qt::CaseInsensitive);
    // 设置补全组件
    urlCompleter = new QCompleter(this);
    setCompleter(urlCompleter);

    // 设置补全选择组件为popup的焦点
    completerView->setFocus(Qt::FocusReason::PopupFocusReason);

    updateHistory();
}

void AddressBarPrivate::updateHistory()
{
    historyList.clear();
    historyList.append(SearchHistroyManager::instance()->getSearchHistroy());
    isHistoryInCompleterModel = false;

    ipHistroyList.clear();
    ipHistroyList = SearchHistroyManager::instance()->getIPHistory();
}

/*!
 * \brief Set the indicator type of address bar.
 * \param type Indicator type
 *
 * This will also update the indicator icon's pixmap.
 * \sa updateIndicatorIcon
 */
void AddressBarPrivate::setIndicator(AddressBar::IndicatorType type)
{
    if (indicatorType != type) {
        clearCompleterModel();
    }
    indicatorType = type;
    updateIndicatorIcon();
}

void AddressBarPrivate::setCompleter(QCompleter *c)
{
    if (urlCompleter) {
        urlCompleter->disconnect();
    }

    urlCompleter = c;

    if (!urlCompleter) {
        return;
    }

    urlCompleter->setModel(&completerModel);
    urlCompleter->setPopup(completerView);
    urlCompleter->setCompletionMode(QCompleter::PopupCompletion);
    urlCompleter->setCaseSensitivity(Qt::CaseSensitive);
    urlCompleter->setMaxVisibleItems(10);
    completerView->setItemDelegate(&cpItemDelegate);
    completerView->setAttribute(Qt::WA_InputMethodEnabled);

    connect(urlCompleter, QOverload<const QString &>::of(&QCompleter::activated),
            this, &AddressBarPrivate::insertCompletion);

    connect(urlCompleter, QOverload<const QString &>::of(&QCompleter::highlighted),
            this, &AddressBarPrivate::onCompletionHighlighted);

    connect(urlCompleter->completionModel(), &QAbstractItemModel::modelReset,
            this, &AddressBarPrivate::onCompletionModelCountChanged);
}

void AddressBarPrivate::clearCompleterModel()
{
    isHistoryInCompleterModel = false;
    completerModel.setStringList(QStringList());
}

void AddressBarPrivate::updateCompletionState(const QString &text)
{
    if (ipRegExp.exactMatch(text)) {
        inputIsIpAddress = true;
        completeIpAddress(text);
    } else {
        inputIsIpAddress = false;
        int slashIndex = text.lastIndexOf('/');
        bool hasSlash = (slashIndex != -1);
        QString strLocalPath(text);
        strLocalPath = hasSlash ? strLocalPath.left(slashIndex + 1) : strLocalPath;

        const auto &currentDir = QDir::currentPath();
        QUrl curUrl = q->currentUrl();
        if (dfmbase::FileUtils::isLocalFile(curUrl))
            QDir::setCurrent(curUrl.toLocalFile());

        const QUrl &url = UrlRoute::fromUserInput(strLocalPath, false);
        QDir::setCurrent(currentDir);

        // Check if the entered text is a string to search or a url to complete.
        if (hasSlash && url.isValid() && !url.scheme().isEmpty()) {
            completeLocalPath(text, url, slashIndex);
        } else {
            completeSearchHistory(text);
        }
    }
}

/*!
 * \brief Do complete by calling QCompleter::complete()
 *
 * Fake inline completion means auto select the only matched completion when
 * there are only one matched item avaliable.
 */
void AddressBarPrivate::doComplete()
{
    if (completerView->isHidden()) {
        urlCompleter->complete(q->rect().adjusted(0, 5, 0, 5));
    } else {
        urlCompleter->metaObject()->invokeMethod(urlCompleter, "_q_autoResizePopup");
    }
    if (urlCompleter->completionCount() == 1
        && lastPressedKey != Qt::Key_Backspace
        && lastPressedKey != Qt::Key_Delete
        && isKeyPressed   //判断是否按键按下，时间设定的时100ms
        && !(lastPressedKey == Qt::Key_X && lastPreviousKey == Qt::Key_Control)   //键盘剪切事件
        && q->cursorPosition() == q->text().length()) {
        completerView->setCurrentIndex(urlCompleter->completionModel()->index(0, 0));
    }
    completerView->show();
    completerView->activateWindow();

    return;
}

void AddressBarPrivate::appendToCompleterModel(const QStringList &stringList)
{
    for (const QString &str : stringList) {
        // 防止出现空的补全提示
        if (str.isEmpty())
            continue;

        QStandardItem *item = new QStandardItem(str);
        completerModel.appendRow(item);
    }
}

void AddressBarPrivate::onTravelCompletionListFinished()
{
    if (urlCompleter->completionCount() > 0) {
        if (urlCompleter->popup()->isHidden() && q->isVisible())
            doComplete();
    } else {
        completerView->hide();
        q->setFocus();   // Hide will cause lost focus (weird..), so setFocus() here.
    }
}

void AddressBarPrivate::onIndicatorTriggerd()
{
    onReturnPressed();
}

void AddressBarPrivate::requestCompleteByUrl(const QUrl &url)
{
    if (!crumbController || !crumbController->isSupportedScheme(url.scheme())) {
        if (crumbController) {
            crumbController->cancelCompletionListTransmission();
            crumbController->disconnect();
            crumbController->deleteLater();
        }
        crumbController = CrumbManager::instance()->createControllerByUrl(url);
        // Still not found? Then nothing here...
        if (!crumbController) {
            clearCompleterModel();
            qDebug() << "Unsupported url / scheme for completion: " << url;
            return;
        }
        crumbController->setParent(q);
        // connections
        connect(crumbController, &CrumbInterface::completionFound, this, &AddressBarPrivate::appendToCompleterModel);
        connect(crumbController, &CrumbInterface::completionListTransmissionCompleted, this, &AddressBarPrivate::onTravelCompletionListFinished);
    }
    crumbController->requestCompletionList(url);
}

void AddressBarPrivate::completeSearchHistory(const QString &text)
{
    // Update Icon
    setIndicator(AddressBar::IndicatorType::Search);

    // set completion prefix.
    urlCompleter->setCompletionPrefix(text);

    // Check if we already loaded history list in model
    if (isHistoryInCompleterModel)
        return;

    // Set Base String
    this->completerBaseString = "";

    // History completion.
    isHistoryInCompleterModel = true;
    completerModel.setStringList(historyList);
}

void AddressBarPrivate::completeIpAddress(const QString &text)
{
    // Update Icon
    setIndicator(AddressBar::IndicatorType::Search);

    // set completion prefix.
    urlCompleter->setCompletionPrefix("");

    // Set Base String
    this->completerBaseString = text;

    completerModel.setRowCount(3);
    completerModel.setItem(0, 0, new QStandardItem("smb://" + text));
    completerModel.setItem(1, 0, new QStandardItem("ftp://" + text));
    completerModel.setItem(2, 0, new QStandardItem("sftp://" + text));

    QIcon recentIcon = QIcon::fromTheme("document-open-recent-symbolic");
    for (const auto &data : ipHistroyList) {
        if (data.ipData == text && data.isRecentlyAccessed()) {
            if (!data.accessedType.compare("smb", Qt::CaseInsensitive)) {
                auto item = completerModel.item(0);
                item->setIcon(recentIcon);
            } else if (!data.accessedType.compare("ftp", Qt::CaseInsensitive)) {
                auto item = completerModel.item(1);
                item->setIcon(recentIcon);
            } else if (!data.accessedType.compare("sftp", Qt::CaseInsensitive)) {
                auto item = completerModel.item(2);
                item->setIcon(recentIcon);
            }
        }
    }
}

void AddressBarPrivate::completeLocalPath(const QString &text, const QUrl &url, int slashIndex)
{
    // Update Icon
    setIndicator(AddressBar::IndicatorType::JumpTo);

    // Check if (now is parent) url exist.
    auto info = InfoFactory::create<FileInfo>(url);
    if (url.isValid() && info && !info->exists())
        return;

    // Check if we should start a new completion transmission.
    if (!isHistoryInCompleterModel
        && (this->completerBaseString == text.left(slashIndex + 1)
            || UrlRoute::fromUserInput(completerBaseString) == UrlRoute::fromUserInput(text.left(slashIndex + 1)))) {
        urlCompleter->setCompletionPrefix(text.mid(slashIndex + 1));   // set completion prefix first
        onCompletionModelCountChanged();   // will call complete()
        return;
    }

    // Set Base String
    completerBaseString = text.left(slashIndex + 1);

    // start request
    // 由于下方urlCompleter->setCompletionPrefix会触发onCompletionModelCountChanged接口
    // 因此在这之前需要将completerModel清空，否则会使用上一次model中的数据
    clearCompleterModel();

    // set completion prefix.
    urlCompleter->setCompletionPrefix(text.mid(slashIndex + 1));

    // URL completion.
    requestCompleteByUrl(url);
}

void AddressBarPrivate::startSpinner()
{
    spinner.start();
    spinner.show();
}

void AddressBarPrivate::stopSpinner()
{
    spinner.stop();
    spinner.hide();
}

void AddressBarPrivate::onTextEdited(const QString &text)
{
    lastEditedString = text;
    if (text.isEmpty()) {
        urlCompleter->popup()->hide();
        completerBaseString = "";
        setIndicator(AddressBar::IndicatorType::Search);
        return;
    }

    // Stop the animation if user start editing text
    if (timer.isActive()) {
        timer.stop();
    }

    // blumia: Assume address is: /aa/bbbb/cc , completion prefix should be "cc",
    //         completerBaseString should be "/aa/bbbb/"
    updateCompletionState(text);
}

void AddressBarPrivate::onReturnPressed()
{
    QString text { q->text() };
    if (text.isEmpty())
        return;

    // add search history list
    if (!dfmbase::FileUtils::isLocalFile(UrlRoute::fromUserInput(text))) {
        if (!historyList.contains(text))
            historyList.removeAll(text);
        historyList.append(text);
        SearchHistroyManager::instance()->writeIntoSearchHistory(text);

        if (protocolIPRegExp.exactMatch(text)) {
            IPHistroyData data(text, QDateTime::currentDateTime());
            if (ipHistroyList.contains(data)) {
                // update
                int index = ipHistroyList.indexOf(data);
                ipHistroyList.replace(index, data);
            } else {
                ipHistroyList << data;
            }
            SearchHistroyManager::instance()->writeIntoIPHistory(text);
        }
    }

    bool isSearch { false };
    TitleBarHelper::handlePressed(q, text, &isSearch);

    if (isSearch) {
        startSpinner();
    } else {
        emit q->urlChanged();
    }
}

void AddressBarPrivate::insertCompletion(const QString &completion)
{
    if (urlCompleter->widget() != q) {
        return;
    }

    if (inputIsIpAddress) {
        q->setText(completion);
    } else {
        q->setText(completerBaseString + completion);
    }
}

void AddressBarPrivate::onCompletionHighlighted(const QString &highlightedCompletion)
{
    if (inputIsIpAddress) {
        if (highlightedCompletion.isEmpty()) {
            q->setText(completerBaseString);
            return;
        }

        int selectLength = highlightedCompletion.length() - completerBaseString.length();
        q->setText(highlightedCompletion);
        q->setSelection(0, selectLength);
    } else {
        int completionPrefixLen = urlCompleter->completionPrefix().length();
        int selectBeginPos = highlightedCompletion.length() - completionPrefixLen;
        q->setText(completerBaseString + highlightedCompletion);
        q->setSelection(q->text().length() - selectBeginPos, q->text().length());
    }
}

void AddressBarPrivate::updateIndicatorIcon()
{
    QIcon indicatorIcon;
    QString scope = indicatorType == AddressBar::IndicatorType::Search ? "search_indicator" : "go-right";
    indicatorIcon = QIcon::fromTheme(scope);
    indicatorAction.setIcon(indicatorIcon);
}

void AddressBarPrivate::onCompletionModelCountChanged()
{
    if (urlCompleter->completionCount() <= 0) {
        completerView->hide();
        q->setFocus();
        return;
    }

    if (q->isVisible())
        doComplete();
}

bool AddressBarPrivate::eventFilterResize(AddressBar *addressbar, QResizeEvent *event)
{
    Q_UNUSED(addressbar)
    spinner.setFixedSize(q->height() - 8, q->height() - 8);
    spinner.setGeometry(event->size().width() - spinner.size().width() - 45,
                        (event->size().height() - spinner.size().height()) / 2,
                        spinner.size().width(), spinner.size().height());

    pauseButton->setGeometry(event->size().width() - pauseButton->size().width() - 45,
                             (event->size().height() - pauseButton->size().height()) / 2,
                             pauseButton->size().width(), pauseButton->size().height());

    return false;
}

bool AddressBarPrivate::eventFilterHide(AddressBar *addressbar, QHideEvent *event)
{
    Q_UNUSED(addressbar)
    Q_UNUSED(event)
    timer.stop();
    return false;
}

bool AddressBarPrivate::eventFilter(QObject *watched, QEvent *event)
{

    if (watched == q && event->type() == QEvent::Hide) {
        return eventFilterHide(qobject_cast<AddressBar *>(watched),
                               dynamic_cast<QHideEvent *>(event));
    }

    if (watched == q && event->type() == QEvent::Resize) {
        return eventFilterResize(qobject_cast<AddressBar *>(watched),
                                 dynamic_cast<QResizeEvent *>(event));
    }

    return false;
}

/*!
 * \class AddressBar
 * \brief parent
 */

AddressBar::AddressBar(QWidget *parent)
    : QLineEdit(parent), d(new AddressBarPrivate(this))
{
}

bool AddressBar::completerViewVisible()
{
    return d->completerView->isVisible();
}

void AddressBar::setCurrentUrl(const QUrl &url)
{
    QString text = dfmbase::FileUtils::isLocalFile(url) ? url.toLocalFile() : UrlRoute::urlToLocalPath(url.toString());
    this->setText(text);
}

QUrl AddressBar::currentUrl()
{
    auto id = FMWindowsIns.findWindowId(this);
    auto window = FMWindowsIns.findWindowById(id);
    if (window)
        return window->currentUrl();
    return {};
}

bool AddressBar::event(QEvent *e)
{
    // blumia: When window lost focus and then get activated, we should hide
    //         addressbar if it's visiable.
    if (e->type() == QEvent::WindowActivate) {
        if (!hasFocus() && isVisible()) {
            Q_EMIT lostFocus();
        }
    }

    if (e->type() == QEvent::KeyPress) {
        keyPressEvent(static_cast<QKeyEvent *>(e));
        return true;
    }

    return QLineEdit::event(e);
}

void AddressBar::focusInEvent(QFocusEvent *e)
{
    if (d->urlCompleter)
        d->urlCompleter->setWidget(this);
    return QLineEdit::focusInEvent(e);
}

void AddressBar::focusOutEvent(QFocusEvent *e)
{
    // blumia: Sometimes completion will trigger weird Qt::ActiveWindowFocusReason event,
    //         right click context menu will trigger Qt::PopupFocusReason event. It will
    //         cause focusOutEvent. So we simply ignore it here.
    // blumia: 2019/12/01: seems now based on current 5.11.3.2+c1-1+deepin version of Qt,
    //         completion will no longer trigger Qt::ActiveWindowFocusReason reason focus
    //         out event, so we comment out this case for now and see if it still happens.
    // fix bug#38455 文管启动后第一次点击搜索，再点击筛选按钮，会导致搜索框隐藏
    // 第一次点击筛选按钮，会发出Qt::OtherFocusReason信号导致搜索框隐藏，所以将其屏蔽
    if (/*e->reason() == Qt::ActiveWindowFocusReason || */ e->reason() == Qt::PopupFocusReason || e->reason() == Qt::OtherFocusReason) {
        e->accept();
        setFocus();
        return;
    }
    d->completerView->hide();
    emit lostFocus();
    return QLineEdit::focusOutEvent(e);
}

void AddressBar::keyPressEvent(QKeyEvent *e)
{
    d->isKeyPressed = true;
    QTimer::singleShot(100, this, [=]() {   //设定100ms，若有问题可视情况改变
        d->isKeyPressed = false;
    });
    d->lastPreviousKey = d->lastPressedKey;
    d->lastPressedKey = e->key();

    switch (e->key()) {
    case Qt::Key_Escape:
        emit escKeyPressed();
        e->accept();
        return;
    default:
        break;
    }

    auto urlComplter = d->urlCompleter;
    if (urlComplter && urlComplter->popup()->isVisible()) {
        if (d->isHistoryInCompleterModel && e->modifiers() == Qt::ShiftModifier && e->key() == Qt::Key_Delete) {
            QString completeResult = d->completerView->currentIndex().data().toString();
            bool ret = SearchHistroyManager::instance()->removeSearchHistory(completeResult);
            if (ret) {
                d->historyList.clear();
                d->historyList.append(SearchHistroyManager::instance()->getSearchHistroy());
                d->completerModel.setStringList(d->historyList);
            }
        }

        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
        case Qt::Key_Backtab:
            e->ignore();
            return;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            e->accept();
            d->completerView->hide();
            emit returnPressed();
            return;
        case Qt::Key_Tab:
            if (d->urlCompleter->completionCount() > 0) {
                if (selectedText().isEmpty()) {
                    QString completeResult = d->urlCompleter->completionModel()->index(0, 0).data().toString();
                    d->insertCompletion(completeResult);
                }
                if (dfmbase::FileUtils::isLocalFile(UrlRoute::fromUserInput(text()))) {
                    setText(text() + '/');
                    emit textEdited(text());
                }
            }
            e->accept();
            return;
        //解决bug19609文件管理器中，文件夹搜索功能中输入法在输入过程中忽然失效然后恢复
        case Qt::Key_Up:
        case Qt::Key_Down:
            d->completerView->keyPressEvent(e);
            break;
        default:
            break;
        }
        setFocus();
    } else {
        // If no compiler
        switch (e->key()) {
        case Qt::Key_Tab:
            e->accept();
            return;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (e->isAutoRepeat()) {
                e->ignore();
            } else {
                emit returnPressed();
                e->accept();
            }
            return;
        default:
            break;
        }
    }

    return QLineEdit::keyPressEvent(e);
}

void AddressBar::paintEvent(QPaintEvent *e)
{
    QLineEdit::paintEvent(e);

    // addressbar animation
    QPainter painter(this);

    //设置提示text
    if (text().isEmpty()) {
        QPen oldpen = painter.pen();
        QColor phColor = palette().text().color();
        const int flags = static_cast<int>(QStyle::visualAlignment(Qt::LeftToRight, QFlag(Qt::AlignCenter)));

        phColor.setAlpha(128);
        painter.setPen(phColor);
        painter.drawText(rect(), flags, d->placeholderText);
        painter.setPen(oldpen);
    }
    //绘制波纹效果
    if (d->animation.state() != QAbstractAnimation::Stopped) {

        QIcon icon = QIcon::fromTheme("dfm_addressbar_glowing");
        if (icon.availableSizes().isEmpty())
            return;
        const QSize size = icon.availableSizes().first();
        QPixmap glowingImg = icon.pixmap(size);
        float curValue = d->animation.currentValue().toFloat();
        float xPos = (width() + glowingImg.width()) * curValue - glowingImg.width();

        painter.drawPixmap(static_cast<int>(xPos), 0, glowingImg);
    }
}

void AddressBar::showEvent(QShowEvent *event)
{
    d->timer.start();
    d->updateIndicatorIcon();
    d->updateHistory();
    return QLineEdit::showEvent(event);
}

// 解决bug19609文件管理器中，文件夹搜索功能中输入法在输入过程中忽然失效然后恢复
void AddressBar::inputMethodEvent(QInputMethodEvent *e)
{
    if (hasSelectedText()) {
        // fix bug#31692 搜索框输入中文后,全选已输入的,再次输入未覆盖之前的内容
        int pos = d->selectPosStart;
        setText(d->lastEditedString.remove(d->selectPosStart, d->selectLength));
        // 设置光标到修改处
        setCursorPosition(pos);
    }
    QLineEdit::inputMethodEvent(e);
}

void AddressBar::enterEvent(QEvent *e)
{

    if (d->indicatorType == AddressBar::Search && d->spinner.isPlaying()) {
        d->spinner.hide();
        d->pauseButton->setVisible(true);
    }

    QLineEdit::enterEvent(e);
}

void AddressBar::leaveEvent(QEvent *e)
{
    if (d->indicatorType == AddressBar::Search && d->spinner.isPlaying()) {
        d->pauseButton->setVisible(false);
        d->spinner.show();
    }

    QLineEdit::leaveEvent(e);
}

void AddressBar::startSpinner()
{
    d->startSpinner();
}

void AddressBar::stopSpinner()
{
    d->pauseButton->setVisible(false);
    d->stopSpinner();
}
