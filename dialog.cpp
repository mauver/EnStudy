#include "dialog.h"
#include "ui_dialog.h"

#include <QMessageBox>
#include <QDebug>
#include <QKeyEvent>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    currentSentIdx(-1),
    isRetry(false)
{
    ui->setupUi(this);

    initDialog();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::initDialog(){
    ui->textEditAnswerInput->installEventFilter(this);
    ui->textEditAnswerInput->setEnabled(false);

    ui->listViewSearchResult->setModel(&listViewItems);

    ui->lineEditWord2->installEventFilter(this);
    ui->lineEditSearchKeyword->installEventFilter(this);

    refreshCount();
}

void Dialog::refreshCount(){
    int allCount = dbManager.getAllCount();
    int studiedCount = dbManager.getStudiedCount();
    int easyCount = dbManager.getEasyCount();
    int hardCount = dbManager.getHardCount();

    ui->labelSavedText->setText(QString::number(allCount));
    ui->labelSavedText2->setText(QString::number(allCount));
    ui->labelStudiedText->setText(QString::number(studiedCount));
    ui->labelFriendlyText->setText(QString::number(easyCount));
    ui->labelDifficultText->setText(QString::number(hardCount));
    ui->labelSelectedText->setText(QString::number(allCount));
}

bool Dialog::eventFilter(QObject *obj, QEvent *event){
    if( event->type() == QEvent::KeyPress ){
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if( obj == ui->textEditAnswerInput ){
            if( keyEvent->key() == Qt::Key_Return && keyEvent->modifiers().testFlag(Qt::ControlModifier)){
                // Ctrl+enter
                if( ui->pushButtonAnswer->isEnabled() )
                    on_pushButtonAnswer_clicked();
            }
            else if( keyEvent->modifiers().testFlag(Qt::ControlModifier) && keyEvent->nativeVirtualKey() == 39 ){
                // Ctrl + right
                if( ui->pushButtonNext->isEnabled() )
                    on_pushButtonNext_clicked();
            }
            else if( keyEvent->modifiers().testFlag(Qt::ControlModifier) && keyEvent->nativeVirtualKey() == 37 ){
                // Ctrl + left
                if( ui->pushButtonRetry->isEnabled() )
                    on_pushButtonRetry_clicked();
            }
        }
        else if( obj == ui->lineEditSearchKeyword ){
            if( keyEvent->key() == Qt::Key_Return ){
                if( ui->pushButtonSearch->isEnabled() )
                    on_pushButtonSearch_clicked();
            }
        }
        else if( obj == ui->lineEditWord2 ){
            if( keyEvent->key() == Qt::Key_Return ){
                if( ui->pushButtonSave->isEnabled() )
                    on_pushButtonSave_clicked();
            }
        }
    }

    return QDialog::eventFilter(obj, event);
}

void Dialog::on_pushButtonHelp_clicked()
{
    QMessageBox::information(this, tr("Study range notification"),
                             tr("This is the description for study range.\n\n"
                                "Saved Sentence: Inserted sentence by using sentence insertion tab\n"
                                "New Sentence: the newly inserted sentence\n"));
}

void Dialog::on_pushButtonSave_clicked()
{
    QString english = ui->textEditInputEnglish->toPlainText().toLower().trimmed();
    QString korean = ui->textEditInputKorean->toPlainText().toLower().trimmed();
    QString word = ui->lineEditWord->text().toLower().trimmed();
    QString word2 = ui->lineEditWord2->text().toLower().trimmed();

    // replace "'" to "''" for sql query
    english = english.replace("'", "''");

    if( korean.isEmpty() || english.isEmpty() ||
        word.isEmpty() || word2.isEmpty() ){
        QMessageBox::warning(this, tr("error"), tr("checking the empty space!"));
        return;
    }
    else if( dbManager.checkSentence(english) ){    // if exists, return true
        QMessageBox::warning(this, tr("error"), tr("already exists sentence!"));
        return;
    }

    if( dbManager.insertSentence(english, korean, word, word2) ){
        QMessageBox::information(this, tr("Notification"), tr("Ok!"));

        refreshCount();
        ui->textEditInputEnglish->clear();
        ui->textEditInputKorean->clear();
        ui->lineEditWord->clear();
        ui->lineEditWord2->clear();

        ui->textEditInputEnglish->setFocus();
    }
    else{
        QMessageBox::warning(this, tr("error"), tr("Failed to insert the sentence! please contact to developer!"));
    }
}

void Dialog::on_pushButtonStart_clicked(){
    QString buttonStatus = ui->pushButtonStart->text();

    if( buttonStatus.compare("Study Start") == 0 ){
        if( ui->labelSelectedText->text().toInt() == 0 ){
            QMessageBox::warning(this, tr("error"), tr("No selected sentence! Please insert your sentence"));
            return;
        }

        // getting the study sentence
        if( dbManager.getStudySentences(studyList, ui->checkBoxSavedText->isChecked(),
                                        ui->checkBoxFriendlyText->isChecked(), ui->checkBoxDifficultText->isChecked()) ){
            ui->progressBarProgress->setRange(0, studyList.size());

            qDebug() << studyList.size();

            on_pushButtonNext_clicked();
        }else{
            QMessageBox::warning(this, tr("error"), tr("Failed to get the sentence! please contact to developer!"));
            return;
        }

        ui->textEditAnswerInput->setEnabled(true);
        ui->pushButtonStart->setText("Study Stop");
        ui->checkBoxSavedText->setEnabled(false);
        ui->checkBoxDifficultText->setEnabled(false);
        ui->checkBoxFriendlyText->setEnabled(false);

        ui->pushButtonAnswer->setEnabled(true);
        ui->pushButtonNext->setEnabled(true);
    }
    else{
        ui->textEditAnswerInput->setEnabled(false);
        ui->pushButtonStart->setText("Study Start");

        currentSentIdx = -1;
        studyList.clear();
        ui->labelProgress->setText("0/0");
        ui->progressBarProgress->setValue(0);

        ui->textEditQuestion->clear();
        ui->textEditAnswer->clear();
        ui->textEditAnswerInput->clear();
        ui->labelContainWord->clear();

        ui->checkBoxSavedText->setEnabled(true);

        if( !ui->checkBoxSavedText->isChecked() ){
            ui->checkBoxDifficultText->setEnabled(true);
            ui->checkBoxFriendlyText->setEnabled(true);
        }

        ui->pushButtonAnswer->setEnabled(false);
        ui->pushButtonNext->setEnabled(false);
        ui->pushButtonRetry->setEnabled(false);
    }
}

void Dialog::on_checkBoxSavedText_clicked(bool checked){
    if( checked ){
        ui->checkBoxFriendlyText->setChecked(true);
        ui->checkBoxFriendlyText->setEnabled(false);
        ui->checkBoxDifficultText->setChecked(true);
        ui->checkBoxDifficultText->setEnabled(false);

        ui->labelSelectedText->setText(QString::number(dbManager.getAllCount()));
    }
    else{
         ui->checkBoxFriendlyText->setEnabled(true);
         ui->checkBoxDifficultText->setEnabled(true);

         ui->labelSelectedText->setText(QString::number(dbManager.getEasyCount() + dbManager.getHardCount()));
    }
}

void Dialog::on_checkBoxFriendlyText_clicked()
{
    int res = 0;

    if( ui->checkBoxFriendlyText->isChecked() )
        res += dbManager.getEasyCount();
    if( ui->checkBoxDifficultText->isChecked() )
        res += dbManager.getHardCount();

    ui->labelSelectedText->setText(QString::number(res));
}

void Dialog::on_checkBoxDifficultText_clicked()
{
    int res = 0;

    if( ui->checkBoxFriendlyText->isChecked() )
        res += dbManager.getEasyCount();
    if( ui->checkBoxDifficultText->isChecked() )
        res += dbManager.getHardCount();

    ui->labelSelectedText->setText(QString::number(res));
}

void Dialog::on_pushButtonAnswer_clicked()
{
    QString input = ui->textEditAnswerInput->toPlainText().trimmed().toLower();
    QString answer = studyList[currentSentIdx].english.toLower();

    QStringList sInput = input.split(" ");
    QStringList sAnswer = answer.split(" ");
    QString coloredAnswer = "";

    bool *checker = new bool[sInput.size()];
    memset(checker, 0, sizeof(bool)*sInput.size());

    int findCount = 0;
    for(int i=0; i<sAnswer.size(); ++i){
        QString tempAns = sAnswer[i];

        int find = -1;
        for(int j=0; j<sInput.size() && j<=i; ++j){
            QString tempIn = sInput[j];

            // equal and not checked word only.
            if( tempAns.compare(tempIn) == 0 && !checker[j] ){
                find = j;
                checker[j] = true;
                findCount++;
                break;
            }
        }

        if( find == -1 ){
            QString dash = "";
            dash.fill('-', tempAns.length());
            coloredAnswer += "<font color='red'><b> " + dash + " </b></font>";
        }
        else{
            coloredAnswer += "<font color='blue'><b>" + sInput[find] + " </b></font>";
        }
    }

    delete[] checker;

    coloredAnswer.append("(" + QString::number( (int)(findCount/(double)sAnswer.size()* 100.) ) + "% correct!)");

    QString output = "<b>* Input&nbsp;&nbsp;&nbsp;: </b>" + coloredAnswer + "<br><b>* Answer : </b><font color='blue'><b>" + answer + "</b>";
    ui->textEditAnswer->setText(output);

    ui->pushButtonAnswer->setEnabled(false);

    // enable retry
    ui->pushButtonRetry->setEnabled(true);

    if( !isRetry )
        dbManager.setStudyResult(answer, sAnswer.size() == findCount);
}

void Dialog::on_pushButtonNext_clicked()
{
    if( currentSentIdx != -1 && ui->textEditAnswer->toPlainText().trimmed().isEmpty() ){
        int res = QMessageBox::question(this, tr("Skip check"), tr("Skip this question? You didn't answer!"));
        if( res != QMessageBox::Yes )
            return;
    }

    isRetry = false;
    currentSentIdx++;
    if( currentSentIdx >= studyList.size() ){
         QMessageBox::information(this, tr("Study ended!"), tr("Study ended!"));
         on_pushButtonStart_clicked();
         return;
    }

    // setting the question
    int level = dbManager.getSentenceLevel(studyList[currentSentIdx].english);

    if(level == level::easy)
        ui->textEditQuestion->setText("<font color='blue'>" + studyList[currentSentIdx].korean + "</font>");
    else if(level == level::hard)
        ui->textEditQuestion->setText("<font color='red'>" + studyList[currentSentIdx].korean + "</font>");
    else
        ui->textEditQuestion->setText(studyList[currentSentIdx].korean);

    ui->labelContainWord->setText(studyList[currentSentIdx].word + " : " + studyList[currentSentIdx].word2);

    // setting the progress
    ui->labelProgress->setText(QString("%1/%2").arg(QString::number(currentSentIdx+1), QString::number(studyList.size())));
    ui->progressBarProgress->setValue(currentSentIdx+1);
    ui->pushButtonAnswer->setEnabled(true);

    // clear previous input, question
    ui->textEditAnswer->clear();
    ui->textEditAnswerInput->clear();

    // retry disable
    ui->pushButtonRetry->setEnabled(false);

    refreshCount();
}

void Dialog::on_pushButtonRetry_clicked()
{
    isRetry = true;

    // Clear input, answer edittext
    ui->pushButtonAnswer->setEnabled(true);
    ui->textEditAnswer->clear();
    ui->textEditAnswerInput->clear();
}

/** This section is sentence change functions **/
void Dialog::initModifyTab(){
    searchList.clear();
    idList.clear();

     ui->labelSearchCount->setText("0");

     // clear previous result
     ui->textEditSelectEnglish->clear();
     ui->textEditSelectKorean->clear();
     ui->lineEditSelectWord->clear();
     ui->lineEditSelectWord2->clear();

     // Remove previous result
     if( listViewItems.rowCount() > 0 )
         listViewItems.removeRows(0, listViewItems.rowCount());

     ui->textEditSelectEnglish->setEnabled(false);
     ui->textEditSelectKorean->setEnabled(false);
     ui->lineEditSelectWord->setEnabled(false);
     ui->lineEditSelectWord2->setEnabled(false);
     ui->listViewSearchResult->setEnabled(false);
     ui->pushButtonChange->setEnabled(false);
     ui->pushButtonDelete->setEnabled(false);
}

void Dialog::on_pushButtonSearch_clicked()
{
    QString searchKeyword = ui->lineEditSearchKeyword->text().toLower().trimmed();

    if( searchKeyword.isEmpty() ){
        QMessageBox::warning(this, tr("error"), tr("No search keyword!"));
        return;
    }

    // replace "'" to "''" for sql query
    searchKeyword = searchKeyword.replace("'", "''");


    initModifyTab();

    if( dbManager.searchSentence(searchList, idList, searchKeyword) ){
        int resCount = searchList.size();

        ui->labelSearchCount->setText(QString::number(resCount));

        if( resCount > 0 ){
            // enable all components for result
            ui->textEditSelectEnglish->setEnabled(true);
            ui->textEditSelectKorean->setEnabled(true);
            ui->lineEditSelectWord->setEnabled(true);
            ui->lineEditSelectWord2->setEnabled(true);
            ui->listViewSearchResult->setEnabled(true);

            // showing the result to result list
            for(int i=0; i<searchList.size(); ++i)
                listViewItems.append(searchList[i].english);

            ui->listViewSearchResult->setCurrentIndex(ui->listViewSearchResult->model()->index(0, 0));
            on_listViewSearchResult_clicked(ui->listViewSearchResult->model()->index(0, 0));
        }
        else{
            QMessageBox::information(this, tr("Search result"), tr("No result!"));
        }
    }
    else
        QMessageBox::warning(this, tr("error"), tr("DB connection error! contact to developer"));
}

void Dialog::on_listViewSearchResult_clicked(const QModelIndex &index)
{
    ui->textEditSelectEnglish->setText(searchList[index.row()].english);
    ui->textEditSelectKorean->setText(searchList[index.row()].korean);
    ui->lineEditSelectWord->setText(searchList[index.row()].word);
    ui->lineEditSelectWord2->setText(searchList[index.row()].word2);
    ui->pushButtonChange->setEnabled(true);
    ui->pushButtonDelete->setEnabled(true);
}

void Dialog::on_pushButtonChange_clicked()
{
    int id = idList[ui->listViewSearchResult->selectionModel()->selectedRows()[0].row()];
    QString english = ui->textEditSelectEnglish->toPlainText().toLower().trimmed();
    QString korean = ui->textEditSelectKorean->toPlainText().toLower().trimmed();
    QString word = ui->lineEditSelectWord->text().toLower().trimmed();
    QString word2 = ui->lineEditSelectWord2->text().toLower().trimmed();

    // replace "'" to "''" for sql query
    english = english.replace("'", "''");
    korean = korean.replace("'", "''");
    word = word.replace("'", "''");
    word2 = word2.replace("'", "''");

    if( dbManager.modifySentence(id, english, korean, word, word2) ){
        QMessageBox::information(this, tr("Modify notification"), tr("Ok!"));

        initModifyTab();
    }
    else
        QMessageBox::warning(this, tr("error"), tr("DB connection error! contact to developer"));
}

void Dialog::on_pushButtonDelete_clicked()
{
    int id = idList[ui->listViewSearchResult->selectionModel()->selectedRows()[0].row()];

    if( dbManager.deleteSentence(id) ){
        QMessageBox::information(this, tr("Delete notification"), tr("Ok!"));

        initModifyTab();
        refreshCount();
    }
    else
        QMessageBox::warning(this, tr("error"), tr("DB connection error! contact to developer"));
}
