#include "statistics.h"

statistics stat;

void setStatistics(){
    stat.numFrameSend = 0;
    stat.numFrameReceive = 0;
    stat.numFrameRepeat = 0;
    stat.numTimeOut = 0;
    stat.numREJ = 0;
}

void incFrameSend(){
    stat.numFrameSend ++;
}

void incFrameReceive(){
    stat.numFrameReceive ++;
}

void incFrameRepeat(){
    stat.numFrameRepeat ++;
}

void incTimeOut(){
    stat.numTimeOut ++;
}

void incREJ(){
    stat.numREJ ++;
}

statistics getStatistics() {
    return stat;
}
