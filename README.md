# 基于Qt5.15的CRC校验小工具开发项目


@[TOC]


</font>

<hr style=" border:solid; width:100px; height:1px;" color=#000000 size=1">

# 前言

<font >
        近期用到CRC校验算法，网上搜索很多示例都有问题，所以研究了一下基于Qt写了个CRC校验小工具。</font>

<hr style=" border:solid; width:100px; height:1px;" color=#000000 size=1">



# 一、开发准备
- 开发环境：Windows10（开始基于Mac开发，感觉Mac系统下Qt的适配性不如Windows好，最终选择万能的Windows）
- Qt基础：掌握信号与槽，UI界面，Qt资源文件等
- C++基础：构造与析构函数，类的继承，数组与指针，正则表达式，初始化列表等
- CRC算法（基于多项式的CRC8、CRC16和CRC32）



# 二、开发流程
## 1.Qt配置与界面设置

 - 新建项目->选择Qt Widgets Application->选择QWidget基类->勾选Generate form（注意：勾选后才能生成.ui后缀文件，才能手动拖控件）

![Qt配置](https://img-blog.csdnimg.cn/20200903204849725.PNG?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl8zOTU0OTE2MQ==,size_16,color_FFFFFF,t_70#pic_center)
 - 项目树如下图所示，只需要修改画圈的文件，项目树介绍：
 
   1.==crcTools.pro==：项目配置文件，这个一般会自动配置，不需要管，引入Qt所用到的核心库以及项目文件。
   2.==main.cpp==：主进程循环文件，一般不需要变动。
   3.==mywidget.h==和==mywidget.cpp==：需要你去编程的两个主要继承类文件，用于写你的方法和算法。
   4.==mywidget.ui==：用于推拽控件的UI界面。
   5.==images.qrc==：Qt的资源文件，用于存储图片等资源信息。
   
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200903210853668.PNG?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl8zOTU0OTE2MQ==,size_16,color_FFFFFF,t_70#pic_center)

- <font color=#999AAA >双击生成的myWidget.ui文件，便可进入控件拖拽UI界面，最终如下图所示UI分为数据输入区，算法设置区和结果校验区。
 ![CRC小工具界面](https://img-blog.csdnimg.cn/20200903210010554.PNG?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl8zOTU0OTE2MQ==,size_16,color_FFFFFF,t_70#pic_center)
## 2.控件基本配置
- 自定义myWidget类继承自基类QWidget，其中QWidget类又继承于QObject类。
- this指针指向ui界面，**通过this指针获取ui界面的控件**。
- 界面标题设置通过setWindowTitle函数，其中要注意加入**QString::fromLocal8Bit**用于本地字符GB到Unicode转换，**解决中文乱码问题**；其他控件初始化设置如下面程序可得。
- **信号与槽**通过connect函数连接，比如计算按钮的点击事件函数会响应CRC校验算法函数进行计算，可以将一个connect函数看作一个事件响应过程，包括四个事件（**算法选择事件，复制校验码事件，CRC算法计算事件和界面清空事件**）。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200903223255442.PNG?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl8zOTU0OTE2MQ==,size_16,color_FFFFFF,t_70#pic_center)

```cpp
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
    //计算CRC事件
    connect(ui->pushButton_2,SIGNAL(clicked()),this,SLOT(crcCalculate()));
}
```
## 3.CRC算法介绍
- CRC校验（循环冗余校验）是数据通讯中最常采用的校验方式，就是将需要校验的数据与多项式进行循环异或（XOR），将得到的校验码附在数据末尾发送出去。
- 常见CRC多项式参数模型参考CRC在线校验工具[CRC在线校验工具](http://www.ip33.com/crc.html)。
- CRC算法参数模型解释：
   1.POLY：多项式的简写，以16进制表示。例如：CRC-32即是0x04C11DB7，忽略了最高位的"1"，即完整的生成项是0x104C11DB7。
   2.INIT：这是算法开始时寄存器（crc）的初始化预置值，十六进制表示。
   3.REFIN：输入数据每个字节是否反转。
   4.REFOUT：计算输出之后，异或输出之前，得到的数据进行反转。
   5.XOROUT：计算结果与该参数异或得到最终的CRC校验码。

![在这里插入图片描述](https://img-blog.csdnimg.cn/20200903232256658.PNG?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl8zOTU0OTE2MQ==,size_16,color_FFFFFF,t_70#pic_center)

## 4.信号与槽的四个响应事件
- 算法初始化响应事件，界面开始运行时的参数指向设置。

```cpp
//算法选择初始化函数
//算法参数peizhi
QString myWidget::crcName[15] = {"CRC8","CRC8/ITU","CRC8/ROHC","CRC8/MAXIM","CRC16/IBM","CRC16/MAXIM","CRC16/USB","CRC16/MODBUS","CRC16/CCITT",
                             "CRC16/CCITT-FALSE","CRC16/X25","CRC16/XMODEM","CRC16/DNP","CRC32","CRC32/MPEG-2"};
QString myWidget::crcPoly[15] = {"07","07","07","31","8005","8005","8005","8005","1021","1021","1021","1021","3D65","04C11DB7","04C11DB7"};
QString myWidget::crcInit[15] = {"00","00","FF","00","0000","0000","FFFF","FFFF","0000","FFFF","FFFF","0000","0000","FFFFFFFF","FFFFFFFF"};
QString myWidget::crcXout[15] = {"00","55","00","00","0000","FFFF","FFFF","0000","0000","0000","FFFF","0000","FFFF","FFFFFFFF","00000000"};
bool myWidget::inputReverse[15] = {false,false,true,true,true,true,true,true,true,false,true,false,true,true,false};
bool myWidget::outputReverse[15] = {false,false,true,true,true,true,true,true,true,false,true,false,true,true,false};

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
```
- 点击清空响应事件。

```cpp
void myWidget::allClearEvent()
{
    ui->plainTextEdit->clear();
    ui->lineEdit->clear();
    ui->comboBox->setCurrentIndex(0);
}
```
- 复制事件，将得出的校验码复制到剪切板。

```cpp
void myWidget::copyCheckCode()
{
    //获取系统剪切板指针
    QClipboard *clipBoard = QApplication::clipboard();
    QString checkCode = ui->lineEdit->text();
    clipBoard->setText(checkCode);
}
```
- CRC校验事件。

```cpp
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
        }
    }
}
```
## 5.打包发布
- 将编译生成的exe文件拷贝到一个空文件夹，这时候打不开提示缺少qt核心库，这是因为qt动态链接库没被编译连接到，打开Qt的管理员终端，如下图所示。
- cd到你刚才拷贝exe的文件夹位置，输入：windeployqt crcTools.exe，会自动将需要链接的文件拷贝到文件夹，之后便可以运行exe文件。


![在这里插入图片描述](https://img-blog.csdnimg.cn/20200903233436488.PNG?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl8zOTU0OTE2MQ==,size_16,color_FFFFFF,t_70#pic_center)

<hr style=" border:solid; width:100px; height:1px;" color=#000000 size=1">

# 总结
该工具可以计算CRC8，CRC16以及CRC32位的数据校验码，欢迎大家评论、下载与转发，最后附上我的Github地址，欢迎大家骚扰哈！
