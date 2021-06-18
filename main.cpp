#include <QCoreApplication>

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
    parser.setRect(QRect(1750, 0, 350, 256));
    //parser.loadMask("d:/develop/dir15/data/14052021_1/FF/00000_00000.txt", QRect(0, 0, 3840, 256));
    parser.setMax(255);
    parser.setAngleRange(0, 360);
    parser.setThreshold(0.5);
    parser.setThresholdAsDynamicRange(true);
    parser.setNeededWidth(350);
    //parser.setUseInv(true);
    //parser.setUseFilter(true);
    parser.setRemove256RemoveLine(true);
    //parser.setBlurIter(3);
    parser.setKernelSize(5);
    parser.setUseMedianFilter(false);
    parser.scanDirPgm("d://develop//dir15//data//PGM//Pattern_with_M3", "");

}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug("begin...");

    gen3();

    qDebug("end");

    return 0;
}