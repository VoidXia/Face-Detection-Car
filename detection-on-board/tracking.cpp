/*
By downloading, copying, installing or using the software you agree to this license.
If you do not agree to this license, do not download, install,
copy or use the software.
                  License Agreement For libfacedetection
                     (3-clause BSD License)
Copyright (c) 2018-2020, Shiqi Yu, all rights reserved.
shiqi.yu@gmail.com
Copyright (c) 2022, Xiaoheng Xia, all rights reserved.
xia.xh@sjtu.edu.cn
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.
This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are disclaimed.
In no event shall copyright holders or contributors be liable for any direct,
indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
*/



// #define LOCAL_DEBUGGING

// #define SHOW_IMAGE



#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "facedetectcnn.h"

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <thread>
#include <queue>
#include <chrono>
#include <unistd.h>
#include <X11/Xlib.h>
#include <atomic>
#include <gpiod.h>

#ifndef	CONSUMER
#define	CONSUMER	"Consumer"
#endif

#ifdef _OPENMP
#include <omp.h>
#endif



using namespace std;
using namespace std::chrono; // calc fps

//define the buffer size. Do not change the size!
#define DETECT_BUFFER_SIZE 0x20000
using namespace cv;
using namespace cv::ml;


// SET INPUT VIDEO PARA HERE



std::mutex mtx;
vector<Mat> frame_buffer;
atomic <bool> exit_thread_flag{false};
atomic <bool> body_detect_done_thread_flag{false};

int * pResults = NULL; 
//pBuffer is used in the detection functions.
//If you call functions in multiple threads, please create one buffer for each thread!
unsigned char * pBuffers[1024];//large enough
int num_thread = 4;
unsigned char * p0;
// unsigned char * pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);

HOGDescriptor hog;

int right_or_left = 0;


//car value define begin
unsigned int line_num_18 ;	// GPIO 18
unsigned int line_num_23 ;	// GPIO 18
unsigned int line_num_24 ;	// GPIO 24
unsigned int line_num_25 ;	// GPIO 25
struct gpiod_chip *chip;
struct gpiod_line *line18,*line23, *line24, *line25;
//car value define over


void gpio_init(){
    line_num_18 = 18;	// GPIO 18
    line_num_23 = 23;	// GPIO 18
    line_num_24 = 24;	// GPIO 24
    line_num_25 = 25;	// GPIO 25

    chip = gpiod_chip_open_by_name("gpiochip0");

    line18 = gpiod_chip_get_line(chip, line_num_18);

    line23 = gpiod_chip_get_line(chip, line_num_23);

    line24 = gpiod_chip_get_line(chip, line_num_24);

    line25 = gpiod_chip_get_line(chip, line_num_25);

    gpiod_line_request_output(line18, CONSUMER, 0);
    gpiod_line_request_output(line23, CONSUMER, 0);
    gpiod_line_request_output(line24, CONSUMER, 0);
    gpiod_line_request_output(line25, CONSUMER, 0);
}

void show_left_right(Mat &result_image, int right_or_left){
    // if(right_or_left > 0){
    //     cv::putText(result_image, "RIGHT", cv::Point(100, 100), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 1);       
    // }
    // else if(right_or_left < 0){
    //     cv::putText(result_image, "LEFT", cv::Point(100, 100), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 1);       
    // }
    // else{
    //     // cv::putText(result_image, "NONE", cv::Point(100, 100), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 1);       
    // }
    cv::putText(result_image, to_string(right_or_left), cv::Point(100, 100), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 1);
}

int is_forwarding = 0;

void show_forwarding(Mat &result_image, int is_forwarding){
    // if(right_or_left > 0){
    //     cv::putText(result_image, "RIGHT", cv::Point(100, 100), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 1);       
    // }
    // else if(right_or_left < 0){
    //     cv::putText(result_image, "LEFT", cv::Point(100, 100), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 1);       
    // }
    // else{
    //     // cv::putText(result_image, "NONE", cv::Point(100, 100), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 1);       
    // }
    cv::putText(result_image, to_string(is_forwarding), cv::Point(0, 100), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 1);
}


 //car instructions define begin
#ifndef LOCAL_DEBUGGING
            void move_forward(){
                gpiod_line_set_value(line23, 0);
                gpiod_line_set_value(line18, 1);
                gpiod_line_set_value(line24, 1);
                gpiod_line_set_value(line25, 0);
                is_forwarding = 3;
            }
            void move_back(){
                gpiod_line_set_value(line23, 1);
                gpiod_line_set_value(line18, 0);
                gpiod_line_set_value(line24, 0);
                gpiod_line_set_value(line25, 1);
                
            }
            void stop(){
                gpiod_line_set_value(line23, 0);
                gpiod_line_set_value(line18, 0);
                gpiod_line_set_value(line24, 0);
                gpiod_line_set_value(line25, 0);
                is_forwarding = 0;
            }
            void turn_right(){
                gpiod_line_set_value(line18, 1);
                gpiod_line_set_value(line23, 0);	
                gpiod_line_set_value(line25, 1);
                gpiod_line_set_value(line24, 0);
                right_or_left--;
                // launch a thread
            }

            void turn_left(){
                gpiod_line_set_value(line23, 1);
                gpiod_line_set_value(line18, 0);
                gpiod_line_set_value(line24, 1);
                gpiod_line_set_value(line25, 0);
                right_or_left++;
                // launch a thread
                // lets launch a overall thread to do that
            }
#else
            void move_forward(){
                is_forwarding = 3;
            }
            void move_back(){
                
            }
            void stop(){
                is_forwarding = 0;
            }
            void turn_right(){
                right_or_left--;
                // launch a thread
            }

            void turn_left(){
                right_or_left++;
                // launch a thread
                // lets launch a overall thread to do that
            }
#endif
//car instructions define over

void want_right(){
    right_or_left=3;
}

void want_left(){
    right_or_left=-3;
}

void want_center(){
    right_or_left=0;
}

int camera_read_pause = 0;



void resume(){
    if(is_forwarding){
        gpiod_line_set_value(line23, 0);
        gpiod_line_set_value(line18, 1);
        gpiod_line_set_value(line24, 1);
        gpiod_line_set_value(line25, 0);
    }
}

void left_burst(){
    stop();
    camera_read_pause = 1;
    turn_left();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    stop();
    camera_read_pause = 0;
    resume();
}

void right_burst(){
    stop();
    camera_read_pause = 1;
    turn_right();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    stop();
    camera_read_pause = 0;
    resume();
}



void turning(){
    while(1){
        if(is_forwarding > 0){
            is_forwarding --;
        }
        else if(is_forwarding <= 0){
            stop();
        }

        if(right_or_left > 0){ 
            stop();
            printf("RIGHT\n");  
            right_burst();
        }
        else if(right_or_left < 0){
            stop();
            printf("left\n");     
            left_burst();  
        }
        else{

            // cv::putText(result_image, "NONE", cv::Point(100, 100), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 1);       
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}


void frame_write(){
    cout << "this is write" << endl;
    Mat input, blob;
    VideoCapture capture;
    frame_buffer.clear();
    // capture.open("rtsp://pi:11Appleseed@192.168.50.56:554/h264/ch36/main/av_stream");
    // capture.open(0);
    capture.open("rtsp://localhost:8554/mystream");
    if(capture.isOpened())
    {
        cout << "Capture is opened" << endl;
        while(!exit_thread_flag)
        {
            while(camera_read_pause == 1);
            capture >> input;
            if(input.empty() || input.cols == 0)
                continue;
            // cout<<input.rows<<'\n';
            mtx.lock();
            if(frame_buffer.size() < 120){
                frame_buffer.push_back(input);
                if (frame_buffer.size() > 15){ // 隔帧抽取一半删除
                    auto iter = frame_buffer.begin();
                    for(int inde = 0; inde < frame_buffer.size()/2 ; inde++)
                        frame_buffer.erase(iter++);
                }
            }
            else{
                cout << "thread ==============> after read stop, frame_buffer.size() > 100 , write stop";
                return;
            }
            mtx.unlock();
        }
    } else{
        cout << "open camera failed" << endl;
    }
}

void frame_read(){
    cout << "this is read" << endl;
    Mat frame;
    Mat image;

    frame_buffer.clear();

    while(frame_buffer.empty());
    if(!frame_buffer.empty()){
        frame = frame_buffer.back();
        // pResults = facedetect_cnn(pBuffers[0], image.ptr<unsigned char>(0), (int)image.cols, (int)image.rows, (int)image.step);
    }

    int original_width = frame.cols;
    int original_height = frame.rows;
    printf("original_width = %d, original_height = %d", original_width, original_height);

    int im_width0 = original_width / 5;
    int im_height0 = original_height / 5; //face detection
    int new_im_width0 = int(0.7*original_width);
    int new_im_height0 = int(0.7*original_height); // body detection
    int im_width;
    int im_height;
    int new_im_width;
    int new_im_height;

    while(true){

        // if(right_or_left>0){
        //     turn_right();
        // }
        // else if(right_or_left<0){
        //     turn_left();
        // }



        if (!frame_buffer.empty()){
            frame = frame_buffer.back();
            

            TickMeter total;
            total.start();

            //cout << "Image size: " << im.rows << "X" << im.cols << endl;
            im_width = im_width0;//540
            im_height = im_height0;//290
            // while(!frame)frame = frame_buffer.back();
            resize(frame,image,Size(im_width,im_height),INTER_LINEAR);

            ///////////////////////////////////////////
            // CNN face detection 
            // Best detection rate
            //////////////////////////////////////////
            //!!! The input image must be a BGR one (three-channel) instead of RGB
            //!!! DO NOT RELEASE pResults !!!
            TickMeter cvtm;
            cvtm.start();
#ifdef _OPENMP
        int idx = omp_get_thread_num();
#else
        int idx = 0;
#endif
            pResults = facedetect_cnn(pBuffers[idx], (unsigned char*)(image.ptr(0)), image.cols, image.rows, (int)image.step);
            
            cvtm.stop();    
            printf("Face detection time = %gms\n", cvtm.getTimeMilli());
            

            // Mat result_image = image.clone();
            new_im_width = new_im_width0;//540
            new_im_height = new_im_height0;//290
            Mat result_image;
            resize(frame,result_image,Size(new_im_width,new_im_height),INTER_LINEAR);



            int largest_indx_of_confidence = 0;
            for(int i = 0; i < (pResults ? *pResults : 0); i++) 
            {   
                largest_indx_of_confidence = ((((short*)(pResults+1))+142*i)[0]>(((short*)(pResults+1))+142*largest_indx_of_confidence)[0])?i:largest_indx_of_confidence;
            }
                

            bool detected = 0;
            //print the detection results
            for(int i = 0; i < (pResults ? *pResults : 0); i++) 
            {
                short * p = ((short*)(pResults+1))+142*i;
                int confidence = p[0];
                int x = p[1]*new_im_width/im_width;
                int y = p[2]*new_im_height/im_height;
                int w = p[3]*new_im_width/im_width;
                int h = p[4]*new_im_height/im_height;
                int centerx = x + w/2;
                int centery = y + h/2;
                if(confidence < 35){
                    continue;
                }

                if((i==largest_indx_of_confidence)&&(confidence>50||(confidence<50&&confidence>30&&(w>20||h>20)))){
                    detected = 1;
                    rectangle(result_image, Rect(x, y, w, h), Scalar(0, 0, 255), 2);
                    if (centerx > 3*new_im_width/5 ){
                        cv::putText(result_image, "RIGHT", cv::Point(x, y+h-3), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);     
                        want_right(); 
                    }
                    else if (centerx < 2*new_im_width/5 ){
                        cv::putText(result_image, "LEFT", cv::Point(x, y+h-3), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);       
                        want_left();
                    }
                    else{
                        want_center();
                        if(w*h<im_width*im_height/18){
                            move_forward();
                        }
                        // cv::putText(result_image, "CEN", cv::Point(x, y+h+3), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);       
                    }
                }
                else{
                    rectangle(result_image, Rect(x, y, w, h), Scalar(0, 255, 0), 2);
                }

                //show the score of the face. Its range is [0-100]
                char sScore[256];
                snprintf(sScore, 256, "%d", confidence);
                cv::putText(result_image, sScore, cv::Point(x, y-3), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);       
                
                //draw face rectangle
                
                // draw five face landmarks in different colors
                cv::circle(result_image, cv::Point(p[5]*new_im_height/im_height, p[5 + 1]*new_im_height/im_height), 1, cv::Scalar(255, 0, 0), 2);
                cv::circle(result_image, cv::Point(p[5 + 2]*new_im_height/im_height, p[5 + 3]*new_im_height/im_height), 1, cv::Scalar(0, 0, 255), 2);
                cv::circle(result_image, cv::Point(p[5 + 4]*new_im_height/im_height, p[5 + 5]*new_im_height/im_height), 1, cv::Scalar(0, 255, 0), 2);
                cv::circle(result_image, cv::Point(p[5 + 6]*new_im_height/im_height, p[5 + 7]*new_im_height/im_height), 1, cv::Scalar(255, 0, 255), 2);
                cv::circle(result_image, cv::Point(p[5 + 8]*new_im_height/im_height, p[5 + 9]*new_im_height/im_height), 1, cv::Scalar(0, 255, 255), 2);
                
                //print the result
                printf("face %d: confidence=%d, [%d, %d, %d, %d] (%d,%d) (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n", 
                        i, confidence, x, y, w, h, 
                        p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14]);

            }

            im_width = new_im_width;//540
            im_height = new_im_height;//290
            
            resize(frame,image,Size(im_width,im_height),INTER_LINEAR);


            // if(!(pResults ? *pResults : 0)||(((short*)(pResults+1)))[0] < 50){
            if(!detected){
                vector<Rect> found;
                vector<double> weights;
                
                TickMeter cvtm_body;
                cvtm_body.start();

                hog.detectMultiScale(image, found, weights, 0, Size(8,8), Size(/*10,10*/12,12), 1.05, 2, false);
                // hog.detectMultiScale(image, found, weights);

                cvtm_body.stop();    
                printf("Body detection time = %gms\n", cvtm_body.getTimeMilli());
                printf("%d bodies detected.\n", found.size());
                int largest_indx_of_body = 0;
                if(!detected){
                    for( size_t i = 0; i < found.size(); i++ )
                    {
                        largest_indx_of_body = (weights[i]>weights[largest_indx_of_body])?i:largest_indx_of_body;
                    }
                }

                /// draw detections and store location
                for( size_t i = 0; i < found.size(); i++ )
                {
                    Rect r = found[i];
                    if(i == largest_indx_of_body && weights[largest_indx_of_body]>0.45){
                        cv::rectangle(result_image, r, Scalar(0, 0, 255), 2);
                        detected = 1;

                        if (r.x + r.width/2 > 3*im_width/5 ){
                            cv::putText(result_image, "RIGHT", cv::Point(r.x, r.y+r.height-3), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);       
                            want_right();
                        }
                        else if (r.x + r.width/2 < 2*im_width/5 ){
                            cv::putText(result_image, "LEFT", cv::Point(r.x, r.y+r.height-3), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);       
                            want_left();
                        }
                        else{
                            want_center();
                            if(r.width*r.height<new_im_height*new_im_width/15){
                                move_forward();
                            }
                            // cv::putText(result_image, "CEN", cv::Point(r.x, r.y+r.height+3), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);       
                        }   
                    }
                    else{
                        cv::rectangle(result_image, found[i], cv::Scalar(255,0,0), 3);
                    }
                    stringstream temp;
                    temp << weights[i];
                    cv::putText(result_image, temp.str(), cv::Point(found[i].x, found[i].y-3), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1); 
                    // cv::putText(result_image, temp.str(),Point(found[i].x,found[i].y+50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,255));
                    printf("Body confidence = %f\n", weights[i]);
                }


            }


            show_left_right(result_image,right_or_left);
            show_forwarding(result_image,is_forwarding);
            resize(result_image,image,Size(original_width, original_height),INTER_LINEAR);
#ifdef SHOW_IMAGE
            imshow("result", image);
#endif

            total.stop();
            printf("total time = %gms\n", total.getTimeMilli());
            printf("%d right_or_left.\n", right_or_left);
            
            if((cv::waitKey(2)& 0xFF) == 'q')
                break;






            // imshow("Thread Sample", frame);
            if(waitKey(10) == 27) // ’q‘ ASCII == 113
                break;
        }
    }
    exit_thread_flag = True;
    cout << "thread ==============> read stop" <<endl;
}

int main(int argc, char* argv[])
{
  //gpio set part begin
    #ifndef LOCAL_DEBUGGING
        gpio_init();
    #endif

  
 //gpio set part over
 


    if(argc != 2)
    {
        printf("Usage: %s <camera index>\n", argv[0]);
    }

    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());




    VideoCapture cap;
    Mat im;
    
    #ifdef _OPENMP
        num_thread = omp_get_num_procs();
        omp_set_num_threads(num_thread);
        printf("There are %d threads, %d processors.\n", num_thread, omp_get_num_procs());
    #else
        num_thread = 1;
        printf("There is %d thread.\n", num_thread);
    #endif
    p0 = (unsigned char *)malloc(DETECT_BUFFER_SIZE * num_thread);


    if (!p0)
    {
        fprintf(stderr, "Can not alloc buffer.\n");
        return -1;
    }

    for (int i = 0; i < num_thread; i++)
        pBuffers[i] = p0 + (DETECT_BUFFER_SIZE)*i;


    XInitThreads();
    std::thread tw(frame_write); // pass by value
    std::thread tr(frame_read); // pass by value
    std::thread tturn(turning);

    tturn.detach();
 
    tw.detach();
    tr.join();
   
	


    //release the buffer
    free(p0);
    
	return 0;
}

