#include "syscall.h"


#define MAX_LINE_SIZE  60
#define MAX_ARG_COUNT  32
#define ARG_SEPARATOR  ' '

#define NULL  ((void *) 0)

static inline unsigned
strlen(const char *s)
{
    unsigned i = 0;
    if(s != NULL) {
        for (; s[i] != '\0'; i++);
    }
    return i;
}

static inline void
WritePrompt(OpenFileId output)
{
    static const char PROMPT[] = "--> ";
    Write(PROMPT, sizeof PROMPT - 1, output);
}

static inline void
WriteError(const char *description, OpenFileId output)
{
    static const char PREFIX[] = "Error: ";
    static const char SUFFIX[] = "\n";
    if(description != NULL) {
        Write(PREFIX, sizeof PREFIX - 1, output);
        Write(description, strlen(description), output);
        Write(SUFFIX, sizeof SUFFIX - 1, output);
    }
}

static unsigned
ReadLine(char *buffer, unsigned size, OpenFileId input)
{
    if(buffer != NULL) {
        unsigned i;
        for (i = 0; i < size; i++) {
            Read(&buffer[i], 1, input);
            if (buffer[i] == '\n' || buffer[i] == '\0') {
                buffer[i] = '\0';
                break;
            }
        }
    return i;
    }
}

static int
PrepareArguments(char *line, char **argv, unsigned argvSize)
{
    // TO DO: how to make sure that `line` and `argv` are not `NULL`?, and
    //        for `argvSize`, what precondition should be fulfilled?
    //
    // PENDIENTE: use `bool` instead of `int` as return type; for doing this,
    //            given that we are in C and not C++, it is convenient to
    //            include `stdbool.h`.
    if(line != NULL && argv != NULL) {
        unsigned argCount;

        argv[0] = line;
        argCount = 1;

        // Traverse the whole line and replace spaces between arguments by null
        // characters, so as to be able to treat each argument as a standalone
        // string.
        //
        // TO DO: what happens if there are two consecutive spaces?, and what
        //        about spaces at the beginning of the line?, and at the end?
        //
        // TO DO: what if the user wants to include a space as part of an
        //        argument?
        unsigned i;
        for (i = 0; line[i] != '\0'; i++)
            if (line[i] == ARG_SEPARATOR) {
                if (argCount == argvSize - 1)
                    // The maximum of allowed arguments is exceeded, and
                    // therefore the size of `argv` is too.  Note that 1 is
                    // decreased in order to leave space for the NULL at the end.
                    return 0;

                line[i] = '\0';
                i++;
                for(; line[i] == ARG_SEPARATOR; i++)
                    line[i] = '\0';

                argv[argCount] = &line[i];
                argCount++;
            }

        argv[argCount] = NULL;
        return 1;
    }
}

int
main(void)
{
    const OpenFileId INPUT  = ConsoleInput;
    const OpenFileId OUTPUT = ConsoleOutput;
    char  line[MAX_LINE_SIZE];
    char  *argv[MAX_ARG_COUNT];
    char *line_;
    int bg = 0, i;

    for (;;) {
        WritePrompt(OUTPUT);
        ReadLine(line, MAX_LINE_SIZE, INPUT);

        for(i = 0; line[i] == ARG_SEPARATOR; i++);

        if(line[i] == '&') {
            bg = 1;
            i++;
            for(; line[i] == ARG_SEPARATOR; i++)
                ;
        }

        line_ = line + i;

        if(strlen(line_) == 0)
            continue;

        if (PrepareArguments(line, argv, MAX_ARG_COUNT) == 0) {
            WriteError("too many arguments.", OUTPUT);
            continue;
        }

        // Comment and uncomment according to whether command line arguments
        // are given in the system call or not.

        if(bg == 1) {
            Exec(line_, argv, 0);
            continue;
        }

        const SpaceId newProc = Exec(line_, argv, 1);
        Join(newProc);
    }

    return 0;  // Never reached.
}
