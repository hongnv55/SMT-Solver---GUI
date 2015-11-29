#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "smtsolver.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_solverBtn_clicked();

private:
    Ui::MainWindow *ui;

    SMTSolver* mSmtSolver;
};

#endif // MAINWINDOW_H
