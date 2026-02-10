// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGMENUSCENE_H
#define TAGMENUSCENE_H

#include "dfmplugin_tag_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_tag {
class TagColorListWidget;
class TagMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "TagMenu";
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class TagMenuScenePrivate;
class TagMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit TagMenuScene(QObject *parent = nullptr);
    virtual ~TagMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

public slots:
    void onHoverChanged(const QColor &color);
    void onColorClicked(const QColor &color);

private:
    QScopedPointer<TagMenuScenePrivate> d;

    TagColorListWidget *getMenuListWidget() const;

    QAction *createTagAction(QMenu *parent) const;
    QAction *createColorListAction(QMenu *parent) const;
    QWidget *findDesktopView(QWidget *root) const;
};

}

#endif   // TAGMENUSCENE_H
