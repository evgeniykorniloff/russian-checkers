#include "checkers.h"

TBitItem a1h8,a8h1;
unsigned char BitN[129];
const BitBoard
 a1h8_mask = 0xFF7F3F1F0F070301ULL,
 a8h1_mask = 0x0103070F1F3F7FFFULL;

int
__a1h8[64] =
{
 A8,  B1,C2,D3,E4,F5,G6,H7,
 A7,B8,  C1,D2,E3,F4,G5,H6,
 A6,B7,C8,  D1,E2,F3,G4,H5,
 A5,B6,C7,D8,  E1,F2,G3,H4,
 A4,B5,C6,D7,E8,  F1,G2,H3,
 A3,B4,C5,D6,E7,F8,  G1,H2,
 A2,B3,C4,D5,E6,F7,G8,  H1,
 A1,B2,C3,D4,E5,F6,G7,H8
},

__a8h1[64] =
{
 A8,B7,C6,D5,E4,F3,G2,H1,
 A7,B6,C5,D4,E3,F2,G1, H8,
 A6,B5,C4,D3,E2,F1, G8,H7,
 A5,B4,C3,D2,E1, F8,G7,H6,
 A4,B3,C2,D1, E8,F7,G6,H5,
 A3,B2,C1, D8,E7,F6,G5,H4,
 A2,B1, C8,D7,E6,F5,G4,H3,
 A1, B8,C7,D6,E5,F4,G3,H2
};


void InitBrd(TBitItem *p, BitBoard bit_mask, int from[64])
{
   BitBoard m;
   int i,j,x,y,mask,shift;


   for(i = 0; i < 64; i++)
   {
     p->from[i] = from[i];
     p->to[from[i]] = i;
   }

   for(i = 0; i < 64; i++)
   {
     shift = i/8*8;
     y = i/8;
     if( ((U64)1 << i) &  bit_mask)
       mask = ((int)(bit_mask >> shift) & 0xFF);
     else
       mask = ((int)(~bit_mask >> shift) & 0xFF);
     for(j = 0; j < 256; j++)
     {
        m = 0;
        x = i%8-1;
        while(x >= 0 && ((1<<x)&mask) )
        {
          m |= (U64)1 << p->from[y*8+x];
          if((1<<x)&j) break;
          x--;
        }
        x = i%8+1;
        while(x < 8 && ((1<<x)&mask) )
        {
          m |= (U64)1 << p->from[y*8+x];
          if((1<<x)&j) break;
          x++;
        }
        p->hlat[i][j] = m;
     }//for
   }//for
}//InitBrd


void InitBitBoard(void)
{
  int j;
  InitBrd(&a1h8,a1h8_mask, __a1h8);
  InitBrd(&a8h1,a8h1_mask, __a8h1);


  for(j = 0; j < 8; j++)
     BitN[1<<j] = (unsigned char)j;

}

int GetOne(BitBoard w)
{
  unsigned int b;
  int n;
  if(w & 0xFFFFFFFF) b = (unsigned int)w, n = 0;
  else b = (unsigned int)(w>>32), n = 32;
  b &= -b;
  if(b & 0xFFFF)
  {
      if(b & 0xFF) return BitN[b]+n;
      else return BitN[b>>8]+8+n;
  }else{
       if(b & 0xFFFFFF) return BitN[b>>16]+16+n;
       else return BitN[b>>24]+24+n;
  }
}



