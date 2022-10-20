#ifndef CHECKERS_H_INCLUD
#define CHECKERS_H_INCLUD
/////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

void assert(int expr);


#define PAWN 0
#define KING 1
#define NOPIECE 2

#define WHITE 0
#define BLACK 1
#define NEUTRAL 2

#define VALUE_P 1000
#define VALUE_K 3000
#define INFINITY 100000


typedef unsigned long long U64;
typedef U64 BitBoard;
typedef struct{
  int from[64],to[64];
  BitBoard b,hlat[64][256];
}TBitItem;

enum squares  {
 A8,B8,C8,D8,E8,F8,G8,H8,
 A7,B7,C7,D7,E7,F7,G7,H7,
 A6,B6,C6,D6,E6,F6,G6,H6,
 A5,B5,C5,D5,E5,F5,G5,H5,
 A4,B4,C4,D4,E4,F4,G4,H4,
 A3,B3,C3,D3,E3,F3,G3,H3,
 A2,B2,C2,D2,E2,F2,G2,H2,
 A1,B1,C1,D1,E1,F1,G1,H1
};


extern TBitItem a1h8,a8h1;
#define KingAttack(sq)\
 (a1h8.hlat[a1h8.to[sq]][(int)(a1h8.b >> (a1h8.to[sq]&~7))&0xFF] |\
  a8h1.hlat[a8h1.to[sq]][(int)(a8h1.b >> (a8h1.to[sq]&~7))&0xFF])
#define SET(b,sq)    (b |= (U64)1 << sq)
#define RESET(b,sq)  (b &= ~((U64)1 << sq))





#define MTL (mtl[side]-mtl[xside])
#define L_MASK ~0x0101010101010101ULL
#define R_MASK ~0x8080808080808080ULL



#define MAP(sq)     ((sq&7) | ((sq&~7)<<1))
#define UNMAP(sq)   ((sq&7) | ((sq&~7)>>1))
#define ROW(sq) (sq>>3)
#define COLUMN(sq) (sq&7)


#define MAX_GAME 300
#define MAX_PLY  60




typedef struct{
  unsigned long mv,capMask;
}Move;

#define FROM(mv)       (int)((mv>>6)&63)
#define TO(mv)         (int)(mv&63)
#define PIECE(mv)      (int)((mv>>12)&7)
#define NEW_PIECE(mv)  (int)((mv>>15)&7)


#define INCLUDE(m32,sq64)  (m32 |=  1<<to32[sq64])
#define EXCLUDE(m32,sq64)  (m32 &= ~(1<<to32[sq64]))


typedef struct{
  Move hashMv,temp;
  unsigned long killMv;
  int id,cnt,isSort;
}PhaseInfo;



extern BitBoard all[2],pieces[2];
extern int color[64],pos[64];
extern int side,xside,ply;
extern int mtl[2];
extern int const value[3];
extern Move tree[MAX_PLY*100];
extern int treeCnt[MAX_PLY+1];
extern int fromTo[64][64];
extern Move game_list[MAX_GAME+MAX_PLY+1];
extern int game_max,game_cnt;
extern int to32[64],from32[32];

////////
int GetOne(BitBoard w);
void InitBitBoard(void);
unsigned long BitCnt(const BitBoard b);
void InitGen(void);
void InsertPiece(int p, int sq, int c);
void RemovePiece(int p, int sq, int c);
void InitNewGame(void);
void MakeMove(Move *m);
void UnMakeMove(Move *m);
void GenCap(void);
void GenNotCap(void);
void PhaseInfoReset(PhaseInfo *p);
Move *NextMove(PhaseInfo *p);
Move *StrToMove(char *s);
int InsertMoveInGame(Move m);



#define HASH_SIZE  (1<<18)

#define HASH_ALPHA 1
#define HASH_EXACT 2
#define HASH_BETA  3

////////HASH//////////
typedef U64 HashKey;
typedef struct{
  HashKey key;
  Move mv;
  int score;
  signed char flag,depth;
}HashItem;
extern void HashReset(void);
extern void HashInsert(Move mv, int score, int depth, int color, int flag);
extern HashItem* HashLook(int color);
extern void HashStep(int p, int sq, int c);
extern HashKey key_list[MAX_GAME+MAX_PLY+1];
extern HashKey key;
extern HashKey key_list[MAX_GAME+MAX_PLY+1];

extern int print_pos(void);

//////////////////
#endif



