#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <vector>

#include <sentence.h>
#include <QString>
#include <QStringListModel>

using namespace std;

#include "sqlitemanager.h"

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

    bool isRetry;

    ListviewModel listViewItems;

    void initDialog();
    void refreshCount();
    void initModifyTab();

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_pushButtonHelp_clicked();

    void on_pushButtonSave_clicked();

    void on_pushButtonStart_clicked();

    void on_checkBoxSavedText_clicked(bool checked);

    void on_checkBoxFriendlyText_clicked();

    void on_checkBoxDifficultText_clicked();

    void on_pushButtonAnswer_clicked();

    void on_pushButtonNext_clicked();

    void on_pushButtonRetry_clicked();

    void on_pushButtonSearch_clicked();

    void on_listViewSearchResult_clicked(const QModelIndex &index);

    void on_pushButtonChange_clicked();

    void on_pushButtonDelete_clicked();

private:
    Ui::Dialog *ui;

};

#endif // DIALOG_H
