# Print Syscall

## Makefile
```diff
diff --git a/Makefile b/Makefile
diff --git a/Makefile b/Makefile
index 6483959..3c867c4 100644
--- a/Makefile
+++ b/Makefile
@@ -1,7 +1,7 @@
 # Set flag to correct CS333 project number: 1, 2, ...
 # 0 == original xv6-pdx distribution functionality
 CS333_PROJECT ?= 0
-PRINT_SYSCALLS ?= 0
+PRINT_SYSCALLS ?= 1
 CS333_CFLAGS ?= -DPDX_XV6
 ifeq ($(CS333_CFLAGS), -DPDX_XV6)
 CS333_UPROGS +=        _halt _uptime
 ```
 ## syscall.c
 ```diff
 diff --git a/syscall.c b/syscall.c
index 9105b52..a8de2dc 100644
--- a/syscall.c
+++ b/syscall.c
@@ -172,6 +175,9 @@ syscall(void)
   num = curproc->tf->eax;
   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
     curproc->tf->eax = syscalls[num]();
+    #ifdef PRINT_SYSCALLS
+      cprintf("%s -> %d \n", syscallnames[num], curproc->tf->eax);
+    #endif 
   } else {
     cprintf("%d %s: unknown sys call %d\n",
             curproc->pid, curproc->name, num);
 ```
 
 # Date
 
 ## user.h
 ```diff
 @@ -39,6 +40,12 @@ void* memset(void*, int, uint);
 void* malloc(uint);
 void free(void*);
 int atoi(const char*);
+
+
+#ifdef CS333_P1
+int date(struct rtcdate*);
+#endif
+
 #ifdef PDX_XV6
 int atoo(const char*);
 int strncmp(const char*, const char*, uint);
 ```
## usys.S
```diff
diff --git a/usys.S b/usys.S
index 0d4eaed..e28e4e5 100644
--- a/usys.S
+++ b/usys.S
@@ -30,3 +30,4 @@ SYSCALL(sbrk)
 SYSCALL(sleep)
 SYSCALL(uptime)
 SYSCALL(halt)
+SYSCALL(date)
```

## syscall.h
```diff
diff --git a/syscall.h b/syscall.h
index 7fc8ce1..6fbd22a 100644
--- a/syscall.h
+++ b/syscall.h
@@ -21,4 +21,5 @@
 #define SYS_mkdir   SYS_link+1
 #define SYS_close   SYS_mkdir+1
 #define SYS_halt    SYS_close+1
+#define SYS_date    SYS_halt+1 
```


## syscall.c
```diff
diff --git a/syscall.c b/syscall.c
index 9105b52..1c1c1a1 100644
--- a/syscall.c
+++ b/syscall.c
@@ -103,6 +103,9 @@ extern int sys_unlink(void);
 extern int sys_wait(void);
 extern int sys_write(void);
 extern int sys_uptime(void);
+#ifdef CS333_P1
+extern int sys_date(void);
+#endif
 #ifdef PDX_XV6
 extern int sys_halt(void);
 #endif // PDX_XV6
@@ -129,6 +132,9 @@ static int (*syscalls[])(void) = {
 [SYS_link]    sys_link,
 [SYS_mkdir]   sys_mkdir,
 [SYS_close]   sys_close,
+#ifdef CS333_P1
+[SYS_date]    sys_date,
+#endif
 #ifdef PDX_XV6
 [SYS_halt]    sys_halt,
 #endif // PDX_XV6
@@ -157,12 +163,18 @@ static char *syscallnames[] = {
   [SYS_link]    "link",
   [SYS_mkdir]   "mkdir",
   [SYS_close]   "close",
+#ifdef CS333_P1
+  [SYS_date]    "date",
+#endif
 #ifdef PDX_XV6
   [SYS_halt]    "halt",
 #endif // PDX_XV6
 };
 #endif // PRINT_SYSCALLS

+
+
+
 void
   [SYS_link]    "link",
   [SYS_mkdir]   "mkdir",
   [SYS_close]   "close",
+#ifdef CS333_P1
+  [SYS_date]    "date",
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
index 98563ea..63094e8 100644
--- a/sysproc.c
+++ b/sysproc.c
@@ -88,6 +88,19 @@ sys_uptime(void)
   return xticks;
 }
 
+#ifdef CS333_P1
+int
+sys_date(void)
+{
+  struct rtcdate *d;
+
+  if(argptr(0, (void*)&d, sizeof(struct rtcdate))<0)
+    return -1;
+  cmostime(d);
+    return 0;
+}
+#endif
+
 #ifdef PDX_XV6
 // shutdown QEMU
 int
```

## date.c
```diff
diff --git a/date.c b/date.c
index cff33a2..c1bdd96 100644
--- a/date.c
+++ b/date.c
@@ -6,7 +6,7 @@
 #include "user.h"
 #include "date.h"
 
-#define PAD(x) ((x) < 10 ? "0" : "")
+#define PAD(x) ((x) < 10 ? "" : "")

 static char *months[] = {"NULL", "Jan", "Feb", "Mar", "Apr",
   "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
@@ -23,7 +23,7 @@ int
 main(int argc, char *argv[])
 {
   int day;
-  char *s;
+  // char *s;
   struct rtcdate r;

   if (date(&r)) {
@@ -33,14 +33,13 @@ main(int argc, char *argv[])
   }

   day = dayofweek(r.year, r.month, r.day);
-  s = r.hour < 12 ? "AM" : "PM";
+  // s = r.hour < 12 ? "AM" : "PM";

-  r.hour %= 12;
-  if (r.hour == 0) r.hour = 12;
+  // r.hour %= 12;
+  // if (r.hour == 0) r.hour = 12;

-  printf(1, "%s %s%d %s %d %s%d:%s%d:%s%d %s UTC\n", days[day], PAD(r.day), r.day,
-      months[r.month], r.year, PAD(r.hour), r.hour, PAD(r.minute), r.minute,
-      PAD(r.second), r.second, s);
+  printf(1, "%s %s %s%d %s%d:%s%d:%s%d UTC %d\n", days[day], months[r.month], PAD(r.day), r.day,
+   PAD(r.hour), r.hour, PAD(r.minute), r.minute, PAD(r.second), r.second, r.year);

   exit();
 }
```

# Control-P

## proc.h
```diff
diff --git a/proc.h b/proc.h
index 0a0b4c5..c7ee129 100644
--- a/proc.h
+++ b/proc.h
@@ -49,6 +49,7 @@ struct proc {
   struct file *ofile[NOFILE];  // Open files
   struct inode *cwd;           // Current directory
   char name[16];               // Process name (debugging)
+  uint start_ticks;
 };
```

## proc.c
```diff
diff --git a/proc.c b/proc.c
index d030537..1b2208c 100644
--- a/proc.c
+++ b/proc.c
@@ -148,7 +148,7 @@ allocproc(void)
   p->context = (struct context*)sp;
   memset(p->context, 0, sizeof *p->context);
   p->context->eip = (uint)forkret;
-
+  p->start_ticks = ticks;
   return p;
 }

@@ -563,7 +563,20 @@ procdumpP2P3P4(struct proc *p, char *state_string)
 void
 procdumpP1(struct proc *p, char *state_string)
 {
-  cprintf("TODO for Project 1, delete this line and implement procdumpP1() in proc.c to print a row\n");
+  int ticks_now = ticks - (p->start_ticks);
+  int sisa = 0;
+  if (ticks_now < 1000) {
+    cprintf("%d\t%s\t\t0.%d\t%s\t%d\t", p->pid, p->name, ticks_now, states[p->state], p->sz);
+  }  else {
+  int sisa = ticks_now % 1000;
+  ticks_now = ticks_now / 1000;
+  if (sisa == 0){
+    cprintf("%d\t%s\t\t0.%d\t%s\t%d\t", p->pid, p->name, ticks_now, states[p->state], p->sz);
+  }else{
+      cprintf("%d\t%s\t\t%d.%d\t%s\t%d\t", p->pid, p->name, ticks_now, sisa, states[p->state], p->sz);
+    }
+  }
+
   return;
 }
 #endif
```
