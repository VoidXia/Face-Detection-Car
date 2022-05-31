#include <stdio.h>
#include <stdlib.h>
#include <softPwm.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <time.h>
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

void run() // 前进
{
    softPwmWrite(4, 0); //左轮前进
    softPwmWrite(1, 250);
    softPwmWrite(6, 0); //右轮前进
    softPwmWrite(5, 250);
    sleep(1);
}

void brake() //刹车，停车
{
    softPwmWrite(1, 0); //左轮
    softPwmWrite(4, 0);
    softPwmWrite(5, 0); // stop
    softPwmWrite(6, 0);
}

void left() //左转()
{
    softPwmWrite(4, 250); //左轮
    softPwmWrite(1, 0);
    softPwmWrite(6, 0); //右轮前进
    softPwmWrite(5, 250);
}

void right() //右转()
{
    softPwmWrite(4, 0); //左轮前进
    softPwmWrite(1, 250);
    softPwmWrite(6, 250); //右轮
    softPwmWrite(5, 0);
}

void back() //后退
{
    softPwmWrite(1, 250); //左轮back
    softPwmWrite(4, 0);
    softPwmWrite(5, 250); //右轮back
    softPwmWrite(6, 0);
    sleep(1);
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
    pinMode(1, OUTPUT); // IN1
    pinMode(4, OUTPUT); // IN2
    pinMode(5, OUTPUT); // IN3
    pinMode(6, OUTPUT); // IN4
    // pinMode (27, OUTPUT);	//舵机信号输出
    softPwmCreate(1, 1, 500);
    softPwmCreate(4, 1, 500);
    softPwmCreate(5, 1, 500);
    softPwmCreate(6, 1, 500);
    // softPwmCreate(27,1,50);
    // softPwmWrite(27,1);

    while (1)
    {
        //有信号为LOW  没有信号为HIGH
        // SR = digitalRead(RIGHT);//有信号表明在白色区域，车子底板上L亮；没信号表明压在黑线上，车子底板上L灭
        // SL = digitalRead(LEFT);//有信号表明在白色区域，车子底板上L亮；没信号表明压在黑线上，车子底板上L灭
        if (SL == LOW && SR == LOW)
        {
            // printf("SR=%d   ",SR);
            // printf("SL=%d\n");
            printf("GO\n");
            run();
        }
        else if (SL == HIGH && SR == LOW)
        {
            // printf("SR=%d   ",SR);
            // printf("SL=%d\n");
            printf("RIGHT\n");

            left();
        }
        else if (SR == HIGH && SL == LOW)
        {   // 右循迹红外传感器,检测到信号，车子向左偏离轨道，向右转
            // printf("SR=%d   ",SR);
            // printf("SL=%d\n");
            printf("LEFT\n");

            right();
        }
        else
        {   // 都是白色, 停止
            // printf("SR=%d   ",SR);
            // printf("SL=%d\n");
            printf("STOP\n");
            brake();
        }
        sleep(2);
    }

    return 0;
}
