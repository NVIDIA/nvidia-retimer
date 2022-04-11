/***    Copyright 2022 Nvidia               	                **
 ***                                                            **
 ***            All Rights Reserved.                            **
 ***                                                            **
 *****************************************************************
 *****************************************************************
 ******************************************************************
 *
 * compmngrretimer.h
 *
 *  2022.03.06
 ******************************************************************/
#ifndef COMPMNGRRETIMER_H_
#define COMPMNGRRETIMER_H_

#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <stdint.h>

#define MAXFILESIZE 0x64

#define INT32U uint32_t


typedef struct
{
    unsigned short program_mode_time;
    unsigned short write_time;
    unsigned short block_size;
} FirmwareDownloadPara_T;


int file_lock_write(int fd)
{
    /* Lock the file from beginning to end, blocking if it is already locked */
    struct flock lock, savelock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 50;
    savelock = lock;
    fcntl(fd, F_GETLK, &lock); /* Overwrites lock structure with preventors. */
    if (lock.l_type == F_WRLCK)
    {
        printf("Process %i has a write lock already!\n", lock.l_pid);
        exit(1);
    }
    return fcntl(fd, F_SETLK, &savelock);
}

int file_lock_read(int fd)
{
    /* Lock the file from beginning to end, blocking if it is already locked */
    struct flock lock, savelock;
    lock.l_type = F_WRLCK; /* Test for any lock on any part of file. */
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    savelock = lock;
    fcntl(fd, F_GETLK, &lock); /* Overwrites lock structure with preventors. */
    if (lock.l_type == F_WRLCK)
    {
        printf("Process %i has a write lock already!\n", lock.l_pid);
        exit(1);
    }
    else if (lock.l_type == F_RDLCK)
    {
        printf("Process %i has a read lock already!\n", lock.l_pid);
        exit(1);
    }
    return fcntl(fd, F_SETLK, &savelock);
}

int file_unlock(int fd)
{

    struct flock lock;
    lock.l_type = F_UNLCK; /* Test for any lock on any part of file. */
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;

    fcntl(fd, F_GETLK, &lock); /* Overwrites lock structure with preventors. */
    if (lock.l_type == F_WRLCK || lock.l_type == F_RDLCK)
    {
        printf("Process %i has a write lock already!\n", lock.l_pid);
        return -1;
    }
    else
    {
        printf("Process %i has been Unlocked !\n", lock.l_pid);
        return 0;
    }
}

#endif /* COMPMNGRRETIMER_H_ */
