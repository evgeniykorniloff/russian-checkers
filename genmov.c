#include "checkers.h"

BitBoard all[2],pieces[2];
int color[64],pos[64];
int side,xside,ply;
int mtl[2],stScore[2];
const int value[3] = {VALUE_P,VALUE_K,0};
Move tree[MAX_PLY*100];
int treeCnt[MAX_PLY+1];
int sortVal[MAX_PLY*100];
int fromTo[64][64];
Move game_list[MAX_GAME+MAX_PLY+1];
HashKey key_list[MAX_GAME+MAX_PLY+1];
int game_max,game_cnt;
unsigned char history[2*64*64],king_history[2*64*64];

#define R_SHIFT (16+1)
#define L_SHIFT (16-1)
int dir[4] = {R_SHIFT,L_SHIFT,-L_SHIFT,-R_SHIFT};
int startSq,startPiece;
unsigned int capMask;
Move *list;


int to32[64],from32[32];

int blackSq[64] =
{
  0,1,0,1,0,1,0,1,
  1,0,1,0,1,0,1,0,
  0,1,0,1,0,1,0,1,
  1,0,1,0,1,0,1,0,
  0,1,0,1,0,1,0,1,
  1,0,1,0,1,0,1,0,
  0,1,0,1,0,1,0,1,
  1,0,1,0,1,0,1,0
};



void InitNewGame(void)
{
  static int first = 1;
  int j;

  if(first){
     first = 0;
     InitBitBoard();
     InitGen();
     HashReset();
  }
  key = 0;

  side = WHITE; xside = BLACK;
  game_cnt = game_max = 0;
  all[0]=all[1]=pieces[0]=pieces[1]=0;
  mtl[0]=mtl[1]=stScore[0]=stScore[1]=0;
  for(j=0;j<64;j++){
    pos[j] = NOPIECE;
    color[j] = NEUTRAL;
  }


  for(j=0;j<64;j++)
   if(blackSq[j])
   {
    if(ROW(j)<=2) InsertPiece(PAWN,j,BLACK);
    else if(ROW(j)>=5)InsertPiece(PAWN,j,WHITE);
   }
   memset(king_history,0,sizeof(king_history));
}





void InitGen(void)
{
 int cnt,j,from,to,x1,y1,x2,y2;;
   cnt = 0;
   for(j=0; j<64; j++)
    if(blackSq[j]){
       from32[cnt] = j;
       to32[j] = cnt;
       cnt++;
   }

  for(from=0;from<64;from++)
   for(to=0;to<64;to++)
    if(from != to){
     x1 = COLUMN(from);
     y1 = ROW(from);
     x2 = COLUMN(to);
     y2 = ROW(to);

     /*
     if(x1==x2){
        fromTo[from][to] = x2>x1? 1:-1;
     }else if(y1==y2){
        fromTo[from][to] = y2>y1? 16:-16;
     }else
      */
     if(x1+y1==x2+y2){ //a1h8

        fromTo[from][to] = y2<y1? -L_SHIFT:L_SHIFT;
     }else if(x1-y1==x2-y2){
        fromTo[from][to] = y2<y1? -R_SHIFT:R_SHIFT;
     }
   }
}


void LinkCap(int sq, int newPiece)
{
  list->mv = (newPiece<<15) | (startPiece<<12) | (startSq<<6) | sq;
  list->capMask = capMask;
  list++;
}


int CapKing(int sq, int d)
{
   int cnt = 0, isCap = 0, u,sq1,u1,j;
   for(sq += d; (sq&0x88)==0; sq+=d){
      u = UNMAP(sq);
      if(color[u]==NEUTRAL) continue;
      else if(color[u]==side) break;
      else{
        sq1 = sq+d;
        if((sq1&0x88)==0 && color[UNMAP(sq1)]==NEUTRAL){
          INCLUDE(capMask,u);
          color[u] ^= 1;
          cnt++;
        }else break;
      }
   }

   sq -= d;
   while(cnt > 0){
     u = UNMAP(sq);
     if(color[u]!=NEUTRAL){
        if(isCap==0){
           sq1 = sq+d;
           while((sq1&0x88)==0 && color[u1=UNMAP(sq1)]==NEUTRAL){
              LinkCap(u1, KING);
              sq1 += d;
           }
        }
        color[u] ^= 1;
        EXCLUDE(capMask,u);
        cnt--;
        isCap = 1;
     }else{
       for(j=0; j<4; j++)
        if(dir[j] != d && dir[j] != -d && CapKing(sq,dir[j]))
          isCap = 1;
     }
     sq -= d;
   }//while
   return isCap;
}

int CapPawn(int sq, int d)
{
  int isCap = 0,u,sq1,u1,j,newPiece=PAWN;
   sq += d;
   if((sq&0x88)==0){
     u = UNMAP(sq);
     if(color[u]==xside){
       sq1 = sq+d;
       if((sq1&0x88)==0){
         u1 = UNMAP(sq1);
         if(color[u1]==NEUTRAL){
            if(side==BLACK){
              if(ROW(u1)==7) newPiece = KING;
            }else{
              if(ROW(u1)==0) newPiece = KING;
            }
            color[u] ^= 1;
            INCLUDE(capMask,u);
            for(j=0; j<4; j++)
             if(dir[j] != -d){
               if(newPiece==KING){
                 if(CapKing(sq1,dir[j])) isCap = 1;
               }else if(CapPawn(sq1,dir[j])) isCap = 1;

             }
            if(isCap==0) LinkCap(u1,newPiece);
            color[u] ^= 1;
            EXCLUDE(capMask,u);
            isCap = 1;
         }
       }
     }
   }
  return isCap;
}


void GenCap(void)
{
  BitBoard op,empty,p,w,left,right;
  int to;

   list = tree+treeCnt[ply];
   op = all[xside];
   empty = ~(all[0]|all[1]);

   startPiece = PAWN;
   p = pieces[PAWN] & all[side];
   if(p){
       right = (p<<9)&L_MASK;
       left  = (p<<7)&R_MASK;
       for(w = (((right&op)<<9)&L_MASK)&empty; w; w&=w-1)
       {
         to = GetOne(w);
         startSq = to-9-9;
         color[startSq] = NEUTRAL;
         CapPawn(MAP(startSq),R_SHIFT);
         color[startSq] = side;
       }
       for(w = (((left&op)<<7)&R_MASK)&empty; w; w&=w-1)
       {
         to = GetOne(w);
         startSq = to-7-7;
         color[startSq] = NEUTRAL;
         CapPawn(MAP(startSq),L_SHIFT);
         color[startSq] = side;
       }
       right = (p>>7)&L_MASK;
       left  = (p>>9)&R_MASK;
       for(w = (((right&op)>>7)&L_MASK)&empty; w; w&=w-1)
       {
         to = GetOne(w);
         startSq = to+7+7;
         color[startSq] = NEUTRAL;
         CapPawn(MAP(startSq),-L_SHIFT);
         color[startSq] = side;
       }
       for(w = (((left&op)>>9)&R_MASK)&empty; w; w&=w-1)
       {
         to = GetOne(w);
         startSq = to+9+9;
         color[startSq] = NEUTRAL;
         CapPawn(MAP(startSq),-R_SHIFT);
         color[startSq] = side;
       }
    }


    startPiece = KING;
    for(p = pieces[KING]&all[side]; p; p&=p-1)
    {
      startSq = GetOne(p);
      color[startSq] = NEUTRAL;
      for(w = KingAttack(startSq)&op; w; w&=w-1)
      {
        int u0 = GetOne(w);
        int d   = fromTo[startSq][u0];
        int sq1 = MAP(u0)+d;
        if((sq1&0x88)==0 && color[UNMAP(sq1)]==NEUTRAL)
          CapKing(sq1-d-d,d);
      }
      color[startSq] = side;
    }

   treeCnt[ply+1] = list-tree;
}


void LinkPromote(BitBoard w, int shift)
{
 int from,to;
  for(; w; w&=w-1)
  {
    to = GetOne(w);
    from = to-shift;
    list->mv = (KING<<15)|(PAWN<<12)|(from<<6)|to;
    list->capMask = 0;
    list++;
  }
}



#define LINK(from,to,piece)\
   list->mv = (piece<<15)|(piece<<12)|(from<<6)|to;\
   list->capMask = 0;\
   list++


void GenNotCap(void)
{
  BitBoard empty,p,w,left,right;
  int from,to;

   list = tree+treeCnt[ply];
   empty = ~(all[0]|all[1]);

   p = pieces[PAWN] & all[side];
   if(p)
   {
    if(side==BLACK){
       right = (p<<9)&L_MASK&empty;
       left  = (p<<7)&R_MASK&empty;
       if(p&0x00FF000000000000ULL){
         w = right & 0xFF00000000000000ULL;
         if(w)LinkPromote(w,9);
         w = left & 0xFF00000000000000ULL;
         if(w)LinkPromote(w,7);
       }
       for(w=right&~0xFF00000000000000ULL; w; w&=w-1)
       {
         to = GetOne(w);
         from = to - 9;
         LINK(from,to,PAWN);
       }
       for(w=left&~0xFF00000000000000ULL; w; w&=w-1)
       {
         to = GetOne(w);
         from = to - 7;
         LINK(from,to,PAWN);
       }
    }else{
       right = (p>>7)&L_MASK&empty;
       left  = (p>>9)&R_MASK&empty;
       //PROMOTE
       if(p&0x000000000000FF00ULL){
         w = right & 0x00000000000000FFULL;
         if(w)LinkPromote(w,-7);
         w = left & 0x00000000000000FFULL;
         if(w)LinkPromote(w,-9);
       }
       for(w=right&~0x00000000000000FFULL; w; w&=w-1)
       {
         to = GetOne(w);
         from = to + 7;
         LINK(from,to,PAWN);
       }
       for(w=left& ~0x00000000000000FF; w; w&=w-1)
       {
         to = GetOne(w);
         from = to + 9;
         LINK(from,to,PAWN);
       }

    }
   }
    for(p = pieces[KING]&all[side]; p; p&=p-1)
    {
       from = GetOne(p);
       for(w = KingAttack(from)&empty; w; w&=w-1)
       {
         to = GetOne(w);
         LINK(from,to,KING);
       }
    }

   treeCnt[ply+1] = list-tree;
}

void GenPromote(void)
{
  BitBoard empty,p,w,left,right;

   list = tree+treeCnt[ply];
   empty = ~(all[0]|all[1]);

   p = pieces[PAWN] & all[side];
   if(p)
   {
    if(side==BLACK){
       if(p&0x00FF000000000000ULL){
         right = (p<<9)&L_MASK&empty;
         left  = (p<<7)&R_MASK&empty;
         w = right & 0xFF00000000000000ULL;
         if(w)LinkPromote(w,9);
         w = left & 0xFF00000000000000ULL;
         if(w)LinkPromote(w,7);
       }
    }else{
       //PROMOTE
       if(p&0x000000000000FF00ULL){
         right = (p>>7)&L_MASK&empty;
         left  = (p>>9)&R_MASK&empty;
         w = right & 0x00000000000000FFULL;
         if(w)LinkPromote(w,-7);
         w = left & 0x00000000000000FFULL;
         if(w)LinkPromote(w,-9);
       }
    }
   }
   treeCnt[ply+1] = list-tree;
}


extern int whiteKing[64];
extern int whitePawn[64];

void InsertPiece(int p, int sq, int c)
{
   assert(color[sq]==NEUTRAL);

   SET(a1h8.b, a1h8.to[sq]);
   SET(a8h1.b, a8h1.to[sq]);
   SET(all[c], sq);
   SET(pieces[p], sq);

   pos[sq] = p;
   color[sq] = c;

   mtl[c] += value[p];

   if(p==PAWN){
        if(c==WHITE)  stScore[WHITE] += whitePawn[sq];
        else          stScore[BLACK] += whitePawn[63-sq];
   }else{
        if(c==WHITE)  stScore[WHITE] += whiteKing[sq];
        else          stScore[BLACK] += whiteKing[63-sq];
   }

   HashStep(p, sq, c);
}

void RemovePiece(int p, int sq, int c)
{
   assert(color[sq]==c && pos[sq]==p && c!=NEUTRAL);

   RESET(a1h8.b, a1h8.to[sq]);
   RESET(a8h1.b, a8h1.to[sq]);
   RESET(all[c], sq);
   RESET(pieces[p], sq);

   pos[sq] = NOPIECE;
   color[sq] = NEUTRAL;

   mtl[c] -= value[p];

   if(p==PAWN){
        if(c==WHITE)  stScore[WHITE] -= whitePawn[sq];
        else          stScore[BLACK] -= whitePawn[63-sq];
   }else{
        if(c==WHITE)  stScore[WHITE] -= whiteKing[sq];
        else          stScore[BLACK] -= whiteKing[63-sq];
   }

   HashStep(p, sq, c);
}


unsigned int capKingMask[MAX_GAME+MAX_PLY+1];

void MakeMove(Move *m)
{
   unsigned int w,mv,capKing,b;
   int p,sq;

   w = m->capMask;
   mv = m->mv;
   if(w){
     capKing = 0;
     do{
       b = w & -w;
       sq = from32[GetOne(b)];
       w &= ~b;
       p = pos[sq];
       if(p==KING) capKing |= b;
       RemovePiece(p,sq,xside);
     }while(w);
     capKingMask[game_cnt+ply] = capKing;
   }
   RemovePiece(PIECE(mv), FROM(mv), side);
   InsertPiece(NEW_PIECE(mv),TO(mv), side);
}



void UnMakeMove(Move *m)
{
   unsigned int w,mv,capKing,b;
   int p,sq;

   w = m->capMask;
   mv = m->mv;
   if(w){
      capKing = capKingMask[game_cnt+ply];
      do{
       b = w & -w;
       sq = from32[GetOne(b)];
       w &= ~b;
       if(b & capKing) p = KING;
       else p = PAWN;
       InsertPiece(p,sq,xside);
      }while(w);
   }
   RemovePiece(NEW_PIECE(mv),TO(mv), side);
   InsertPiece(PIECE(mv), FROM(mv), side);
}



void PhaseInfoReset(PhaseInfo *p)
{
  p->hashMv.mv = p->killMv = p->hashMv.capMask = 0;
  p->id = p->cnt = p->isSort = 0;
}


void Swap(int i1, int i2)
{
   Move tmp_move;
   int tmp_val;

   tmp_move = tree[i1];
   tree[i1] = tree[i2];
   tree[i2] = tmp_move;

   tmp_val = sortVal[i1];
   sortVal[i1] = sortVal[i2];
   sortVal[i2] = tmp_val;
}


int IsLegalNotCapMove(unsigned int mv){
  int from = FROM(mv), to = TO(mv);

  if( mv && pos[from]==PIECE(mv) && color[from]==side &&
      color[to]==NEUTRAL && (PIECE(mv)==PAWN ||
      (KingAttack(from) & ((U64)1<<to))   ))
    return 1;
  return 0;
}

void Pick(int low,int high)
{
  int *s, *stop, *maxP;

  maxP = s = sortVal+low;
  stop = sortVal+high;

  for(; s <= stop; s++)
   if(*s > *maxP)
     maxP = s;

  if(maxP - sortVal != low)
  {
    Swap(low,maxP - sortVal);
  }
}


void SortCap(int low, int high){

   if(high >= low){
     if(high == low) sortVal[low] = 1; // 1 move
     else{
       Move *m = tree + low;
       int *p = sortVal + low;
       int *stop = sortVal + high;
       unsigned int temp;
       int val;

       for(; p <= stop; p++, m++){
         val = 0;
         for(temp = m->capMask; temp; temp &= temp-1)
          val++; //COUNT CAPTURES
         if(PIECE(m->mv) != NEW_PIECE(m->mv))
          val += 5; //PROMOTE
         *p = val;
       }
     }
   }
}


void SortNotCap(int low, int high){

   if(high >= low){
     if(high == low) sortVal[low] = 1; // 1 move
     else{
       Move *m = tree + low;
       int *p = sortVal + low;
       int *stop = sortVal + high;
       unsigned int mv;
       int val;
       int promY = side==BLACK?6:1;

       for(; p <= stop; p++, m++){

         mv  = m->mv;
         val = history[(side<<12) | (mv&((63<<6)|63))];
        // if(PIECE(mv)==PAWN){
        //    if(NEW_PIECE(mv)==KING) val+=1000;
        //    else if(ROW(TO(mv))==promY) val+=10;
        // }
         *p = val;
       }
     }
   }
}






Move *NextMove(PhaseInfo *p)
{
// unsigned int mv;

  if(ply==0) goto ply0;

   switch(p->id){
     case 0:{
    //   GenCap(); //call in Search function
       p->id++;
       if(treeCnt[ply]==treeCnt[ply+1]){
         p->id = 3;
         goto notCapture;
       }else{
         Move *m,*stop,*start;
         p->cnt = treeCnt[ply];
         //PICK HASH MOVE (captures)
        if(p->hashMv.mv)
         for(start=m=tree+p->cnt, stop=tree+treeCnt[ply+1]; m<stop; m++)
          if(m->mv==p->hashMv.mv && m->capMask==p->hashMv.capMask){
            if(start != m) Swap(start-tree,m-tree);
            p->cnt++;
            return start;
          }
       }
     }
     case 1:{
        p->id++;
        if(treeCnt[ply+1] > p->cnt)
          SortCap(p->cnt,treeCnt[ply+1]-1);
     }
     case 2:{
       if(p->cnt < treeCnt[ply+1]){
         Pick(p->cnt, treeCnt[ply+1]-1);
         return &tree[p->cnt++];
       }
       return NULL;
     }
     case 3:{
notCapture:
       // TRY HASH MOVE
       p->id++;
       if(p->hashMv.capMask) p->hashMv.mv = 0;
       else{
           if(p->hashMv.mv && IsLegalNotCapMove(p->hashMv.mv))
              return &p->hashMv;
           else p->hashMv.mv = 0;
       }
     }
     case 4:{
       //TRY KILLER
       p->id++;
       if(p->killMv && p->killMv != p->hashMv.mv &&
          IsLegalNotCapMove(p->killMv)){
            p->temp.mv = p->killMv;
            p->temp.capMask = 0;
            return &p->temp;
       }else p->killMv = 0;
     }
     case 5:{
       GenNotCap();
       p->cnt = treeCnt[ply];
       p->id++;
       p->isSort = 0;

       if(treeCnt[ply+1] > p->cnt)
        if(ply < 8 || (pieces[KING] & all[side])){
         SortNotCap(p->cnt,treeCnt[ply+1]-1);
         p->isSort = 1;
        }

     }
     case 6:{
       while(p->cnt < treeCnt[ply+1]){
         Move *m;
         if(p->isSort)
           Pick(p->cnt, treeCnt[ply+1]-1);
         m = &tree[p->cnt++];
         if(m->mv != p->hashMv.mv && m->mv != p->killMv)
           return m;
       }
     }
   }//switch
  return NULL;


ply0:

  switch(p->id){
    case 0:{
       p->id++;
       p->cnt = treeCnt[ply];
    }
    case 1:{
      if(p->cnt < treeCnt[ply+1]){
         Pick(p->cnt, treeCnt[ply+1]-1);
         return &tree[p->cnt++];
      }
    }
  }//switch
  return 0;
}








