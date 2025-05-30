#include <ctype.h>
#include "checkers.h"

#define M1__  0x5555555555555555ULL
#define M2__  0x3333333333333333ULL


unsigned long BitCnt(const BitBoard b)
{
    unsigned long n;
    const BitBoard a = b - ((b >> 1) & M1__);
    const BitBoard c = (a & M2__) + ((a >> 2) & M2__);
    n = ((unsigned long) c) + ((unsigned long) (c >> 32));
    n = (n & 0x0F0F0F0F) + ((n >> 4) & 0x0F0F0F0F);
    n = (n & 0xFFFF) + (n >> 16);
    n = (n & 0xFF) + (n >> 8);
    return n;
}



const char *chb[] = {
"A8","B8","C8","D8","E8","F8","G8","H8",
"A7","B7","C7","D7","E7","F7","G7","H7",
"A6","B6","C6","D6","E6","F6","G6","H6",
"A5","B5","C5","D5","E5","F5","G5","H5",
"A4","B4","C4","D4","E4","F4","G4","H4",
"A3","B3","C3","D3","E3","F3","G3","H3",
"A2","B2","C2","D2","E2","F2","G2","H2",
"A1","B1","C1","D1","E1","F1","G1","H1", NULL
};


int StrToSq(char *s, int *sq)
{
  char **pt,*s1,*s2;

  if(s==NULL) return 0;
  for(pt = (char**)chb; *pt; pt++){
    for(s1=s, s2=*pt; toupper(*s1)==*s2; s1++,s2++)
      if(*s1=='\0'){
          *sq = pt-(char**)chb;
          return 1;
      }
  }
  return 0;
}

/*
  преобразует строку вида:
    b3a4 (простое перемещение)
    b4d6:c5 (взятие на c5)
*/
Move *StrToMove(char *s)
{

  const char *DEL = " :\n";
  unsigned long capMask = 0;
  int from,to,capSq,j,len;
  Move *m;
  char *StrTok(char *str, const char *del);
  char buf[256],*ps;

if( s && (len=strlen(s)) >= 4){

  strcpy(buf,s);

  j = len;
  while(j > 2){
       buf[j] = buf[j-1];
       j--;
  }
  buf[2] = ' ';
  buf[len+1] = ' ';
  buf[len+2] = '\0';



  if(StrToSq(strtok(buf,DEL),&from) && StrToSq(strtok(NULL,DEL),&to)){


    while( (ps=strtok(NULL,DEL)) != NULL ){
      if(StrToSq(ps,&capSq)==0) return NULL;
      INCLUDE(capMask,capSq);
    }

    GenCap();
    if(treeCnt[1]==treeCnt[0]) GenNotCap();
    for(m=tree; m < tree+treeCnt[1]; m++)
     if(FROM(m->mv)==from && TO(m->mv)==to &&
        (capMask==m->capMask || capMask==0))
        return m;
  }
 }
  return NULL;
}


int InsertMoveInGame(Move m)
{
  if(game_cnt < MAX_GAME){
     MakeMove(&m);
     game_list[game_cnt] = m;
     key_list[game_cnt] = key;
     game_max = ++game_cnt;
     side = 1^side;
     xside = 1^xside;
     return 1;
  }
  return 0;
}

int foo;
void assert(int expr)
{
  if(expr==0){
    foo++;
//   abort();
  }
}



char *MoveToStr(Move m)
{
  static char s[256];

  s[0] = '\0';
  strcat(s, chb[FROM(m.mv)]);
  strcat(s, chb[TO(m.mv)]);

  while(m.capMask){
    int capSq = from32[GetOne(m.capMask)];
    m.capMask &= m.capMask-1;
    strcat(s, ":");
    strcat(s, chb[capSq]);
  }
  return s;
}


void SavePosition(FILE *f){

  do{
    int c,p,j;
    for(c = WHITE; c <= BLACK; c++){
      fprintf(f,"%s:\n",c==WHITE?"WHITE":"BLACK");
      for(p = PAWN; p <= KING; p++){
        fprintf(f,"  %s: ",p==PAWN?"PAWNS":"KINGS");
        for(j = 0; j < 64; j++)
         if(color[j]==c && pos[j]==p)
          fprintf(f,"%s ",chb[j]);
        fprintf(f,"\n");
      }//for p
    }//for c
  }while(0);

}
//-----------------------------
void SavePositionAsEpd(FILE *f){
  int j;
   
   fprintf(f,"position ");
  
   for(j = 0; j < 64; j++)
   {
      if(j%8==0)
        if(j > 0)
          fprintf(f,"/");
      switch(color[j])
      {
        case WHITE:{
           if(pos[j]==PAWN)
             fprintf(f,"P");
            else
             fprintf(f,"K"); 
        }break;
        case BLACK:{
           if(pos[j]==PAWN)
             fprintf(f,"p");
            else
             fprintf(f,"k"); 
        }break;
        default:
          fprintf(f,"1");
      }//switch
   }
   
   if(side==WHITE)
     fprintf(f," w");
   else
     fprintf(f," b");
     
     
   fprintf(f,"\n");
}




void GameSave(char *fname)
{
  int save = game_cnt;
  FILE *f = fopen(fname,"w");
  if(!f) goto error;

  //BACK TO START POSITION
  for(game_cnt-=1; game_cnt >= 0; game_cnt--){
     side = 1^side;
     xside = 1^xside;
     UnMakeMove(&game_list[game_cnt]);
  }
  game_cnt = 0;


  fprintf(f,"# last game\n");


  //SAVE POSITION
  SavePosition(f);

  //SAVE SIDE OF MOVE
  fprintf(f,"SIDE: %s\n", side==BLACK?"BLACK":"WHITE");

  //SAVE MOVE LIST
  do{
    int lines = 1;
    char *StrLower(char *s);

    fprintf(f,"MOVES-LIST:\n");

    while(game_cnt < save){
      fprintf(f,"%3d. ",lines++);
      if(side==WHITE){
         MakeMove(&game_list[game_cnt]);
         fprintf(f,"%-7s ", StrLower( MoveToStr(game_list[game_cnt])) );
         side = 1^side;
         xside = 1^xside;
         game_cnt++;
      }else fprintf(f,"%-7s "," ");

      if(game_cnt < save)
       if(side==BLACK){
         MakeMove(&game_list[game_cnt]);
         fprintf(f,"%-7s\n", StrLower( MoveToStr(game_list[game_cnt])) );
         side = 1^side;
         xside = 1^xside;
         game_cnt++;
       }
    }//while
  }while(0);

  fclose(f);
  return;

error:
 return;

}


char *StrUpper(char *s)
{
 char *p;
 for(p = s; *p; p++) *p = (char)toupper(*p);
 return p;
}

char *StrLower(char *s)
{
 char *p;
 for(p = s; *p; p++) *p = (char)tolower(*p);
 return s;
}


char *StrTok(char *str, const char *del){
  static char *s, *end, save_ch;

  if(str) s = str;
  else{
    if(save_ch=='\0') return NULL;
    *end = save_ch;
    s = end+1;
  }
  while(*s!='\0' && strchr(del,*s)) s++;
  end = s;
  while(*end != '\0' && !strchr(del,*end)) end++;
  save_ch = *end;
  *end = '\0';
  if(*s=='\0') return NULL;
  return s;
}

int StrCmp(char *s1, char *s2){
  if(s1==NULL || s2==NULL) return -1;
  while(toupper(*s1)==toupper(*s2)){
    if(*s1=='\0') return 0;
    s1++; s2++;
  }
  return (int)((unsigned char)(*s2)) - (unsigned char)(*s1);
}

char *FGetS(char *buf, int n, FILE* f)
{
   char *s;
loop:
   if(fgets(buf,n,f)==NULL) return NULL;
   for(s=buf; *s==' ';s++);
   if(*s=='#' || *s=='\n') goto loop; //COMMENT
   return buf;
}



int GameLoad(char* fname)
{
  Move *m;
  char s[256];
  FILE *f = fopen(fname,"r");
  if(f == 0) goto error;
    InitNewGame();

    //LOAD POSITION
    do{
      const char *DEL = " :\n,.";
      char s[256],*ps;
      int c,p,j,sq;
      //REMOVE ALL PIECES
      for(j = 0; j < 64; j++)
       if(color[j] != NEUTRAL)
          RemovePiece(pos[j], j, color[j]);

      //READ
      for(c = WHITE; c <= BLACK; c++){
         if(FGetS(s,255,f)==NULL ||
            StrCmp(c==WHITE?"WHITE":"BLACK",
                   StrTok(s,DEL)) != 0) goto error;
         for(p = PAWN; p <= KING; p++){
            if(FGetS(s,255,f)==NULL ||
               StrCmp(p==PAWN?"PAWNS":"KINGS",
                   StrTok(s,DEL)) != 0) goto error;

            while((ps=StrTok(NULL,DEL)) != NULL &&
                   StrToSq(ps,&sq))
                 InsertPiece(p,sq,c);

         }//for p
      }//for c
      if(mtl[WHITE]==0 || mtl[BLACK]==0) goto error;
      //READ SIDE
      if(FGetS(s,255,f)==NULL ||
         StrCmp("SIDE",StrTok(s,DEL)) != 0) goto error;
      ps = StrTok(NULL,DEL);
      if(StrCmp(ps,"BLACK")==0) side = BLACK;
      else if(StrCmp(ps,"WHITE")==0) side = WHITE;
      else goto error;
      xside = 1^side;
    }while(0);


   do{
      const char *DEL = " .\n";
      int lines;
      char *ps;

      if(FGetS(s,255,f)==NULL) goto success;
      if(StrCmp("MOVES-LIST",StrTok(s," :.\n")) != 0) goto error;

      lines = 0;
      while(FGetS(s,255,f)){
        if( (ps = StrTok(s,DEL))==NULL ) break;
        if(atoi(ps) != ++lines) goto error;
        while( (ps = StrTok(NULL,DEL)) != NULL){
          if( (m=StrToMove(ps)) == NULL ||
            InsertMoveInGame(*m)==0) goto error;
            fprintf(stdout,"%s\n",MoveToStr(*m));
        }
      }
//      fflush(stdout);

   }while(0);
success:
  fclose(f);
  return 1;
error:
  if(f)fclose(f);
  return 0;
}

void MoveList(void)
{
  Move *m;
    GenCap();
    if(treeCnt[1]==0) GenNotCap();
    fprintf(stdout,"# ");
    for(m=tree; m < tree+treeCnt[1]; m++)
      fprintf(stdout,"%s; ", MoveToStr(*m));
    fprintf(stdout,"\n");  
//    fflush(stdout);
}



