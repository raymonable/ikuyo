//
// Created by Raymond on 4/6/2026.
//

#include <ikuyo.h>
#include <common/texture.h>
#include <common/image.h>

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

int main(int argc, char** argv) {
    FILE* file = NULL;
    uint8_t* buffer = NULL;
    struct TextureArray array = {0};

    textureLoadImplementationsInit();

    struct CommandLineFlag flags[] = {
        { {"h", "help"}, NULL, "Displays this help message.", AcceptsNone },
        { {"v", "verbose"}, NULL, "Logs all information.", AcceptsNone },
        { {"t", "type"},  NULL, "Specifies the input format type (enumeration above).", AcceptsString },
        { {"f", "format"}, NULL, "Specifies the format to output files in.", AcceptsString },
        { {"o", "output"}, NULL, "Specifies the output file name.", AcceptsString },
        { {"s", "select"}, NULL, "Selects a specific image index if available.", AcceptsString },
        { {"q", "quality"}, NULL, "Percentage (1 to 100) to prefer quality (excludes `png` format).", AcceptsString }
    };

    // NOTE: use `individualArguments` and `argumentCount` instead of `argv` and `argc`
    char** individualArguments = NULL;
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
            if (individualArguments != NULL) {
                char** allocatedArguments = realloc(individualArguments, argumentCount * sizeof(char*));
                if (!allocatedArguments) goto CommandLineEnd;
                individualArguments = allocatedArguments;
            } else
                individualArguments = (char**)malloc(argumentCount * sizeof(char*));
            individualArguments[argumentCount - 1] = argv[argumentIndex];
        }
        argumentIndex++;
    }
    if (flags[0].value || argumentCount <= 0) {
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

    // BEGIN: process flags

    const bool verbose = flags[1].value != NULL;
    enum TextureContainer textureContainer = UnknownContainer;
    enum ImageContainer imageContainer = PNG;

    if (flags[2].value != NULL) {
        textureContainer = textureContainerGetFromString(flags[2].value);
        if (verbose) printf("Specific input format %s requested\n", flags[2].value);
    } else if (verbose) printf("No input format specified\n");

    if (flags[3].value != NULL) {
        if (strcmpi(flags[3].value, "webp") == 0) imageContainer = WebP;
        if (strcmpi(flags[3].value, "avif") == 0) imageContainer = AVIF;
    }

    // BEGIN: read file
    const char* fileName = individualArguments[0];
    file = fopen(fileName, "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file %s\n", individualArguments[0]);
        goto CommandLineEnd;
    }
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = malloc(fileSize);
    fread(buffer, fileSize, 1, file);
    fclose(file);

    array = textureLoad(textureContainer, buffer, fileSize);
    if (verbose || array.count == 0) printf("Decoded %i textures\n", array.count);

    for (size_t index = 0; array.count > index; index++) {
        if (flags[5].value != NULL && strtoul(flags[5].value, NULL, 10) != index) continue;
        struct TextureInformation information = array.data[index];
        if (information.format != RGBA) {
            fprintf(stderr, "Decoder didn't provide RGBA, exiting\n");
            goto CommandLineEnd;
        }
        struct ImageBuffer imageBuffer = {0};
        switch (imageContainer) {
            case PNG: { imageBuffer = pngGenerate(information); break; }
            // TODO: handle compression value
            case WebP: { imageBuffer = webpGenerate(information); break; }
            // TODO: handle compression value
            case AVIF: { imageBuffer = avifGenerate(information); break; }
            default:
                fprintf(stderr, "Unknown output format, exiting\n");
                goto CommandLineEnd;
        }
        if (imageBuffer.buffer != NULL) {
            char outputFileName[128] = {0};
            uint32_t length = strlen(fileName);
            if (flags[4].value != NULL) {
                // Use preferred filename
                length = strlen(flags[4].value);
                memcpy(outputFileName, flags[4].value, length);
            } else {
                // Use existing filename
                while (fileName[length] != '.' && length > 0)
                    length--;
                memcpy(outputFileName, fileName, length);
            }
            // Append a (%i) if there are multiple images
            if (!flags[5].value && array.count > 1) {
                sprintf(outputFileName + length, "(%lu)", index + 1);
                length = strlen(outputFileName);
            }
            outputFileName[length] = '.';

            switch (imageContainer) {
                case PNG: { memcpy(outputFileName + length + 1, "png", 3); break; }
                case WebP: { memcpy(outputFileName + length + 1, "webp", 4); break; }
                case AVIF: { memcpy(outputFileName + length + 1, "avif", 4); break; }
                default: break;
            }

            file = fopen(outputFileName, "wb");
            fwrite(imageBuffer.buffer, imageBuffer.size, 1, file);
            fclose(file);
        }
    }

CommandLineEnd:

    textureArrayFree(&array);

    free(individualArguments);

    free(buffer);
    fclose(file);

    return 0;
}