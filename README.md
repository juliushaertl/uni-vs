uni-vs
======

tcp server/client for distributed systems lecture

## Request Format

    Name:   REQUESTID   FUNCTION    ARGC    ARGV
    Count:  1           1           1       0-12
    Type:   char        char        char    uint32_t


## Functions

    0x00    SUM         Calculate the sum of all arguments
    0x01    COUNT       Count the given arguments
    0x02    EXIT        Close the server program


## Response Format

    REQUESTID   FUNCTION    RESULT
    1           1           1
    char        char        uint32_t

