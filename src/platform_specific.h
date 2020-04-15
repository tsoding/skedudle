#ifndef PLATFORM_SPECIFIC_H_
#define PLATFORM_SPECIFIC_H_

// this is quite lame because there is no one definition for all BSDs
// and i don't really wanna assume !linux == BSD

#if __linux__
#include <sys/sendfile.h>

static inline
ssize_t sendfile_wrapper(int out_fd, int in_fd, off_t* offset, size_t count)
{
    return sendfile(out_fd, in_fd, offset, count);
}

#elif (__FreeBSD__ || __NetBSD__ || __OpenBSD__ || __DragonFly__)

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

static inline
ssize_t sendfile_wrapper(int out_fd, int in_fd, off_t* offset, size_t count)
{
    // bsd signature is:
    // sendfile(int fd, int sock, off_t offset, size_t nbytes, struct sf_hdtr*, off_t* sbytes, int flags)

    // we need to do this, because the behaviour of linux sendfile is to read from the
    // current seek location in the file if the offset pointer is NULL. BSD sendfile presumably
    // always uses the offset, and it does not seek in the file after sending either.
    off_t file_offset = 0;

    if(offset != NULL)
        file_offset = *offset;

    else
        file_offset = lseek(in_fd, 0, SEEK_CUR);

    off_t sent_bytes = 0;
    int result = sendfile(in_fd, out_fd, file_offset, count, NULL, &sent_bytes, 0);

    if(result < 0)
        return (ssize_t) result;

    // if the offset pointer was null, then update the file cursor
    // if not, then update the user-given offset pointer.
    if(offset == NULL)
        lseek(in_fd, sent_bytes, SEEK_CUR);

    else
        *offset += sent_bytes;

    return (ssize_t) (file_offset + sent_bytes);
}

#elif (__APPLE__ && __MACH__)


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

// see above for most of the comments. the OSX version is a bit more complicated
// because the number of bytes is a "value-result parameter".

static inline
ssize_t sendfile_wrapper(int out_fd, int in_fd, off_t* offset, size_t count)
{
    // osx signature is:
    // sendfile(int fd, int sock, off_t offset, off_t* len, struct sf_hdtr*, int flags)
    off_t file_offset = 0;

    if(offset != NULL)
        file_offset = *offset;

    else
        file_offset = lseek(in_fd, 0, SEEK_CUR);

    // we set this, because the third parameter is both the (input) number of bytes to send,
    // and the (output) actual number of bytes sent.
    off_t sent_bytes = count;
    int result = sendfile(in_fd, out_fd, file_offset, &sent_bytes, NULL, 0);

    if(result < 0)
        return (ssize_t) result;

    // if the offset pointer was null, then update the file cursor
    // if not, then update the user-given offset pointer.
    if(offset == NULL)
        lseek(in_fd, sent_bytes, SEEK_CUR);

    else
        *offset += sent_bytes;

    return (ssize_t) (file_offset + sent_bytes);
}


#endif

#endif  // PLATFORM_SPECIFIC_H_
