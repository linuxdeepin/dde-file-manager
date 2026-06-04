// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGHELPER_H
#define TAGHELPER_H

#include <DCrumbEdit>

#include <QObject>
#include <QColor>

class QPainter;

namespace dfmplugin_tag {

struct TagColorDefine
{
    QString colorName;
    QString iconName;
    QString displayName;
    QColor color;

public:
    TagColorDefine(const QString &colorName,
                   const QString &iconName,
                   const QString &dispaly,
                   const QColor &color);
};

class TagPainter;
class TagHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TagHelper)
public:
    static TagHelper *instance();
    inline static QString scheme()
    {
        return "tag";
    }

    static QUrl rootUrl();

    static QList<QUrl> commonUrls(const QList<QUrl> &urls);

    QList<QColor> defaultColors() const;

    QColor queryColorByColorName(const QString &name) const;
    QColor queryColorByDisplayName(const QString &name) const;
    QString queryColorNameByColor(const QColor &color) const;
    QString queryIconNameByColorName(const QString &colorName) const;
    QString queryIconNameByColor(const QColor &color) const;
    QString queryDisplayNameByColor(const QColor &color) const;
    QString queryColorNameByDisplayName(const QString &name) const;

    QString getTagNameFromUrl(const QUrl &url) const;
    QUrl makeTagUrlByTagName(const QString &tag) const;

    QString getColorNameByTag(const QString &tagName) const;
    bool isDefaultTag(const QString &tagName) const;

    void paintTags(QPainter *painter, QRectF &rect, const QList<QColor> &colors) const;

    QVariantMap createSidebarItemInfo(const QString &tag);

    void showTagEdit(const QRectF &parentRect, const QRectF &iconRect, const QList<QUrl> &fileList, bool showInTagDir = false);

    void crumbEditInputFilter(DTK_WIDGET_NAMESPACE::DCrumbEdit *edit);

private:
    explicit TagHelper(QObject *parent = nullptr);
    void initTagColorDefines();
    TagColorDefine randomTagDefine() const;

    QList<TagColorDefine> colorDefines;
};

}

#endif   // TAGHELPER_H
