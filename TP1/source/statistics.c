#include "statistics.h"

statistics stat;

void setStatistics(){
    stat.numFrameSend = 0;
    stat.numFrameReceive = 0;
    stat.numTimeOut = 0;
    stat.numREJSend = 0;
    stat.numREJReceive = 0;
}

void incFrameSend(){
    stat.numFrameSend ++;
}

void incFrameReceive(){
    stat.numFrameReceive ++;
}

void incTimeOut(){
    stat.numTimeOut ++;
}

void incREJSend(){
    stat.numREJSend ++;
}

void incREJReceive(){
    stat.numREJReceive ++;
}

statistics getStatistics() {
    return stat;
}