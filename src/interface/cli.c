//
// Created by Raymond on 4/6/2026.
//

#include <ikuyo.h>

#include <stdio.h>

enum CommandLineFlagType {
    AcceptsNone,
    AcceptsBoolean,
    AcceptsString
};
struct CommandLineFlag {
    char* name[2]; char* value;
    char* description; enum CommandLineFlagType type;
};

#ifdef WIN32
    #define strcmpi _strcmpi
#endif

int main(int argc, char** argv) {
    struct CommandLineFlag flags[] = {
        { {"h", "help"}, NULL, "Displays this help message.", AcceptsNone },
        { {"v", "verbose"}, NULL, "Logs all information.", AcceptsNone },
        { {"t", "type"},  NULL, "Specifies the input format type (enumeration above).", AcceptsString },
        { {"s", "select"}, NULL, "Selects a specific image index if available.", AcceptsString },
        { {"o", "output"}, NULL, "Specifies the output file name.", AcceptsString },
        { {"f", "format"}, NULL, "Specifies the format to output files in.", AcceptsString },
        { {"q", "quality"}, NULL, "Percentage (1 to 100) to prefer quality (excludes `png` format).", AcceptsString }
    };

    // NOTE: use `individualArguments` and `argumentCount` instead of `argv` and `argc`
    char* individualArguments = NULL;
    int argumentCount = 0; int argumentIndex = 1;

    while (argumentIndex < argc) {
        struct CommandLineFlag* flag = NULL;
        if (argv[argumentIndex][0] == '-' || (argv[argumentIndex][0] == '-' && argv[argumentIndex][1] == '-')) {
            for (uint32_t index = 0; index < sizeof(flags) / sizeof(struct CommandLineFlag); index++)
                if ((argv[argumentIndex][0] == '-' && strcmpi(flags[index].name[0], argv[argumentIndex] + 1) == 0) ||
                    (argv[argumentIndex][0] == '-' && argv[argumentIndex][1] == '-' && strcmpi(flags[index].name[1], argv[argumentIndex] + 2) == 0))
                { flag = &flags[index]; break; }
            if (flag != NULL) {
                switch (flag->type) {
                    case AcceptsNone: flag->value = (char*)1; break;
                    case AcceptsBoolean: {
                        char* valueString = argv[argumentIndex + 1];
                        flag->value = strcmpi(valueString, "true") == 0 ||
                              strcmpi(valueString, "on") == 0 ||
                              strcmpi(valueString, "yes") == 0 ? (char*)1 : (char*)0;
                        break;
                    }
                    case AcceptsString: flag->value = argv[argumentIndex + 1]; break;
                    default: break;
                }
                argumentIndex += flag->type > AcceptsNone;
            } else
                printf("Unknown flag %s\n", argv[argumentIndex]);
        } else {
            argumentCount++;
            char* argumentBuffer = realloc(individualArguments, argumentCount * sizeof(char*));
            if (!argumentBuffer)
                goto CommandLineEnd;
            individualArguments = argumentBuffer;
        }
        argumentIndex++;
    }
    if (flags[0].value) {
#ifdef WIN32
        const char* executable = strrchr(argv[0], '\\');
#else
        const char* executable = strrchr(argv[0], '/');
#endif
        if (executable)
            ++executable;
        else
            executable = argv[0];

        printf("Usage:\n\n%s [input file name] (options)\n\n", executable);
        printf(" * The input format can typically be determined automatically based on the file's header.\n");
        printf(" * All textures in the input will be output if no specific option is selected.\n\n");
        printf("Options:\n");

        for (uint32_t index = 0; index < sizeof(flags) / sizeof(struct CommandLineFlag); index++) {
            char flagPrefix[20] = {0};
            snprintf(flagPrefix, sizeof(flagPrefix), " * -%s (--%s):",  flags[index].name[0], flags[index].name[1]);
            printf("%s", flagPrefix);
            for (uint32_t stringIndex = strlen(flagPrefix); sizeof(flagPrefix) > stringIndex; stringIndex++)
                printf(" ");
            printf("%s\n", flags[index].description);
        }

        goto CommandLineEnd;
    }

    const bool verbose = flags[1].value > 0;


CommandLineEnd:
    free(individualArguments);

    return 0;
}