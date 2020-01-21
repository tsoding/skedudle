#ifndef REQUEST_H_
#define REQUEST_H_

typedef struct {
    String method;
    String path;
} Status_Line;

Status_Line chop_status_line(String *buffer)
{
    String line = trim_end(chop_line(buffer));
    Status_Line result;
    result.method = chop_word(&line);
    result.path = chop_word(&line);
    return result;
}

typedef struct {
    String name;
    String value;
} Header;

Header parse_header(String line)
{
    Header result;
    result.name = trim(chop_until_char(&line, ':'));
    result.value = trim(line);
    return result;
}

#endif  // REQUEST_H_
