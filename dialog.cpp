#include "dialog.h"
#include "ui_dialog.h"

#include <QMessageBox>
#include <QDebug>
#include <QKeyEvent>

#include "webmanager.h"
#include "filemanager.h"
#include "userdefs.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    currentSentIdx(-1),
    isRetry(false),
    speech(new QTextToSpeech(this))
{
    ui->setupUi(this);

    initDialog();
}

Dialog::~Dialog()
{
    delete ui;

    if( speech != NULL )
        delete speech;
    if( progressDialog != NULL )
        delete progressDialog;
}

void Dialog::initDialog(){
    this->setMaximumWidth(this->width());
    this->setMaximumHeight(this->height());

    ui->textEditAnswerInput->installEventFilter(this);
    ui->textEditAnswerInput->setEnabled(false);

    ui->listViewSearchResult->setModel(&listViewItems);

    ui->lineEditWord2->installEventFilter(this);
    ui->lineEditSearchKeyword->installEventFilter(this);

    QString ver = FileManager::iniRead("version");
    this->setWindowTitle(QString("EnStudy v.").append(ver));

    // reading db list
    QStringList dbList = FileManager::dbList();
    foreach(QString db, dbList)
        ui->comboBoxDB->addItem(db);

    connect(ui->comboBoxDB, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_comboBoxDB_IndexChanged(QString)));

    if( dbList.isEmpty() ){
        dbManager.initializeDB("default.sqlite");
        ui->comboBoxDB->addItem("default.sqlite");
        ui->comboBoxDB->setCurrentIndex(0);
    }
    else{
        QString usedDB = FileManager::iniRead("db");

        bool selectFlag = false;

        if( !usedDB.isEmpty() ){
            for(int i=0; i<dbList.size(); ++i){
                if( usedDB.compare(dbList[i]) == 0) {
                    ui->comboBoxDB->setCurrentIndex(i);
                    selectFlag = true;
                    break;
                }
            }
        }
        if( !selectFlag )
            ui->comboBoxDB->setCurrentIndex(0);

        dbManager.initializeDB(ui->comboBoxDB->currentText());
    }

    // add the slider bar's event
    connect(ui->horizontalSliderVolume, &QSlider::valueChanged, this, &Dialog::on_settings_changed);
    connect(ui->horizontalSliderRate, &QSlider::valueChanged, this, &Dialog::on_settings_changed);
    connect(ui->horizontalSliderPitch, &QSlider::valueChanged, this, &Dialog::on_settings_changed);

    // read the previous saved values
    QString volume = FileManager::iniRead("volume");
    QString rate = FileManager::iniRead("rate");
    QString pitch = FileManager::iniRead("pitch");

    if( !volume.isEmpty() )
        ui->horizontalSliderVolume->setValue(volume.toInt());
    if( !rate.isEmpty() )
        ui->horizontalSliderRate->setValue(rate.toInt());
    if( !pitch.isEmpty() )
        ui->horizontalSliderPitch->setValue(pitch.toInt());

    on_settings_changed();

    // reading counter values
    refreshCount();

    // initilaize the web db list
    connect(&webManager, SIGNAL(notifyFinished(QString)), this, SLOT(on_web_download_finished(QString)));
    connect(&webManager, SIGNAL(notifyProgress(QString,qint64,qint64)), this, SLOT(on_web_download_progress(QString,qint64,qint64)));

    webManager.request(DB_LIST_URL, false);

    // for temporary progress dialog
    progressDialog = new ProgressDialog();
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
        if( dbManager.getStudySentences(studyList, ui->radioButtonAll->isChecked(), ui->radioButtonStudied->isChecked(),
            ui->radioButtonNotStudied->isChecked(), ui->radioButtonHard->isChecked(), ui->radioButtonNew->isChecked()) ){
            ui->progressBarProgress->setRange(0, studyList.size());

            on_pushButtonNext_clicked();
        }else{
            QMessageBox::warning(this, tr("error"), tr("Failed to get the sentence! please contact to developer!"));
            return;
        }

        ui->textEditAnswerInput->setEnabled(true);
        ui->pushButtonStart->setText("Study Stop");

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

        ui->pushButtonAnswer->setEnabled(false);
        ui->pushButtonNext->setEnabled(false);
        ui->pushButtonRetry->setEnabled(false);
        ui->pushButtonSpeech->setEnabled(false);
    }
}

void Dialog::on_radioButtonAll_clicked(bool checked)
{
    if( checked )
        ui->labelSelectedText->setText(QString::number(dbManager.getAllCount()));
}

void Dialog::on_radioButtonStudied_clicked(bool checked)
{
    if( checked )
        ui->labelSelectedText->setText(QString::number(dbManager.getStudiedCount()));
}

void Dialog::on_radioButtonNotStudied_clicked(bool checked)
{
    if( checked )
        ui->labelSelectedText->setText(QString::number(dbManager.getNotStudiedCount()));
}

void Dialog::on_radioButtonHard_clicked(bool checked)
{
    if( checked )
        ui->labelSelectedText->setText(QString::number(dbManager.getHardCount()));
}

void Dialog::on_radioButtonNew_clicked(bool checked)
{
    if( checked )
        ui->labelSelectedText->setText(QString::number(dbManager.getNewCount()));
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

    // enable retry and speech
    ui->pushButtonRetry->setEnabled(true);
    ui->pushButtonSpeech->setEnabled(true);

    // click the speech button.
    on_pushButtonSpeech_clicked();

    if( !isRetry )
        dbManager.setStudyResult(answer.replace("'","''"), sAnswer.size() == findCount);
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
    ui->pushButtonSpeech->setEnabled(false);

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

// 151112 text to speech function
void Dialog::on_pushButtonSpeech_clicked()
{
    // studyList[currentSentIdx].english
    speech->say(studyList[currentSentIdx].english);
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

void Dialog::on_settings_changed(){
    int volume = ui->horizontalSliderVolume->value();
    int rate = ui->horizontalSliderRate->value();
    int pitch = ui->horizontalSliderPitch->value();

    speech->setVolume(volume);
    speech->setRate(rate/10.0);
    speech->setPitch(pitch/10.0);

    QString currentDB = ui->comboBoxDB->currentText();

    // save to ini file
    FileManager::iniWrite("volume", QString::number(volume));
    FileManager::iniWrite("rate", QString::number(rate));
    FileManager::iniWrite("pitch", QString::number(pitch));
}

void Dialog::on_pushButtonSoundTest_clicked(){
    speech->say(QString("This program is for english study."));
}

void Dialog::on_pushButtonAddDB_clicked(){
    QString selectDB = ui->comboBoxWebDB->currentText();

    webManager.requestPush(QString(DB_URL).append(selectDB), true, QString("./db/").append(selectDB));
    webManager.requestRun();
}

void Dialog::on_web_download_finished(QString dstPath){
    if( dstPath.isEmpty() ){
        QStringList dbList = webManager.getResultString().split("\n");
        foreach(QString db, dbList)
            ui->comboBoxWebDB->addItem(db);
        if( !dbList.isEmpty() ){
            ui->comboBoxWebDB->setCurrentIndex(0);
            ui->pushButtonAddDB->setEnabled(true);
        }
    }
    else{
        progressDialog->hide();
        QFileInfo info(dstPath);
        QMessageBox::information(this, tr("Complete"), QString("<b>%1</b> download & add Ok!").arg(info.fileName()));

        ui->comboBoxDB->addItem(info.fileName());
    }
}

void Dialog::on_web_download_progress(QString dstPath, qint64 readBytes, qint64 totalBytes){
    if( !dstPath.isEmpty() ){
        QFileInfo info(dstPath);
        progressDialog->show();
        progressDialog->setLabelText(info.fileName().append(" downloading.."));
        progressDialog->setMaximum(totalBytes);
        progressDialog->setValue(readBytes);
    }
}

void Dialog::on_comboBoxDB_IndexChanged(QString text){
    dbManager.initializeDB(text);
    FileManager::iniWrite("db", text);
    refreshCount();
}
