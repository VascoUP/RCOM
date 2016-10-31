#ifndef STATISTICS_H
#define STATISTICS_H

typedef struct {
    int numFrameSend;
    int numFrameReceive;
    int numTimeOut;
    int numREJSend;
    int numREJReceive;
} statistics;

void setStatistics();

void incFrameSend();
void incFrameReceive();
void incTimeOut();
void incREJSend();
void incREJReceive();

statistics getStatistics();

#endif