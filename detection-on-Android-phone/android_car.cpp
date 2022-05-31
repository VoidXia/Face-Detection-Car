#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>
#include <thread>
#include <mutex>
#include <map>
#include <softPwm.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <wiringPi.h>

#include <array>
#include <cstdio>
#include <map>
#include <memory>
#include <stdexcept>

#define Trig 28
#define Echo 29
#define LEFT 27
#define RIGHT 26
#define BUFSIZE 512
#define BUFLEN 9600 // Max length of buffer
#define PORT 62345  // The port on which to listen for incoming data
using namespace std;

mutex mtx;

int ans[10];

std::string exec(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

bool allzero_ans()
{
    for (int i = 0; i < 8; i++)
        if (ans[i] != 0)
            return 0;
    return 1;
}

void hexdumpinit(void *ptr, int buflen)
{
    unsigned char *buf = (unsigned char *)ptr;
    int i, j;
    string s;
    map<string, int> mp;
    ofstream of("out.txt");
    for (i = 0; i < buflen; i += 8)
    {
        printf("%d: ", i / 8);
        for (j = 0; j <= 7; j++)
            printf("%02x  ", buf[i + j]);
        printf("\n");
        // printf(" ");
        s = buf[i];
        for (j = 1; j <= 7; j++)
            s += buf[i + j];
        mp[s] = i / 8;
        of << s;
    }
    // ofstream of("out.txt");
    // for (const auto &i : mp) {
    //     of << i.first;
    // }
}
map<string, int> m2;
void mapinit()
{
    char val;
    int i, j;

    ifstream ifs("out.txt", ios::in | ios::binary);
    string key;
    for (i = 0; i < 1200; i++)
    {
        ifs.seekg(8 * i);
        ifs.read((char *)&val, 1);
        key = (char)val;
        for (j = 1; j <= 7; j++)
        {
            ifs.seekg(8 * i + j);
            ifs.read((char *)&val, 1);
            key += (char)val;
        }
        // printf("%02x  ", val);
        m2[key] = i;
    }
}

void hexdumpconvert(void *ptr, int buflen)
{
    unsigned char *buf = (unsigned char *)ptr;
    int i, j;
    string s;
    for (i = 0; i < buflen; i += 8)
    {
        // printf("%02x ", buf[i]);
        // printf(" ");
        s = buf[i];
        for (j = 1; j <= 7; j++)
            s += buf[i + j];
        // printf("%d ", m2[s]);
        ans[i / 8] = m2[s];
    }
    //   printf("\n");
}

void die(char *s)
{
    perror(s);
    exit(1);
}

void read_udp()
{
    struct sockaddr_in si_me, si_other;
    socklen_t slen = sizeof(si_other);

    int s, i, recv_len;
    char buf[BUFLEN];

    // create a UDP socket
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    // zero out the structure
    memset((char *)&si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind socket to port
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
    {
        die("bind");
    }

    printf("Using port %d, ctrl + C abort\n", PORT);

    // keep listening for data
    mapinit();
    while (1)
    {
        // printf("Waiting for data...");
        fflush(stdout);

        // try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, BUFLEN, MSG_WAITALL, (struct sockaddr *)&si_other, &slen)) == -1)
        {
            die("recvfrom()");
        }

        // print details of the client/peer and the data received
        hexdumpconvert(buf, recv_len);
        if (allzero_ans())
            continue;
        // printf("\nReceived packet from %s:%d. ", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        //  printf("Data: %s" , buf);
        // printf("faceEdge: (%3d, %3d), (%3d, %3d), (%3d, %3d), (%3d, %3d)", ans[0], ans[1], ans[2], ans[3], ans[4], ans[5], ans[6], ans[7]);
        // hexdumpinit(buf, recv_len);
        // hexdumpconvert(buf, recv_len, ans);
    }

    close(s);
}

int right_or_left = 0;

void stop()
{
    softPwmWrite(1, 0); //左轮stop
    softPwmWrite(4, 0);
    softPwmWrite(5, 0); // stop
    softPwmWrite(6, 0);
    // delay(time * 100);//执行时间，可以调整
    // printf("Stopped\n");
    // SET GPIO TO 0
}

void turn_right()
{
    mtx.lock();
    softPwmWrite(4, 0); //左轮前进
    softPwmWrite(1, 180);
    softPwmWrite(6, 180); //右轮
    softPwmWrite(5, 0);
    printf("Turning right.\n");
    // SET GPIO TO 1
    mtx.unlock();
}

void turn_right_voice()
{
    mtx.lock();
    softPwmWrite(4, 0); //左轮前进
    softPwmWrite(1, 250);
    softPwmWrite(6, 250); //右轮
    softPwmWrite(5, 0);
    printf("Turning right.\n");
    delay(900);
    stop();
    // SET GPIO TO 1
    mtx.unlock();
}

void turn_left()
{
    mtx.lock();
    softPwmWrite(4, 180); //左轮
    softPwmWrite(1, 0);
    softPwmWrite(6, 0); //右轮前进
    softPwmWrite(5, 180);
    // delay(200);
    // stop();
    // delay(time * 300);
    printf("Turning left.\n");
    // SET GPIO TO 1
    mtx.unlock();
}
void turn_left_voice()
{
    mtx.lock();
    softPwmWrite(4, 250); //左轮
    softPwmWrite(1, 0);
    softPwmWrite(6, 0); //右轮前进
    softPwmWrite(5, 250);
    // delay(200);
    // stop();
    // delay(time * 300);
    printf("Turning left.\n");
    delay(900);
    stop();
    // SET GPIO TO 1
    mtx.unlock();
}

void want_center()
{
    right_or_left = 0;
    mtx.lock();
    // printf("Face is in center and acceptable range.\n");
    stop();
    mtx.unlock();
}

void move_back() //后退
{
    mtx.lock();
    softPwmWrite(4, 250); //左轮back
    softPwmWrite(1, 0);
    softPwmWrite(6, 250); //右轮back
    softPwmWrite(5, 0);
    right_or_left = 0;
    printf("Moving back.\n");
    // delay(time *200);     //执行时间，可以调整
    mtx.unlock();
}

void move_back_voice() //后退
{
    mtx.lock();
    softPwmWrite(4, 250); //左轮back
    softPwmWrite(1, 0);
    softPwmWrite(6, 250); //右轮back
    softPwmWrite(5, 0);
    right_or_left = 0;
    printf("Moving back.\n");
    delay(2000); //执行时间，可以调整
    stop();
    mtx.unlock();
}
void move_forward()
{
    mtx.lock();
    softPwmWrite(4, 0); //左轮前进
    softPwmWrite(1, 200);
    softPwmWrite(6, 0); //右轮前进
    softPwmWrite(5, 200);
    right_or_left = 0;
    printf("Moving forward.\n");
    // SET GPIO TO 1
    mtx.unlock();
}

void move_forward_voice()
{
    mtx.lock();
    softPwmWrite(4, 0); //左轮前进
    softPwmWrite(1, 250);
    softPwmWrite(6, 0); //右轮前进
    softPwmWrite(5, 250);
    right_or_left = 0;
    printf("Moving forward.\n");
    delay(2000);
    stop();
    // SET GPIO TO 1
    mtx.unlock();
}

int prevs = 0;

// void turning(){
//     while(1){
//         if(prevs!=right_or_left)stop();
//         if(right_or_left > 0){
//             turn_right();
//         }
//         else if(right_or_left < 0){
//             turn_left();
//         }
//         else{
//             stop();
//         }
//         prevs = right_or_left;
//     }
// }
void voice()
{
    string s;
    while (1)
    {
        // while(allzero_ans());
        cout << "Listening for voice commands:\n";
        s = ((exec("python3 /home/pi/.mic_over_Mumble/asrt.py")));
        cout << "Listening Stopped.\n";
        cout << s;
        if (stoi(s) == 0)
        {
            cout << "voice forward\n";
            move_forward_voice();
        }

        if (stoi(s) == 4)
        {
            cout << "voice backward\n";
            move_back_voice();
        }
        if (stoi(s) == 1)
        {
            cout << "voice right\n";
            turn_right_voice();
        }
        if (stoi(s) == 2)
        {
            cout << "voice left\n";
            turn_left_voice();
        }
    }
}

void consumer()
{
    while (1)
    {
        // printf("\n\nTAKING: (%3d, %3d), (%3d, %3d), (%3d, %3d), (%3d, %3d)\n\n",ans[0], ans[1], ans[2], ans[3], ans[4], ans[5], ans[6], ans[7]);
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        int centerx = (ans[0] + ans[2] + ans[4] + ans[6]) / 4;
        int centery = (ans[1] + ans[3] + ans[5] + ans[7]) / 4;
        if (allzero_ans())
        { // stop();
          // printf("Android device is offline.\n");
            mtx.lock();
            stop();
            mtx.unlock();
            continue;
        }
        if (centerx > 2 * 640 / 3)
        {
            if (prevs != 1) // stop();
                turn_right();
            prevs = 1;
        }
        else if (centerx < 1 * 640 / 3)
        {
            if (prevs != -1) // stop();
                turn_left();
            prevs = -1;
        }
        else
        {
            prevs = 0;
            if (abs((ans[2] - ans[0]) * (ans[7] - ans[1])) < (640 * 480 / 25))
            {
                move_forward();
            }
            else if (abs((ans[2] - ans[0]) * (ans[7] - ans[1])) > (640 * 480 / 8))
            {
                move_back();
            }
            else
                want_center();
        }
    }
}

int main(int argc, char *argv[])
{
    /*RPI*/
    wiringPiSetup();
    /*WiringPi GPIO*/
    pinMode(1, OUTPUT); // IN1
    pinMode(4, OUTPUT); // IN2
    pinMode(5, OUTPUT); // IN3
    pinMode(6, OUTPUT); // IN4
    // pinMode (27, OUTPUT);	//
    softPwmCreate(1, 1, 500);
    softPwmCreate(4, 1, 500);
    softPwmCreate(5, 1, 500);
    softPwmCreate(6, 1, 500);

    std::thread udpr(read_udp); // pass by value
    std::thread cons(consumer); // pass by value
    std::thread voic(voice);

    udpr.join();
    cons.join();
    voic.join();
    while (1)
        ;
    return 0;
}