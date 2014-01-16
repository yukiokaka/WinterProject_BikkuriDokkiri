//
//  input_test.cpp
//  Bikkuri_Dokkiri_Meka
//
//  入力に応じて正しくシリアル転送ができているかをチェックする．
//  必ず25行目を自分の環境に合わせること．
//
//  Created by koya on 2014/01/13.
//  Copyright (c) 2014 koya. All rights reserved.
//
//

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <boost/asio.hpp>
#include <cstdio>
#include <iostream>
#include <cstdlib>
using namespace std;
using namespace boost::asio;

#define _BV(x) 1 << (x)

const int    NUM_LED = 16;
const char*  FILENAME = "init.png";
// ここは自分の環境に合わせて変えること
const string PATH = "/dev/tty.usbserial-A4VNMJ7B";

io_service io;
serial_port port(io, PATH.c_str());
int config_counter = 0;

cv::Mat img0, img;
unsigned char led_array[1];
short led[NUM_LED];

// LEDをすべてオフで初期化
void initializeLed(void)
{
    for(int i = 0; i < NUM_LED; i++){
        led[i] = 0x0000;
    }
    return;
}

// マウス操作のコールバック関数
void on_mouse(int event, int x, int y, int flags, void *)
{
    int xpos = -1;
    int ypos = -1;
    
    if(img.empty()){
        return;
    }
    
    if(event == CV_EVENT_LBUTTONUP){
        xpos = x / 30;
        ypos = y / 30;
        printf("L button pushed:(y, x) = (%d, %d)\n", ypos, xpos);
        led[ypos] |= _BV(xpos);
        
        cv::Point p = cv::Point(xpos*30+15, ypos*30+15);
        cv::circle(img, p, 10, 0, -1);
        cv::imshow("dev0", img);
    }
    else if(event == CV_EVENT_RBUTTONUP){
        xpos = x / 30;
        ypos = y / 30;
        printf("R button pushed:(y, x) = (%d, %d)\n", ypos, xpos);
        led[ypos] &= ~_BV(xpos);
        
        cv::Point p = cv::Point(xpos*30+15, ypos*30+15);
        cv::circle(img, p, 10, cv::Scalar::all(255), -1);
        cv::circle(img, p, 10, 0, 2);
        cv::imshow("dev0", img);
    }
}

// LEDの点灯箇所を表示
void printResult(void)
{
    for(int y = 0; y < NUM_LED; y++){
        for(int x = 0; x < NUM_LED; x++){
            printf("%d", (led[y] >> x) & 0x0001);
        }
        printf("\n");
    }
    return;
}

// S信号，R信号によるハンドシェーク
bool succeedS_R(serial_port& port)
{
    //Sは'S'
    char S[2] = "S";
    if(0 < port.write_some(buffer(S))){
        printf("S sent successfully\n");
        char R[2] = "0";
        //Rを受信して，かつ'R'だったら
        if(0 < port.read_some(buffer(R)) && 'R' == R[0]){
            return true;
        }
    }
    return false;
}

// デバイスに送信する1bitのLED情報を作る
void makeLedArray(int row, int p)
{
    led_array[0] = 0x00;
    led_array[0] |= p ? (led[row] >> (NUM_LED / 2)) & 0xFF : led[row] & 0xFF;
}

// 1bitのLED情報を表示
void printLedArray()
{
    printf("data send :");
    for(int i = 0; i < NUM_LED / 2; i++){
        printf("%d", (led_array[0] >> i) & 0x01);
    }
    printf("\n");
}

// 1行毎にLEDの情報を送信
bool sendImage(serial_port& port)
{
    char P[2] = "P";
    for(int i = 0; i < NUM_LED; i++){
        port.write_some(buffer(P)); //P信号を送信
        
        char dev[3] = "\0\0"; //デバイスの番号, 行数を受け取る
        if(0 < port.read_some(buffer(dev))){
            int dev_num = atoi(dev);
            printf("dev number[%d] : %s\n", dev_num, dev);
            
            //P信号を送信
            port.write_some(buffer(P));
            
            dev[0] = dev[1] = '\0';
            if(0 < port.read_some(buffer(dev))){
                int dev_row = atoi(dev);
                printf("dev row[%d] : %s\n", dev_row, dev);
                
                port.write_some(buffer(P)); //P信号を送信
                makeLedArray(dev_row, 0);
                port.write_some(buffer(led_array));
                printLedArray();
                
                char C[2] = "\0";
                if(0 < port.read_some(buffer(C)) && 'C' == C[0]){
                    //printf("\tC received : %s\n", C);
                    port.write_some(buffer(P)); //P信号を送信
                    makeLedArray(dev_row, 1);
                    port.write_some(buffer(led_array));
                    printLedArray();
                }
            }
        }
    }
    
    //終了信号(F)を待って終了
    char dev[2] = "\0";
    while(1){
        if(0 < port.read_some(buffer(dev)) && 'F' == dev[0]){
            return true;
        }
    }
    
}

// シリアル通信を行う
void writeBySerial(void)
{
    // 1回目の通信なら初期設定
    if(0 == config_counter){
        port.set_option(serial_port_base::baud_rate(38400));
        port.set_option(serial_port_base::character_size(8));
        port.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));
        port.set_option(serial_port_base::parity(serial_port_base::parity::none));
        port.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
        config_counter++;
    }
        
    if(succeedS_R(port)){
        printf("S_R Handshake successfully done\n");
        if(sendImage(port)){
            printf("data correctly written\n");
        }
    }
    
    return;
}

int main(int argc, char** argv)
{
    img0 = cv::imread(FILENAME);
    if(img0.empty()){
        fprintf(stderr, "cannot open %s\n", FILENAME);
        exit(1);
    }
    
    // print hot keys
    printf( "hot keys: \n"
           "\tESC - quit the program\n"
           "\ti or ENTER - send the LED image by serial transport\n"
           "\tr - paint from the beginning\n"
           "\t\t(before running it, paint something on the image)\n");
    
    cv::namedWindow("dev0", 1);
    img = img0.clone();
    cv::imshow("dev0", img);
    
    // set callback function for mouse operations
    initializeLed();
    cv::setMouseCallback("dev0", on_mouse, 0);
    
    bool loop_flag = true;
    while(loop_flag){
        int c = cv::waitKey(0);
        switch(c){
            case 27: //ESC
            case 'q':
                loop_flag = false;
                break;
            case 'r':
                img0.copyTo(img);
                cv::imshow("dev0", img);
                initializeLed();
                break;
            case 'i':
            case 10: //ENTER
                printResult();
                writeBySerial();
                break;
        }
    }
    
    return 0;
}
