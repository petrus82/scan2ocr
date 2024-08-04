#include "chatwindow.h"
#include <QScrollBar>
#include <QCoreApplication>

chatWindow::chatWindow (QWidget *parent) : QWidget(parent) {
    this->resize(765, 833);
    setupWidgets();
}

void chatWindow::setupWidgets() {

    // Setup scrollArea and textPrompt using QHBoxLayout below each other
    this->setLayout(&chatLayout);
    chatLayout.addWidget(&scrollArea);
    chatLayout.addWidget(&textPrompt);

    // Setup scrollArea
    chatLayout.setSpacing(SCROLLAREA_SPACING);
    chatLayout.setContentsMargins(SCROLLAREA_MARGIN, SCROLLAREA_MARGIN, SCROLLAREA_MARGIN, SCROLLAREA_MARGIN);

    scrollArea.setWidget(&scrollAreaWidgetContent);
    scrollArea.setWidgetResizable(true);
    scrollLayout.addStretch(1);
    
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    scrollAreaWidgetContent.setSizePolicy(sizePolicy);

    scrollLayout.setSpacing(SCROLLAREA_SPACING);
    scrollLayout.setContentsMargins(SCROLLAREA_MARGIN, SCROLLAREA_MARGIN, SCROLLAREA_MARGIN, SCROLLAREA_MARGIN);

    // Connect textPrompt returnPressed to newQuestion
    QObject::connect(&textPrompt, &QLineEdit::returnPressed, this, &chatWindow::newQuestion);
    textPrompt.setPlaceholderText("Ask me anything...");
}

void chatWindow::newQuestion() {

    std::unique_ptr<QLabel> newQuestion = std::make_unique<QLabel>();
    newQuestion->setObjectName("chatQuestion");
    newQuestion->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    newQuestion->setWordWrap(true);
    newQuestion->setText(textPrompt.text());
    newQuestion->setMargin(QLABEL_MARGIN);
    newQuestion->setAlignment(Qt::AlignJustify|Qt::AlignVCenter);
    newQuestion->setFixedWidth(scrollArea.width() - 2 * QLABEL_MARGIN);
    newQuestion->adjustSize();

    scrollLayout.addWidget(newQuestion.get());
    qlabel_Questions.push_back(std::move(newQuestion));

    textPrompt.clear();
    QCoreApplication::processEvents();

    if (answerCallback) {
        answerCallback();
    }
}

void chatWindow::newAnswer(QString answer) {
    std::unique_ptr<QLabel> newAnswer = std::make_unique<QLabel>();
    newAnswer->setObjectName("chatAnswer");
    newAnswer->setWordWrap(true);
    newAnswer->setMargin(QLABEL_MARGIN);
    newAnswer->setAlignment(Qt::AlignJustify|Qt::AlignVCenter);
    newAnswer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    newAnswer->setText(answer);
    newAnswer->adjustSize();
    
    scrollLayout.addWidget(newAnswer.get());
    
    
    // Scroll to bottom
    scrollArea.widget()->resize(scrollArea.widget()->sizeHint());
    QCoreApplication::processEvents();
    scrollArea.verticalScrollBar()->setValue(scrollArea.verticalScrollBar()->maximum());
    QCoreApplication::processEvents();
    scrollArea.verticalScrollBar()->setValue(scrollArea.verticalScrollBar()->maximum());

    qlabel_Answers.push_back(std::move(newAnswer));
}

/*  scan2ocr takes a pdf file, transcodes it to TIFF G4 and assists in renaming the file.
    Copyright (C) 2024 Simon-Friedrich Böttger email ( at ) simonboettger. de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>
*/