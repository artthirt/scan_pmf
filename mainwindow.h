#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QStandardItemModel>

#include "parserpmf.h"

namespace Ui {
class MainWindow;
}

class Worker;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showStatus(const QString msg, float val);

private slots:
    void on_pbOpenMask_clicked();

    void on_pbShowMask_clicked();

    void on_pbOpenImages_clicked();

    void onTimeout();

    void on_listViewInputs_doubleClicked(const QModelIndex &index);

    void on_pbStart_clicked();

    void on_listViewOutputs_doubleClicked(const QModelIndex &index);

    void on_chbShowLine_clicked(bool checked);

    void on_dsbYOffset_valueChanged(double arg1);

private:
    Ui::MainWindow *ui;

    QScopedPointer<Worker> mWorker;
    QStringList mFileList;
    QStringList mFileOutputList;
    QString mFileDir;
    QString mMaskFile;
    QStandardItemModel mModelInput;
    QStandardItemModel mModelOutput;

    QTimer mTimer;

    void updateModel();
    void loadSettings();
    void saveSettings();

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // MAINWINDOW_H
