// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONNECTTOSERVERDIALOG_H
#define CONNECTTOSERVERDIALOG_H

#include "dfmplugin_titlebar_global.h"

#include <DDialog>
#include <DFrame>
#include <QUrl>
#include <QRegularExpression>

DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DListView;
class DComboBox;
class DLabel;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QCompleter;
QT_END_NAMESPACE

class CollectionDelegate;
class CollectionModel;

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

protected:
    void initIfWayland();
    void updateAddButtonState(bool collected);
    void initServerDatas();
    QStringList updateCollections(const QString &newUrl, bool insertWhenNoExist);
    QString getCurrentUrlString();

private slots:
    void initUiForSizeMode();
    void updateUiState();

private:
    void initializeUi();
    void initConnect();
    void onAddButtonClicked();
    void onDelButtonClicked();
    QString schemeWithSlash(const QString &scheme) const;

    enum DialogButton {
        kCannelButton,
        kConnectButton
    };

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QRegExp protocolIPRegExp;   // smb://ip, ftp://ip, sftp://ip
#else
    QRegularExpression protocolIPRegExp;
#endif
    QUrl currentUrl;
    QStringList supportedSchemes;
    DTK_WIDGET_NAMESPACE::DComboBox *serverComboBox { nullptr };
    DTK_WIDGET_NAMESPACE::DComboBox *schemeComboBox { nullptr };
    QCompleter *completer { nullptr };
    DTK_WIDGET_NAMESPACE::DFrame *emptyFrame { nullptr };
    bool isAddState { true };
    DTK_WIDGET_NAMESPACE::DIconButton *theAddButton { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *theDelButton { nullptr };
    DTK_WIDGET_NAMESPACE::DListView *collectionServerView { nullptr };
    CollectionDelegate *delegate { nullptr };
    CollectionModel *model { nullptr };

    DTK_WIDGET_NAMESPACE::DLabel *charsetLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DComboBox *charsetComboBox { nullptr };
};

}

#endif   // CONNECTTOSERVERDIALOG_H
