// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKMOUNTPLUGIN_H
#define DISKMOUNTPLUGIN_H

#include <QLabel>
#include <QPainter>

#include "pluginsiteminterface.h"
#include "diskcontrolwidget.h"
#include "diskpluginitem.h"

// TipsWidget from dde-dock.
// TODO: move the disk mount plugin to dde-dock project.
class TipsWidget : public QFrame
{
    Q_OBJECT
public:
    explicit TipsWidget(QWidget *parent = nullptr) : QFrame(parent) {}

    void setText(const QString &text)
    {
        m_text = text;
        setFixedSize(fontMetrics().width(text) + 20, fontMetrics().height());
        update();
    }

    void refreshFont()
    {
        setFixedSize(fontMetrics().width(m_text) + 20, fontMetrics().height());
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QFrame::paintEvent(event);
        refreshFont();
        QPainter painter(this);
        painter.setPen(QPen(palette().brightText(), 1));
        QTextOption option;
        option.setAlignment(Qt::AlignCenter);
        painter.drawText(rect(), m_text, option);
    }

private:
    QString m_text;
};


class DiskMountPlugin : public QObject, PluginsItemInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "disk-mount.json")

public:
    explicit DiskMountPlugin(QObject *parent = nullptr);
    explicit DiskMountPlugin(bool usingAppLoader, QObject *parent = nullptr);

    const QString pluginName() const override;
    void init(PluginProxyInterface *proxyInter) override;

    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;
    QWidget *itemPopupApplet(const QString &itemKey) override;

    const QString itemContextMenu(const QString &itemKey) override;
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;

    int itemSortKey(const QString &itemKey) override;
    void setSortKey(const QString &itemKey, const int order) override;

    void refreshIcon(const QString &itemKey) override;

private:
    void initCompoments();

    void displayModeChanged(const Dock::DisplayMode mode) override;

private slots:
    void diskCountChanged(const int count);

private:
    bool m_pluginAdded;
    bool m_pluginLoaded;
    bool m_usingAppLoader;

    TipsWidget *m_tipsLabel;
    DiskPluginItem *m_diskPluginItem;
    DiskControlWidget *m_diskControlApplet;
};

#endif // DISKMOUNTPLUGIN_H
