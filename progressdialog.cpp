#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);

    this->setMaximumWidth(this->width());
    this->setMaximumHeight(this->height());
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::setLabelText(QString text){
    ui->labelText->setText(text);
}

void ProgressDialog::setMaximum(int maximum){
    ui->progressBar->setMaximum(maximum);
}

void ProgressDialog::setValue(int value){
    ui->progressBar->setValue(value);
}
