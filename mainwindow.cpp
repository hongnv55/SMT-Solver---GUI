#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mSmtSolver = new SMTSolver();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_solverBtn_clicked()
{

//    mSmtSolver->setBIT_LENGTH(ui->numOfBits->value());

    QStringList expressList = ui->inputTextEdit->toPlainText().trimmed().split("\n");

    QStringList results =  mSmtSolver->solve(expressList);

    ui->outputResult->clear();

    foreach (QString var, results) {
        ui->outputResult->append(var);
    }
}
