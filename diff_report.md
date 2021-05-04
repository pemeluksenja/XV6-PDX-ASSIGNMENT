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
