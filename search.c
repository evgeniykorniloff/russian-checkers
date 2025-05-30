#include "checkers.h"

int white_mtl_search_path[MAX_PLY];

extern time_t startTime,currTime,limitTime;
extern int isTimeOver;
extern U64 cntNodes;

extern int Evaluate(int alpha, int beta);
extern void TimerReset(void);
extern int TimeUp(void);
extern unsigned char history[2*64*64];
extern int sortVal[MAX_PLY*100];
extern void GenPromote(void);
extern void SortCap(int low, int high);
extern void SortNotCap(int low, int high);
extern void Pick(int low,int high);

int s_depth, glScore, glFindOne,__repCnt;


int Quies(int alpha, int beta)
{
   Move *m;
   int tmp, findCap=0, stop, j;

   if(TimeUp()) return 0;
   if(mtl[side]==0) return -INFINITY+ply;
   if(ply >= MAX_PLY-2) return Evaluate(alpha,beta);
   GenCap();
   if(treeCnt[ply+1] > treeCnt[ply]){
      SortCap(treeCnt[ply],treeCnt[ply+1]-1);
      findCap = 1;
   }else{
     // NOT CAPTURES? - TRY PROMOTION
      tmp = Evaluate(alpha,beta);
      if(tmp > alpha) alpha = tmp;
      if(alpha >= beta) return alpha;
      GenPromote();
   }

   j = treeCnt[ply];
   stop = treeCnt[ply+1]-1;
   for(; j <= stop; j++){
       if(findCap && j < stop) Pick(j,stop);
       m = tree + j;

       game_list[game_cnt+ply] = *m;
       MakeMove(m);
       side = 1^side; xside = 1^xside; ply++;

       tmp = -Quies(-beta,-alpha);

       side = 1^side; xside = 1^xside; ply--;
       UnMakeMove(m);
       if(isTimeOver) return 0;
       if(tmp > alpha){
          alpha = tmp;
          if(alpha >= beta) return alpha;
       }
   }//for m
   return alpha;
}


int Repetition(void){
  if(game_cnt + ply > 1){
    Move *mv = game_list + (game_cnt+ply-1);
    HashKey* h = key_list + (game_cnt+ply-2);
    int cnt = 0, nodes = 0, t_ply = ply-1;
    while(h >= key_list){
      if(PIECE(mv->mv) != KING || mv->capMask)
        return 0;
      if(*h == key){
        if(t_ply >= 0) return 1;
        if(++cnt==2) return 1;
      }
      if(++nodes==30) return 1;
      mv--; h--; t_ply--;
    }
  }
  return 0;
}

//-----------------------------------
typedef Move Line[MAX_PLY+1];
Line pv;
Line killer;

int Search(int alpha, int beta, int depth, Line pv_line)
{
  PhaseInfo f;
  Move *m,best,last;
  int extend = 0, tmp,cnt = 0;
  Line tmp_line;
  int GSR = (rand() % 10)+1; //00010
   //0x001
   white_mtl_search_path[ply] = mtl[WHITE] - mtl[BLACK];


   if(TimeUp()) return 0;
   if(mtl[side]==0) return -INFINITY+ply;
   if(ply >= MAX_PLY-2) return Evaluate(alpha,beta);


   PhaseInfoReset(&f);


   f.hashMv = killer[ply];   // KILLER AS FIRST MOVE


   if(game_cnt+ply-1 > 0)
    last = game_list[game_cnt+ply-1];
   else
    last.mv=0;
   //EXTENSIENS   0x001
   if(ply > 2){
   //  if(last.capMask) //CAPTURE
   //    extend++;


   //0x001
     //GOOD CAPTURES AND RECAPTURE EXTENSIENS
     if(ply>5 && last.capMask && depth > 2 && s_depth>8 && ply<(s_depth<<1))
     {
       int M = mtl[WHITE] - mtl[BLACK];
       if(xside==WHITE){
           if(M >= white_mtl_search_path[ply-5])
             extend++;
       }else{
           if(M <= white_mtl_search_path[ply-5])
             extend++;
       }
     }

     if(NEW_PIECE(last.mv)==PAWN &&
        ROW(TO(last.mv))==(side==BLACK?6:1))
        extend++; //PROMOTE

     if(extend > 1) extend = 1;
     depth += extend;
   }
   if(depth <= 0) return Quies(alpha,beta);

   //0001  search
//   if(isPv==0){
   if(1){
     HashItem* ph = HashLook(side);
     if(ph)
      switch(ph->flag){
        case HASH_ALPHA:{
           if(ph->depth >= depth && ph->score <= alpha)
              return alpha;
        }break;
        case HASH_BETA:{
           if(ph->depth >= depth && ph->score >= beta)
             return beta;
              f.killMv = ph->mv.mv;
        }break;
        case HASH_EXACT:{
           if(ph->depth >= depth)
           {
             if(ph->score <= alpha  ||  ph->score >= beta)
               return ph->score;
           }
           f.killMv = ph->mv.mv;
        }break;
      }//switch
   }

   if(PIECE(last.mv)==KING && last.capMask==0 && ply > 0)
    if(Repetition()){
      __repCnt++;
      return 0;
    }




   if(ply > 0){
     GenCap();
     if(treeCnt[ply+1]==treeCnt[ply]) //NOT CAPTURES
       if(depth <= 1){
          tmp = Evaluate(alpha,beta);
         if(tmp > alpha) alpha = tmp;
         if(alpha >= beta) return alpha;
       }
   }




   best.mv = 0;
   while( (m = NextMove(&f)) != NULL ){
      int nextDepth,max_ext;
      cnt++;
      MakeMove(m);
      game_list[game_cnt+ply] = *m;
      key_list[game_cnt+ply] = key;
      tmp_line[ply+1].mv = 0;

      nextDepth = depth-1;

  //    if(m->capMask && cnt==1 && nextDepth <= 0 && ply < 16)//&& ply >= 2 && ply < 16)
  //      nextDepth = 2;

      if(s_depth+6 > 16) max_ext = s_depth+6;
      else max_ext = 16;
      /*
      if(m->capMask && cnt==1 && ply > 4 &&
         ply < max_ext &&
         extend==0)
        nextDepth = depth;
        */



      side=1^side; xside=1^xside; ply++;


      //00010
      if(cnt==1 || depth<=2 || cnt<GSR || m->capMask || ply<=2)
        tmp = -Search(-beta,-alpha,nextDepth,tmp_line);
      else{
        tmp = -Search(-(alpha+1),-alpha,nextDepth-1,tmp_line);
       // if(tmp>alpha){
       //   tmp_line[ply+1].mv = 0;
       //   tmp = -Search(-(alpha+1),-alpha,nextDepth-1,tmp_line);
       // }
        if(tmp>alpha){
          tmp_line[ply+1].mv = 0;
          tmp = -Search(-beta,-alpha,nextDepth,tmp_line);
        }
        
      }


      side=1^side; xside=1^xside; ply--;
      UnMakeMove(m);

      if(isTimeOver) return 0;
      if(tmp > alpha){
         alpha = tmp;
         best = *m;
         killer[ply] = *m;
         if(m->capMask==0){ //00011
            int D = (rand()%depth)+1;
            //int D = (rand()%3)+1 + (tmp>100) + (tmp>300);
            unsigned char *h = &history[(side<<12) | (m->mv&((63<<6)|63))];
            if((int)(*h) + D <= 255)
              (*h) += (unsigned char)D;
            else *h = 255;
         }
         if(ply==0){
           void PrintSearchStatus(int depth, Move best, int score, Line pv);
            sortVal[m-tree] = 10000 + depth*1000 + cnt;
            tmp_line[0] = best;
            glScore = alpha;
            glFindOne = 1;
            PrintSearchStatus(depth,best,alpha,tmp_line);
         }
         if(alpha >= beta) break;

         //SAVE PV
         do{
           Move *d, *s;
           int j;

           d = &pv_line[ply];
           *d = *m;
           d++;
           s = &tmp_line[ply+1];
           j = ply;
           while(s->mv && j < MAX_PLY-4){
             *d = *s;
             d++; s++; j++;
           }
           d->mv = d->capMask = 0; //MARKED END OF LINE
         }while(0);
      }

   }//while
   if(cnt==0) return -INFINITY+ply;



   do{
     int flag;
     if(best.mv==0) flag = HASH_ALPHA;
     else if(alpha >= beta) flag = HASH_BETA;
     else flag = HASH_EXACT;

     HashInsert(best,alpha,depth,side,flag);


   }while(0);



   return alpha;
}

int max(int a, int b) { return a > b ? a : b; }
int min(int a,int b) { return a < b ? a : b; }

//---------------------------------
int main_search(int alpha, int beta, int depth, Line pv_line)
{
  PhaseInfo f;
  Move *m,best,last;
  int extend = 0,  tmp,cnt = 0;
  Line tmp_line;

   //0x001
   white_mtl_search_path[ply] = mtl[WHITE] - mtl[BLACK];



   if(TimeUp()) return 0;
   if(mtl[side]==0) return -INFINITY+ply;
   if(ply >= MAX_PLY-2) return Evaluate(alpha,beta);


   PhaseInfoReset(&f);
   f.hashMv = killer[ply];


   if(game_cnt+ply-1 > 0)
    last = game_list[game_cnt+ply-1];
   else
    last.mv=0;

   //EXTENSIENS 0x001
   if(ply > 2){
   //  if(last.capMask) //CAPTURE
   //    extend++;

     //0x001
     //GOOD CAPTURES AND RECAPTURE EXTENSIENS
     if(ply>5 && last.capMask && depth > 2 && s_depth>8 && ply<(s_depth<<1))
     {
       int M = mtl[WHITE] - mtl[BLACK];
       if(xside==WHITE){
           if(M >= white_mtl_search_path[ply-5])
             extend++;
       }else{
           if(M <= white_mtl_search_path[ply-5])
             extend++;
       }
      // if(extend>0)
      //   printf("#---->ext\n");
     }


     if(NEW_PIECE(last.mv)==PAWN &&
        ROW(TO(last.mv))==(side==BLACK?6:1))
        extend++; //PROMOTE
     if(extend > 1) extend = 1;
     depth += extend;
   }
   if(depth <= 0) return Quies(alpha,beta);



   if(PIECE(last.mv)==KING && last.capMask==0 && ply > 0)
    if(Repetition()){
      __repCnt++;
      return 0;
    }



   //CAPTURES
   if(ply > 0){
     GenCap();
   }
   best.mv = 0;
   while( (m = NextMove(&f)) != NULL ){
      int nextDepth,max_ext;
      cnt++;
      MakeMove(m);
      game_list[game_cnt+ply] = *m;
      key_list[game_cnt+ply] = key;
      tmp_line[ply+1].mv = 0;

      nextDepth = depth-1;

      if(s_depth+6 > 16) max_ext = s_depth+6;
      else max_ext = 16;

      /*
      if(m->capMask && cnt==1 && ply > 4 &&
         ply < max_ext &&
         extend==0)
        nextDepth = depth;
       */

      side=1^side; xside=1^xside; ply++;

    //00010
      if(depth <= 2) //8
      {
          tmp_line[ply+1].mv = 0;
          tmp = -Search(-beta,-alpha,nextDepth,tmp_line);
      }else{
         //int nd;
         //tmp = alpha+1;
         //nd = max(0, nextDepth-2);
        //if(cnt==1){
            tmp_line[ply+1].mv = 0;
            tmp = -Search(-beta,-alpha,nextDepth-1,tmp_line);
       // }else{
       //     tmp_line[ply+1].mv = 0;
       //     tmp = -Search(-(alpha+1),-alpha,nextDepth,tmp_line);
       // }
         //if((tmp > alpha && cnt==1) || (tmp<= alpha && cnt>1))
         if(tmp>alpha) //??!!
         {
             tmp_line[ply+1].mv = 0;
             tmp = -main_search(-beta,-alpha,nextDepth,tmp_line);
         }
      }
      side=1^side; xside=1^xside; ply--;
      UnMakeMove(m);

      if(isTimeOver) return 0;
      if(tmp > alpha){
         alpha = tmp;
         best = *m;
         killer[ply] = *m;
         if(m->capMask==0){
            int D = (rand()%depth)+1;//0x00011
            //int D = (rand()%3)+1 + (tmp>100) + (tmp>300);
            unsigned char *h = &history[(side<<12) | (m->mv&((63<<6)|63))];
            if((int)(*h) + D <= 255)
              (*h) += (unsigned char)D;
            else *h = 255;
         }
         if(ply==0){
           void PrintSearchStatus(int depth, Move best, int score, Line pv);
            sortVal[m-tree] = 10000 + depth*1000 + cnt;
            tmp_line[0] = best;
            glScore = alpha;
            glFindOne = 1;
            PrintSearchStatus(depth,best,alpha,tmp_line);
         }
         if(alpha >= beta) break;

         //SAVE PV
         do{
           Move *d, *s;
           int j;

           d = &pv_line[ply];
           *d = *m;
           d++;
           s = &tmp_line[ply+1];
           j = ply;
           while(s->mv && j < MAX_PLY-4){
             *d = *s;
             d++; s++; j++;
           }
           d->mv = d->capMask = 0; //MARKED END OF LINE
         }while(0);
      }

   }//while
   if(cnt==0) return -INFINITY+ply;



   do{
     int flag;
     if(best.mv==0) flag = HASH_ALPHA;
     else if(alpha >= beta) flag = HASH_BETA;
     else flag = HASH_EXACT;

     HashInsert(best,alpha,
                //depth,
                1,
                side,
                flag);


   }while(0);
   return alpha;
}
//---------------------------------


void PrintSearchStatus(int depth, Move best, int score, Line pv)
{
   int pos_sec,t,j;
   Move mv;
   char buf[32];
   extern char *chb[];

   t = (int)(currTime-startTime);
   if(t > 0)
     pos_sec = (int)(cntNodes/(unsigned)t);
   else
     pos_sec = 0;

   strcpy(buf, chb[ FROM(best.mv) ]);
   strcat(buf, chb[ TO(best.mv) ]);
//   fprintf(stdout,"# d:%d, mv:%s, score:%d, p/s:%d, rep:%d, time:%d\n",
//                  depth,buf,score,pos_sec,__repCnt,t);




   if(pv){
     fprintf(stdout,"%4d %4d %4d %10d ",
          depth,
          score/VALUE_P*100 + score%VALUE_P ,
          t*100,pos_sec);

     //fprintf(stdout,"# pv: ");
     for(j = 0; j < 10 && pv[j].mv; j++){
        mv = pv[j];
        strcpy(buf, chb[ FROM(mv.mv) ]);
        strcat(buf, chb[ TO(mv.mv) ]);
        fprintf(stdout,"%s ",buf);
     }
     fprintf(stdout,"\n");
   }
//   fflush(stdout);
}
//-----------------------------





Move* SearchMove(void){

  Line line;
  int tmp,j;


   //0x001
   white_mtl_search_path[0] = mtl[WHITE] - mtl[BLACK];


   glScore = glFindOne = __repCnt = 0;
   TimerReset();
   memset(history,0,sizeof(history));
   HashReset();

   srand(time(0));
   GenCap();
   if(treeCnt[1] > treeCnt[0]){
      //SortCap(0,treeCnt[1]-1);
   }else{
     GenNotCap();
     if(treeCnt[1] > treeCnt[0])
     {
        //SortNotCap(0,treeCnt[1]-1);
     }else
        return NULL;
   }

   for(j = treeCnt[0]; j < treeCnt[1]; j++)
     sortVal[j] = rand()%20;



   memset(line,0,sizeof(line));


   if(treeCnt[1]==treeCnt[0]+1)
     return &tree[0];

do{
   int a,b;

   tmp = 0;
   for(s_depth = 1; s_depth < MAX_PLY-4; s_depth++)
   {
     memset(pv,0,sizeof(pv));
     for(j = 0; j < MAX_PLY-2; j++){
       pv[j] = line[j];
       if(line[j].mv==0) break;
     }
     glFindOne = 0;


     //a = tmp - VALUE_P; if(a < -INFINITY) a = -INFINITY;
     //b = tmp + 3*VALUE_P; if(b > INFINITY) b = INFINITY;

     //0001
     a = -INFINITY+100;
     b = INFINITY+100;


     tmp =  main_search(a,b,s_depth,line);
     if(isTimeOver) break;
     if(tmp <= a)
     {
       tmp =  main_search(-INFINITY,b,s_depth,line);

     }else if(tmp >= b){

       tmp =  main_search(a,INFINITY,s_depth,line);

     }
     /*
     {
       int j;
       for(j=0;j<treeCnt[1])
        printf("$%d,
     }
     */
     if(isTimeOver) break;
   }

}while(0);

   Pick(0,treeCnt[1]-1);
   if(!glFindOne) s_depth--;
   PrintSearchStatus(s_depth,tree[0],glScore,NULL);
   return &tree[0];
}











