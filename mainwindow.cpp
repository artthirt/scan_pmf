#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QThread>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>

#include "outputimage.h"

class Worker: public QThread{
public:
    QScopedPointer<ParserPmf> parser;
    OutputImage* mOutput = nullptr;
    MainWindow* mMain = nullptr;

    Worker(): QThread(){

        parser.reset(new ParserPmf);

        moveToThread(this);
        start();
    }
    ~Worker(){
        quit();
        wait();

        parser.reset();
    }

    void loadImage(const QString& fn, int id){
        QTimer::singleShot(0, this, [fn, this](){
            int max = 0;

            matrixus_t ret;
            if(fn.endsWith(".pgm") || fn.endsWith(".pfm")){
                ret = parser->scanFile(fn, "", max, QRect());
                if(ret.empty())
                    return ;

                Mat mat(ret.rows, ret.cols, CV_16UC3);

                uint16_t* put = reinterpret_cast<uint16_t*>(mat.ptr(0));
                for(int i = 0; i < ret.total(); ++i){
                    put[i * 3 + 0] = 65535. / max * ret[i];
                    put[i * 3 + 1] = put[i * 3 + 0];
                    put[i * 3 + 2] = put[i * 3 + 0];
                }
                if(mOutput){
                    mOutput->setImage(mat);
                }
            }else{
                QImage image;
                image.load(fn);
                if(image.isNull())
                    return;

                int type = CV_8U;
                if(image.format() == QImage::Format_RGB888){
                    type = CV_8UC3;
                }
                if(image.format() == QImage::Format_Grayscale16){
                    image.convertTo(QImage::Format_RGB888);
                    type = CV_8UC3;
                }

                Mat mat(image.height(), image.width(), type, image.bits());

                if(mOutput){
                    mOutput->setImage(mat);
                }
            }

        });
    }
    void loadMask(const QString& fn, int id){
        QTimer::singleShot(0, this, [fn, this](){
            parser->loadMask(fn, QRect());
            auto ret = parser->mask();
            float max = parser->MaximumMask();

            if(ret.empty())
                return ;

            Mat mat(ret.rows, ret.cols, CV_16U);

            uint16_t* put = reinterpret_cast<uint16_t*>(mat.ptr(0));
            for(int i = 0; i < ret.total(); ++i){
                put[i] = 65535. / max * ret[i];
            }

            if(mOutput){
                mOutput->setImage(mat);
            }
        });
    }

    void showMask(){
        QTimer::singleShot(0, this, [this](){
            auto ret = parser->mask();
            float max = parser->MaximumMask();

            if(ret.empty())
                return ;

            Mat mat(ret.rows, ret.cols, CV_16U);

            uint16_t* put = reinterpret_cast<uint16_t*>(mat.ptr(0));
            for(int i = 0; i < ret.total(); ++i){
                put[i] = 65535. / max * ret[i];
            }

            if(mOutput){
                mOutput->setImage(mat);
            }
        });

    }

    void Start(const QStringList& fileList, const QString &saveDir){
        QTimer::singleShot(0, this, [this, fileList, saveDir](){
            mIsRunning = true;
            parser->clearOutputDir();
            parser->setSaveDir(saveDir);
            parser->scanDirPgm(fileList, "");
            mIsRunning = false;
            if(mMain){
                mMain->showStatus("Process ended", 1);
            }
        });
    }

    QStringList filesOutput() const{
        return parser->filesOutputs();
    }

    bool isRunning() const{
        return mIsRunning;
    }

    float progress() const{
        return parser->progress();
    }

protected:
    virtual void run(){
        exec();
    }

private:
    bool mIsRunning = false;
};

/////////////////////////////////

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    loadSettings();

    mWorker.reset(new Worker);

    mWorker->mOutput = ui->widgetOutput;
    mWorker->mMain = this;

    updateModel();

    connect(&mTimer, &QTimer::timeout, this, &MainWindow::onTimeout);
    mTimer.start(200);

    setWindowState(Qt::WindowMaximized);
}

MainWindow::~MainWindow()
{
    saveSettings();

    mWorker.reset();
    delete ui;
}

void MainWindow::showStatus(const QString msg, float val)
{
    QTimer::singleShot(0, this, [this, msg, val](){
        ui->statusbar->showMessage(msg);

        if(val == 1){
            mFileOutputList = mWorker->filesOutput();
            updateModel();
        }
    });
}

void MainWindow::on_pbOpenMask_clicked()
{
    QString sl = QFileDialog::getOpenFileName(nullptr, "Open image", mFileDir,
                                                    "*.pgm; *.pmf");

    if(!sl.isEmpty()){
        mWorker->loadMask(sl, 0);
    }
}

void MainWindow::on_pbShowMask_clicked()
{
    mWorker->showMask();
}

void MainWindow::on_pbOpenImages_clicked()
{
    QStringList sl = QFileDialog::getOpenFileNames(nullptr, "Open images", mFileDir,
                                                    "*.pgm; *.pmf");

    if(!sl.empty()){
        QFileInfo ffi(sl.front());
        mFileDir = ffi.absolutePath();
        mFileList = sl;
        saveSettings();
        updateModel();
    }
}

void MainWindow::onTimeout()
{
    if(mWorker){
        if(mWorker->isRunning()){
            float prg = mWorker->progress() * 100;
            ui->progressBarProcess->setValue(prg);
        }
    }
}

void MainWindow::updateModel()
{
    if(!mFileList.empty()){
        mModelInput.clear();

        int id = 0;
        for(auto s: mFileList){
            QFileInfo fi(s);
            mModelInput.appendRow(new QStandardItem(QString::number(id++) + ":  " + fi.fileName()));
        }
        ui->listViewInputs->setModel(&mModelInput);
    }

    if(!mFileOutputList.empty()){
        mModelOutput.clear();

        int id = 0;
        for(auto s: mFileOutputList){
            QFileInfo fi(s);
            mModelOutput.appendRow(new QStandardItem(QString::number(id++) + ":  " + fi.fileName()));
        }
        ui->listViewOutputs->setModel(&mModelOutput);
    }
}

void MainWindow::loadSettings()
{
    QSettings settings("settings.ini", QSettings::IniFormat);

    mFileDir = settings.value("filedir").toString();
    mFileList = settings.value("filelist").toStringList();
}

void MainWindow::saveSettings()
{
    QSettings settings("settings.ini", QSettings::IniFormat);

    settings.setValue("filedir", mFileDir);
    settings.setValue("filelist", mFileList);
}


void MainWindow::on_listViewInputs_doubleClicked(const QModelIndex &index)
{
    if(mWorker){
        int id = index.row();
        QString fileName = mFileList[id];
        mWorker->loadImage(fileName, id);
    }
}


void MainWindow::on_pbStart_clicked()
{
    if(mWorker && !mFileList.empty()){

        mWorker->parser->setUseMedianFilter(!ui->chbUseGaussian->isChecked());
        mWorker->parser->setUseMask(ui->chbUseMask->isChecked());
        mWorker->parser->setUseInv(ui->chbUseInvert->isChecked());
        mWorker->parser->setUseFilter(ui->chbUseFilter->isChecked());
        mWorker->parser->setBlurIter(ui->sbIterFilter->value());
        mWorker->parser->setKernelSize(ui->sbKernelSize->value());
        if(ui->chbUseNeededWidth->isChecked()){
            mWorker->parser->setNeededWidth(ui->sbNeededWidth->value());
        }
        if(ui->chbUseRect->isChecked()){
            QRect rect;
            rect.setX(ui->sbXRect->value());
            rect.setY(ui->sbYRect->value());
            rect.setWidth(ui->sbWRect->value());
            rect.setHeight(ui->sbHRect->value());
            mWorker->parser->setRect(rect);
        }

        mWorker->Start(mFileList, ui->lineEditSaveDir->text());
    }
}


void MainWindow::on_listViewOutputs_doubleClicked(const QModelIndex &index)
{
    if(mWorker){
        int id = index.row();
        QString fileName = mFileOutputList[id];
        mWorker->loadImage(fileName, id);
    }
}

