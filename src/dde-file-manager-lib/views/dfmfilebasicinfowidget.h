// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMFILEBASICINFOWIDGET_H
#define DFMFILEBASICINFOWIDGET_H

#include <QFrame>
#include <QLabel>
#include <dfmglobal.h>
#include "durl.h"

DFM_BEGIN_NAMESPACE

namespace FileBasicSection {
class SectionKeyLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionKeyLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};


class SectionValueLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionValueLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});

protected:
    //! Set the width of the label after the font is changed
    virtual bool event(QEvent *e) override;
    //! Set the width of the label after the selected file changes
    virtual void showEvent(QShowEvent *e) override;
};

class LinkSectionValueLabel: public SectionValueLabel
{
    Q_OBJECT

public:
    explicit LinkSectionValueLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});

    DUrl linkTargetUrl() const;
    void setLinkTargetUrl(const DUrl &linkTargetUrl);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    DUrl m_linkTargetUrl;
};
}

class DFMFileBasicInfoWidgetPrivate;
class DFMFileBasicInfoWidget : public QFrame
{
    Q_OBJECT
public:
    explicit DFMFileBasicInfoWidget(QWidget *parent = nullptr);
    virtual ~DFMFileBasicInfoWidget();

    void setUrl(const DUrl &url);

    bool showFileName();
    void setShowFileName(bool visible);

    bool showMediaInfo();
    void setShowMediaInfo(bool visible);
    bool showSummary(); // 目录只显示概要， 包含几个文件等。
    void setShowSummary(bool enable);

protected:
    void updateSizeText(qint64 size, int filesCount, int directoryCount);
    virtual void showEvent(QShowEvent *event);
private:
    QScopedPointer<DFMFileBasicInfoWidgetPrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, DFMFileBasicInfoWidget)
};

DFM_END_NAMESPACE

#endif // DFMFILEBASICINFOWIDGET_H
