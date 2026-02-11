// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONTITLEBAR_P_H
#define COLLECTIONTITLEBAR_P_H

#include "collectiontitlebar.h"

#include <DStackedWidget>
#include <DLineEdit>
#include <DLabel>
#include <DIconButton>
#include <DMenu>

#include <QHBoxLayout>
#include <QAtomicInteger>

namespace ddplugin_organizer {

class OptionButton : public Dtk::Widget::DIconButton
{
    Q_OBJECT
public:
    OptionButton(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
    void initStyleOption(Dtk::Widget::DStyleOptionButton *option) const override;
};

class CollectionTitleBarPrivate : public QObject
{
    Q_OBJECT
public:
    explicit CollectionTitleBarPrivate(const QString &uuid, CollectionTitleBar *qq = nullptr);
    ~CollectionTitleBarPrivate();

    void modifyTitleName();
    void titleNameModified();
    void updateDisplayName();
    void showMenu();

    void sendRequestClose();

public:
    CollectionTitleBar *q = nullptr;
    QString id;

    QHBoxLayout *mainLayout = nullptr;
    Dtk::Widget::DLabel *nameLabel = nullptr;
    Dtk::Widget::DLineEdit *nameLineEdit = nullptr;
    Dtk::Widget::DStackedWidget *nameWidget = nullptr;
    OptionButton *menuBtn = nullptr;
    Dtk::Widget::DMenu *menu = nullptr;

    QAtomicInteger<bool> needHidden = false;
    bool renamable = false;
    bool closable = false;
    bool adjustable = false;
    QString titleName;
    CollectionFrameSize size = CollectionFrameSize::kSmall;
};

}

#endif // COLLECTIONTITLEBAR_P_H
