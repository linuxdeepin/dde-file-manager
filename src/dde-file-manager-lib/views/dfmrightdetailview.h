// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMRIGHTDETAILVIEW_H
#define DFMRIGHTDETAILVIEW_H

#include<QFrame>
#include <durl.h>

class DFMRightDetailViewPrivate;
class DFMRightDetailView : public QFrame{
    Q_OBJECT
public:
    explicit DFMRightDetailView(const DUrl &fileUrl, QWidget *parent = nullptr);
    virtual ~DFMRightDetailView();

    void setUrl(const DUrl &url);
    void setTagWidgetVisible(bool visible);
protected:
    void initUI();
    void initTagWidget();
    void LoadFileTags();

private:
    QScopedPointer<DFMRightDetailViewPrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, DFMRightDetailView)

};


#endif // DFMRIGHTDETAILVIEW_H
