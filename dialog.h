#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
namespace Ui {
class Dialog;
}

struct dataToRead
{
    int countVoltage;
    int countCurrent;
    int countEffectivePower;
    int countReactivePower;
    int countApparentPower;
    int countPowerFactor;
    QByteArray voltage;
    QByteArray current;
    QByteArray effectivePower;
    QByteArray reactivePower;
    QByteArray apprentPower;
    QByteArray powerFactor;
};
enum readType
{
    NoneType=0,
    ReadVoltage,
    ReadCurrent,
    ReadEffectivePower,
    ReadReactivePower,
    ReadApparentPower,
    ReadPowerFactor
};

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    void initPort();
    void initData();
    void getVoltage();
    void getCurrent();
    void getEffectivePower();
    void getReactivePower();
    void getApparentPower();
    void getPowerFactor();
    ~Dialog();

private slots:
    void on_launch_clicked();
    void getData();


private:
    Ui::Dialog *ui;
    QSerialPort * portWrite;
    QSerialPort * portRead;
    dataToRead m_data;
    readType m_readType;

};

#endif // DIALOG_H
