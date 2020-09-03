#include "mywidget.h"
#include "ui_mywidget.h"
#include <QString>
#include <QIcon>
#include <QPushButton>
#include <QStringList>
#include <QDebug>
#include <QComboBox>
#include <QClipboard>
#include <QMessageBox>

typedef unsigned char uint_8;
typedef unsigned short uint_16;
typedef unsigned int uint_32;

//算法参数peizhi
QString myWidget::crcName[15] = {"CRC8","CRC8/ITU","CRC8/ROHC","CRC8/MAXIM","CRC16/IBM","CRC16/MAXIM","CRC16/USB","CRC16/MODBUS","CRC16/CCITT",
                             "CRC16/CCITT-FALSE","CRC16/X25","CRC16/XMODEM","CRC16/DNP","CRC32","CRC32/MPEG-2"};
QString myWidget::crcPoly[15] = {"07","07","07","31","8005","8005","8005","8005","1021","1021","1021","1021","3D65","04C11DB7","04C11DB7"};
QString myWidget::crcInit[15] = {"00","00","FF","00","0000","0000","FFFF","FFFF","0000","FFFF","FFFF","0000","0000","FFFFFFFF","FFFFFFFF"};
QString myWidget::crcXout[15] = {"00","55","00","00","0000","FFFF","FFFF","0000","0000","0000","FFFF","0000","FFFF","FFFFFFFF","00000000"};
bool myWidget::inputReverse[15] = {false,false,true,true,true,true,true,true,true,false,true,false,true,true,false};
bool myWidget::outputReverse[15] = {false,false,true,true,true,true,true,true,true,false,true,false,true,true,false};

myWidget::myWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::myWidget)
{
    ui->setupUi(this);

    //UI参数设置
    this->setWindowTitle(QString::fromLocal8Bit("CRC小工具"));
    this->setWindowIcon(QIcon(":/images/C:/Users/Administrator/Pictures/icon/yang16.png"));
    this->setFixedSize(581,281);

    //算法UI列表初始化,共16个算法
    QStringList CRCList;
    CRCList.clear();
    CRCList <<QStringLiteral("CRC8")<<QStringLiteral("CRC8/ITU")<<QStringLiteral("CRC8/ROHC")
            <<QStringLiteral("CRC8/MAXIM")<<QStringLiteral("CRC16/IBM")<<QStringLiteral("CRC16/MAXIM")<<QStringLiteral("CRC16/USB")
            <<QStringLiteral("CRC16/MODBUS")<<QStringLiteral("CRC16/CCITT")<<QStringLiteral("CRC16/CCITT-FALSE")<<QStringLiteral("CRC16/X25")
            <<QStringLiteral("CRC16/XMODEM")<<QStringLiteral("CRC16/DNP")<<QStringLiteral("CRC32")<<QStringLiteral("CRC32/MPEG-2");
    ui->comboBox->clear();
    ui->comboBox->addItems(CRCList);
    crcSelectInit();
    //算法选择初始化
    connect(ui->comboBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(crcSelectInit()));
    //点击清空事件
    connect(ui->pushButton_3,SIGNAL(clicked()),this,SLOT(allClearEvent()));
    //复制校验码
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(copyCheckCode()));
    //计算初始化判断事件
    //connect(ui->pushButton_2,SIGNAL(clicked()),this,SLOT(crcCalculateInit()));
    //计算CRC事件
    connect(ui->pushButton_2,SIGNAL(clicked()),this,SLOT(crcCalculate()));
}

myWidget::~myWidget()
{
    delete ui;
}
//算法选择初始化函数
void myWidget::crcSelectInit()
{
    int tempCRC = ui->comboBox->currentIndex();
    //qDebug()<<"算法检索位："<<tempCRC;
    ui->lineEdit_2->setText(QString(myWidget::crcPoly[tempCRC]));
    ui->lineEdit_3->setText(QString(myWidget::crcInit[tempCRC]));
    ui->lineEdit_4->setText(QString(myWidget::crcXout[tempCRC]));
    ui->checkBox->setChecked(myWidget::inputReverse[tempCRC]);
    ui->checkBox_2->setChecked(myWidget::outputReverse[tempCRC]);
    ui->lineEdit_2->setEnabled(false);
    ui->lineEdit_3->setEnabled(false);
    ui->lineEdit_4->setEnabled(false);
    ui->checkBox->setEnabled(false);
    ui->checkBox_2->setEnabled(false);
}

void myWidget::allClearEvent()
{
    ui->plainTextEdit->clear();
    ui->lineEdit->clear();
    ui->comboBox->setCurrentIndex(0);
}

void myWidget::copyCheckCode()
{
    //获取系统剪切板指针
    QClipboard *clipBoard = QApplication::clipboard();
    QString checkCode = ui->lineEdit->text();
    clipBoard->setText(checkCode);
}

void myWidget::crcCalculate()
{
    //判断输入数据和配置参数是否为空
    QString dataInput = ui->plainTextEdit->toPlainText();
    QString line2 = ui->lineEdit_2->text();
    QString line3 = ui->lineEdit_3->text();
    QString line4 = ui->lineEdit_4->text();

    //判断输入数据格式
    bool isFormat = dataInput.contains(QRegExp("[^0-9a-fA-F\\s]+"));
    if(isFormat || dataInput.isEmpty())
    {
        QMessageBox::information(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("输入数据格式不正确！"));
        allClearEvent();
        return;
    }
    else
    {
        //重置输入数据格式
        //qDebug()<<"原始输入数据："<<dataInput;
        QStringList tempDataList = dataInput.split(" ");
        QString tempData;//去掉空格后拼接完整的字符串
        QString crcData16;
        for(int i=0;i<tempDataList.count();i++)
        {
            tempData += tempDataList[i];
        }
        crcData16 = tempData;
        //将字符串进行重新安排显示
        int crcDataLength = tempData.length();
        int m8 = 0;
        int n8 = 0;
        while(m8<crcDataLength-1)
        {
            m8 += 2;
            tempData.insert(m8+n8," ");
            n8 += 1;
        }
        tempData = tempData.simplified();
        ui->plainTextEdit->clear();
        ui->plainTextEdit->setPlainText(tempData);//将字符串进行重新安排显示

        //最大校验字节限制在1028个
        //CRC8校验
        QStringList dataList8 = tempData.split(" ");
        unsigned char buf8[1028] = {0};
        QString temp1;
        for(int i=0;i<dataList8.length();i++)
        {
            temp1 = dataList8.at(i);
            buf8[i] = temp1.toInt(0,16);
        }

        int tempCRC = ui->comboBox->currentIndex();

        if(tempCRC<4)
        {
            CRC8(buf8,dataList8.length(),tempCRC);
        }
        if(tempCRC>12)
        {
            CRC32(buf8,dataList8.length(),tempCRC);
        }
        if(tempCRC>3 & tempCRC<13)
        {
            CRC16(buf8,dataList8.length(),tempCRC);
            //qDebug()<<dataList8.length();
        }

    }
}

void myWidget::CRC8(unsigned char *data,int num,int index)
{
    uint_8 crc8 = (ui->lineEdit_3->text()).toUInt(0,16);
    uint_8 temp;

    for(;num>0;num--)
    {
        temp = *data++;
        if(inputReverse[index])
        {
            temp = reverse8(temp);
        }
        crc8 = crc8 ^ temp;
        for (int i=0; i<8;i++)
            {
                if (crc8 & 0x80)
                    crc8 = (crc8 << 1) ^ (ui->lineEdit_2->text()).toUInt(0,16);
                else
                    crc8 = crc8 << 1;
            }
    }
    if(outputReverse[index])
    {
        crc8 = reverse8(crc8);
    }

    crc8 = crc8^(ui->lineEdit_4->text()).toUInt(0,16);

    QString crc = QString::number(crc8,16);
    ui->lineEdit->setText(crc);
}
void myWidget::CRC32(unsigned char *data,int num,int index)
{
    uint_32 crc32 = (ui->lineEdit_3->text()).toUInt(0,16);
    uint_8 temp;
    for(;num>0;num--)
    {
        temp = *data++;
        if(inputReverse[index])
        {
            temp = reverse8(temp);
        }
        crc32 = crc32 ^ (temp<<24);
        for (int i=0; i<8;i++)
            {
                if (crc32 & 0x80000000)
                    crc32 = (crc32 << 1) ^ (ui->lineEdit_2->text()).toUInt(0,16);
                else
                    crc32 = crc32 << 1;
            }
    }
    if(outputReverse[index])
    {
        crc32 = reverse32(crc32);
    }
    crc32 = crc32^(ui->lineEdit_4->text()).toUInt(0,16);
    QString crc = QString::number(crc32,16);
    ui->lineEdit->setText(crc);
}
void myWidget::CRC16(unsigned char *data,int num,int index)
{
    uint_16 crc16 = (ui->lineEdit_3->text()).toUInt(0,16);
    uint_8 temp1;
    for(;num>0;num--)
    {
        temp1 = *data++;
        if(inputReverse[index])
        {
            temp1 = reverse8(temp1);
        }
        crc16 = crc16 ^ (temp1<<8);
        for (int i=0; i<8;i++)
            {
                if (crc16 & 0x8000)
                    crc16 = (crc16 << 1) ^ (ui->lineEdit_2->text()).toUInt(0,16);
                else
                    crc16 = crc16 << 1;
            }
    }
    if(outputReverse[index])
    {
        crc16 = reverse16(crc16);
    }
    crc16 = crc16^(ui->lineEdit_4->text()).toUInt(0,16);
    QString crc = QString::number(crc16,16);
    ui->lineEdit->setText(crc);
}

uint8_t myWidget::reverse8(uint8_t data)
{
    int i;
    uint8_t temp = 0;
    for(i=0;i<8;i++)
    {
        temp |= ((data>>i) & 0x01)<<(7-i);
    }
    return temp;
}
uint16_t myWidget::reverse16(uint16_t data)
{
    int i;
    uint16_t temp = 0;
    for(i=0;i<16;i++)
    {
        temp |= ((data>>i) & 0x01)<<(15-i);
    }
    return temp;
}
uint32_t myWidget::reverse32(uint32_t data)
{
    int i;
    uint32_t temp = 0;
    for(i=0;i<32;i++)
    {
        temp |= ((data>>i) & 0x01)<<(31-i);
    }
    return temp;
}
