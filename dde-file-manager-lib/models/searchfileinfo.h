#ifndef SEARCHFILEINFO_H
#define SEARCHFILEINFO_H

#include "dfileinfo.h"

class SearchFileInfo : public DAbstractFileInfo
{
public:
    SearchFileInfo(const DUrl &url);

    bool exists() const Q_DECL_OVERRIDE;
    bool isReadable() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;

    int filesCount() const Q_DECL_OVERRIDE;

    DUrl parentUrl() const Q_DECL_OVERRIDE;

    QList<int> userColumnRoles() const Q_DECL_OVERRIDE;
    QVariant userColumnDisplayName(int userColumnRole) const Q_DECL_OVERRIDE;
    /// get custom column data
    QVariant userColumnData(int userColumnRole) const Q_DECL_OVERRIDE;

    bool canRedirectionFileUrl() const Q_DECL_OVERRIDE;
    DUrl redirectedFileUrl() const Q_DECL_OVERRIDE;

    bool canIteratorDir() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type = SingleFile) const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;
    int userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const Q_DECL_OVERRIDE;

    MenuAction menuActionByColumnRole(int userColumnRole) const Q_DECL_OVERRIDE;

    bool isEmptyFloder(const QDir::Filters &filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System) const Q_DECL_OVERRIDE;

    CompareFunction compareFunByColumn(int columnRole) const Q_DECL_OVERRIDE;
    bool hasOrderly() const Q_DECL_OVERRIDE;

    DUrl getUrlByNewFileName(const QString &fileName) const Q_DECL_OVERRIDE;

    QString loadingTip() const Q_DECL_OVERRIDE;
    QString subtitleForEmptyFloder() const Q_DECL_OVERRIDE;

    QString fileDisplayName() const Q_DECL_OVERRIDE;

    DUrl mimeDataUrl() const Q_DECL_OVERRIDE;

    QString toLocalFile() const Q_DECL_OVERRIDE;

private:
    DUrl m_parentUrl;
};

#endif // SEARCHFILEINFO_H
