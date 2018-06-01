#include "syscall.h"


#define MAX_LINE_SIZE  60
#define MAX_ARG_COUNT  32
#define ARG_SEPARATOR  ' '

#define NULL  ((void *) 0)

static inline unsigned
strlen(const char *s)
{
    unsigned i;
    if(s != NULL)
        for (i = 0; s[i] != '\0'; i++);
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
    unsigned i;
    if(buffer != NULL) {
        for (i = 0; i < size; i++) {
            Read(&buffer[i], 1, input);
            if (buffer[i] == '\n' || buffer[i] == '\0') {
                i--;
                break;
            }
        }
        buffer[i+1] = '\0';
    }
    return i;
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
    unsigned i = 0;
    for (; line[i] != '\0'; i++)
        if (line[i] == ARG_SEPARATOR) {
            if (argCount == argvSize - 1)
                // The maximum of allowed arguments is exceeded, and
                // therefore the size of `argv` is too.  Note that 1 is
                // decreased in order to leave space for the NULL at the end.
                return 0;

            while(line[i+1] == ARG_SEPARATOR)
                i++;

            line[i] = '\0';
            argv[argCount] = &line[i + 1];
            argCount++;
        }

    argv[argCount] = NULL;
    return 1;
}

int
main(void)
{
    const OpenFileId INPUT  = ConsoleInput;
    const OpenFileId OUTPUT = ConsoleOutput;
    char  line[MAX_LINE_SIZE];
    char  *argv[MAX_ARG_COUNT];

    for (;;) {
        WritePrompt(OUTPUT);
        const unsigned lineSize = ReadLine(line, MAX_LINE_SIZE, INPUT);
        if (lineSize <= 0)
            continue;

        char *line_;
        int bg;

        if(line[0] == '&') {
            bg = 1;
            line_ = line + 2*sizeof(char);
        } else {
            bg = 0;
            line_ = line;
        }

        if (PrepareArguments(line, argv, MAX_ARG_COUNT) == 0) {
            WriteError("too many arguments.", OUTPUT);
            continue;
        }

        // Comment and uncomment according to whether command line arguments
        // are given in the system call or not.
        const SpaceId newProc = Exec(line_, argv);

        if(newProc > 0) {
            if(!bg) {
                if(Join(newProc) < 0)
                    WriteError("Join failed or program exit with error", OUTPUT);
            }
        } else {
            WriteError("process execution error.", OUTPUT);
        }
    }

    return 0;  // Never reached.
}
