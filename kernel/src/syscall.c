#include "klib.h"
#include "cte.h"
#include "sysnum.h"
#include "vme.h"
#include "serial.h"
#include "loader.h"
#include "proc.h"
#include "timer.h"
#include "file.h"

typedef int (*syshandle_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

extern void *syscall_handle[NR_SYS];

void do_syscall(Context *ctx) {
  // TODO: Lab1-5 call specific syscall handle and set ctx register
  int sysnum = ctx->eax;
  uint32_t arg1 = ctx->ebx;
  uint32_t arg2 = ctx->ecx;
  uint32_t arg3 = ctx->edx;
  uint32_t arg4 = ctx->esi;
  uint32_t arg5 = ctx->edi;
  int res;
  if (sysnum < 0 || sysnum >= NR_SYS) {
    res = -1;
  } else {
    res = ((syshandle_t)(syscall_handle[sysnum]))(arg1, arg2, arg3, arg4, arg5);
	//printf("%d\n", res);
  }
  ctx->eax = res;
  //printf("%x\n", ctx->eip);
}

int sys_write(int fd, const void *buf, size_t count) {
  // TODO: rewrite me at Lab3-1
  return serial_write(buf, count);
}

int sys_read(int fd, void *buf, size_t count) {
  // TODO: rewrite me at Lab3-1
  return serial_read(buf, count);
}

int sys_brk(void *addr) {
  // TODO: Lab1-5
  //static size_t brk = 0; // use brk of proc instead of this in Lab2-1
  proc_t *cur_proc = proc_curr();
  size_t new_brk = PAGE_UP(addr);
  if (cur_proc->brk == 0) {
    cur_proc->brk = new_brk;
  } else if (new_brk > cur_proc->brk) {
    //TODO();
	vm_map(vm_curr(), cur_proc->brk, new_brk - cur_proc->brk, 7);
	cur_proc->brk = new_brk;
  } else if (new_brk < cur_proc->brk) {
    // can just do nothing
	vm_unmap(vm_curr(), new_brk, cur_proc->brk - new_brk);
	cur_proc->brk = new_brk;
  }
  return 0;
}

void sys_sleep(int ticks) {
  //TODO(); // Lab1-7
  uint32_t beginticks = get_tick();
  while (1) {
	uint32_t nowticks = get_tick();
	if (nowticks - beginticks >= ticks) break;
	else {
		//sti(); hlt(); cli();
		proc_yield();
	}
  }
}

int sys_exec(const char *path, char *const argv[]) {
  //TODO(); // Lab1-8, Lab2-1
  //printf("here!\n");
  PD *newpg = vm_alloc();
  Context ctx;
  //printf("here\n");
  if (load_user(newpg, &ctx, path, argv) != 0) {
	//printf("here\n");
	//printf("%p\n", vm_curr());
	vm_teardown(newpg);
	//printf("here\n");
	return -1;
  }
  PD *oldpg = vm_curr();
  proc_t *cur_proc = proc_curr();
  cur_proc->pgdir = newpg;
  set_cr3(newpg);
  vm_teardown(oldpg);
  irq_iret(&ctx);
}

int sys_getpid() {
  //TODO(); // Lab2-1
  proc_t *cur_proc = proc_curr();
  return cur_proc->pid;
}

void sys_yield() {
  proc_yield();
}

int sys_fork() {
  //TODO(); // Lab2-2
  proc_t *new_proc = proc_alloc();
  //printf("syscall %p\n", new_proc->ctx);
  if (new_proc == NULL) return -1;
  proc_copycurr(new_proc);
  proc_addready(new_proc);
  return new_proc->pid;
}

void sys_exit(int status) {
  //TODO(); // Lab2-3
  proc_t *cur_proc = proc_curr();
  proc_makezombie(cur_proc, status);
  for (int i = 0; i < MAX_USEM; i++)
	if (cur_proc->usems[i] != NULL)
		usem_close(cur_proc->usems[i]);
  INT(0x81);
  assert(0);
}

int sys_wait(int *status) {
  //TODO(); // Lab2-3, Lab2-4
  proc_t *cur_proc = proc_curr();
  if (cur_proc->child_num == 0) return -1;
  proc_t *zchild;
  sem_p(&(cur_proc->zombie_sem));
  assert((zchild = proc_findzombie(cur_proc)) != NULL);
  if (status != NULL) *status = zchild->exit_code;
  int p = zchild->pid;
  proc_free(zchild);
  cur_proc->child_num--;
  return p;
}

int sys_sem_open(int value) {
  //TODO(); // Lab2-5
  proc_t *cur_proc = proc_curr();
  int i = proc_allocusem(cur_proc);
  if (i == -1) return -1;
  usem_t *new_usem = usem_alloc(value);
  if (new_usem == NULL) return -1;
  cur_proc->usems[i] = new_usem;
  return i;
}

int sys_sem_p(int sem_id) {
  //TODO(); // Lab2-5
  proc_t *cur_proc = proc_curr();
  usem_t *now_usem = proc_getusem(cur_proc, sem_id);
  if (now_usem == NULL) return -1;
  sem_p(&now_usem->sem);
  return 0;
}

int sys_sem_v(int sem_id) {
  //TODO(); // Lab2-5
  proc_t *cur_proc = proc_curr();
  usem_t *now_usem = proc_getusem(cur_proc, sem_id);
  if (now_usem == NULL) return -1;
  sem_v(&now_usem->sem);
  return 0;
}

int sys_sem_close(int sem_id) {
  //TODO(); // Lab2-5
  proc_t *cur_proc = proc_curr();
  usem_t *now_usem = proc_getusem(cur_proc, sem_id);
  if (now_usem == NULL) return -1;
  cur_proc->usems[sem_id] = NULL;
  usem_close(now_usem);
  return 0;
}

int sys_open(const char *path, int mode) {
  TODO(); // Lab3-1
}

int sys_close(int fd) {
  TODO(); // Lab3-1
}

int sys_dup(int fd) {
  TODO(); // Lab3-1
}

uint32_t sys_lseek(int fd, uint32_t off, int whence) {
  TODO(); // Lab3-1
}

int sys_fstat(int fd, struct stat *st) {
  TODO(); // Lab3-1
}

int sys_chdir(const char *path) {
  TODO(); // Lab3-2
}

int sys_unlink(const char *path) {
  return iremove(path);
}

// optional syscall

void *sys_mmap() {
  TODO();
}

void sys_munmap(void *addr) {
  TODO();
}

int sys_clone(void (*entry)(void*), void *stack, void *arg) {
  TODO();
}

int sys_kill(int pid) {
  TODO();
}

int sys_cv_open() {
  TODO();
}

int sys_cv_wait(int cv_id, int sem_id) {
  TODO();
}

int sys_cv_sig(int cv_id) {
  TODO();
}

int sys_cv_sigall(int cv_id) {
  TODO();
}

int sys_cv_close(int cv_id) {
  TODO();
}

int sys_pipe(int fd[2]) {
  TODO();
}

int sys_link(const char *oldpath, const char *newpath) {
  TODO();
}

int sys_symlink(const char *oldpath, const char *newpath) {
  TODO();
}

void *syscall_handle[NR_SYS] = {
  [SYS_write] = sys_write,
  [SYS_read] = sys_read,
  [SYS_brk] = sys_brk,
  [SYS_sleep] = sys_sleep,
  [SYS_exec] = sys_exec,
  [SYS_getpid] = sys_getpid,
  [SYS_yield] = sys_yield,
  [SYS_fork] = sys_fork,
  [SYS_exit] = sys_exit,
  [SYS_wait] = sys_wait,
  [SYS_sem_open] = sys_sem_open,
  [SYS_sem_p] = sys_sem_p,
  [SYS_sem_v] = sys_sem_v,
  [SYS_sem_close] = sys_sem_close,
  [SYS_open] = sys_open,
  [SYS_close] = sys_close,
  [SYS_dup] = sys_dup,
  [SYS_lseek] = sys_lseek,
  [SYS_fstat] = sys_fstat,
  [SYS_chdir] = sys_chdir,
  [SYS_unlink] = sys_unlink,
  [SYS_mmap] = sys_mmap,
  [SYS_munmap] = sys_munmap,
  [SYS_clone] = sys_clone,
  [SYS_kill] = sys_kill,
  [SYS_cv_open] = sys_cv_open,
  [SYS_cv_wait] = sys_cv_wait,
  [SYS_cv_sig] = sys_cv_sig,
  [SYS_cv_sigall] = sys_cv_sigall,
  [SYS_cv_close] = sys_cv_close,
  [SYS_pipe] = sys_pipe,
  [SYS_link] = sys_link,
  [SYS_symlink] = sys_symlink};
