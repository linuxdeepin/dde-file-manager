#pragma once

#include <diconitemdelegate.h>

class DesktopItemDelegate : public DIconItemDelegate
{
    Q_OBJECT
public:
    explicit DesktopItemDelegate(DFileViewHelper *parent);
    ~DesktopItemDelegate();

    QString iconSizeLevelDescription(int i) const;
    int iconSizeLevel() const Q_DECL_OVERRIDE;
    int minimumIconSizeLevel() const Q_DECL_OVERRIDE;
    int maximumIconSizeLevel() const Q_DECL_OVERRIDE;

    int increaseIcon() Q_DECL_OVERRIDE;
    int decreaseIcon() Q_DECL_OVERRIDE;
    int setIconSizeByIconSizeLevel(int level) Q_DECL_OVERRIDE;
    QSize iconSizeByIconSizeLevel() const;

    void updateItemSizeHint() Q_DECL_OVERRIDE;

private:
    QStringList iconSizeDescriptions;
    QList<int> iconSizes;
    // default icon size is 48px.
    int currentIconSizeIndex = -1;
};

