#ifndef TAGFILEINFO_H
#define TAGFILEINFO_H


#include <QObject>



#include "dfmevent.h"
#include "dabstractfileinfo.h"


class TagFileInfo : public DAbstractFileInfo
{
public:
    TagFileInfo(const DUrl& url);
    virtual ~TagFileInfo()=default;

    virtual bool isDir() const override;
    virtual bool makeAbsolute() override;
    virtual bool exists() const override;

    virtual bool isTaged() const override;
    virtual bool isReadable() const override;
    virtual bool isWritable() const override;
    virtual bool canRedirectionFileUrl() const override;
//    virtual bool columnDefaultVisibleForRole(int role) const override;


    virtual DUrl parentUrl() const override;
    virtual QString iconName() const override;
    virtual DUrl goToUrlWhenDeleted() const override;
    virtual DUrl redirectedFileUrl() const override;
    virtual Qt::ItemFlags fileItemDisableFlags() const override;
    virtual QSet<MenuAction> disableMenuActionList() const override;
    virtual QVector<MenuAction> menuActionList(MenuType type) const override;
};



#endif // TAGFILEINFO_H
