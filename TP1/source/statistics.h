#ifndef STATISTICS_H
#define STATISTICS_H

//Statistics field
typedef struct {
    int numFrameSend;       //Number of frames sent
    int numFrameReceive;    //Number of frames received
    int numFrameRepeat;     //Number of dulicated frames
    int numTimeOut;         //Number of time outs
    int numREJSend;         //Number of REJ sent
    int numREJReceive;      //Number of REJ received
} statistics;

//Sets the statistics field with 0
void setStatistics();

//Increments the numFrameSend
void incFrameSend();
//Increments the numFrameReceive
void incFrameReceive();
//Increments the numFrameRepeat
void incFrameRepeat();
//Increments the numTimeOut
void incTimeOut();
//Increments the numREJSend
void incREJSend();
//Increments the numREJReceive
void incREJReceive();

//Gets the statistics field
statistics getStatistics();

#endif
