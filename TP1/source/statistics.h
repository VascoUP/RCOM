#ifndef STATISTICS_H
#define STATISTICS_H

typedef struct {
    int numFrameSend;
    int numFrameReceive;
    int numFrameRepeat;
    int numTimeOut;
    int numREJ;
} statistics;

void setStatistics();

void incFrameSend();
void incFrameReceive();
void incFrameRepeat();
void incTimeOut();
void incREJ();

statistics getStatistics();

#endif
