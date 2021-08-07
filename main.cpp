#include <QCoreApplication>
#include <QApplication>

#include "mainwindow.h"

#include "parserpmf.h"

void gen1()
{
    ParserPmf parser;
    parser.loadMask("d:\\develop\\dir15\\data\\KALAN_SCANS\\FlatField_r0000000000.pmf", QRect(0, 512, 3840, 256));
    parser.setUseMask(true);
    parser.setRect(QRect(720, 0, 1100, 256));
    parser.setUseFilter(true);
    parser.setUseInv(true);
    parser.setNeededWidth(1100);
    parser.scanDir("d:\\develop\\dir15\\data\\KALAN_SCANS\\", "sample_AngleNumber");
}

void gen2()
{
    ParserPmf parser;

    parser.clearOutputDir();

    //parser.loadMask("d:\\develop\\dir15\\data\\M4\\FF\\000.pmf", QRect(0, 0, 3840, 256));
    parser.setUseMask(true);
    parser.setRect(QRect(1950, 0, 480, 256));
    parser.loadMask("d:/develop/dir15/data/14052021_1/FF/00000_00000.txt", QRect(0, 0, 3840, 256));
    parser.setMax(50000);
    parser.setAngleRange(0, 360);
    parser.setThreshold(0.01);
    //d:\develop\dir15\data\M4_\Scans\
    //parser.scanDir("d:\\develop\\dir15\\data\\M4\\Scans", "");
    //parser.scanDir("d:\\develop\\dir15\\data\\M4_\\Scans\\", "");
    parser.setNeededWidth(480);
    //parser.setUseInv(true);
    parser.setUseFilter(true);
    parser.setRemove256RemoveLine(true);
    parser.setBlurIter(1);
    parser.setKernelSize(9);
    parser.scanDir("d:/develop/dir15/data/14052021_1/Scans/", "");
}

void gen3()
{
    ParserPmf parser;

    parser.clearOutputDir();

    //parser.setUseMask(true);
    parser.setRect(QRect(1763, 0, 350, 256));
    //parser.loadMask("d:/develop/dir15/data/14052021_1/FF/00000_00000.txt", QRect(0, 0, 3840, 256));
    parser.setMax(255);
    parser.setAngleRange(0, 360);
    parser.setThreshold(0.55);
    parser.setThresholdAsDynamicRange(true);
    parser.setNeededWidth(350);
    //parser.setUseInv(true);
    parser.setUseFilter(true);
    parser.setRemove256RemoveLine(true);
    parser.setBlurIter(3);
    parser.setKernelSize(3);
    parser.setUseNonLinearLut(true);
    //parser.setUseMedianFilter(false);
    parser.scanDirPgm("d://develop//dir15//data//PGM//Pattern_with_M3", "");

}

void gen4()
{
    ParserPmf parser;

    parser.clearOutputDir();

    parser.setUseMask(true);
    parser.setRect(QRect(1178, 0, 540, 256));
    parser.loadMask("d:\\develop\\dir15\\data\\PGM 2\\Pattern_FF\\0000.pgm", QRect(0, 0, 3840, 256));
    parser.setMax(255);
    parser.setUseInv(true);
    parser.setAngleRange(0, 360);
    //parser.setThreshold(0.1);
    //parser.setThresholdAsDynamicRange(true);
    parser.setNeededWidth(540);
    parser.setUseFilter(true);
    //parser.setRemove256RemoveLine(true);
    //parser.setBlurIter(1);
    parser.setKernelSize(5);
    //parser.setUseNonLinearLut(true);
    //parser.setUseMedianFilter(false);
    parser.scanDirPgm("d:\\develop\\dir15\\data\\PGM 2\\Pattern\\", "");

}

int main(int argc, char *argv[])
{
#if 0
    QCoreApplication a(argc, argv);

    qDebug("begin...");

    gen4();

    qDebug("end");

    return 0;
#else
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
#endif
}
