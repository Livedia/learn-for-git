#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>        //串口
#include <QSerialPortInfo>
#include <QMap>               //字典
#include <QVector>            //容器
#include <thread>             //线程
#include "thread.h"

//定义一个结构存储机械臂某点的名称及其各个电机的位置
struct armPoint
{
  QString name;                 //该点的名称
  QVector<int> armValue;        //各电机位置
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void PathArray(int *);
    void arrmNow(QByteArray);

private slots:
    void on_ConnectArm_clicked();
    void serialPort_readyRead();             //读取串口数据
    void serialPort_write(QByteArray data);  //发送串口数据
    void on_Joint_1_valueChanged(int value);
    void on_pushButton_2_clicked();
    void on_excute_clicked();
    void gopoint(QString name);
    void on_pushin_clicked();
    void on_pushout_clicked();
    void on_GoButton_clicked();
    void GetSign(int xuhao);                //线程函
    void on_save_clicked();
    void on_load_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort serial;                       //要定义串口变量
    QByteArray buffer_r;                      //串口接收数据的变量
    QByteArray buffer;                        //要使用的接收变量
    threadSon *t;

    int cmd[2][7] = {{0,0,0,0,0,0,0},         //角度
                     {0,0,0,0,0,0,0}};        //方向
    int arm[7] = {0, 0, 0, 0, 0, 0, 0};       //当前机械臂各电机的角度
    int slider[7] = {0, 0, 0, 0, 0, 0, 0};    //滑动条的数值

    // int arms[100][7];     // 存储arm信息的数组
    // int armcol =0;
    int namecount = 0;

    QString pointnames[100];

    QMap<QString,int> Points;

    QVector<QString> pathName;   //保存路径名字变量

    bool emitsign=0;//信号发送子线程标志位


    //使用结构体
    QVector<armPoint> path;   //存储轨迹点
    armPoint point;           //单个点的临时变量
};
#endif // MAINWINDOW_H
