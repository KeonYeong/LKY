#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

int
exec(char *path, char **argv)
{
  if(proc->isThread) orphanMake();
  char *s, *last;
  int i, off;
  //int j, pgno;
  //int tmp = 0;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;
  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);
  pgdir = 0;

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pgdir = setupkvm()) == 0)
    goto bad;

  // Load program into memory.
  sz = 0;
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD){
      continue;}
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    /*if(proc->isThread){
        for(j=0; j < 64; j ++){
            if(proc->parent->spaceValid[j] == proc->threadID + 1){
                deallocuvm(proc->pgdir, proc->parent->sz + PGSIZE * (j+1), proc->parent->sz + PGSIZE * j);
                proc->parent->spaceValid[j] = 0;
            }
        }
        pgno = (ph.vaddr + ph.memsz) / PGSIZE;
        if(pgno == 0 && ((ph.vaddr + ph.memsz) % PGSIZE)!= 0) pgno = 1;
        cprintf("pgno:%d\n", pgno);
        for(j = 0 ; j < 64; j ++ ){
            if(proc->parent->spaceValid[j] == 0){
                proc->parent->spaceValid[j] = proc->threadID + 1;
                proc->sz = allocuvm(proc->pgdir, proc->parent->sz + PGSIZE * j, proc->parent->sz + PGSIZE * (j+1));
                tmp++;
                if(tmp == pgno) break;}
        }
    }*/
    //else {
    if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
   // }
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    //if(proc->isThread)pgdir = proc->pgdir;
    if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  end_op();
  ip = 0;

  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  /*if(proc->isThread){
      for(j = 0 ; j < 64; j ++ ){
            if(proc->parent->spaceValid[j] == 0){
                proc->parent->spaceValid[j] = proc->threadID + 1;
                proc->sz = allocuvm(proc->pgdir, proc->parent->sz + PGSIZE * j, proc->parent->sz + PGSIZE * (j+1));
      }
      }
      sp = proc->sz;
  }*/
  //else{
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
  sp = sz;
  //}

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    //if(proc->isThread)pgdir = proc->pgdir;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;
  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  //if(proc->isThread) pgdir = proc->pgdir;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;
  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(proc->name, last, sizeof(proc->name));
  // Commit to the user image.
  oldpgdir = proc->pgdir;
  proc->pgdir = pgdir;
  proc->sz = sz;
  proc->tf->eip = elf.entry;  // main
  proc->tf->esp = sp;
  switchuvm(proc);
  if(!proc->isThread){
      freevm(oldpgdir);
  }
  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}
