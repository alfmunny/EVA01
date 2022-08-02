A simple server framework written in C++

## Logging
- Logger
    - level
    - multiple appenders
- LogAppender: StdOut, File, Stream
    - its own formatter
    - its own level
- LogEvent
- LogLevel: UNKNOW, DEBUG, INFO, WARN, ERROR, FATAL
- LogFormatter

        """
        Use log4j-like pattern:

        {%Y-%m-%d %a %H:%M:%S}

        "%d%T%f%T%l%T[%p]%T[%c]%T%m%n";


        Every pattern has also has sub format in {}

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

## Thread

- Thread class using pthread
    - Thread name
    - Thread id
