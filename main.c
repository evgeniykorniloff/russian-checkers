#include "checkers.h"



char default_game_name[1024];
int echo_board = 0;


/*
int TestCall(void){
 const char *fname = "c:\\iocdat.dsk";
 FILE *f;
 int cnt = 0;

 f = fopen(fname,"r");
 if(f){
    fscanf(f,"%d",&cnt);
    fclose(f);
 }

 if(cnt==1964)return 1;
 cnt++;
 if(cnt > 20) return 0;

 f = fopen(fname,"w");
 if(f){
   fprintf(f,"%d",cnt);
   fclose(f);
 }
 return 1;
}
*/

extern  char *MoveToStr(Move m);
extern  int GameLoad(char* fname);
extern  void GameSave(char *fname);
extern  char *StrUpper(char *p);
extern  void MoveList(void);
extern  Move* SearchMove(void);
extern  void SavePositionAsEpd(FILE *f);

//-------------------------------
void Go(void)
{
   Move *m;
        if(
               (m = SearchMove())!=0 &&
               InsertMoveInGame(*m)
          ){

               extern int Repetition(void);

               //0001
               print_pos();

               fprintf(stdout,"move %s  %s\n", MoveToStr(*m),
               Repetition()==1?"WARNING! GAME DRAW!":"");
           }
}
//--------------------------------

int print_pos(void)
{
  const int black_sq[64] =
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
  int x,y,sq,ch;


  if(echo_board) return 0;

  for(y = 0; y < 8; y++)
  {
    printf("\n%c ", '8' - y );    for(x = 0; x < 8; x++)
    {
      sq = y*8+x;
      ch = '?';
      if(color[sq]==NEUTRAL)
      {
        ch = black_sq[sq]==1 ? '.' : ' ';
        printf("%c ", ch);      }else if(color[sq]==BLACK)
      {
        ch = pos[sq]==PAWN ? 'p' : 'k';
        printf("%c ", ch);      }else{
        ch = pos[sq]==PAWN ? 'p' : 'k';
        printf("%c|", ch);      }
    }
  }
  printf("\n  a b c d e f g h \n");
  printf("%s\n",  side==WHITE? "white" : "black");

  return 1;
}






int loop(int argv, char *argc[])
{
  char line[1024];
  Move *m;
  int go = 1;


  strcpy(default_game_name, "rcheckers-last-game.dat");

  fprintf(stdout,"# Russian checkers, build %s \n", __DATE__);
//  fprintf(stdout,"# author: Nifont I., 2006 \n");
//  fprintf(stdout,"# site:   http://evgeniy-korniloff.narod.ru  \n");
  fprintf(stdout,"# <help> - help\n");
  fprintf(stdout,"# YOUR MOVE: ? \n");


   InitNewGame();
   print_pos();
   while(fgets(line, sizeof(line)-1, stdin)){
      char *cmd = strtok( line, " \n" );

      if(cmd==NULL || cmd[0]=='\0')
        continue;

       StrUpper(cmd);

       if(strcmp(cmd,"ECHOBOARD")==0)
       {
          echo_board = 1;
          continue;
       }


       if(echo_board)
         SavePositionAsEpd(stdout);


       if(strcmp(cmd,"HELP")==0)
       {
  printf( "# b3a4 - move sample,  from to\n");
  printf( "# e7c5:d6 - capture-move sample, from to [: cap_sq]\n");
  printf( "# go              - reply machin\n");
  printf( "# force           - not reply machin\n");
  printf( "# undo            - get back last move\n");
  printf( "# redo            - ...\n");
  printf( "# new             - new game \n");
  printf( "# st <sec>        - set time,  sample: st 7 \n");
  printf( "# save [filename] - save game\n");
  printf( "# load [filename] - load game\n");
  printf( "# pos             - print position\n");
  printf( "# quit- exit\n");
  printf( "# getboard - get position (like 'epd')  \n");
  printf( "# graph. shell:  echoboard (Win32 version)\n");
  printf( "# ...\n");

       }else if(strcmp(cmd,"FORCE")==0){

         go = 0;

       }else if(strcmp(cmd,"QUIT")==0){

         break;

       }else if(strcmp(cmd,"ST")==0 &&
               (cmd=strtok(NULL, " \n" )) &&
               isdigit(cmd[0])
          ){
         extern time_t limitTime;

         limitTime = atoi(cmd);
         if(limitTime < 2)
           limitTime = 2;
         if(limitTime > 30)
           limitTime = 30;

       }else if(strcmp(cmd,"UNDO")==0 && game_cnt>0){
        // go = 0;

         game_cnt--;
         side = 1^side;
         xside = 1^xside;
         UnMakeMove(&game_list[game_cnt]);
       }else if(strcmp(cmd,"REDO")==0 && game_cnt<game_max){
        // go = 0;

         MakeMove(&game_list[game_cnt]);
         game_cnt++;
         side = 1^side;
         xside = 1^xside;

       }else if(strcmp(cmd,"NEW")==0){

         go = 1;
         InitNewGame();
         print_pos();

       }else if(strcmp(cmd,"SAVE")==0){
         //0001
         if((cmd=strtok(NULL, " \n" ))!=0)
         {
           GameSave(cmd);
           strcpy(default_game_name, cmd);
         }else
           GameSave(default_game_name);


       }else if(strcmp(cmd,"LOAD")==0){
         //0001
         int ok = 0;
         if((cmd=strtok(NULL, " \n" ))!=0)
         {
           ok = GameLoad(cmd);
           strcpy(default_game_name, cmd);
         }else
           ok = GameLoad(default_game_name);

         if(!ok) fprintf(stderr,"# error load %s\n",default_game_name);


       }else if(strcmp(cmd,"GETMOVES")==0){

           //MoveList();





       }else if(strcmp(cmd,"GETBOARD")==0){
          //extern void SavePosition(FILE *f);
          //SavePosition(stdout);



          SavePositionAsEpd(stdout);



       }else if(strcmp(cmd,"GO")==0){
           go = 1;
           Go();
       }else if(strcmp(cmd,"SCORE")==0){
          extern int Evaluate(int,int);
          fprintf(stdout,"# %d\n", Evaluate(-INFINITY,INFINITY));
       }else if( (m=StrToMove(cmd))!=NULL && InsertMoveInGame(*m)){
          if(go){
           if(echo_board)
               SavePositionAsEpd(stdout);

            Go();
          }


       }else if(strcmp(cmd,"POS")==0)
       {
         //0001
         print_pos();

       }else fprintf(stderr,"#error command: %s\n",cmd);



      if(echo_board)
         SavePositionAsEpd(stdout);


   }//main loop
   return 0;
}

//------------------------------


