// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONTITLEBAR_H
#define COLLECTIONTITLEBAR_H

#include "ddplugin_organizer_global.h"
#include "organizer_defines.h"

#include <DBlurEffectWidget>

namespace ddplugin_organizer {

class CollectionTitleBarPrivate;

class CollectionTitleBar : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit CollectionTitleBar(const QString &uuid, QWidget *parent = nullptr);
    ~CollectionTitleBar() override;

    bool setTitleBarVisible(const bool &visible);
    bool titleBarVisible() const;
    void setRenamable(const bool renamable = false);
    bool renamable() const;
    void setClosable(const bool closable = false);
    bool closable() const;
    void setAdjustable(const bool adjustable = false);
    bool adjustable() const;
    void setTitleName(const QString &name);
    QString titleName() const;
    void setCollectionSize(const CollectionFrameSize &size);
    CollectionFrameSize collectionSize() const;

signals:
    void sigRequestClose(const QString &id);
    void sigRequestAdjustSizeMode(const CollectionFrameSize &size);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *) override;
    void contextMenuEvent(QContextMenuEvent *) override;
    void rounded();

private:
    QSharedPointer<CollectionTitleBarPrivate> d = nullptr;
};

}

#endif   // COLLECTIONTITLEBAR_H
