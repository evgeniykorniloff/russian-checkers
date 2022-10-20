#include "checkers.h"


time_t startTime,currTime,limitTime = 5;
int isTimeOver;
U64 cntNodes;


void TimerReset(void)
{
  currTime = startTime = time(0);
  isTimeOver = 0;
  cntNodes = 0;
}

int TimeUp(void)
{
 extern int s_depth;
 time_t limit;

  if(!isTimeOver){
    cntNodes++;
    if((cntNodes & 0xFFF) == 0){
      currTime = time(0);
      if(currTime < startTime) startTime = currTime;
      limit = limitTime;
      if(s_depth <= 6  && 60 > limit)
        limit = 60;
      if(currTime - startTime >= limit)
          isTimeOver = 1;
    }
  }
  return isTimeOver;
}
