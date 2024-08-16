/*
 *  Oluwaseun Ladipo
 *  08/11/24
 *  
 *  Original Example from Sam Siewert 
 *  Updated 12/6/18 for OpenCV 3.1
 */
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <syslog.h>
#include <time.h>

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

char difftext[20];
char timetext[20];

int main( int argc, char** argv ) {
    Mat mat_frame, mat_gray, mat_diff, mat_gray_prev;
    VideoCapture vcap;
    unsigned int diffsum, maxdiff, framecnt=0;
    double percent_diff=0.0, percent_diff_old = 0.0;
    double ma_percent_diff = 0.0, fcurtime=0.0, start_fcurtime=0.0;
    struct timespec curtime;
    std::time_t t = std::time(nullptr);
    std::tm* tm_ptr = std::localtime(&t);
    char timeStr[100];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_ptr);

    std::string timestamp = timeStr;
    cv::Point topPosition(50, 50);
    cv::Point bottomPosition(50, 80); 
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    int anonCount = 0;
    double fontScale = 1;
    cv::Scalar white(255, 255, 255); 
    cv::Scalar red(0, 0, 255); 
    cv::Scalar green(0, 255, 0); 
    int thickness = 2;
    int lineType = cv::LINE_AA;

    clock_gettime(CLOCK_REALTIME, &curtime);
    start_fcurtime = (double)curtime.tv_sec + ((double)curtime.tv_nsec/1000000000.0);
  
    if(!vcap.open(0)) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    } else {
	   std::cout << "Opened default camera interface" << std::endl;
    }

    while(!vcap.read(mat_frame)) {
        std::cout << "No frame" << std::endl;
        cv::waitKey(33);
    }
	
    cv::cvtColor(mat_frame, mat_gray, COLOR_BGR2GRAY);
    mat_diff = mat_gray.clone();
    mat_gray_prev = mat_gray.clone();
    maxdiff = (mat_diff.cols)*(mat_diff.rows)*255;

    while(1) {
        if(!vcap.read(mat_frame)) {
            std::cout << "No frame" << std::endl;
            cv::waitKey();
        } else {
            framecnt++;
            clock_gettime(CLOCK_REALTIME, &curtime);
            fcurtime = (double)curtime.tv_sec + ((double)curtime.tv_nsec/1000000000.0) - start_fcurtime;
        }
        
        cv::cvtColor(mat_frame, mat_gray, COLOR_BGR2GRAY);
        absdiff(mat_gray_prev, mat_gray, mat_diff);
        diffsum = (unsigned int)cv::sum(mat_diff)[0];
        percent_diff = ((double)diffsum / (double)maxdiff)*100.0;

        if(framecnt < 3)
            ma_percent_diff=(percent_diff+percent_diff_old)/(double)framecnt;
        else
            ma_percent_diff = ( (ma_percent_diff * (double)framecnt) + percent_diff ) / (double)(framecnt+1);

        syslog(LOG_CRIT, "TICK: percent diff, %lf, old, %lf, ma, %lf, cnt, %u, change, %lf\n", percent_diff, percent_diff_old, ma_percent_diff, framecnt, (percent_diff - percent_diff_old));
        sprintf(difftext, "%8d",  diffsum);
        sprintf(timetext, "%6.3lf",  fcurtime);
        percent_diff_old = percent_diff;

        if(percent_diff > 0.8){
            cv::putText(mat_frame, "Motion Detected", bottomPosition, fontFace, fontScale, red, thickness, lineType);
            anonCount++;
            printf("Anomalies Detected: %i\n", anonCount);
        } else {
            cv::putText(mat_frame, "No Motion Detected", bottomPosition, fontFace, fontScale, green, thickness, lineType);
        }

        cv::putText(mat_frame, timestamp, topPosition, fontFace, fontScale, white, thickness, lineType);
        cv::imshow("Image Stream", mat_frame); 

        char c = cv::waitKey(200); 
        if( c == 'q' ) break;

        mat_gray_prev = mat_gray.clone();
    }
};