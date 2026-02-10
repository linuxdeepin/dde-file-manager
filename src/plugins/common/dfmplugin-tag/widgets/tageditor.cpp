// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tageditor.h"
#include "utils/tagmanager.h"
#include "utils/taghelper.h"

#include <dfm-base/utils/windowutils.h>

#include <QAbstractTextDocumentLayout>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_tag;

TagEditor::TagEditor(QWidget *const parent, bool inTagDir)
    : DArrowRectangle { DArrowRectangle::ArrowTop, parent }, isShowInTagDir(inTagDir)
{
    initializeWidgets();
    initializeParameters();
    initializeLayout();
    initializeConnect();

    installEventFilter(this);

    // NOTE: in wayland you must specify the window flags manually
    if (dfmbase::WindowUtils::isWayLand())
        setWindowFlags(windowFlags() | Qt::ToolTip | Qt::FramelessWindowHint);
    else
        setWindowFlags(Qt::ToolTip);
}

void TagEditor::setFocusOutSelfClosing(bool value) noexcept
{
    bool excepted { !value };
    flagForShown.compare_exchange_strong(excepted, value, std::memory_order_release);
}

void TagEditor::setFilesForTagging(const QList<QUrl> &files)
{
    this->files = files;
}

void TagEditor::setDefaultCrumbs(const QStringList &list)
{
    const auto &tagsMap = TagManager::instance()->getTagsColor(list);
    updateCrumbsColor(tagsMap);
}

void TagEditor::onFocusOut()
{
    if (flagForShown.load(std::memory_order_acquire)) {
        if (!crumbEdit->toPlainText().remove(QChar::ObjectReplacementCharacter).isEmpty())
            crumbEdit->appendCrumb(crumbEdit->toPlainText().remove(QChar::ObjectReplacementCharacter));
        processTags();
        close();
    }
}

void TagEditor::filterInput()
{
    TagHelper::instance()->crumbEditInputFilter(crumbEdit);
}

void TagEditor::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape: {
        this->processTags();
        event->accept();
        this->close();
        break;
    }
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        QObject::disconnect(this, &TagEditor::windowDeactivate, this, &TagEditor::onFocusOut);
        processTags();
        event->accept();
        close();
        break;
    }
    default:
        break;
    }

    DArrowRectangle::keyPressEvent(event);
}

void TagEditor::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void TagEditor::initializeWidgets()
{
    crumbEdit = new DCrumbEdit;
    promptLabel = new QLabel(tr("Input tag info, such as work, family. A comma is used between two tags."));
    totalLayout = new QVBoxLayout;
    totalLayout->setSizeConstraint(QLayout::SetFixedSize);
    backgroundFrame = new QFrame;
}

void TagEditor::initializeParameters()
{
    crumbEdit->setFixedSize(140, 40);
    crumbEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbEdit->setCrumbReadOnly(true);
    crumbEdit->setCrumbRadius(2);
    setupEditHeight();

    promptLabel->setFixedWidth(140);
    promptLabel->setWordWrap(true);
    backgroundFrame->setContentsMargins(QMargins { 0, 0, 0, 0 });

    setMargin(0);
    setFixedWidth(140);
    setFocusPolicy(Qt::StrongFocus);
    setBorderColor(palette().color(QPalette::Base));
    setBackgroundColor(palette().color(QPalette::Base));
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
}

void TagEditor::initializeLayout()
{
    totalLayout->addStretch(1);
    totalLayout->addWidget(crumbEdit);
    totalLayout->addSpacing(8);
    totalLayout->addWidget(promptLabel);
    totalLayout->addStretch(1);

    backgroundFrame->setLayout(totalLayout);
    setContent(backgroundFrame);
}

void TagEditor::initializeConnect()
{
    QObject::connect(this, &TagEditor::windowDeactivate, this, &TagEditor::onFocusOut);
    QObject::connect(crumbEdit, &QTextEdit::textChanged, this, &TagEditor::filterInput);

    if (!isShowInTagDir) {
        QObject::connect(crumbEdit, &DCrumbEdit::crumbListChanged, this, [=] {
            if (!crumbEdit->property("updateCrumbsColor").toBool())
                processTags();
        });
    }
}

void TagEditor::setupEditHeight()
{
    QTextEdit *edit = qobject_cast<QTextEdit *>(crumbEdit);
    if (edit) {
        auto doc = edit->document();
        doc->setDocumentMargin(doc->documentMargin() + 5);
        auto layout = edit->document()->documentLayout();
        connect(layout, &QAbstractTextDocumentLayout::documentSizeChanged,
                this, [this, edit] {
                    qreal docHeight = edit->document()->size().height();
                    QMargins margins = edit->contentsMargins();
                    int totalHeight = qCeil(docHeight) + margins.top() + margins.bottom();
                    edit->setFixedHeight(totalHeight);
                    QTimer::singleShot(0, this, &TagEditor::resizeWithContent);
                });
    }
}

void TagEditor::processTags()
{
    QList<QString> tags = crumbEdit->crumbList();
    QList<QUrl> tempFiles = files;

    updateCrumbsColor(TagManager::instance()->assignColorToTags(tags));
    TagManager::instance()->setTagsForFiles(tags, tempFiles);
}

void TagEditor::updateCrumbsColor(const QMap<QString, QColor> &tagsColor)
{
    if (tagsColor.isEmpty())
        return;

    crumbEdit->setProperty("updateCrumbsColor", true);
    crumbEdit->clear();

    for (auto it = tagsColor.begin(); it != tagsColor.end(); ++it) {
        DCrumbTextFormat format = crumbEdit->makeTextFormat();
        format.setText(it.key());

        format.setBackground(QBrush(it.value()));
        format.setBackgroundRadius(5);

        crumbEdit->insertCrumb(format);
    }

    crumbEdit->setProperty("updateCrumbsColor", false);
}
