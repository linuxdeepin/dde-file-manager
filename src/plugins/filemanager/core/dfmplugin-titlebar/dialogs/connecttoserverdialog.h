/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef CONNECTTOSERVERDIALOG_H
#define CONNECTTOSERVERDIALOG_H

#include "dfmplugin_titlebar_global.h"

#include <DDialog>
#include <QUrl>

DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DListView;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QComboBox;
class QCompleter;
QT_END_NAMESPACE

class CollectionDelegate;

namespace dfmplugin_titlebar {

class ConnectToServerDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit ConnectToServerDialog(const QUrl &url, QWidget *parent = nullptr);

public slots:
    void collectionOperate();
    void onButtonClicked(const int &index);
    void onCurrentTextChanged(const QString &string);
    void doDeleteCollection(const QString &text, int row = -1);
    void onCurrentInputChanged(const QString &text);
    void onCollectionViewClicked(const QModelIndex &index);
    void onCompleterActivated(const QString &text);
private:
    void initializeUi();
    void initConnect();
    void onAddButtonClicked();
    void onDelButtonClicked();
    void upateUiState();

    enum DialogButton {
        kCannelButton,
        kConnectButton
    };

    QRegExp protocolIPRegExp;   // smb://ip, ftp://ip, sftp://ip
    QUrl currentUrl;
    QStringList supportedSchemes;
    QComboBox *serverComboBox { nullptr };
    QComboBox *schemeComboBox { nullptr };
    QCompleter *completer { nullptr };
    QLabel *centerNotes = { nullptr };
    bool isAddState = { true };
    DTK_WIDGET_NAMESPACE::DIconButton *theAddButton { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *theDelButton { nullptr };
    DTK_WIDGET_NAMESPACE::DListView *collectionServerView { nullptr };
    CollectionDelegate *delegate { nullptr };
};

}

#endif   // CONNECTTOSERVERDIALOG_H
