#ifndef POP_STATES
#define POP_STATES

typedef enum {
    ANY,
    AUTHENTICATION,
    TRANSACTION,
    UPDATE,
} pop_state;

#define MAXCOMMANDSIZE 4 //without null term (according to RFC 1939)
#define MAXARGSIZE 40 //without null term (according to RFC 1939)
#define MAXCOMMANDLENGTH 255 //max size of command sent by client with final crlf included (according to RFC 2449)
#define MAXANSWERSIZE 512 //max size for the first line of server response (including crlf) (according to RFC 2449)

#endif