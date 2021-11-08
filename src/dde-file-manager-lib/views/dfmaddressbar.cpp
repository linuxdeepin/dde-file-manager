/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmaddressbar.h"
#include "dfmcrumbmanager.h"
#include "dfmcrumbfactory.h"
#include "dfmcrumbbar.h"

#include "views/dcompleterlistview.h"
#include "interfaces/dfmcrumbinterface.h"
#include "controllers/searchhistroymanager.h"
#include "controllers/vaultcontroller.h"
#include "accessibility/ac-lib-file-manager.h"

#include "singleton.h"
#include "dfileservices.h"

#include <DSpinner>
#include <DAnchors>
#include <DIconButton>

#include <QAction>
#include <QCompleter>
#include <QAbstractItemView>
#include <QTimer>
#include <QPainter>
#include <QScrollBar>
#include <QWidgetAction>
#include <QDebug>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

DCompleterStyledItemDelegate::DCompleterStyledItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void DCompleterStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // prepare
    QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                              ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    // draw background
    if (option.showDecorationSelected && (option.state & (QStyle::State_Selected | QStyle::State_MouseOver))) {
        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    }

    // draw text
    if (option.state & (QStyle::State_Selected | QStyle::State_MouseOver)) {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    painter->setFont(option.font);
    painter->drawText(option.rect.adjusted(31, 0, 0, 0), Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
}

QSize DCompleterStyledItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(24);

    return s;
}

/*!
 * \class DFMAddressBar
 * \inmodule dde-file-manager-lib
 *
 * \brief DFMAddressBar is the address bar of the file manager
 *
 * The address bar is also the search bar, it can also handle search job. The address
 * bar is managed by the crumb bar.
 *
 * \sa DFMCrumbBar, DFMCrumbInterface
 */

DFMAddressBar::DFMAddressBar(DFMCrumbBar *parent)
    : QLineEdit(parent)
{
    setObjectName(AC_COMPUTER_TITLE_BAR_ADRESS);
    setObjectName(AC_COMPUTER_TITLE_BAR_ADRESS);

    initUI();
    initConnections();
    initData();
}

QCompleter *DFMAddressBar::completer() const
{
    return urlCompleter;
}

QString DFMAddressBar::placeholderText() const
{
    return m_placeholderText;
}

/*!
 * \brief Set current url \a path
 *
 * \param path Current url
 */
void DFMAddressBar::setCurrentUrl(const DUrl &path)
{
    QString text = path.isLocalFile() ? path.toLocalFile() : path.toString();

    currentUrl = path;
    //m_disableCompletion = true;
    this->setText(text);
    this->setSelection(0, text.length());
}

void DFMAddressBar::setCompleter(QCompleter *c)
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
    connect(urlCompleter, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
    connect(urlCompleter, SIGNAL(highlighted(QString)), this, SLOT(onCompletionHighlighted(QString)));
    connect(urlCompleter->completionModel(), &QAbstractItemModel::modelReset,
            this, &DFMAddressBar::onCompletionModelCountChanged);

    completerView->setItemDelegate(&styledItemDelegate);
    //解决bug19609文件管理器中，文件夹搜索功能中输入法在输入过程中忽然失效然后恢复，设置这个属性listview就可以拥有地址兰的输入法
    completerView->setAttribute(Qt::WA_InputMethodEnabled);

}

void DFMAddressBar::setPlaceholderText(const QString &text)
{
    // Since the placeholder text provided by QLineEdit can not have separate alignment
    // alone with the user entered edit text.
    m_placeholderText = text;
}

void DFMAddressBar::playAnimation()
{
    if (animationSpinner)
        return;

    // Right animation widget
    animationSpinner = new DSpinner(this);
    animationSpinner->setAttribute(Qt::WA_TransparentForMouseEvents);
    animationSpinner->setFocusPolicy(Qt::NoFocus);
    animationSpinner->setFixedSize(height() - 8, height() - 8);

    DAnchorsBase::setAnchor(animationSpinner, Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    DAnchorsBase::setAnchor(animationSpinner, Qt::AnchorRight, this, Qt::AnchorRight);
    DAnchorsBase::getAnchorBaseByWidget(animationSpinner)->setRightMargin(height() + 8);

    animationSpinner->show();
    animationSpinner->start();
}

void DFMAddressBar::stopAnimation()
{
    pauseButton->setVisible(false);
    if (!animationSpinner)
        return;

    //fix 64394 输入框状态组件没有stop会一直start
    animationSpinner->stop();
    animationSpinner->close();
    update();
    animationSpinner->deleteLater();
    animationSpinner = nullptr;
}

void DFMAddressBar::hide()
{
    QLineEdit::hide();
    completerView->hideMe();
    // fix bug 69679
    stopAnimation();
}

void DFMAddressBar::focusInEvent(QFocusEvent *e)
{
    if (urlCompleter) {
        urlCompleter->setWidget(this);
    }

    return QLineEdit::focusInEvent(e);
}

void DFMAddressBar::focusOutEvent(QFocusEvent *e)
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
    completerView->hideMe();
    emit lostFocus();

    return QLineEdit::focusOutEvent(e);
}

void DFMAddressBar::keyPressEvent(QKeyEvent *e)
{
    isKeyPressed = true;
    QTimer::singleShot(100, this, [ = ]() { //设定100ms，若有问题可视情况改变
        isKeyPressed = false;
    });
    lastPreviousKey = lastPressedKey;
    lastPressedKey = e->key();
    switch (e->key()) {
    case Qt::Key_Escape:
        emit escKeyPressed();
        completerView->hideMe();
        e->accept();
        return;
    default:
        break;
    }

    if (urlCompleter && urlCompleter->popup()->isVisible()) {
        if (isHistoryInCompleterModel && e->modifiers() == Qt::ShiftModifier && e->key() == Qt::Key_Delete) {
            QString completeResult = completerView->currentIndex().data().toString();
            bool ret = Singleton<SearchHistroyManager>::instance()->removeSearchHistory(completeResult);
            if (ret) {
                historyList.clear();
                historyList.append(Singleton<SearchHistroyManager>::instance()->toStringList());
                completerModel.setStringList(historyList);
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
            completerView->hideMe();
            emit returnPressed();
            return;
        case Qt::Key_Tab:
            if (completer()->completionCount() > 0) {
                if (selectedText().isEmpty()) {
                    QString completeResult = urlCompleter->completionModel()->index(0, 0).data().toString();
                    insertCompletion(completeResult);
                }
                if (DUrl::fromUserInput(text()).isLocalFile()) {
                    setText(text() + '/');
                    emit textEdited(text());
                }
            }
            e->accept();
            return;
        //解决bug19609文件管理器中，文件夹搜索功能中输入法在输入过程中忽然失效然后恢复
        case Qt::Key_Up:
        case Qt::Key_Down:
            completerView->keyPressEvent(e);
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

void DFMAddressBar::paintEvent(QPaintEvent *e)
{
    QLineEdit::paintEvent(e);

    // addressbar animation
    QPainter painter(this);

    // check if we should draw placeholder text
    if (text().isEmpty()) {
        QPen oldpen = painter.pen();
        QColor phColor = palette().text().color();
        const Qt::Alignment alignPhText = QStyle::visualAlignment(Qt::LeftToRight, QFlag(Qt::AlignCenter));

        phColor.setAlpha(128);
        painter.setPen(phColor);

        painter.drawText(rect(), alignPhText, placeholderText());

        painter.setPen(oldpen);
    }

    // Draw growing animation
    if (animation && animation->state() != QAbstractAnimation::Stopped) {

        QIcon icon = QIcon::fromTheme("dfm_addressbar_glowing");
        if (icon.availableSizes().isEmpty())
            return;
        const QSize size = icon.availableSizes().first();
        QPixmap glowingImg = icon.pixmap(size);
        float curValue = animation->currentValue().toFloat();
        float xPos = (this->width() + glowingImg.width()) * curValue - glowingImg.width();

        painter.drawPixmap(xPos, 0, glowingImg);
    }
}

void DFMAddressBar::showEvent(QShowEvent *event)
{
    timer.start();
    updateIndicatorIcon();

    return QLineEdit::showEvent(event);
}
//解决bug19609文件管理器中，文件夹搜索功能中输入法在输入过程中忽然失效然后恢复
void DFMAddressBar::inputMethodEvent(QInputMethodEvent *e)
{
    if (hasSelectedText()) {
        // fix bug#31692 搜索框输入中文后,全选已输入的,再次输入未覆盖之前的内容
        int pos = selectPosStart;
        setText(lastEditedString.remove(selectPosStart, selectLength));
        // 设置光标到修改处
        setCursorPosition(pos);
    }
    QLineEdit::inputMethodEvent(e);
}

void DFMAddressBar::enterEvent(QEvent *e)
{
    if (indicatorType == Search && animationSpinner && animationSpinner->isPlaying()) {
        animationSpinner->hide();
        pauseButton->setVisible(true);
    }

    QLineEdit::enterEvent(e);
}

void DFMAddressBar::leaveEvent(QEvent *e)
{
    if (indicatorType == Search && animationSpinner && animationSpinner->isPlaying()) {
        pauseButton->setVisible(false);
        animationSpinner->show();
    }

    QLineEdit::leaveEvent(e);
}

void DFMAddressBar::initUI()
{
    // pause button
    pauseButton = new DIconButton(this);
    pauseButton->setIcon(QIcon::fromTheme("dfm_search_pause"));
    pauseButton->setFocusPolicy(Qt::NoFocus);
    pauseButton->setCursor({Qt::ArrowCursor});
    pauseButton->setFixedSize(24, 24);
    pauseButton->setIconSize({24, 24});
    pauseButton->setFlat(true);
    pauseButton->setVisible(false);

    DAnchorsBase::setAnchor(pauseButton, Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    DAnchorsBase::setAnchor(pauseButton, Qt::AnchorRight, this, Qt::AnchorRight);
    DAnchorsBase::getAnchorBaseByWidget(pauseButton)->setRightMargin(height() + 15);

    // Left indicator (clickable! did u know that?)
    indicator = new QAction(this);
    addAction(indicator, QLineEdit::LeadingPosition);

    // Clear text button
    setClearButtonEnabled(true);

    // Completer List
    completerView = new DCompleterListView(this);

    // Animation
    timer.setInterval(200); // animation delay
    timer.setSingleShot(true);
    animation = new QVariantAnimation(this);
    animation->setDuration(616);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setStartValue(QVariant(1.0f));
    animation->setEndValue(QVariant(0.0f));

    connect(animation, &QVariantAnimation::valueChanged,
            this, static_cast<void(DFMAddressBar::*)()>(&DFMAddressBar::update));

    // Other misc..
    //setFixedHeight(24);
    setObjectName("DSearchBar");
    setMinimumWidth(1);
    setAlignment(Qt::AlignHCenter);
    setAlignment(Qt::AlignLeft);
    setPlaceholderText(QCoreApplication::translate("DFMAddressBar", "Search or enter address"));

    setFocusPolicy(Qt::ClickFocus);
}

void DFMAddressBar::initConnections()
{
    connect(indicator, &QAction::triggered, this, [this]() {
        emit returnPressed();
    });
    connect(this, &DFMAddressBar::returnPressed, this, [this]() {
        if (text().isEmpty()) {
            return;
        }

        //! 如果为保险箱路径则进行路径转换
        QString str = VaultController::toInternalPath(text());

        if (!DUrl::fromUserInput(str).isLocalFile()) {
            if (!historyList.contains(str)) {
                historyList.append(str);
                Singleton<SearchHistroyManager>::instance()->writeIntoSearchHistory(str);
            }
        }
    });
    connect(this, &DFMAddressBar::textEdited, this, &DFMAddressBar::onTextEdited);

    QAction *clear_action = findChild<QAction *>("_q_qlineeditclearaction");

    if (clear_action) {
        connect(clear_action, &QAction::triggered, this, &DFMAddressBar::clearButtonPressed);
    }

    // animation delay timer
    connect(&timer, &QTimer::timeout, animation, [this]() {
        animation->start();
    });

    // fix bug#31692 搜索框输入中文后,全选已输入的,再次输入未覆盖之前的内容
    // 选中内容时，记录光标开始位置以及选中的长度
    connect(this, &DFMAddressBar::selectionChanged, this, [this] {
        int posStart = selectionStart();
        int posEnd = selectionEnd();
        selectPosStart = posStart < posEnd ? posStart : posEnd;
        selectLength = selectionLength();
    });

    connect(pauseButton, &DIconButton::clicked, this, &DFMAddressBar::pauseButtonClicked);
}

void DFMAddressBar::initData()
{
    // Completer
    urlCompleter = new QCompleter(this);
    this->setCompleter(urlCompleter);

    // History
    historyList.clear();
    historyList.append(Singleton<SearchHistroyManager>::instance()->toStringList());
}

/*!
 * \brief Set the indicator type of address bar.
 * \param type Indicator type
 *
 * This will also update the indicator icon's pixmap.
 * \sa updateIndicatorIcon
 */
void DFMAddressBar::setIndicator(DFMAddressBar::IndicatorType type)
{
    if (indicatorType != type) {
        clearCompleterModel();
    }
    indicatorType = type;
    updateIndicatorIcon();
}

/*!
 * \brief Update the indicator icon via the indicatorType value.
 *
 * \sa setIndicator
 */
void DFMAddressBar::updateIndicatorIcon()
{
    QIcon indicatorIcon;
    QString scope = indicatorType == IndicatorType::Search ? "search" : "go-right";
    indicatorIcon = QIcon::fromTheme(scope);
    indicator->setIcon(indicatorIcon);
}

/*!
 * \brief Do complete by calling QCompleter::complete()
 *
 * Fake inline completion means auto select the only matched completion when
 * there are only one matched item avaliable.
 */
void DFMAddressBar::doComplete()
{
    if (completerView->isHidden()) {
        completer()->complete(rect().adjusted(0, 5, 0, 5));
    } else {
        urlCompleter->metaObject()->invokeMethod(urlCompleter, "_q_autoResizePopup");
    }
    if (completer()->completionCount() == 1
            && lastPressedKey != Qt::Key_Backspace
            && lastPressedKey != Qt::Key_Delete
            && isKeyPressed //判断是否按键按下，时间设定的时100ms
            && !(lastPressedKey == Qt::Key_X && lastPreviousKey == Qt::Key_Control) //键盘剪切事件
            && cursorPosition() == text().length()) {
        completerView->setCurrentIndex(urlCompleter->completionModel()->index(0, 0));
    }
    completerView->showMe();
    completerView->activateWindow();

    return;
}

void DFMAddressBar::clearCompleterModel()
{
    isHistoryInCompleterModel = false;
    completerModel.setStringList(QStringList());
}

/*!
 * \brief Update completion prefix and start completion transmission.
 * \param text User entered text in the address bar.
 *
 * Will analysis and set the completion prefix,
 * Will also start a new completion when we should.
 */
void DFMAddressBar::updateCompletionState(const QString &text)
{
    int slashIndex = text.lastIndexOf('/');
    bool hasSlash = (slashIndex != -1);
    // 修复bug-62112 对保险箱虚拟路径特殊判断，将保险箱虚拟路径转化为本地路径
    QString strLocalPath(text);
    if (strLocalPath.startsWith(DFMVAULT_ROOT)) {
        strLocalPath = VaultController::virtualPathToLocalPath(hasSlash ? strLocalPath.left(slashIndex + 1) : strLocalPath);
    } else {
        strLocalPath = hasSlash ? strLocalPath.left(slashIndex + 1) : strLocalPath;
    }
    const DUrl &url = DUrl::fromUserInput(strLocalPath, false);
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

    // Check if the entered text is a string to search or a url to complete.
    if (hasSlash && url.isValid() && !url.scheme().isEmpty()) {
        // Update Icon
        setIndicator(IndicatorType::JumpTo);

        // Check if (now is parent) url exist.
        if (url.isValid() && info && !info->exists()) {
            return;
        }

        // Check if we should start a new completion transmission.
        if (!isHistoryInCompleterModel && (this->completerBaseString == text.left(slashIndex + 1)
                                           || DUrl::fromUserInput(this->completerBaseString) == DUrl::fromUserInput(text.left(slashIndex + 1)))) {
            urlCompleter->setCompletionPrefix(text.mid(slashIndex + 1)); // set completion prefix first
            onCompletionModelCountChanged(); // will call complete()
            return;
        }

        // Set Base String
        this->completerBaseString = text.left(slashIndex + 1);

        // 修复bug-62110 修复bug-62112 设置成保险箱路径虚拟路径根目录
        if (this->completerBaseString == (DFMVAULT_SCHEME + QString(":/")))
            this->completerBaseString = DFMVAULT_ROOT;

        // start request
        // 由于下方urlCompleter->setCompletionPrefix会触发onCompletionModelCountChanged接口
        // 因此在这之前需要将completerModel清空，否则会使用上一次model中的数据
        clearCompleterModel();

        // set completion prefix.
        urlCompleter->setCompletionPrefix(text.mid(slashIndex + 1));

        // URL completion.
        if (!crumbController || !crumbController->supportedUrl(url)) {
            if (crumbController) {
                crumbController->cancelCompletionListTransmission();
                crumbController->disconnect();
                crumbController->deleteLater();
            }
            DFMCrumbBar *crumbBar = qobject_cast<DFMCrumbBar *>(this->parent());
            Q_CHECK_PTR(crumbBar);
            crumbController = DFMCrumbManager::instance()->createControllerByUrl(url, crumbBar);
            // Still not found? Then nothing here...
            if (!crumbController) {
                clearCompleterModel();
                qDebug() << "Unsupported url / scheme for completion: " << url;
                return;
            }
            crumbController->setParent(this);
            // connections
            connect(crumbController, &DFMCrumbInterface::completionFound, this, [this](const QStringList & list) {
                // append list to completion list.
                appendToCompleterModel(list);
            });
            connect(crumbController, &DFMCrumbInterface::completionListTransmissionCompleted, this, [this]() {
                if (urlCompleter->completionCount() > 0) {
                    if (urlCompleter->popup()->isHidden())
                        doComplete();
                } else {
                    completerView->hideMe();
                    setFocus(); // Hide will cause lost focus (weird..), so setFocus() here.
                }
            });
        }

        //fix 33750 在匹配到smb:/，就创建url地址smb：///,去拉取url的目录，目前不知道什么原因造成gio mount smb：///失败就会出现提示框
        if (text.endsWith("smb:/")) {
            return;
        }
        crumbController->requestCompletionList(url);
    } else {
        // set completion prefix.
        urlCompleter->setCompletionPrefix(text);

        // Update Icon
        setIndicator(IndicatorType::Search);

        // Check if we already loaded history list in model
        if (isHistoryInCompleterModel) {
            return;
        }

        // Set Base String
        this->completerBaseString = "";

        // History completion.
        isHistoryInCompleterModel = true;
        // 修复bug-62117 搜索补全中不显示保险箱全路径
        QStringList::iterator itr = historyList.begin();
        while (itr != historyList.end()) {
            if (VaultController::isVaultFile(*itr)) {
                (*itr) = VaultController::toExternalPath(*itr);
                if (VaultController::isVaultFile(*itr)) {
                    itr = historyList.erase(itr);
                    continue;
                }
            }
            ++itr;
        }
        completerModel.setStringList(historyList);
    }

    return;
}

void DFMAddressBar::appendToCompleterModel(const QStringList &stringList)
{
    for (const QString &str : stringList) {
        //防止出现空的补全提示
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

void DFMAddressBar::insertCompletion(const QString &completion)
{
    if (urlCompleter->widget() != this) {
        return;
    }

    this->setText(completerBaseString + completion);
}

void DFMAddressBar::onCompletionHighlighted(const QString &highlightedCompletion)
{
    int completionPrefixLen = urlCompleter->completionPrefix().length();
    int selectBeginPos = highlightedCompletion.length() - completionPrefixLen;
    setText(completerBaseString + highlightedCompletion);
    setSelection(text().length() - selectBeginPos, text().length());
}

void DFMAddressBar::onCompletionModelCountChanged()
{
    if (completer()->completionCount() <= 0) {
        completerView->hideMe();
        setFocus();
        return;
    }

    doComplete();
}

void DFMAddressBar::onTextEdited(const QString &text)
{
    lastEditedString = text;
    if (text.isEmpty()) {
        urlCompleter->popup()->hide();
        completerBaseString = "";
        setIndicator(IndicatorType::Search);
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

bool DFMAddressBar::event(QEvent *e)
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

DFM_END_NAMESPACE
