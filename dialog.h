#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <vector>
#include <sentence.h>
#include <QString>
#include <QStringListModel>

#include <QtTextToSpeech/QTextToSpeech>

#include "sqlitemanager.h"
#include "webmanager.h"

#include "progressdialog.h"

using namespace std;

namespace Ui {
class Dialog;
}

class ListviewModel : public QStringListModel
{
public:
  void append (const QString& string){
    insertRows(rowCount(), 1);
    setData(index(rowCount()-1), string);
  }

  ListviewModel& operator<<(const QString& string){
    append(string);
    return *this;
  }
};

class Dialog : public QDialog
{
    Q_OBJECT

private:
    sqliteManager dbManager;
    vector<Sentence> studyList;
    vector<Sentence> searchList;
    vector<int> idList;
    int currentSentIdx;

    // For speech
    QTextToSpeech *speech;

    bool isRetry;

    ListviewModel listViewItems;

    // For web db
    QWebManager webManager;

    void initDialog();
    void refreshCount();
    void initModifyTab();

    void on_settings_changed();

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_pushButtonSave_clicked();

    void on_pushButtonStart_clicked();

    void on_pushButtonAnswer_clicked();

    void on_pushButtonNext_clicked();

    void on_pushButtonRetry_clicked();

    void on_pushButtonSearch_clicked();

    void on_listViewSearchResult_clicked(const QModelIndex &index);

    void on_pushButtonChange_clicked();

    void on_pushButtonDelete_clicked();

    void on_radioButtonAll_clicked(bool checked);

    void on_radioButtonStudied_clicked(bool checked);

    void on_radioButtonNotStudied_clicked(bool checked);

    void on_radioButtonHard_clicked(bool checked);

    void on_radioButtonNew_clicked(bool checked);

    void on_pushButtonSpeech_clicked();

    void on_pushButtonSoundTest_clicked();

    void on_web_download_finished(QString dstPath);
    void on_web_download_progress(QString dstPath, qint64 readBytes, qint64 totalBytes);

    void on_pushButtonAddDB_clicked();

    void on_comboBoxDB_IndexChanged(QString text);

private:
    Ui::Dialog *ui;

    ProgressDialog *progressDialog;
};

#endif // DIALOG_H
