#include <stdio.h>
#include <stdlib.h>
#include <softPwm.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <wiringPi.h>

#define Trig 28
#define Echo 29
#define LEFT 27
#define RIGHT 26
#define BUFSIZE 512

#define MOTOR_GO_FORWARD   \
    digitalWrite(1, HIGH); \
    digitalWrite(4, LOW);  \
    digitalWrite(5, HIGH); \
    digitalWrite(6, LOW)
#define MOTOR_GO_BACK      \
    digitalWrite(4, HIGH); \
    digitalWrite(1, LOW);  \
    digitalWrite(6, HIGH); \
    digitalWrite(5, LOW)
#define MOTOR_GO_RIGHT     \
    digitalWrite(1, HIGH); \
    digitalWrite(4, LOW);  \
    digitalWrite(6, HIGH); \
    digitalWrite(5, LOW)
#define MOTOR_GO_LEFT      \
    digitalWrite(4, HIGH); \
    digitalWrite(1, LOW);  \
    digitalWrite(5, HIGH); \
    digitalWrite(6, LOW)
#define MOTOR_GO_STOP     \
    digitalWrite(1, LOW); \
    digitalWrite(4, LOW); \
    digitalWrite(5, LOW); \
    digitalWrite(6, LOW)

float disMeasure(void)
{
    struct timeval tv1;
    struct timeval tv2;
    long start, stop;
    float dis;

    digitalWrite(Trig, LOW);
    delayMicroseconds(2);

    digitalWrite(Trig, HIGH);
    delayMicroseconds(10); //发出超声波脉冲
    digitalWrite(Trig, LOW);

    while (!(digitalRead(Echo) == 1))
        ;
    gettimeofday(&tv1, NULL); //获取当前时间

    while (!(digitalRead(Echo) == 0))
        ;
    gettimeofday(&tv2, NULL); //获取当前时间

    start = tv1.tv_sec * 1000000 + tv1.tv_usec; //微秒级的时间
    stop = tv2.tv_sec * 1000000 + tv2.tv_usec;

    dis = (float)(stop - start) / 1000000 * 34000 / 2; //求出距离

    return dis;
}

void run() // 前进
{
    softPwmWrite(4, 0); //左轮前进
    softPwmWrite(1, 250);
    softPwmWrite(6, 0); //右轮前进
    softPwmWrite(5, 250);
    delay(500);
}

void brake(int time) //刹车，停车
{
    softPwmWrite(1, 0); //左轮stop
    softPwmWrite(4, 0);
    softPwmWrite(5, 0); // stop
    softPwmWrite(6, 0);
    delay(time * 100); //执行时间，可以调整
}

void left() //左转()
{
    softPwmWrite(4, 250); //左轮
    softPwmWrite(1, 0);
    softPwmWrite(6, 0); //右轮前进
    softPwmWrite(5, 250);
    // delay(time * 300);
    // delay(time * 300);
}

void right() //右转()
{
    softPwmWrite(4, 0); //左轮前进
    softPwmWrite(1, 250);
    softPwmWrite(6, 250); //右轮
    softPwmWrite(5, 0);
    // delay(time * 300);	//执行时间，可以调整
}

void back() //后退
{
    softPwmWrite(4, 250); //左轮back
    softPwmWrite(1, 0);
    softPwmWrite(6, 250); //右轮back
    softPwmWrite(5, 0);
    // delay(time *200);     //执行时间，可以调整
}

int main(int argc, char *argv[])
{

    float dis;

    // char buf[BUFSIZE]={0xff,0x00,0x00,0x00,0xff};

    int SR;
    int SL;
    /*RPI*/
    wiringPiSetup();
    /*WiringPi GPIO*/
    pinMode(Echo, INPUT);
    pinMode(Trig, OUTPUT);
    pinMode(1, OUTPUT); // IN1
    pinMode(4, OUTPUT); // IN2
    pinMode(5, OUTPUT); // IN3
    pinMode(6, OUTPUT); // IN4
    // pinMode (27, OUTPUT);	//
    softPwmCreate(1, 1, 500);
    softPwmCreate(4, 1, 500);
    softPwmCreate(5, 1, 500);
    softPwmCreate(6, 1, 500);
    // softPwmCreate(27,1,50);
    // softPwmWrite(27,1);

    while (1)
    {
        dis = disMeasure();
        printf("distance = %0.2f cm\n", dis); //输出当前超声波测得的距离

        //有信号为LOW  没有信号为HIGH
        SR = digitalRead(RIGHT); //
        SL = digitalRead(LEFT);  //
        printf("SR=%d   ", SR);
        printf("SL=%d\n", SL);
        if ((SL == LOW && SR == LOW) || (dis < 30))
        {
            brake(1);
            printf("BACK\n"); //前面有物体时小车后退？？？ms 再转弯
            back();
            delay(500);
            //后退500ms\
   brake(1);
            left(); //左转400ms
            delay(600);
        }
        else if (SL == HIGH && SR == LOW)
        {
            brake(1);
            printf("RIGHT\n");

            left();
            delay(600);
        }
        else if (SR == HIGH && SL == LOW)
        {
            brake(1);
            printf("LEFT\n");

            right();
            delay(600);
        }
        else
        { // 前面没有物体 前进
            printf("GO\n");
            run();
        }
    }
    return 0;
}
