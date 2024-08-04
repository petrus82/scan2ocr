#pragma once

#include <QObject>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QLayout>
#include <QScrollArea>

QT_BEGIN_NAMESPACE

class chatWindow : public QWidget {
    Q_OBJECT

public:
    explicit chatWindow(QWidget *parent = nullptr);
    ~chatWindow() = default;

    void newAnswer (QString answer);
    void newQuestion();
    using AnswerCallback = std::function<void()>;
    inline void setAnswerFunction (AnswerCallback callback){
        answerCallback = callback;};

private:
    static constexpr int SCROLLAREA_SPACING = 50;
    static constexpr int SCROLLAREA_MARGIN = 20;
    static constexpr int QLABEL_MARGIN = 30;

    QWidget centralWidget;
    QVBoxLayout chatLayout {&centralWidget};
    QScrollArea scrollArea {&centralWidget};
    QWidget scrollAreaWidgetContent;
    QVBoxLayout scrollLayout {&scrollAreaWidgetContent};
    
    

    QLineEdit textPrompt {&centralWidget};

    std::vector<std::unique_ptr<QLabel>> qlabel_Questions;
    std::vector<std::unique_ptr<QLabel>> qlabel_Answers;

    void setupWidgets();
    AnswerCallback answerCallback;
};

QT_END_NAMESPACE

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