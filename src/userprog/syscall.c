#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include <string.h>
#include <stdlib.h>
#include "syscall.h"
#include "threads/synch.h"

// the only global file_lock, that we use to avoid race condition of files
static struct lock file_lock;
static void syscall_handler(struct intr_frame *);
void check_address(const void *ptr);
void exit(int status);
struct opened_file *get_file_descriptor(int fd);
void syscall_exec(struct intr_frame *f); 
void syscall_wait(struct intr_frame *f);
void syscall_create(struct intr_frame *f);
void syscall_remove(struct intr_frame *f);
void syscall_open(struct intr_frame *f);
void syscall_filesize(struct intr_frame *f);
void syscall_read(struct intr_frame *f);
void syscall_write(struct intr_frame *f);
void syscall_seek(struct intr_frame *f);
void syscall_tell(struct intr_frame *f);
void syscall_close(struct intr_frame *f);
void syscall_init(void){
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
    lock_init(&file_lock);
}

void
check_address(const void *t){
    if (t == NULL || !is_user_vaddr(t) || pagedir_get_page(thread_current()->pagedir, t) == NULL)
        exit(-1);
}

static void syscall_handler(struct intr_frame *f) {
    check_address(f->esp);
    int systemCallType = *((int *)(f->esp)); // Fetch system call number from esp
    switch(systemCallType) {
        case SYS_HALT:
            syscall_halt(f);
            break;
        case SYS_EXIT:
        {
            check_address(f->esp + 4);
            int status = *((int *)f->esp + 1);
            exit(status);
        }
            break;
        case SYS_EXEC:
            syscall_exec(f);
            break;
        case SYS_WAIT:
            syscall_wait(f);
            break;
        case SYS_CREATE:
            syscall_create(f);
            break;
        case SYS_REMOVE:
            syscall_remove(f);
            break;
        case SYS_OPEN:
            syscall_open(f);
            break;
        case SYS_FILESIZE:
            syscall_filesize(f);
            break;
        case SYS_READ:
            syscall_read(f);
            break;
        case SYS_WRITE:
            syscall_write(f);
            break;
        case SYS_SEEK:
            syscall_seek(f);
            break;
        case SYS_TELL:
            syscall_tell(f);
            break;
        case SYS_CLOSE:
            syscall_close(f);
            break;
        default:
            thread_exit();
            break;
    }
}

void syscall_halt() {
    shutdown_power_off();
}

void syscall_exec(struct intr_frame *f) {
    check_address(f->esp + 4);
    char *cmd_line = (char *)(*((int *)f->esp + 1));
    if (cmd_line == NULL) {
        exit(-1);
    }
    lock_acquire(&file_lock);
    f->eax = process_execute(cmd_line);
    lock_release(&file_lock);
}

struct opened_file *get_file_descriptor(int fd){
    struct thread *t = thread_current();
    for (struct list_elem *e = list_begin(&t->file_list); e != list_end(&t->file_list);e = list_next(e)){
        struct file_info *opened = list_entry(e, struct file_info, elem);
        if (opened->fd == fd)
            return opened;
    }
    return NULL;
}

void exit(int status) {
    struct thread *cur = thread_current()->parent;
    printf("%s: exit(%d)\n", thread_current()->name, status);
    if (cur)
        cur->child_status = status;
    thread_exit();
}
 void syscall_wait(struct intr_frame *f) {
   check_address(f->esp + 4);
    int pid = (*((int *)f->esp + 1));
    f->eax = process_wait(pid);
}

void syscall_create(struct intr_frame *f) {
    check_address(f->esp + 4); 
    check_address(f->esp + 8);
    char *fileName = (char *)(*((uint32_t *)f->esp + 1));
    unsigned initial_size = *((unsigned *)f->esp + 2);
    if (fileName == NULL) {
        exit(-1);
    }
    lock_acquire(&file_lock);
    f->eax = filesys_create(fileName, initial_size);
    lock_release(&file_lock);
}

 void syscall_remove(struct intr_frame *f) {
    check_address(f->esp + 4);
    char *fileName = (char *)(*((uint32_t *)f->esp + 1));
    if (fileName == NULL) {
        exit(-1);
    }
    lock_acquire(&file_lock);
    f->eax = filesys_remove(fileName);
    lock_release(&file_lock);
}
void syscall_open(struct intr_frame *f) {
    check_address(f->esp + 4);
    char *fileName = (char *)(*((uint32_t *)f->esp + 1));
    if (fileName == NULL) 
        exit(-1);
    struct file_info *fd_elem = palloc_get_page(0);
    if (fd_elem == NULL) {
        f->eax = -1;
        return;
    }
   lock_acquire(&file_lock);
    fd_elem->file = filesys_open(fileName);
    if (fd_elem->file == NULL) {
        lock_release(&file_lock);
        palloc_free_page(fd_elem);
        f->eax = -1; // Return -1 for file open failure
    } else {
        fd_elem->fd = ++thread_current()->fileDirectory;
        list_push_back(&thread_current()->file_list, &fd_elem->elem);
        f->eax = fd_elem->fd;
        lock_release(&file_lock);
    }
}
 void syscall_filesize(struct intr_frame *f) {
    check_address(f->esp + 4);
    int  fd = *((uint32_t *)f->esp + 1);;
    struct file_info *fdObject = get_file_descriptor(fd);
    if (fdObject->file == NULL) {
        f->eax = -1; // Return -1 for invalid file descriptor
    } else {
        lock_acquire(&file_lock);
        f->eax = file_length(fdObject->file);
        lock_release(&file_lock);
    }
}
void syscall_read(struct intr_frame *f) {
    check_address(f->esp + 4);
    check_address(f->esp + 8);
    check_address(f->esp + 12);
    int  fd = *((int *)f->esp + 1);
    void *buffer = (void *)(*((int *)f->esp + 2));
    int size = *((int *)f->esp + 3);
    check_address(buffer+size);
    if (fd == 0) {
        // Reading from stdin (keyboard input)
        for (int i = 0; i < size; i++) {
           lock_acquire(&file_lock);
            ((char*)buffer)[i] = input_getc();
            lock_release(&file_lock);
        }
        f->eax = size;
    } else {
        // Reading from a file
        struct file_info* fileToRead = get_file_descriptor(fd);
        if (fileToRead->file == NULL) {
            f->eax = -1; // Return -1 for invalid file descriptor
        } else {
            lock_acquire(&file_lock);
            f->eax = file_read(fileToRead->file, buffer, size);
            lock_release(&file_lock);
        }
    }
}
void syscall_write(struct intr_frame *f) {
    check_address(f->esp + 4);
    check_address(f->esp + 8);
    check_address(f->esp + 12);
    int  fd = *((int *)f->esp + 1);
    void *buffer = (void *)(*((int *)f->esp + 2));
    int size = *((int *)f->esp + 3);
    if (buffer ==NULL) 
        exit(-1);
    
    if (fd == 1) {
        // Writing to stdout (console)
        lock_acquire(&file_lock);
        putbuf(buffer, size);
        lock_release(&file_lock);
        f->eax = size;
    } else {
        // Writing to a file
        struct file_info *fileTowriteIn = get_file_descriptor(fd);
        if (fileTowriteIn->file == NULL) {
            f->eax = -1; // Return -1 for invalid file descriptor
        } else {
            lock_acquire(&file_lock);
            f->eax = file_write(fileTowriteIn->file, buffer, size);
            lock_release(&file_lock);
        }
    }
}
void syscall_seek(struct intr_frame *f) {
    check_address(f->esp + 4);
    check_address(f->esp + 8);
    int fd = *((uint32_t *)f->esp + 1);
    int position = (*((unsigned *)f->esp + 2));
    struct file_info *fileToSeek = get_file_descriptor(fd);
    if (fileToSeek == NULL) 
        return; // Exit if file descriptor is invalid

    lock_acquire(&file_lock);
    file_seek(fileToSeek->file, position);
    lock_release(&file_lock);
}
void syscall_tell(struct intr_frame *f) {
    check_address(f->esp + 4);
    int fd = *((uint32_t *)f->esp + 1);
    struct file_info *fileToTell = get_file_descriptor(fd);
    if (fileToTell->file == NULL) {
        f->eax = -1; // Return -1 for invalid file descriptor
    } else {
        lock_acquire(&file_lock);
        f->eax = file_tell(fileToTell->file);
        lock_release(&file_lock);
    }
}
void syscall_close(struct intr_frame *f) {
    check_address(f->esp + 4);
    int fd = *((uint32_t *)f->esp + 1);
    struct file_info *fileToClose = get_file_descriptor(fd);
    if (fileToClose->file == NULL)
        exit(-1);

    lock_acquire(&file_lock);
    file_close(fileToClose->file);
    lock_release(&file_lock);
    list_remove(&fileToClose->elem);
    palloc_free_page(fileToClose); 
}