/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#ifndef AddressBar_P_H
#define AddressBar_P_H

#include "windowservice/addressbar.h"
#include "windowservice/completerview.h"
#include "dfm_filemanager_service_global.h"

#include "dfm-base/base/urlroute.h"

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

#include <DSpinner>
#include <DAnchors>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_FM_BEGIN_NAMESPACE

class AddressBarPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(AddressBar)
    AddressBar * const q_ptr;
    CompleterView* completerView = nullptr;
    QStringList inputHistory;
    QTimer timer;
    DSpinner spinner;
    QVariantAnimation animation;
    QString placeholderText = QObject::tr("Search or enter address");
    QAction indicatorAction;
    QAction clearAction;

public:

    explicit AddressBarPrivate(AddressBar * qq)
        : QObject(qq),
          q_ptr(qq)
    {
        if (!completerView)
            completerView = new CompleterView;

        //设置补全组件
        q_ptr->setCompleter(completerView->completer());

        //设置补全选择组件为popup的焦点
        completerView->setFocus(Qt::FocusReason::PopupFocusReason);

        //左侧Action按钮 设置
        q_ptr->addAction(&indicatorAction, QLineEdit::LeadingPosition);
        //default icon is search
        indicatorAction.setIcon(QIcon::fromTheme("search"));

        QObject::connect(&indicatorAction, &QAction::triggered, this, [=]()
        {
            if (indicatorAction.icon().name() == "search") {
                Q_EMIT q_ptr->editingFinishedSearch(q_ptr->text());
            }

            if (indicatorAction.icon().name() == "go-right") {
                QUrl url = UrlRoute::pathToUrl(q_ptr->text());
                if (url.isValid())
                    Q_EMIT q_ptr->editingFinishedUrl(url);
            }
        });

        //lineEdit clear Action按钮
        q_ptr->setClearButtonEnabled(true);

        spinner.setParent(q_ptr);
        spinner.setAttribute(Qt::WA_TransparentForMouseEvents);
        spinner.setFocusPolicy(Qt::NoFocus);
        spinner.hide();

        //        DAnchorsBase::setAnchor(&spinner, Qt::AnchorVerticalCenter, q_ptr, Qt::AnchorVerticalCenter);
        //        DAnchorsBase::setAnchor(&spinner, Qt::AnchorRight, q_ptr, Qt::AnchorRight);
        //        DAnchorsBase::getAnchorBaseByWidget(&spinner)->setRightMargin(q_ptr->height() + 8);

        animation.setParent(q_ptr);
        animation.setDuration(616);
        animation.setEasingCurve(QEasingCurve::OutQuad);
        animation.setStartValue(QVariant(1.0f));
        animation.setEndValue(QVariant(0.0f));

        QObject::connect(&animation, &QVariantAnimation::valueChanged,
                         q_ptr, QOverload<>::of(&AddressBar::update));

        timer.setInterval(200);
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &animation, [=]()
        {
            animation.start();
            q_ptr->update();
        });

        QObject::connect(qq, &QLineEdit::textEdited,
                         this, &AddressBarPrivate::procTextEdited,
                         Qt::ConnectionType::DirectConnection);

        QObject::connect(qq, &QLineEdit::editingFinished,
                         this, &AddressBarPrivate::inputSave);

        QObject::connect(qq, &QLineEdit::returnPressed,
                         this, &AddressBarPrivate::procReturnPressed);

        qq->installEventFilter(this);
    }

public Q_SLOTS:

    void startRunState()
    {
        spinner.start();
        spinner.show();
    }

    void stopRunState()
    {
        spinner.stop();
        spinner.hide();
    }

    void procTextEdited(const QString &string)
    {
        if (string.isEmpty()) {
            indicatorAction.setIcon(QIcon::fromTheme("search"));
            return;
        }

        if (UrlRoute::hasScheme(string)) {
            QStringList rootUrls;
            QString localPath = UrlRoute::schemeRoot(string);
            if (QFileInfo::exists(localPath) && !UrlRoute::schemeIsVirtual(string)) {

                QDirIterator itera(localPath,
                                   QDir::Dirs | QDir::NoDotAndDotDot | QDir::System,
                                   QDirIterator::NoIteratorFlags);

                while(itera.hasNext())
                {
                    itera.next();
                    rootUrls << UrlRoute::pathToUrl(itera.filePath()).toString();
                }
            }

            if (completerView)
                completerView->model()->setStringList(rootUrls);

            indicatorAction.setIcon(QIcon::fromTheme("go-right"));
            return;
        }

        if (QFileInfo::exists(string)) {
            QStringList localPaths;
            QDirIterator itera(string,
                               QDir::Dirs | QDir::NoDotAndDotDot | QDir::System,
                               QDirIterator::NoIteratorFlags);

            while(itera.hasNext())
            {
                itera.next();
                localPaths << itera.filePath();
            }
            if (completerView)
                completerView->model()->setStringList(localPaths);
            indicatorAction.setIcon(QIcon::fromTheme("go-right"));
            return;
        }
        if (completerView)
            completerView->model()->setStringList(inputHistory);
        indicatorAction.setIcon(QIcon::fromTheme("search"));
    }

    void inputSave()
    {
        if (completerView && 0 > completerView->model()->stringList().indexOf(q_ptr->text()))
            inputHistory << q_ptr->text();
    }

    void procReturnPressed()
    {
        QUrl url(q_ptr->text());
        if (!UrlRoute::isVirtualUrl(url)) {
            QString localPath = UrlRoute::urlToPath(url);
            if (QDir(localPath).exists()) {
                qInfo() << "sig editingFinishedUrl :" << url;
                Q_EMIT q_ptr->editingFinishedUrl(url);
                startRunState();
                return;
            }
        }

        if (QDir(q_ptr->text()).exists()) {
            QUrl url = UrlRoute::pathToUrl(q_ptr->text());
            if (!url.isValid()) {
                qInfo() << "sig editingFinishedUrl :" << url;
                Q_EMIT q_ptr->editingFinishedUrl(url);
                startRunState();
                return;
            }
        }

        qInfo() << "sig editingFinishedSearch :" << q_ptr->text();
        Q_EMIT q_ptr->editingFinishedSearch(q_ptr->text());
        startRunState();
        return;
    }

protected:

    virtual bool eventFilterKeyPress(AddressBar* addressbar, QKeyEvent *event)
    {
        Q_UNUSED(addressbar)
        if (Qt::Key_Escape == event->key()) {
            q_ptr->hide();
            return false;
        }

        if (Qt::Key_Tab == event->key()) {
            return false;
        }

        return false;
    }

    virtual bool eventFilterResize(AddressBar *addressbar, QResizeEvent *event)
    {
        Q_UNUSED(addressbar)
        spinner.setFixedSize(q_ptr->height() - 8, q_ptr->height() - 8);
        spinner.setGeometry(event->size().width() - spinner.size().width() - 45,
                            (event->size().height()- spinner.size().height())/2,
                            spinner.size().width(),spinner.size().height());

        return false;
    }

    virtual bool eventFilterShow(AddressBar *addressbar, QShowEvent *event)
    {
        Q_UNUSED(addressbar)
        Q_UNUSED(event)
        timer.start();
        return false;
    }

    virtual bool eventFilterHide(AddressBar *addressbar, QHideEvent *event)
    {
        Q_UNUSED(addressbar)
        Q_UNUSED(event)
        timer.stop();
        stopRunState();
        return false;
    }

    virtual bool eventFilterPaint(AddressBar *addressbar, QPaintEvent *event)
    {
        //此处调用addressbar绘制事件
        addressbar->paintEvent(event);

        QPainter painter(addressbar);
        //设置提示text
        if (addressbar->text().isEmpty()) {
            QPen oldpen = painter.pen();
            QColor phColor = q_ptr->palette().text().color();
            const int flags = static_cast<int>(QStyle::visualAlignment(Qt::LeftToRight, QFlag(Qt::AlignCenter)));

            phColor.setAlpha(128);
            painter.setPen(phColor);

            painter.drawText(q_ptr->rect(), flags, placeholderText);

            painter.setPen(oldpen);
        }
        //绘制波纹效果
        if (animation.state() != QAbstractAnimation::Stopped) {

            QIcon icon = QIcon::fromTheme("dfm_addressbar_glowing");
            if (icon.availableSizes().isEmpty())
                return true;
            const QSize size = icon.availableSizes().first();
            QPixmap glowingImg = icon.pixmap(size);
            float curValue = animation.currentValue().toFloat();
            float xPos = (q_ptr->width() + glowingImg.width()) * curValue - glowingImg.width();

            painter.drawPixmap(static_cast<int>(xPos), 0, glowingImg);
        }
        return true;
    }

    virtual bool eventFilter(QObject *watched, QEvent *event) override
    {

        if (watched == q_ptr && event->type() == QEvent::Show)
        {
            return eventFilterShow(qobject_cast<AddressBar*>(watched),
                                   dynamic_cast<QShowEvent*>(event));
        }

        if (watched == q_ptr && event->type() == QEvent::Hide)
        {
            return eventFilterHide(qobject_cast<AddressBar*>(watched),
                                   dynamic_cast<QHideEvent*>(event));
        }

        if (watched == q_ptr && event->type() == QEvent::Resize)
        {
            return eventFilterResize(qobject_cast<AddressBar*>(watched),
                                     dynamic_cast<QResizeEvent*>(event));
        }

        if (watched == q_ptr && event->type() == QEvent::KeyPress) {
            return eventFilterKeyPress(qobject_cast<AddressBar*>(watched),
                                       dynamic_cast<QKeyEvent*>(event));
        }

        if (watched == q_ptr && event->type() == QEvent::Paint) {
            return eventFilterPaint(qobject_cast<AddressBar*>(watched),
                                    dynamic_cast<QPaintEvent*>(event));
        }

        return false;
    }
};

DSB_FM_END_NAMESPACE

#endif //AddressBar_P_H
