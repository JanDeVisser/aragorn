/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <LibCore/Error.h>

namespace LibCore {

constexpr int ENOERROR = 0;

#define ERRORCODES(S)                                                  \
    S(ENOERROR, "No error")                                            \
    S(EPERM, "Permission")                                             \
    S(ENOENT, "No such file or directory")                             \
    S(ESRCH, "No such process")                                        \
    S(EINTR, "Interrupted system call")                                \
    S(EIO, "Input/output error")                                       \
    S(ENXIO, "Device not configured")                                  \
    S(E2BIG, "Argument list too long")                                 \
    S(ENOEXEC, "Exec format error")                                    \
    S(EBADF, "Bad file descriptor")                                    \
    S(ECHILD, "No child processes")                                    \
    S(EDEADLK, "Resource deadlock avoided")                            \
    S(ENOMEM, "Cannot allocate memory")                                \
    S(EACCES, "Permission denied")                                     \
    S(EFAULT, "Bad address")                                           \
    S(ENOTBLK, "Block device required")                                \
    S(EBUSY, "Device / Resource busy")                                 \
    S(EEXIST, "File exists")                                           \
    S(EXDEV, "Cross-device link")                                      \
    S(ENODEV, "Operation not supported by device")                     \
    S(ENOTDIR, "Not a directory")                                      \
    S(EISDIR, "Is a directory")                                        \
    S(EINVAL, "Invalid argument")                                      \
    S(ENFILE, "Too many open files in system")                         \
    S(EMFILE, "Too many open files")                                   \
    S(ENOTTY, "Inappropriate ioctl for device")                        \
    S(ETXTBSY, "Text file busy")                                       \
    S(EFBIG, "File too large")                                         \
    S(ENOSPC, "No space left on device")                               \
    S(ESPIPE, "Illegal seek")                                          \
    S(EROFS, "Read-only file system")                                  \
    S(EMLINK, "Too many links")                                        \
    S(EPIPE, "Broken pipe")                                            \
    S(EDOM, "Numerical argument out of domain")                        \
    S(ERANGE, "Result too large")                                      \
    S(EAGAIN, "Resource temporarily unavailable")                      \
    S(EINPROGRESS, "Operation now in progress")                        \
    S(EALREADY, "Operation already in progress")                       \
    S(ENOTSOCK, "Socket operation on non-socket")                      \
    S(EDESTADDRREQ, "Destination address required")                    \
    S(EMSGSIZE, "Message too long")                                    \
    S(EPROTOTYPE, "Protocol wrong type for socket")                    \
    S(ENOPROTOOPT, "Protocol not available")                           \
    S(EPROTONOSUPPORT, "Protocol not supported")                       \
    S(ESOCKTNOSUPPORT, "Socket type not supported")                    \
    S(ENOTSUP, "Operation not supported")                              \
    S(EPFNOSUPPORT, "Protocol family not supported")                   \
    S(EAFNOSUPPORT, "Address family not supported by protocol family") \
    S(EADDRINUSE, "Address already in use")                            \
    S(EADDRNOTAVAIL, "Can't assign requested address")                 \
    S(ENETDOWN, "Network is down")                                     \
    S(ENETUNREACH, "Network is unreachable")                           \
    S(ENETRESET, "Network dropped connection on reset")                \
    S(ECONNABORTED, "Software caused connection abort")                \
    S(ECONNRESET, "Connection reset by peer")                          \
    S(ENOBUFS, "No buffer space available")                            \
    S(EISCONN, "Socket is already connected")                          \
    S(ENOTCONN, "Socket is not connected")                             \
    S(ESHUTDOWN, "Can't send after socket shutdown")                   \
    S(ETOOMANYREFS, "Too many references: can't splice")               \
    S(ETIMEDOUT, "Operation timed out")                                \
    S(ECONNREFUSED, "Connection refused")                              \
    S(ELOOP, "Too many levels of symbolic links")                      \
    S(ENAMETOOLONG, "File name too long")                              \
    S(EHOSTDOWN, "Host is down")                                       \
    S(EHOSTUNREACH, "No route to host")                                \
    S(ENOTEMPTY, "Directory not empty")                                \
    S(EPROCLIM, "Too many processes")                                  \
    S(EUSERS, "Too many users")                                        \
    S(EDQUOT, "Disc quota exceeded")                                   \
    S(ESTALE, "Stale NFS file handle")                                 \
    S(EREMOTE, "Too many levels of remote in path")                    \
    S(EBADRPC, "RPC struct is bad")                                    \
    S(ERPCMISMATCH, "RPC version wrong")                               \
    S(EPROGUNAVAIL, "RPC prog. not avail")                             \
    S(EPROGMISMATCH, "Program version wrong")                          \
    S(EPROCUNAVAIL, "Bad procedure for program")                       \
    S(ENOLCK, "No locks available")                                    \
    S(ENOSYS, "Function not implemented")                              \
    S(EFTYPE, "Inappropriate file type or format")                     \
    S(EAUTH, "Authentication error")                                   \
    S(ENEEDAUTH, "Need authenticator")                                 \
    S(EPWROFF, "Device power is off")                                  \
    S(EDEVERR, "Device error, e.g. paper out")                         \
    S(EOVERFLOW, "Value too large to be stored in data type")          \
    S(EBADEXEC, "Bad executable")                                      \
    S(EBADARCH, "Bad CPU type in executable")                          \
    S(ESHLIBVERS, "Shared library version mismatch")                   \
    S(EBADMACHO, "Malformed Macho file")                               \
    S(ECANCELED, "Operation canceled")                                 \
    S(EIDRM, "Identifier removed")                                     \
    S(ENOMSG, "No message of desired type")                            \
    S(EILSEQ, "Illegal byte sequence")                                 \
    S(ENOATTR, "Attribute not found")                                  \
    S(EBADMSG, "Bad message")                                          \
    S(EMULTIHOP, "Reserved")                                           \
    S(ENODATA, "No message available on STREAM")                       \
    S(ENOLINK, "Reserved")                                             \
    S(ENOSR, "No STREAM resources")                                    \
    S(ENOSTR, "Not a STREAM")                                          \
    S(EPROTO, "Protocol error")                                        \
    S(ETIME, "STREAM ioctl timeout")                                   \
    S(EOPNOTSUPP, "Operation not supported on socket")                 \
    S(ENOPOLICY, "No such policy registered")                          \
    S(ENOTRECOVERABLE, "State not recoverable")                        \
    S(EOWNERDEAD, "Previous owner died")                               \
    S(EQFULL, "Interface output queue is full")

LibCError::LibCError(int err) noexcept
{
    err_no = err;
    switch (err) {
#undef S
#define S(N, D)    \
    case N:        \
        code = #N; \
        description = D;
        ERRORCODES(S)
#undef S
        break;
    case ECUSTOM:
        code = "ECUSTOM";
        description = "Custom error message";
    default:
        code = "UNKNOWN";
        description = "Unknown error";
        break;
    }
}

}
