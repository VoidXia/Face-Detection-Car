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

void run() // ǰ��
{
    softPwmWrite(4, 0); //����ǰ��
    softPwmWrite(1, 250);
    softPwmWrite(6, 0); //����ǰ��
    softPwmWrite(5, 250);
    sleep(1);
}

void brake() //ɲ����ͣ��
{
    softPwmWrite(1, 0); //����
    softPwmWrite(4, 0);
    softPwmWrite(5, 0); // stop
    softPwmWrite(6, 0);
}

void left() //��ת()
{
    softPwmWrite(4, 250); //����
    softPwmWrite(1, 0);
    softPwmWrite(6, 0); //����ǰ��
    softPwmWrite(5, 250);
}

void right() //��ת()
{
    softPwmWrite(4, 0); //����ǰ��
    softPwmWrite(1, 250);
    softPwmWrite(6, 250); //����
    softPwmWrite(5, 0);
}

void back() //����
{
    softPwmWrite(1, 250); //����back
    softPwmWrite(4, 0);
    softPwmWrite(5, 250); //����back
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
    // pinMode (27, OUTPUT);	//����ź����
    softPwmCreate(1, 1, 500);
    softPwmCreate(4, 1, 500);
    softPwmCreate(5, 1, 500);
    softPwmCreate(6, 1, 500);
    // softPwmCreate(27,1,50);
    // softPwmWrite(27,1);

    while (1)
    {
        //���ź�ΪLOW  û���ź�ΪHIGH
        // SR = digitalRead(RIGHT);//���źű����ڰ�ɫ���򣬳��ӵװ���L����û�źű���ѹ�ں����ϣ����ӵװ���L��
        // SL = digitalRead(LEFT);//���źű����ڰ�ɫ���򣬳��ӵװ���L����û�źű���ѹ�ں����ϣ����ӵװ���L��
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
        {   // ��ѭ�����⴫����,��⵽�źţ���������ƫ����������ת
            // printf("SR=%d   ",SR);
            // printf("SL=%d\n");
            printf("LEFT\n");

            right();
        }
        else
        {   // ���ǰ�ɫ, ֹͣ
            // printf("SR=%d   ",SR);
            // printf("SL=%d\n");
            printf("STOP\n");
            brake();
        }
        sleep(2);
    }

    return 0;
}
