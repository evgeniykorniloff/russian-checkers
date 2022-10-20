#include "checkers.h"

extern int stScore[2];
int whitePawn[64]={
    0,   32,    0,   32,    0,   32,    0,   32,
   14,    0,   28,    0,   28,    0,   28,    0,
    0,   18,    0,   24,    0,   24,    0,   12,
    7,    0,   17,    0,   20,    0,   15,    0,
    0,   10,    0,   16,    0,   14,    0,    6,
    4,    0,   10,    0,   12,    0,    8,    0,
    0,    5,    0,    8,    0,    7,    0,    2,
    1,    0,    3,    0,    4,    0,    2,    0
};
int whiteKing[64]={
    0,   18,    0,   16,    0,   16,    0,   19,
   17,    0,   21,    0,   21,    0,   23,    0,
    0,   21,    0,   26,    0,   27,    0,   15,
   15,    0,   26,    0,   33,    0,   20,    0,
    0,   20,    0,   32,    0,   25,    0,   15,
   16,    0,   26,    0,   24,    0,   21,    0,
    0,   22,    0,   19,    0,   20,    0,   18,
   19,    0,   15,    0,   15,    0,   17,    0
};

#define A1H8 0x0102040810204080ULL

int Evaluate(int alpha, int beta)
{
  int Max(int v1, int v2);
  int Min(int v1, int v2);
  int IsolPawn(int c);
  int BlockedPawn(int c);
  int PassedPawn(int c);
  int KingA1H8(int c);
  int s[2] = {0,0};
  int m[2];
  #define SCORE (s[side] - s[xside])

  m[WHITE] = mtl[WHITE];
  m[BLACK] = mtl[BLACK];
  //премия за первую дамку

  if(pieces[KING]){
     if(pieces[KING] & all[WHITE])
       m[WHITE] += 2*VALUE_P;
     if(pieces[KING] & all[BLACK])
       m[BLACK] += 2*VALUE_P;
  }

  s[WHITE] += m[WHITE] + stScore[WHITE];
  s[BLACK] += m[BLACK] + stScore[BLACK];



  if(pieces[PAWN]){
    if(SCORE + 64 <= alpha || SCORE - 64 >= beta)
      return SCORE;

   //printf("%d  %d\n", s[WHITE],s[BLACK]);

    s[WHITE] -= IsolPawn(WHITE);
    s[BLACK] -= IsolPawn(BLACK);

   //printf("%d  %d\n", s[WHITE],s[BLACK]);

    s[WHITE] -= BlockedPawn(WHITE);
    s[BLACK] -= BlockedPawn(BLACK);

  // printf("%d  %d\n", s[WHITE],s[BLACK]);



    if(SCORE + 32 <= alpha || SCORE - 32 >= beta)
      return SCORE;

    s[WHITE] += PassedPawn(WHITE);
    s[BLACK] += PassedPawn(BLACK);
  }else{
     //если большей стороны меньше 3 дамок - ничья
     if(Max(mtl[WHITE],mtl[BLACK]) < 3*VALUE_K){
        s[WHITE] -= m[WHITE]; //материал равен
        s[BLACK] -= m[BLACK];
        //если одинокая дамка одна на большой дороге - ничья
     }else if(Min(mtl[WHITE],mtl[BLACK]) == VALUE_K){
        BitBoard b = A1H8 & pieces[KING];
        if(b &&  (b & (b-1))==0){ //1 KING
          if(mtl[color[GetOne(b)]]==VALUE_K){
             s[WHITE] -= m[WHITE]; //материал равен
           s[BLACK]   -= m[BLACK];
          }
        }
     }
     s[WHITE] += KingA1H8(WHITE);
     s[BLACK] += KingA1H8(BLACK);
  }
  return SCORE + rand()%7-3;
}

//пешки, не защищенные сзади
int IsolPawn(int c){
   BitBoard p = pieces[PAWN] & all[c],
            empty = ~(all[0] | all[1]),
            op = all[c^1];

   if(c==WHITE)
     return BitCnt(
           (((p<<9)&L_MASK&(empty|op)) >> 9)
                      &
           (((p<<7)&R_MASK&(empty|op)) >> 7)
                  );
   else
     return BitCnt(
           (((p>>9)&R_MASK&(empty|op)) << 9)
                      &
           (((p>>7)&L_MASK&(empty|op)) << 7)
                  );
}

int BlockedPawn(int c){
   BitBoard p = pieces[PAWN] & all[c],
            empty = ~(all[0] | all[1]);

   if(c==WHITE)
     return BitCnt(  //нет хода всево или бортовая
         ((((p>>9)&R_MASK&~empty) << 9) | (p & ~L_MASK))
             &  //нет хода вправо или бортовая
         ((((p>>7)&L_MASK&~empty) << 7) | (p & ~R_MASK))
     );
   else
     return BitCnt(
         ((((p<<9)&L_MASK&~empty) >> 9) | (p & ~R_MASK))
             &
         ((((p<<7)&R_MASK&~empty) >> 7) | (p & ~L_MASK))
     );
}

int Max(int v1, int v2) { return v1>v2?v1:v2; }
int Min(int v1, int v2) { return v1<v2?v1:v2; }




BitBoard bpm[64],wpm[64];

void InitPromoteMask(void){
  int j;

  for(j = 0; j < 64; j++){
    BitBoard m = 0;
    if(ROW(j)==0) m |= (U64)1<<j;
    else{
       if(COLUMN(j)>0) m |= wpm[j-8-1];
       if(COLUMN(j)<7) m |= wpm[j-8+1];
       m |= (U64)1<<j;
    }
    wpm[j] = m;
  }

  for(j = 63; j >= 0; j--){
    BitBoard m = 0;
    if(ROW(j)==7) m |= (U64)1<<j;
    else{
       if(COLUMN(j)>0) m |= bpm[j+8-1];
       if(COLUMN(j)<7) m |= bpm[j+8+1];
       m |= (U64)1<<j;
    }
    bpm[j] = m;
  }
}



int PassedPawn(int c){
  static int bonus[2][8] =
  {
    {0,16,8,4,0,0,0, 0},
    {0, 0,0,0,4,8,16,0}
  };
  static int first = 1;
  BitBoard p,op, *mask;
  int s = 0;

  if(first){
    first = 0;
    InitPromoteMask();
  }

  if(c==BLACK){
    p = pieces[PAWN] & all[BLACK] & 0xFFFFFFFF00000000ULL;
    if(p==0) return 0;
    op = pieces[PAWN] & all[WHITE];
    mask = bpm;
  }else{
    p = pieces[PAWN] & all[WHITE] & 0x00000000FFFFFFFFULL;
    if(p==0) return 0;
    op = pieces[PAWN] & all[BLACK];
    mask = wpm;
  }

  for(; p; p&=p-1){
     int sq = GetOne(p);
     if((mask[sq] & op)==0)
      s += bonus[c][ ROW(sq) ];
  }

  if(s > 32) s = 32;
  return s;
}


int KingA1H8(int c){
  int s = 0, cnt = 0;
  BitBoard p = pieces[KING] & all[c] & A1H8;
  for(; p; p&=p-1){
    cnt++;
    if(cnt==1) s += 16;
    else s -= 8;
  }
  return s;
}

