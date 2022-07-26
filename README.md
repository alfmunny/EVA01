A simple server framework written in C++

## Logging
- Logger
- LogAppender: StdOut, File, Stream
- LogEvent
- LogLevel: UNKNOW, DEBUG, INFO, WARN, ERROR, FATAL
- LogFormatter
    """
    %m, // m: message
    %p, // p: level
    %r, // r: elapse time
    %c, // c: logger name
    %t, // t: thread id
    %n, // n: new line
    %d, // d: date
    %f, // f: file
    %l, // l: line number
    %T, // T: Tab
    %F, // F: fiber id
    %N, // N: thread name
    """
