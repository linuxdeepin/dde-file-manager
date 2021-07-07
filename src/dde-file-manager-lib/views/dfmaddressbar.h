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
#ifndef DFMADDRESSBAR_H
#define DFMADDRESSBAR_H

#include "dfmglobal.h"

#include <QItemSelection>
#include <QLineEdit>
#include <QVariantAnimation>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QTimer>

#include <dtkwidget_global.h>

DWIDGET_BEGIN_NAMESPACE
class DSpinner;
class DIconButton;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class DCompleterStyledItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DCompleterStyledItemDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class DFMCrumbBar;
class DFMCrumbInterface;
class DCompleterListView;
class DFMAddressBar : public QLineEdit
{
    Q_OBJECT
public:
    enum IndicatorType {
        Search,
        JumpTo
    };

    explicit DFMAddressBar(DFMCrumbBar *parent = nullptr);

    QCompleter *completer() const;
    QString placeholderText() const;

    void setCurrentUrl(const DUrl &path);
    void setCompleter(QCompleter *c);
    void setPlaceholderText(const QString &text);

    void playAnimation();
    void stopAnimation();

public slots:
    void hide();

signals:
    void lostFocus();
    void clearButtonPressed();
    void escKeyPressed();
    void pauseButtonClicked();

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void showEvent(QShowEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *e) override;
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    void initUI();
    void initConnections();
    void initData();
    void setIndicator(enum IndicatorType type);
    void updateIndicatorIcon();
    void doComplete();
    void clearCompleterModel();
    void updateCompletionState(const QString &text);
    void appendToCompleterModel(const QStringList &stringList);

    int lastPressedKey = Qt::Key_D; // just an init value
    int lastPreviousKey = Qt::Key_Control; //记录上前一个按钮
    bool isHistoryInCompleterModel = false;
    QTimer timer;
    DUrl currentUrl = DUrl();
    QString m_placeholderText = QString();
    QString completerBaseString = QString();
    QString lastEditedString = QString();
    // inputMethodEvent中获取不到选中的内容，故缓存光标开始位置以及选中长度
    int selectPosStart = 0;
    int selectLength = 0;
    QStringListModel completerModel;
    DCompleterListView *completerView;
    QStringList historyList;
    QAction *indicator = nullptr;
    QCompleter *urlCompleter = nullptr;
    QVariantAnimation *animation = nullptr;
    DFMCrumbInterface *crumbController = nullptr; // Scheme completion support
    DCompleterStyledItemDelegate styledItemDelegate;
    enum IndicatorType indicatorType = IndicatorType::Search;

    DSpinner *animationSpinner = nullptr;
    bool isKeyPressed = false;

    DIconButton *pauseButton = nullptr;

private slots:
    void insertCompletion(const QString &completion);
    void onCompletionHighlighted(const QString &highlightedCompletion);
    void onCompletionModelCountChanged();
    void onTextEdited(const QString &text);

    bool event(QEvent *e) override;
};

DFM_END_NAMESPACE

#endif // DFMADDRESSBAR_H
