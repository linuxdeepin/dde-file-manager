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

#include "views/dcompleterlistview.h"
#include "interfaces/dfmcrumbinterface.h"

#include "themeconfig.h"
#include "dfileservices.h"

#include "dfmcrumbfactory.h"

#include <QAction>
#include <QCompleter>
#include <QAbstractItemView>
#include <QTimer>
#include <QPainter>
#include <QScrollBar>
#include <QDebug>

#include <DThemeManager>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

DCompleterStyledItemDelegate::DCompleterStyledItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void DCompleterStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // prepare;
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
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

DFMAddressBar::DFMAddressBar(QWidget *parent)
    : QLineEdit(parent)
{
    initUI();
    initConnections();

    // Test
    urlCompleter = new QCompleter(this);
    this->setCompleter(urlCompleter);
    completerModel.setStringList({"macat", "mamacat", "mamamacat", "mamamamacat", "mamamamamacat",
                                  "makat", "mamakat", "mamamakat", "mamamamakat", "mamamamamakat",
                                  "mafox", "teji", "tejilang", "jjy", "jjjjy"});
}

QCompleter *DFMAddressBar::completer() const
{
    return urlCompleter;
}

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
    urlCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    urlCompleter->setMaxVisibleItems(10);
    connect(urlCompleter, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
    connect(urlCompleter, SIGNAL(highlighted(QString)), this, SLOT(onCompletionHighlighted(QString)));

    completerView->setItemDelegate(&styledItemDelegate);

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
    emit focusOut();

    return QLineEdit::focusOutEvent(e);
}

void DFMAddressBar::keyPressEvent(QKeyEvent *e)
{
    if (urlCompleter && urlCompleter->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
        default:
           break;
        }
    }

    if (!urlCompleter || !false/*isShortcut*/) {
        QLineEdit::keyPressEvent(e);
    }

    if (!urlCompleter) {
        return;
    }

    if (text().isEmpty()) {
        urlCompleter->popup()->hide();
        setIndicator(IndicatorType::Search);
        return;
    }

    // blumia: Assume address is: /aa/bbbb/cc , completion prefix should be "cc",
    //         completerBaseString should be "/aa/bbbb/"
    updateCompletionState(this->text());
    urlCompleter->complete(rect().adjusted(0, 5, 0, 5));
}

void DFMAddressBar::initUI()
{
    // Left indicator (clickable! did u know that?)
    indicator = new QAction(this);
    addAction(indicator, QLineEdit::LeadingPosition);

    // Clear text button
    setClearButtonEnabled(true);

    // Completer List
    completerView = new DCompleterListView(this);

    // Other misc..
    setFixedHeight(24);
    setObjectName("DSearchBar");
    setMinimumWidth(1);
    setAlignment(Qt::AlignHCenter);

    setFocusPolicy(Qt::ClickFocus);
}

void DFMAddressBar::initConnections()
{
    connect(DThemeManager::instance(), &DThemeManager::widgetThemeChanged, this, &DFMAddressBar::onWidgetThemeChanged);
    connect(indicator, &QAction::triggered, this, [this](){
        emit returnPressed();
    });
}

void DFMAddressBar::setIndicator(DFMAddressBar::IndicatorType type)
{
    indicatorType = type;
    updateIndicatorIcon();
}

void DFMAddressBar::onWidgetThemeChanged(QWidget *widget, QString theme)
{
    DThemeManager::instance()->setTheme(completerView, theme);

    if (widget == this) {
        updateIndicatorIcon();
    }
}

void DFMAddressBar::updateIndicatorIcon()
{
    QIcon indicatorIcon;
    QString scope = indicatorType == IndicatorType::Search ? "DSearchBar.searchAction" : "DSearchBar.jumpToAction";
    indicatorIcon.addFile(ThemeConfig::instace()->value(scope, "icon").toString());
    indicator->setIcon(indicatorIcon);
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
    bool isSearchText = (slashIndex == -1);

    // set completion prefix.
    urlCompleter->setCompletionPrefix(isSearchText ? text : text.mid(slashIndex + 1));

    DUrl url = DUrl::fromUserInput(text, false);
    const DAbstractFileInfoPointer& info = DFileService::instance()->createFileInfo(this, url);

    if (url.isValid() && info && !info->exists()) {
        url = info->parentUrl();
    }

    // Check if the entered text is a string to search or a url to complete.
    if (!isSearchText && url.isValid() && !url.scheme().isEmpty()) {
        // Update Icon
        setIndicator(IndicatorType::JumpTo);

        // Check again if (now is parent) url exists.
        const DAbstractFileInfoPointer& anotherInfo = DFileService::instance()->createFileInfo(this, url);
        if (url.isValid() || anotherInfo || !anotherInfo->exists()) {
            return;
        }

        // check if we should start a new completion transmission.
        if (this->completerBaseString == text.left(slashIndex + 1)) {
            return;
        }

        // Set Base String
        this->completerBaseString = text.left(slashIndex + 1);

        // URL completion.
        if (!crumbController || !crumbController->supportedUrl(url)) {
            if (crumbController) {
                crumbController->cancelCompletionListTransmission();
                crumbController->disconnect();
            }
            crumbController = DFMCrumbManager::instance()->createControllerByUrl(url);
            // Not found? Search for plugins
            if (!crumbController) {
                crumbController = DFMCrumbFactory::create(url.scheme());
            }
            // Still not found? Then nothing here...
            if (!crumbController) {
                qDebug() << "Unsupported url / scheme for completion: " << url;
                return;
            }
        }
        Q_CHECK_PTR(crumbController);
        // connections
        connect(crumbController, &DFMCrumbInterface::completionFound, this, [](const QStringList &list){
            // append list to completion list.
            qDebug() << list;
        });
        connect(crumbController, &DFMCrumbInterface::completionListTransmissionCompleted, this, [this](){
            // check if we can do inline complete.
            qDebug() << "completion done.";
            crumbController->disconnect();
        });
        // set base url
        crumbController->requestCompletionList(url);
    } else {
        // Update Icon
        setIndicator(IndicatorType::Search);

        // Set Base String
        this->completerBaseString = "";

        // History completion.
        qWarning("DFMAddressBar::updateCompletionState() may need implemented!!!");
        // Set history to string list model here.
        // TODO: Set history to string list model here.
    }

    return;
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
    QString shouldAppend = highlightedCompletion.right(highlightedCompletion.length() - completionPrefixLen);
    setText(completerBaseString + urlCompleter->completionPrefix() + shouldAppend);
    setSelection(text().length() - shouldAppend.length(), text().length());
}

DFM_END_NAMESPACE
