#include<stdio.h>
#include "cycle_map.h"
int cpu2antic[CPU2ANTIC_SIZE*(17*7+1)];
int antic2cpu[CPU2ANTIC_SIZE*(17*7+1)];
void try_all_scroll(int md,int use_char_index,\
  int use_font,int use_bitmap,int *cpu2antic,int *antic2cpu);   
void antic_steal_map(int width,int md,int scroll_offset,int use_char_index,\
  int use_font, int use_bitmap,char *antic_cycles,int *cpucycles, \
  int *actualcycles);
void cpu_cycle_map(char *antic_cycles_orig,int *cpu_cycles,int *actual_cycles);
#undef TEST_CYCLE_MAP
#ifdef TEST_CYCLE_MAP
int main(){
create_cycle_map();
return 0;
};
#endif

void cpu_cycle_map(char *antic_cycles_orig,int *cpu_cycles, int *actual_cycles){
  int i;
  char antic_cycles[CPU2ANTIC_SIZE];
  int cpu_xpos=0;
  int antic_xpos=0;
  char c;
  for(i=0;i<=113;i++){
    antic_cycles[i]=antic_cycles_orig[i];
  } 
  for(i=114;i<=120;i++){
    antic_cycles[i]='.';
  } 
  for(i=0;i<CPU2ANTIC_SIZE;i++){
    cpu_cycles[i]=-1;
  }
  while(antic_xpos<CPU2ANTIC_SIZE){
    c=antic_cycles[antic_xpos];
    actual_cycles[antic_xpos]=cpu_xpos;
    if(c!='R' && c!='S' && c!='F' && c!='I'){  /*Not a stolen cycle*/
      cpu_cycles[cpu_xpos]=antic_xpos; 
      cpu_xpos++;
    }
    
    antic_xpos++;
  }
}

void create_cycle_map(){
  #ifdef TEST_CYCLE_MAP
  int i,j;
  int *cpu_cycles;
  int *actual_cycles;
  #endif
  char antic_cycles[115];
  int k;
  k=0;
  antic_steal_map(1,0,0,0,0,0,antic_cycles,&cpu2antic[k],&antic2cpu[k]); /* blank line*/
  k=CPU2ANTIC_SIZE*(17*0+1);
  try_all_scroll(0,1,1,0,&cpu2antic[k],&antic2cpu[k]); /*mode 2,3,4,5 first line*/
  k=CPU2ANTIC_SIZE*(17*1+1);
  try_all_scroll(0,0,1,0,&cpu2antic[k],&antic2cpu[k]); /*mode 2,3,4,5 following lines*/
  k=CPU2ANTIC_SIZE*(17*2+1);
  try_all_scroll(1,1,1,0,&cpu2antic[k],&antic2cpu[k]); /*mode 6,7 first line*/
  k=CPU2ANTIC_SIZE*(17*3+1);
  try_all_scroll(1,0,1,0,&cpu2antic[k],&antic2cpu[k]); /*mode 6,7 following lines*/
  k=CPU2ANTIC_SIZE*(17*4+1);
  try_all_scroll(0,0,0,1,&cpu2antic[k],&antic2cpu[k]); /*mode 8,9 */
  k=CPU2ANTIC_SIZE*(17*5+1);
  try_all_scroll(1,0,0,1,&cpu2antic[k],&antic2cpu[k]); /*mode A,B,C*/
  k=CPU2ANTIC_SIZE*(17*6+1);
  try_all_scroll(2,0,0,1,&cpu2antic[k],&antic2cpu[k]); /*mode D,E,F*/
  #ifdef TEST_CYCLE_MAP
  for(j=0;j<17*7+1;j++){
    cpu_cycles=&cpu2antic[CPU2ANTIC_SIZE*j];
    actual_cycles=&antic2cpu[CPU2ANTIC_SIZE*j]; 
    printf("%3d ",actual_cycles[114]);
    for(i=0;i<=actual_cycles[114]+6;i++){
       printf("%3d, ",cpu_cycles[i]); 
    }
    printf("\n");
  }
  #endif
}

void try_all_scroll(int md,int use_char_index,\
  int use_font,int use_bitmap,int *cpu2antic,int *antic2cpu){   
  char antic_cycles[115];
  int width;
  int scroll_offset=0;
    width=1; /* narrow width without scroll*/
    antic_steal_map(width,md,scroll_offset,use_char_index,use_font,\
      use_bitmap,antic_cycles,&cpu2antic[CPU2ANTIC_SIZE*0],&antic2cpu[CPU2ANTIC_SIZE*0]);
    width=2; /* standard without scroll or narrow with scroll*/
    for(scroll_offset=0;scroll_offset<=7;scroll_offset++){
      antic_steal_map(width,md,scroll_offset,use_char_index,use_font,\
        use_bitmap,antic_cycles,&cpu2antic[CPU2ANTIC_SIZE*(1+scroll_offset)],&antic2cpu[CPU2ANTIC_SIZE*(1+scroll_offset)]); 
    }
    width=3; /* standard with scroll or wide */
    for(scroll_offset=0;scroll_offset<=7;scroll_offset++){
      antic_steal_map(width,md,scroll_offset,use_char_index,use_font,\
        use_bitmap,antic_cycles,&cpu2antic[CPU2ANTIC_SIZE*(9+scroll_offset)],&antic2cpu[CPU2ANTIC_SIZE*(9+scroll_offset)]); 
    }
}

void antic_steal_map(int width,int md,int scroll_offset,int use_char_index,\
  int use_font, int use_bitmap,char *antic_cycles,int *cpu_cycles,\
  int *actual_cycles){
  
  int char_start;
  int bitmap_start;
  int font_start;
  int i;
  int dram_pending;
  int interval;
  int steal;
  int max_chars;
  /* defaults for wide playfield */
#define CHAR_C 13 
#define BITMAP_C (CHAR_C+2)
#define FONT_C (CHAR_C+3)
#define END_C (CHAR_C+95) 
#define DMARS_C (CHAR_C+15)
#define DMARE_C (DMARS_C+32)
  char_start=CHAR_C+scroll_offset;
  bitmap_start=BITMAP_C+scroll_offset;
  font_start=FONT_C+scroll_offset;  
  max_chars=48;
  if(width==2){ /* standard width */
    char_start+=8;
    bitmap_start+=8;
    font_start+=8;
    max_chars=40;
  }else if(width==1){ /* narrow */
    char_start+=16;
    bitmap_start+=16;
    font_start+=16;
    max_chars=32;
  }

  interval=(2<<md);
  max_chars=(max_chars>>md); 
  for(i=0;i<=113;i++){
    antic_cycles[i]='.';
  }
  antic_cycles[114]='\0';
  antic_cycles[0]='M';
  antic_cycles[1]=antic_cycles[6]=antic_cycles[7]='D';
  antic_cycles[2]=antic_cycles[3]=antic_cycles[4]=antic_cycles[5]='P';
  dram_pending=0;
  for(i=0;i<=114;i++){
    steal=0;
    if(i<=END_C){
      if(use_char_index && i>=char_start && ((i-char_start)%interval == 0) \
        && ((i-char_start)<max_chars*interval)){
        steal='I';
      }
      if(use_font && i>=font_start && ((i-font_start)%interval == 0)\
        && ((i-font_start)<max_chars*interval)){
        steal='F';
      }
      if(use_bitmap && i>=bitmap_start && ((i-bitmap_start)%interval == 0)\
        && ((i-bitmap_start)<max_chars*interval)){
        steal='S';
      }
      if(i>=DMARS_C && i<=DMARE_C && ((i-DMARS_C)%4 ==0)){
        dram_pending=1;
      }
    }
    if(steal==0 && dram_pending==1){
      antic_cycles[i]='R';
      dram_pending=0;
    }else if(steal!=0){
      antic_cycles[i]=steal;
    }
  }
  cpu_cycle_map(antic_cycles,cpu_cycles,actual_cycles);
} 
