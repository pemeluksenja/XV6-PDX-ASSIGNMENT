#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "uproc.h"

int
main(int argc, char * argv[]){
  int max = 72;

  if(argc > 1)
    max = atoi(argv[1]);

  struct uproc * table = malloc(sizeof(*table)*max);

  if(table == 0)
  {
    printf(1, "Unable to initialize table in ps.c\n");
    exit();
  }

    int process = getprocs(max, table);
    int ticks_now;
    int cpu_now;
    printf(1,"PID\tName\tUID\tGID\tPPID\tElapsed CPU\t State\tSize\n");
    if(process < 1)
    {
        free(table);
        printf(1, "Error\n");
    }

     for(int i = 0; i < process; i++)
  {
    printf(1, "%d\t%s\t%d\t%d\t%d\t", 
      table[i].pid, 
      table[i].name, 
      table[i].uid, 
      table[i].gid, 
      table[i].ppid);

    ticks_now = table[i].elapsed_ticks;

    if(ticks_now <10){
      printf(1,"0.00%d\t", ticks_now);
    } else if(ticks_now <100){
      printf(1,"0.0%d\t", ticks_now);
    } else if(ticks_now<1000){
      printf(1, "0.%d\t", ticks_now);
    } else {
      int result_div = ticks_now/1000;
      int result_modulo = ticks_now%1000;
      printf(1, "%d.%d\t",result_div, result_modulo);
    };

    cpu_now = table[i].CPU_total_ticks;

    if(cpu_now<10){
      printf(1, "0.00%d", cpu_now);
    } else if(cpu_now<100){
      printf(1, "0.0%d", cpu_now);
    } else if(cpu_now<1000){
      printf(1,"0.%d", cpu_now);
    } else {
      int result_div = cpu_now/1000;
      int result_modulo = cpu_now%1000;
      printf(1, "%d.%d", result_div, result_modulo);
    }
    printf(1, "\t%s", table[i].state);
    printf(1, "\t%d\n", table[i].size);
  } 
    free(table);
    exit();
}