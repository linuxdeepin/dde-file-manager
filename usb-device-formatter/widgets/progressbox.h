#ifndef PROGRESSBOX_H
#define PROGRESSBOX_H

#include "progressline.h"
#include <QTimer>
#include <QVariantAnimation>

class QImage;

struct Bled{
    QPointF pos;
};

class ProgressBox : public ProgressLine
{
    Q_OBJECT
public:
    explicit ProgressBox(QWidget *parent = 0);
    void initConnections();
    void initUI();

signals:
    void finished(const bool& successful);

public slots:
    void updateAnimation();
    void taskTimeOut();
    void startTask();
    void finishedTask(const bool result);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QTimer* m_taskTimer = NULL;
    QTimer* m_updateTimer = NULL;
    QList<Bled> m_bleds;
    QVariantAnimation* m_taskAni = NULL;
    QLabel* m_textLabel = NULL;
    QImage* m_BackImage = NULL;
    QImage* m_FrontImage = NULL;
    QImage* m_shadow = NULL;

    int m_ForntXOffset1;
    int m_ForntXOffset2;
    int m_BackXOffset1;
    int m_BackXOffset2;
    int m_YOffset;
    int m_FrontWidth;
    int m_BackWidth;

    double m_Pop7YOffset;
    double m_Pop7XOffset;

    double m_Pop8YOffset;
    double m_Pop8XOffset;

    double m_Pop11YOffset;
    double m_Pop11XOffset;

};

#endif // PROGRESSBOX_H
