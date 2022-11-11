// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONNECTTOSERVERDIALOG_H
#define CONNECTTOSERVERDIALOG_H

#include "dfmglobal.h"

#include <QObject>
#include <DDialog>
#include <QPushButton>
DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DListView;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QComboBox;
class QCompleter;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class CollectionDelegate;
class ConnectToServerDialog : public DDialog
{
    Q_OBJECT
    enum CollectionIconType {
        CancelIcon = 0,
        CollectionIcon
    };

public:
    explicit ConnectToServerDialog(QWidget *parent = nullptr);
    ~ConnectToServerDialog();
signals:

public slots:
    void onButtonClicked(const int& index);
private slots:
    void collectionOperate();
    void doDeleteCollection(const QString& text, int row = -1);
    void onCompleterActivated(const QString& text);
    void onCurrentInputChanged(const QString &text);
    void onCollectionViewClicked(const QModelIndex &index);
private:
    void initUI();
    void initConnect();
    void onAddButtonClicked();
    void onDelButtonClicked();
    void upateState();
    QString schemeWithSlash(const QString& scheme) const;
    QString collectionIcon(CollectionIconType icon, bool light = true) const;
    enum DialogButton {
        CannelButton,
        ConnectButton
    };
    QComboBox *m_serverComboBox = nullptr;
    QComboBox *m_schemeComboBox = nullptr;
    QPushButton *m_addButton = nullptr;
    DIconButton *m_delButton = nullptr;
    DListView *m_collectionServerView = nullptr;
    bool m_isAddState = true;
    CollectionDelegate* m_delegate = nullptr;
    QLabel *m_collectionLabel = nullptr;
    QLabel *m_centerNotes = nullptr;
    QCompleter *m_completer = nullptr;
    QStringList m_supportedSchemes;
};

#endif // CONNECTTOSERVERDIALOG_H
