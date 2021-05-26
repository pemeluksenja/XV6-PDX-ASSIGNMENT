# Makefile
```diff
diff --git a/Makefile b/Makefile
index 6483959..b60e434 100644
--- a/Makefile
+++ b/Makefile
@@ -1,6 +1,6 @@
 # Set flag to correct CS333 project number: 1, 2, ...
 # 0 == original xv6-pdx distribution functionality
-CS333_PROJECT ?= 0
+CS333_PROJECT ?= 2
 PRINT_SYSCALLS ?= 0
 CS333_CFLAGS ?= -DPDX_XV6
 ifeq ($(CS333_CFLAGS), -DPDX_XV6)
@@ -13,7 +13,7 @@ endif

 ifeq ($(CS333_PROJECT), 1)
 CS333_CFLAGS += -DCS333_P1
-CS333_UPROGS += #_date
+CS333_UPROGS += _date
 endif

 ifeq ($(CS333_PROJECT), 2)
@@ -229,7 +229,8 @@ UPROGS=\
        _usertests\
        _wc\
        _zombie\
-
+       _ps\
+       
 UPROGS += $(CS333_UPROGS) $(CS333_TPROGS)

 fs.img: mkfs README $(UPROGS)
@@ -312,7 +313,7 @@ qemu-nox-gdb: fs.img xv6.img .gdbinit
 EXTRA=\
        mkfs.c ulib.c user.h cat.c echo.c forktest.c grep.c kill.c\
        ln.c ls.c mkdir.c rm.c stressfs.c usertests.c wc.c zombie.c\
-       printf.c umalloc.c Makefile \
+       printf.c umalloc.c ps.c Makefile \
        README dot-bochsrc *.pl toc.* runoff runoff1 runoff.list\
        .gdbinit.tmpl gdbutil kernel.ld README-PDX\
```
# UID, GID, PPID

## usys.S

```diff
diff --git a/usys.S b/usys.S
index 0d4eaed..7c0ca74 100644
--- a/usys.S
+++ b/usys.S
@@ -30,3 +30,10 @@ SYSCALL(sbrk)
 SYSCALL(sleep)
 SYSCALL(uptime)
 SYSCALL(halt)
+SYSCALL(getuid)
+SYSCALL(getgid)
+SYSCALL(getppid)
+SYSCALL(setuid)
+SYSCALL(setgid)
```

## syscall.h

```diff
diff --git a/syscall.h b/syscall.h
index 7fc8ce1..8b0101c 100644
--- a/syscall.h
+++ b/syscall.h
@@ -22,3 +22,10 @@
 #define SYS_close   SYS_mkdir+1
 #define SYS_halt    SYS_close+1
 // student system calls begin here. Follow the existing pattern.
+#define SYS_getuid  SYS_date+1
+#define SYS_getgid  SYS_getuid+1
+#define SYS_getppid SYS_getgid+1
+#define SYS_setuid  SYS_getppid+1
+#define SYS_setgid  SYS_setuid+1
```

## syscall.c

```diff
diff --git a/syscall.c b/syscall.c
index 9105b52..81e90ab 100644
--- a/syscall.c
+++ b/syscall.c
@@ -103,6 +103,17 @@ extern int sys_unlink(void);
#ifdef CS333_P1
extern int sys_date(void);
#endif
+#ifdef CS333_P2
+extern int sys_getuid(void);
+extern int sys_getgid(void);
+extern int sys_getppid(void);
+extern int sys_setuid(void);
+extern int sys_setgid(void); 
+#endif
 #ifdef PDX_XV6
 extern int sys_halt(void);
 #endif // PDX_XV6
@@ -132,6 +143,18 @@ static int (*syscalls[])(void) = {
#ifdef CS333_P1
[SYS_date]    sys_date,
#endif
+#ifdef CS333_P2
+[SYS_getuid]  sys_getuid,
+[SYS_getgid]  sys_getgid,
+[SYS_getppid] sys_getppid,
+[SYS_setuid]  sys_setuid,
+[SYS_setgid]  sys_setgid,  
+#endif
+
 };
 #ifdef PRINT_SYSCALLS
@@ -157,12 +180,26 @@ static char *syscallnames[] = {
#ifdef CS333_P1
  [SYS_date]    "date",
#endif
+#ifdef CS333_P2
+  [SYS_getuid]    "getuid",
+  [SYS_getgid]    "getgid",
+  [SYS_getppid]   "getppid",
+  [SYS_setuid]    "setuid",
+  [SYS_setgid]    "setgid",
+#endif
 #ifdef PDX_XV6
   [SYS_halt]    "halt",
 #endif // PDX_XV6
 };
 #endif // PRINT_SYSCALLS
```

## sysproc.c

```diff
diff --git a/sysproc.c b/sysproc.c
index 98563ea..d782ece 100644
--- a/sysproc.c
+++ b/sysproc.c
@@ -9,6 +9,9 @@
 #ifdef PDX_XV6
 #include "pdx-kernel.h"
 #endif // PDX_XV6
+#ifdef CS333_P2
+#include "uproc.h"
+#endif
 
 int
 sys_fork(void)
@@ -88,6 +91,70 @@ sys_uptime(void)
   return xticks;
 }
 
+#ifdef CS333_P2
+int
+sys_getuid(void){
+  return myproc()->uid;
+}
+int 
+sys_getgid(void){
+  return myproc()->gid;
+}
+int
+sys_getppid(void){
+  if (myproc()->pid == 1)
+   return myproc()->pid;
+  return myproc()->parent->pid;
+}
+int 
+sys_setuid(void){
+  int cont;
+  
+  if(argint(0, &cont) < 0 || cont < 0 || cont > 32767)
+    return -1;
+  
+  myproc()->uid = (uint)cont;
+  return 0;
+}
+int 
+sys_setgid(void){
+  int cont;
+  
+  if(argint(0, &cont) < 0 || cont < 0 || cont > 32767)
+    return -1;
+  
+  myproc()->gid = (uint)cont;
+  return 0;
+}
+int
+sys_getprocs(void){
+  int max;
+  struct uproc *table;
+
+  if(argint(0, &max) < 0){
+    return -1;
+  }
+  if (argptr(1, (void*)&table, sizeof(struct uproc)*max) < 0){
+    return -1;
+  }
+
+  return getprocs(max, table);
+}
+#endif
+
```

## user.h

```diff
diff --git a/user.h b/user.h
index 31d9134..a4f4329 100644
--- a/user.h
+++ b/user.h
@@ -26,6 +26,7 @@ int sleep(int);
 int uptime(void);
 int halt(void);
 
+
 // ulib.c
 int stat(char*, struct stat*);
 char* strcpy(char*, char*);
@@ -39,6 +40,18 @@ void* memset(void*, int, uint);
 void* malloc(uint);
 void free(void*);
 int atoi(const char*);
#ifdef CS333_P1
int date(struct rtcdate*);
#endif
+#ifdef CS333_P2
+uint getuid(void);
+uint getgid(void);
+uint getppid(void);
+int setuid(uint);
+int setgid(uint);
+#endif
+
```

## proc.h

```diff
diff --git a/proc.h b/proc.h
index 0a0b4c5..3811f7d 100644
--- a/proc.h
+++ b/proc.h
@@ -41,6 +41,8 @@ struct proc {
   char *kstack;                // Bottom of kernel stack for this process
   enum procstate state;        // Process state
   uint pid;                    // Process ID
+  uint uid;
+  uint gid;
   struct proc *parent;         // Parent process. NULL indicates no parent
   struct trapframe *tf;        // Trap frame for current syscall
   struct context *context;     // swtch() here to run process
@@ -49,6 +51,9 @@ struct proc {
   struct file *ofile[NOFILE];  // Open files
   struct inode *cwd;           // Current directory
   char name[16];               // Process name (debugging)
```

## proc.c

```diff
diff --git a/proc.c b/proc.c
index d030537..41d7e21 100644
--- a/proc.c
+++ b/proc.c
@@ -176,6 +182,11 @@ userinit(void)
   p->tf->esp = PGSIZE;
   p->tf->eip = 0;  // beginning of initcode.S

+#ifdef CS333_P2
+  p->uid = DEFAULT_UID;
+  p->gid = DEFAULT_GID;
+#endif
+
   safestrcpy(p->name, "initcode", sizeof(p->name));
   p->cwd = namei("/");

@@ -247,6 +258,8 @@ fork(void)
   safestrcpy(np->name, curproc->name, sizeof(curproc->name));

   pid = np->pid;
+  np-> uid = curproc-> uid;
+  np-> gid = curproc-> gid;

   acquire(&ptable.lock);
   np->state = RUNNABLE;
```


# Process Execution Time

## proc.c
```diff
diff --git a/proc.c b/proc.c
index d030537..41d7e21 100644
--- a/proc.c
+++ b/proc.c

@@ -148,7 +152,9 @@ allocproc(void)
   p->context = (struct context*)sp;
   memset(p->context, 0, sizeof *p->context);
   p->context->eip = (uint)forkret;
-
+  p->start_ticks = ticks;
+  p->cpu_ticks_in = 0;
+  p->cpu_ticks_total= 0;
   return p;
 }
@@ -389,6 +402,7 @@ scheduler(void)
       c->proc = p;
       switchuvm(p);
       p->state = RUNNING;
+      p->cpu_ticks_in = ticks;
       swtch(&(c->scheduler), p->context);
       switchkvm();

@@ -429,6 +443,7 @@ sched(void)
   if(readeflags()&FL_IF)
     panic("sched interruptible");
   intena = mycpu()->intena;
+  p->cpu_ticks_total += ticks - (p->cpu_ticks_in);
   swtch(&p->context, mycpu()->scheduler);
   mycpu()->intena = intena;
 }
```

## proc.h
```diff
@@ -49,6 +51,9 @@ struct proc {
   struct file *ofile[NOFILE];  // Open files
   struct inode *cwd;           // Current directory
   char name[16];               // Process name (debugging)
   uint start_ticks;
+  uint cpu_ticks_total;
+  uint cpu_ticks_in;
 };
```

# The ps Command

## usys.S
```diff
diff --git a/usys.S b/usys.S
index 0d4eaed..7c0ca74 100644
--- a/usys.S
+++ b/usys.S
@@ -30,3 +30,10 @@ SYSCALL(sbrk)
 SYSCALL(sleep)
 SYSCALL(uptime)
 SYSCALL(halt)
SYSCALL(date)
SYSCALL(getuid)
SYSCALL(getgid)
SYSCALL(getppid)
SYSCALL(setuid)
SYSCALL(setgid)
+SYSCALL(getprocs)
```

## syscall.h
```diff
diff --git a/syscall.h b/syscall.h
index 7fc8ce1..8b0101c 100644
--- a/syscall.h
+++ b/syscall.h
@@ -22,3 +22,10 @@
 #define SYS_close   SYS_mkdir+1
 #define SYS_halt    SYS_close+1
 // student system calls begin here. Follow the existing pattern.
#define SYS_date    SYS_halt+1 
#define SYS_getuid  SYS_date+1
#define SYS_getgid  SYS_getuid+1
#define SYS_getppid SYS_getgid+1
#define SYS_setuid  SYS_getppid+1
#define SYS_setgid  SYS_setuid+1
+#define SYS_getprocs SYS_setgid+1
```

## syscall.c
```diff
diff --git a/syscall.c b/syscall.c
index 9105b52..81e90ab 100644
--- a/syscall.c
+++ b/syscall.c
@@ -103,6 +103,17 @@ extern int sys_unlink(void);
 extern int sys_wait(void);
 extern int sys_write(void);
 extern int sys_uptime(void);
#ifdef CS333_P1
extern int sys_date(void);
#endif
#ifdef CS333_P2
extern int sys_getuid(void);
extern int sys_getgid(void);
extern int sys_getppid(void);
extern int sys_setuid(void);
extern int sys_setgid(void);
+extern int sys_getprocs(void); 
#endif
 #ifdef PDX_XV6
 extern int sys_halt(void);
 #endif // PDX_XV6
@@ -132,6 +143,18 @@ static int (*syscalls[])(void) = {
 #ifdef PDX_XV6
 [SYS_halt]    sys_halt,
 #endif // PDX_XV6
#ifdef CS333_P1
[SYS_date]    sys_date,
#endif
#ifdef CS333_P2
[SYS_getuid]  sys_getuid,
[SYS_getgid]  sys_getgid,
[SYS_getppid] sys_getppid,
[SYS_setuid]  sys_setuid,
[SYS_setgid]  sys_setgid,  
+[SYS_getprocs] sys_getprocs,
#endif

 };
 
 #ifdef PRINT_SYSCALLS
@@ -157,12 +180,26 @@ static char *syscallnames[] = {
   [SYS_link]    "link",
   [SYS_mkdir]   "mkdir",
   [SYS_close]   "close",
#ifdef CS333_P1
  [SYS_date]    "date",
#endif
#ifdef CS333_P2
  [SYS_getuid]    "getuid",
  [SYS_getgid]    "getgid",
  [SYS_getppid]   "getppid",
  [SYS_setuid]    "setuid",
  [SYS_setgid]    "setgid",
+  [SYS_getprocs]  "getprocs",
#endif
 #ifdef PDX_XV6
   [SYS_halt]    "halt",
 #endif // PDX_XV6
 };
 #endif // PRINT_SYSCALLS
```

## sysproc.c
```diff
diff --git a/sysproc.c b/sysproc.c
index 98563ea..d782ece 100644
--- a/sysproc.c
+++ b/sysproc.c
@@ -9,6 +9,9 @@
 #ifdef PDX_XV6
 #include "pdx-kernel.h"
 #endif // PDX_XV6
+#ifdef CS333_P2
+#include "uproc.h"
+#endif
 
 int
 sys_fork(void)
@@ -88,6 +91,70 @@ sys_uptime(void)
   return xticks;
 }
 
#ifdef CS333_P2
+int
+sys_getprocs(void){
+  int max;
+  struct uproc *table;
+
+  if(argint(0, &max) < 0){
+    return -1;
+  }
+  if (argptr(1, (void*)&table, sizeof(struct uproc)*max) < 0){
+    return -1;
+  }
+
+  return getprocs(max, table);
+}
#endif
```

## proc.c

```diff
diff --git a/proc.c b/proc.c
index d030537..41d7e21 100644
--- a/proc.c
+++ b/proc.c
@@ -6,6 +6,10 @@
 #include "x86.h"
 #include "proc.h"
 #include "spinlock.h"
+#ifdef CS333_P2
+#include "pdx.h",
+#include "uproc.h"
+#endif

@@ -556,14 +571,102 @@ kill(int pid)
int
+int
+getprocs(uint max, struct uproc * table)
+{
+  sti(); // enable interupts
+  struct proc *p;
+  int hitung = 0;
+ 
+  acquire(&ptable.lock);
+
+  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
+  {
+    if(hitung >= max)
+    {
+      break;
+    }
+   else
+    {
+      if(p->state != EMBRYO && p->state != UNUSED){
+        table[hitung].pid = p->pid;
+        table[hitung].uid = p->uid;
+        table[hitung].gid = p->gid;
+        table[hitung].ppid = p->parent ? p->parent->pid : p->pid;
+        table[hitung].elapsed_ticks = (ticks-p->start_ticks);
+        table[hitung].CPU_total_ticks = p->cpu_ticks_total;
+        safestrcpy(table[hitung].state, states[p->state], STRMAX);
+        table[hitung].size = p->sz;
+        safestrcpy(table[hitung].name, p->name, STRMAX);
+        hitung++;
+      }
+    }
+ }
+
+  release(&ptable.lock);
+ 
+  if(hitung == 0)
+   return -1;
+
+ return hitung;
+}
```

## user.h
```diff
diff --git a/user.h b/user.h
index 31d9134..a4f4329 100644
--- a/user.h
+++ b/user.h
@@ -26,6 +26,7 @@ int sleep(int);
 int uptime(void);
 int halt(void);
 
+
 // ulib.c
 int stat(char*, struct stat*);
 char* strcpy(char*, char*);
@@ -39,6 +40,18 @@ void* memset(void*, int, uint);
 void* malloc(uint);
 void free(void*);
 int atoi(const char*);
#ifdef CS333_P2
uint getuid(void);
uint getgid(void);
uint getppid(void);
int setuid(uint);
int setgid(uint);
+int getprocs(uint max, struct uproc*);
#endif

 #ifdef PDX_XV6
 int atoo(const char*);
 int strncmp(const char*, const char*, uint);

```

## defs.h
```diff
diff --git a/defs.h b/defs.h
index f85557d..7e92829 100644
--- a/defs.h
+++ b/defs.h
@@ -9,6 +9,9 @@ struct spinlock;
 struct sleeplock;
 struct stat;
 struct superblock;
+#ifdef CS333_P2
+struct uproc;
+#endif

 // bio.c
 void            binit(void);
@@ -124,6 +127,9 @@ void            userinit(void);
 int             wait(void);
 void            wakeup(void*);
 void            yield(void);
+#ifdef CS333_P2
+int             getprocs(uint max, struct uproc*);
+#endif
 #ifdef CS333_P3
 void            printFreeList(void);
 void            printList(int);

```
## ps.c
```diff
diff --git a/ps.c b/ps.c
new file mode 100644
index 0000000..322b8b4
--- /dev/null
+++ b/ps.c
@@ -0,0 +1,73 @@
+#include "types.h"
+#include "stat.h"
+#include "user.h"
+#include "fcntl.h"
+#include "uproc.h"
+
+int
+main(int argc, char * argv[]){
+  int max = 72;
+
+  if(argc > 1)
+    max = atoi(argv[1]);
+
+  struct uproc * table = malloc(sizeof(*table)*max);
+
+  if(table == 0)
+  {
+    printf(1, "Unable to initialize table in ps.c\n");
+    exit();
+  }
+
+    int process = getprocs(max, table);
+    int ticks_now;
+    int cpu_now;
+    printf(1,"PID\tName\tUID\tGID\tPPID\tElapsed CPU\t State\tSize\n");
+    if(process < 1)
+    {
+        free(table);
+        printf(1, "Error\n");
+    }
+
+     for(int i = 0; i < process; i++)
+  {
+    printf(1, "%d\t%s\t%d\t%d\t%d\t", 
+      table[i].pid, 
+      table[i].name, 
+      table[i].uid, 
+      table[i].gid, 
+      table[i].ppid);
+
+    ticks_now = table[i].elapsed_ticks;
+
+    if(ticks_now <10){
+      printf(1,"0.00%d\t", ticks_now);
+    } else if(ticks_now <100){
+      printf(1,"0.0%d\t", ticks_now);
+    } else if(ticks_now<1000){
+      printf(1, "0.%d\t", ticks_now);
+    } else {
+      int result_div = ticks_now/1000;
+      int result_modulo = ticks_now%1000;
+      printf(1, "%d.%d\t",result_div, result_modulo);
+    };
+
+    cpu_now = table[i].CPU_total_ticks;
+
+    if(cpu_now<10){
+      printf(1, "0.00%d", cpu_now);
+    } else if(cpu_now<100){
+      printf(1, "0.0%d", cpu_now);
+    } else if(cpu_now<1000){
+      printf(1,"0.%d", cpu_now);
+    } else {
+      int result_div = cpu_now/1000;
+      int result_modulo = cpu_now%1000;
+      printf(1, "%d.%d", result_div, result_modulo);
+    }
+    printf(1, "\t%s", table[i].state);
+    printf(1, "\t%d\n", table[i].size);
+  } 
+    free(table);
+    exit();
+}
```

# Time Command

## time.c
```diff
diff --git a/time.c b/time.c
new file mode 100644
index 0000000..4fd280f
--- /dev/null
+++ b/time.c
@@ -0,0 +1,43 @@
+// uptime. How long has xv6 been up
+#include "types.h"
+#include "user.h"
+
+int
+main(int argc, char *argv[])
+{
+  int current_time = uptime();
+
+  if (argc == 1){
+      int leftover = 0;
+      int time_now = uptime() - current_time;
+      if(time_now > 1000){
+        leftover = time_now % 1000;
+        time_now /= 1000;
+    }
+      if(leftover != 0){
+        printf(1, "(null) ran in %d.%d seconds\n", time_now, leftover);
+    } else{
+        printf(1, "(null) ran in 0.%d seconds\n", time_now);
+     }
+    } else {
+        if (fork() == 0){
+            exec(argv[1], &argv[1]);
+        }else{
+            wait();
+             int leftover = 0;
+             int time_now = uptime() - current_time;
+
+            if(time_now > 1000){
+            leftover = time_now % 1000;
+            time_now /= 1000;
+            }
+            if(leftover != 0){
+            printf(1, "%s ran in %d.%d seconds\n", argv[1], time_now, leftover);
+            } else{
+            printf(1, "%s ran in 0.%d seconds\n", argv[1], time_now);
+            }
+        }
+    }
+
+  exit();
+}
```

# Modifying the console

## proc.c
```diff
diff --git a/proc.c b/proc.c
index d030537..41d7e21 100644
--- a/proc.c
+++ b/proc.c
@@ -6,6 +6,10 @@
 #include "x86.h"
 #include "proc.h"
 #include "spinlock.h"
+#ifdef CS333_P2
+#include "pdx.h",
+#include "uproc.h"
+#endif
@@ -556,14 +571,102 @@ kill(int pid)
 void
 procdumpP2P3P4(struct proc *p, char *state_string)
 {
-  cprintf("TODO for Project 2, delete this line and implement procdumpP2P3P4() in proc.c to print a row\n");
+  int uid = p->uid;
+  int gid = p->gid;
+   cprintf("%d\t%s\t\t%d\t%d\t%d", p->pid, p->name, uid, gid);
+  
+  if(p->pid != 1){
+     cprintf("%d\t", p->parent->pid);
+  }
+  else{
+    cprintf("%d\t", p->pid);
+  }
+
+  int leftover = 0;
+  int ticks_now = ticks - (p->start_ticks);
+  if(ticks_now > 1000){
+    leftover = ticks_now % 1000;
+    ticks_now /= 1000;
+  }
+
+  if(leftover != 0){
+    cprintf("%d.%d\t", ticks_now, leftover);
+    
+  }else{
+    cprintf("0.%d\t", ticks_now);
+  }
+
+  int leftover_cpu = 0;
+  int ticks_now_cpu = p->cpu_ticks_total;
+  if(ticks_now_cpu > 1000){
+    leftover_cpu = ticks_now_cpu % 1000;
+    ticks_now_cpu /= 1000;
+  }
+
+  if(leftover_cpu != 0){
+    cprintf("%d.%d\t%s\t%d\t", ticks_now_cpu, leftover_cpu, states[p->state], p->sz);
+  }else{
+    cprintf("0.%d\t%s\t%d\t", ticks_now_cpu, states[p->state], p->sz);
+  }
   return;
 }
+
```
