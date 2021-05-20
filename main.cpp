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
    parser.setMax(1024);
    parser.setAngleRange(0, 361);
    parser.setThreshold(40);
    //d:\develop\dir15\data\M4_\Scans\
    //parser.scanDir("d:\\develop\\dir15\\data\\M4\\Scans", "");
    //parser.scanDir("d:\\develop\\dir15\\data\\M4_\\Scans\\", "");
    parser.setNeededWidth(480);
    //parser.setUseInv(true);
    parser.setUseFilter(true);
    parser.setRemove256RemoveLine(true);
    parser.setBlurIter(1);
    parser.setMedianSize(9);
    parser.scanDir("d:/develop/dir15/data/14052021_1/Scans/", "");
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug("begin...");

    gen2();

    qDebug("end");

    return 0;
}
