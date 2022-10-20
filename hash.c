#include "checkers.h"

/*
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
*/

HashKey key;
static HashItem *hash[2];
static HashKey randTable[2][2][64];



void HashReset(void)
{
   static int first = 1;
   int c,p,sq;

   if(first){
      first = 0;
      for(c = WHITE; c <= BLACK; c++)
       for(p = PAWN; p <= KING; p++)
        for(sq = 0; sq < 64; sq++)
         randTable[c][p][sq] = ((U64)rand() << 48) |
                               ((U64)rand() << 32) |
                               ((U64)rand() << 16) |
                               ((U64)rand() << 0);
      hash[0] = malloc(sizeof(HashItem)*HASH_SIZE);
      hash[1] = malloc(sizeof(HashItem)*HASH_SIZE);
      if(!hash[0] || !hash[1] ){
         fprintf(stderr,"error init hash\n");
         abort();
      }
   }
   memset(hash[0],0,sizeof(HashItem)*HASH_SIZE);
   memset(hash[1],0,sizeof(HashItem)*HASH_SIZE);
}


void HashInsert(Move mv, int score, int depth, int color, int flag)
{
  HashItem *p;

  p =  &hash[color][ (int)key & (HASH_SIZE-1)];
  //0001

  if(p->depth <= depth){
     p->key = key;
     p->mv = mv;
     p->score = score;
     p->flag = (signed char)flag;
     p->depth = (signed char)depth;
  }

}


HashItem* HashLook(int color)
{
  HashItem *p;
  p =  &hash[color][ (int)key & (HASH_SIZE-1)];

  if(p->key==key)return p;
  return NULL;
}


void HashStep(int p, int sq, int c)
{
   key ^= randTable[c][p][sq];
}






