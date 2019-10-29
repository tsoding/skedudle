#ifndef RESPONSE_H_
#define RESPONSE_H_

void response_status_line(int fd, int code)
{
    dprintf(fd, "HTTP/1.1 %d\n", code);
}

void response_header(int fd, const char *name, const char *value_format, ...)
{
    va_list args;
    va_start(args, value_format);

    dprintf(fd, "%s: ", name);
    vdprintf(fd, value_format, args);
    dprintf(fd, "\n");

    va_end(args);
}

void response_body_start(int fd)
{
    dprintf(fd, "\n");
}

#endif  // RESPONSE_H_
