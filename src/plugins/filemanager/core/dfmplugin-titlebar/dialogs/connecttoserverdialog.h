// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    QString schemeWithSlash(const QString &scheme) const;

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
