#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QThread>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QEvent>
#include <QKeyEvent>
#include <QItemSelectionModel>

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

    if(!mMaskFile.isEmpty()){
        mWorker->loadMask(mMaskFile, 0);
    }

    connect(&mTimer, &QTimer::timeout, this, &MainWindow::onTimeout);
    mTimer.start(200);

    ui->listViewInputs->installEventFilter(this);
    ui->listViewOutputs->installEventFilter(this);

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
            ui->progressBarProcess->setValue(100);
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
        mMaskFile = sl;
        ui->lineEditMask->setText(mMaskFile);
        mWorker->loadMask(sl, 0);
        saveSettings();
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
    ui->lineEditMask->setText(mMaskFile);
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

void setValue(QSettings& settings, const QString&  param, QWidget* w, QVariant def = QVariant())
{
    QCheckBox * v1 = dynamic_cast<QCheckBox*>(w);
    QSpinBox * v2 = dynamic_cast<QSpinBox*>(w);
    QDoubleSpinBox * v3 = dynamic_cast<QDoubleSpinBox*>(w);

    def = settings.value(param, def);

    if(v1){
        v1->setChecked(def.toBool());
    }
    if(v2){
        v2->setValue(def.toInt());
    }
    if(v3){
        v3->setValue(def.toFloat());
    }
}

void MainWindow::loadSettings()
{
    QSettings settings("settings.ini", QSettings::IniFormat);

    mFileDir = settings.value("filedir").toString();
    mMaskFile = settings.value("filemask").toString();
    mFileList = settings.value("filelist").toStringList();
    mFileOutputList = settings.value("fileoutlist").toStringList();

    setValue(settings, "var01", ui->sbIterFilter, 1);
    setValue(settings, "var02", ui->sbKernelSize, 3);
    setValue(settings, "var03", ui->sbMaximum, 255);
    setValue(settings, "var04", ui->sbNeededWidth, 540);
    setValue(settings, "var05", ui->sbXRect, 1178);
    setValue(settings, "var06", ui->sbYRect, 0);
    setValue(settings, "var07", ui->sbWRect, 540);
    setValue(settings, "var08", ui->sbHRect, 256);
    setValue(settings, "var09", ui->dsbAngleEnd, 360);
    setValue(settings, "var10", ui->dsbAngleStart, 0);
    setValue(settings, "var11", ui->dsbTresh, 0);
    setValue(settings, "var12", ui->chbUseFilter, false);
    setValue(settings, "var13", ui->chbUseGaussian, false);
    setValue(settings, "var14", ui->chbUseInvert, false);
    setValue(settings, "var15", ui->chbUseMask, false);
    setValue(settings, "var16", ui->chbUseNeededWidth, false);
    setValue(settings, "var17", ui->chbUseRect, false);
    setValue(settings, "var18", ui->chbUseThreshold, false);
}

void MainWindow::saveSettings()
{
    QSettings settings("settings.ini", QSettings::IniFormat);

    settings.setValue("filedir", mFileDir);
    settings.setValue("filemask", mMaskFile);
    settings.setValue("filelist", mFileList);
    settings.setValue("fileoutlist", mFileOutputList);

    settings.setValue("var01", ui->sbIterFilter->value());
    settings.setValue("var02", ui->sbKernelSize->value());
    settings.setValue("var03", ui->sbMaximum->value());
    settings.setValue("var04", ui->sbNeededWidth->value());
    settings.setValue("var05", ui->sbXRect->value());
    settings.setValue("var06", ui->sbYRect->value());
    settings.setValue("var07", ui->sbWRect->value());
    settings.setValue("var08", ui->sbHRect->value());
    settings.setValue("var09", ui->dsbAngleEnd->value());
    settings.setValue("var10", ui->dsbAngleStart->value());
    settings.setValue("var11", ui->dsbTresh->value());
    settings.setValue("var12", ui->chbUseFilter->isChecked());
    settings.setValue("var13", ui->chbUseGaussian->isChecked());
    settings.setValue("var14", ui->chbUseInvert->isChecked());
    settings.setValue("var15", ui->chbUseMask->isChecked());
    settings.setValue("var16", ui->chbUseNeededWidth->isChecked());
    settings.setValue("var17", ui->chbUseRect->isChecked());
    settings.setValue("var18", ui->chbUseThreshold->isChecked());
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
        mWorker->parser->setAngleRange(ui->dsbAngleStart->value(), ui->dsbAngleEnd->value());
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
        mWorker->parser->setThreshold(ui->chbUseThreshold->isChecked()? ui->dsbTresh->value() : 0);

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

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->listViewInputs){
        if(event->type() == QEvent::KeyRelease){
            auto sm = ui->listViewInputs->selectionModel();
            auto r = sm->selectedIndexes();
            if(r.size() == 1){
                int row = r[0].row();
                qDebug("input %d", row);
                on_listViewInputs_doubleClicked(r[0]);
            }
        }
    }
    if(watched == ui->listViewOutputs){
        if(event->type() == QEvent::KeyRelease){
            auto sm = ui->listViewOutputs->selectionModel();
            auto r = sm->selectedIndexes();
            if(r.size() == 1){
                int row = r[0].row();
                qDebug("input %d", row);
                on_listViewOutputs_doubleClicked(r[0]);
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::on_chbShowLine_clicked(bool checked)
{
    ui->widgetOutput->setShowLine(checked);
}


void MainWindow::on_dsbYOffset_valueChanged(double arg1)
{
    ui->widgetOutput->setYOffsetLine(arg1);
}


void MainWindow::on_listViewOutputs_clicked(const QModelIndex &index)
{
    on_listViewOutputs_doubleClicked(index);
}


void MainWindow::on_listViewInputs_clicked(const QModelIndex &index)
{
    on_listViewInputs_doubleClicked(index);
}

