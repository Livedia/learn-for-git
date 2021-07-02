#ifndef THREAD_H
#define THREAD_H
#include <QThread>
#include <QObject>
#include <QVector>   //容器



class threadSon : public QThread
{
    Q_OBJECT
public:
    threadSon( QObject *parent =0);
protected:
    void run();                  //线程函数
signals:
    void toSignal(int);          //发送信号的函数   机械臂要去的位置编号
public slots:
    void getArm(QByteArray buffer);//接收机械臂状态槽函数

    void getPath(int* Tpath);      //接收路径

private:
    int armnow[7];      //接收到的机械臂位置

    int path[100][7];   //保存路径位置

    int pathcount=100;  //传递路径点的个数

    int pathmsg[1000];  //其它文件中用到的友元变量

    friend class MainWindow;  //声明类中所有元素都为class MainWindow的友元
};

#endif // THREAD_H
