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
#include <QStringListModel>
#include <QStyledItemDelegate>

DFM_BEGIN_NAMESPACE

class DCompleterStyledItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit DCompleterStyledItemDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

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

    explicit DFMAddressBar(QWidget *parent = 0);

    QCompleter *completer() const;

    void setCurrentUrl(const DUrl &path);
    void setCompleter(QCompleter *c);

signals:
    void focusOut();

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    void initUI();
    void initConnections();
    void setIndicator(enum IndicatorType type);
    void onWidgetThemeChanged(QWidget *widget, QString theme);
    void updateIndicatorIcon();
    void doComplete();
    void updateCompletionState(const QString &text);
    void appendToCompleterModel(const QStringList &stringList);

    bool isSearchStarted = false;
    DUrl currentUrl = DUrl();
    QString completerBaseString = QString();
    QStringListModel completerModel;
    DCompleterListView * completerView;
    QAction * indicator = nullptr;
    QCompleter *urlCompleter = nullptr;
    DFMCrumbInterface* crumbController = nullptr; // Scheme completion support
    DCompleterStyledItemDelegate styledItemDelegate;
    enum IndicatorType indicatorType = IndicatorType::Search;

private slots:
    void insertCompletion(const QString &completion);
    void onCompletionHighlighted(const QString &highlightedCompletion);
};

DFM_END_NAMESPACE

#endif // DFMADDRESSBAR_H
