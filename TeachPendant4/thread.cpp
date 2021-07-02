#include "thread.h"
#include <QDebug>
threadSon::threadSon(QObject *parent) : QThread(parent)
{

}
void threadSon::run()          //子线程主函数
{

    qDebug()<<"Open thread";

    getPath(pathmsg);//     通过友元获取数组值并解析

    int i=0;
    emit toSignal(0);//   发射信号

    while(1)
    {

        if(     armnow[0] == path[i][0]&&armnow[1] == path[i][1]&&
                armnow[2] == path[i][2]&&armnow[3] == path[i][3]&&
                armnow[4] == path[i][4]&&armnow[5] == path[i][5]&&
                armnow[6] == path[i][6]        )
        {
            i++;
            if(i==pathcount)
            {
                emit toSignal(999);
                qDebug()<<"这在run主函数999结束信号";
            }
           else
                emit toSignal(i);//   发射信号
           qDebug()<<"这在run主函数";
        }

        sleep(1);                //等待1秒钟
    }
}

void threadSon::getArm(QByteArray buffer)
 {
    int t;
    qDebug()<<"getArm(QByteArray buffer)";
    int n = buffer.count();
    for(int i =0;i<7;i++)
    {
        t = buffer.indexOf(" ");           //第一个空格以左赋值给t
        armnow[i] = buffer.left(t).toInt();   //将t转换为数字存储到arm内
        qDebug()<<armnow[i];
        n=n-t-1;
        buffer = buffer.right(n);          //将第一个 空格后面内容再赋给buffer
    }
//    int i=0;
//    if(armnow[0] == path[i][0]&&armnow[1] == path[i][1]&&
//       armnow[2] == path[i][2]&&armnow[3] == path[i][3]&&
//       armnow[4] == path[i][4]&&armnow[5] == path[i][5]&&
//       armnow[6] == path[i][6])
//    {
//        i++;
//        emit toSignal(i);//   发射信号
//        qDebug()<<"emit toSignal(i);//   发射信号";
//    }

//    if(i==pathcount)
//    {
//        qDebug()<<"结束线程标志";
//        emit toSignal(999);
//    }


 }
void threadSon::getPath(int* Tpath)  //接收路径
{
    int i=0;
    int j=0;
    int w;
    for(int t=0;Tpath[t]!=999;t++)
    {
        path[i][j] = Tpath[t];
        qDebug()<<path[i][j];
        j++;
        w=t+1;
        if(w%7==0)
        {
           j=0;
           i++;
        }
    }
    pathcount=i;
    qDebug()<<"接收的路径点个数为："<<pathcount;

}
