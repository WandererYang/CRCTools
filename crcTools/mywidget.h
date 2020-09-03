#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class myWidget; }
QT_END_NAMESPACE

class myWidget : public QWidget
{
    Q_OBJECT

public:
    myWidget(QWidget *parent = nullptr);
    ~myWidget();
    static QString crcName[15];
    static QString crcPoly[15];
    static QString crcInit[15];
    static QString crcXout[15];
    static bool inputReverse[15];
    static bool outputReverse[15];
    void CRC8(unsigned char *data,int num,int index);
    void CRC16(unsigned char *data,int num,int index);
    void CRC32(unsigned char *data,int num,int index);
    uint8_t reverse8(uint8_t data);
    uint16_t reverse16(uint16_t data);
    uint32_t reverse32(uint32_t data);
public slots:
    void crcSelectInit();
    void allClearEvent();
    void copyCheckCode();
    void crcCalculate();
private:
    Ui::myWidget *ui;
};
#endif // MYWIDGET_H
