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
#include "views/private/addressbar_p.h"
#include "views/addressbar.h"
#include "utils/crumbmanager.h"
#include "utils/crumbinterface.h"
#include "utils/searchhistroymanager.h"
#include "utils/titlebarhelper.h"

#include "dfm-base/base/schemefactory.h"

#include <QCompleter>

DPTITLEBAR_USE_NAMESPACE

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

    QAction *clearAction = findChild<QAction *>("_q_qlineeditclearaction");
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
}

void AddressBarPrivate::initData()
{
    // 设置补全组件
    urlCompleter = new QCompleter(this);
    setCompleter(urlCompleter);

    // 设置补全选择组件为popup的焦点
    completerView->setFocus(Qt::FocusReason::PopupFocusReason);

    historyList.clear();
    historyList.append(SearchHistroyManager::instance()->toStringList());
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
    int slashIndex = text.lastIndexOf('/');
    bool hasSlash = (slashIndex != -1);
    QString strLocalPath(text);
    strLocalPath = hasSlash ? strLocalPath.left(slashIndex + 1) : strLocalPath;
    const QUrl &url = UrlRoute::fromUserInput(strLocalPath);

    // Check if the entered text is a string to search or a url to complete.
    if (hasSlash && url.isValid() && !url.scheme().isEmpty()) {
        // Update Icon
        setIndicator(AddressBar::IndicatorType::JumpTo);

        // Check if (now is parent) url exist.
        auto info = InfoFactory::create<AbstractFileInfo>(url);
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
    } else {
        // set completion prefix.
        urlCompleter->setCompletionPrefix(text);

        // Update Icon
        setIndicator(AddressBar::IndicatorType::Search);

        // Check if we already loaded history list in model
        if (isHistoryInCompleterModel)
            return;

        // Set Base String
        this->completerBaseString = "";

        // History completion.
        isHistoryInCompleterModel = true;
        completerModel.setStringList(historyList);
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

        if (completerModel.insertRow(completerModel.rowCount())) {
            QModelIndex index = completerModel.index(completerModel.rowCount() - 1, 0);
            completerModel.setData(index, str);
        } else {
            qWarning("Failed to append some data to completerModel.");
        }
    }
}

void AddressBarPrivate::onTravelCompletionListFinished()
{
    if (urlCompleter->completionCount() > 0) {
        if (urlCompleter->popup()->isHidden())
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
    if (!crumbController || !crumbController->supportedUrl(url)) {
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
        // connections
        connect(crumbController, &CrumbInterface::completionFound, this, &AddressBarPrivate::appendToCompleterModel);
        connect(crumbController, &CrumbInterface::completionListTransmissionCompleted, this, &AddressBarPrivate::onTravelCompletionListFinished);
    }
    crumbController->requestCompletionList(url);
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
    if (!UrlRoute::fromUserInput(text).isLocalFile()) {
        if (!historyList.contains(text)) {
            historyList.append(text);
            SearchHistroyManager::instance()->writeIntoSearchHistory(text);
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
    q->setText(completerBaseString + completion);
}

void AddressBarPrivate::onCompletionHighlighted(const QString &highlightedCompletion)
{
    int completionPrefixLen = urlCompleter->completionPrefix().length();
    int selectBeginPos = highlightedCompletion.length() - completionPrefixLen;
    q->setText(completerBaseString + highlightedCompletion);
    q->setSelection(q->text().length() - selectBeginPos, q->text().length());
}

void AddressBarPrivate::updateIndicatorIcon()
{
    QIcon indicatorIcon;
    QString scope = indicatorType == AddressBar::IndicatorType::Search ? "search" : "go-right";
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

    doComplete();
}

bool AddressBarPrivate::eventFilterResize(AddressBar *addressbar, QResizeEvent *event)
{
    Q_UNUSED(addressbar)
    spinner.setFixedSize(q->height() - 8, q->height() - 8);
    spinner.setGeometry(event->size().width() - spinner.size().width() - 45,
                        (event->size().height() - spinner.size().height()) / 2,
                        spinner.size().width(), spinner.size().height());

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
    QString text = url.isLocalFile() ? url.toLocalFile() : UrlRoute::urlToLocalPath(url.toString());
    this->setText(text);
    this->setSelection(0, text.length());
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
                d->historyList.append(SearchHistroyManager::instance()->toStringList());
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
                if (UrlRoute::fromUserInput(text()).isLocalFile()) {
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
            emit returnPressed();
            e->accept();
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
    stopSpinner();
    d->updateIndicatorIcon();
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
        // TODO(zhangs): pauseButton->setVisible(true);
    }

    QLineEdit::enterEvent(e);
}

void AddressBar::leaveEvent(QEvent *e)
{
    if (d->indicatorType == AddressBar::Search && d->spinner.isPlaying()) {
        // TODO(zhangs): pauseButton->setVisible(false);
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
    d->stopSpinner();
}
