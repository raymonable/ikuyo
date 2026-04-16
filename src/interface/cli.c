//
// Created by Raymond on 4/6/2026.
//

#include <ikuyo.h>
#include <common/texture.h>
#include <common/image.h>

#include <stdio.h>

#ifndef WIN32
    #define fopen_s(a,b,c) *(a) = fopen(b,c)
#endif

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
    struct TextureLoaderImplementations implementations = {0};

    textureLoadImplementationsInit(&implementations);

    struct CommandLineFlag flags[] = {
        { {"h", "help"}, NULL, "Displays this help message.", AcceptsNone },
        { {"v", "verbose"}, NULL, "Logs all information.", AcceptsNone },
        { {"t", "type"},  NULL, "Specifies the input format type (enumeration above).", AcceptsString },
        { {"f", "format"}, NULL, "Specifies the format to output files in.", AcceptsString },
        { {"o", "output"}, NULL, "Specifies the output file name.", AcceptsString },
        { {"s", "select"}, NULL, "Selects a specific image index if available.", AcceptsString },
        { {"q", "quality"}, NULL, "Percentage (1 to 100) to prefer quality (excludes `png` format).", AcceptsString },
        { {"r", "resolution"}, NULL, "Resizes the output image(s) to a specific resolution (ex: 250x250)", AcceptsString }
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
    if (flags[0].value || argumentCount <= 0 || !individualArguments) {
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
            char flagPrefix[24] = {0};
            snprintf(flagPrefix, sizeof(flagPrefix), " * -%s (--%s):",  flags[index].name[0], flags[index].name[1]);
            printf("%s", flagPrefix);
            for (uint32_t stringIndex = strlen(flagPrefix); sizeof(flagPrefix) > stringIndex; stringIndex++)
                printf(" ");
            printf("%s\n", flags[index].description);
        }

        printf("\nTexture Formats:\n");
        for (uint32_t index = 0; index < implementations.count; index++)
            printf("* %s (%s)\n", implementations.data[index].name, implementations.data[index].description);

        goto CommandLineEnd;
    }

    // BEGIN: process flags

    const bool verbose = flags[1].value != NULL;
    enum TextureContainer textureContainer = UnknownContainer;
    enum ImageContainer imageContainer = PNG;
    int preferredQuality = 75; // 75% is ideal for WebP, 50% works well with AVIF

    if (flags[2].value != NULL) {
        for (size_t index = 0; implementations.count > index; index++)
            if (strcmpi(implementations.data[index].name, flags[2].value) == 0)
                textureContainer = implementations.data[index].container;
        if (verbose) printf("Specific input format %s requested\n", flags[2].value);
    } else if (verbose) printf("No input format specified, detecting automatically\n");

    if (flags[3].value != NULL) {
        if (strcmpi(flags[3].value, "webp") == 0) imageContainer = WebP;
        if (strcmpi(flags[3].value, "avif") == 0) imageContainer = AVIF;
    }

    if (flags[6].value != NULL)
        preferredQuality = (int)strtol(flags[6].value, NULL, 10);
    if (imageContainer != PNG && verbose) printf("Compressing output image to %i%%\n", preferredQuality);

    // BEGIN: read file
    const char* fileName = individualArguments[0];
    fopen_s(&file, fileName, "rb");
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

    array = textureLoad(&implementations, textureContainer, buffer, fileSize);
    if (verbose) printf("Processed %i textures\n", array.count);
    if (!array.count) printf("No textures were processed. The file may be corrupt or not match any specific format.");

    for (size_t index = 0; array.count > index; index++) {
        if (flags[5].value != NULL && strtoul(flags[5].value, NULL, 10) != index) continue;

        if (flags[7].value != NULL) {
            // BEGIN: get individual pieces of resolution string & scale
            char* resolutionString = malloc(strlen(flags[7].value));
            memcpy(resolutionString, flags[7].value, strlen(flags[7].value));
            uint32_t resolutionDivision = 0;
            while (resolutionString[resolutionDivision] != 'x' && resolutionString[resolutionDivision] != '*' &&
                resolutionString[resolutionDivision] != '/' && resolutionDivision < strlen(flags[7].value))
                    resolutionDivision++;
            if (resolutionDivision >= strlen(flags[7].value) || !resolutionDivision) goto ResolutionResizeEnd;
            resolutionString[resolutionDivision] = 0;

            int w = (int)strtol(resolutionString, NULL, 10);
            int h = (int)strtol(resolutionString + resolutionDivision + 1, NULL, 10);
            if (w <= 0 && h <= 0) goto ResolutionResizeEnd;

            struct TextureInformation* informationPtr = array.data + (index * sizeof(struct TextureInformation));

            // TODO: fix lol
            if (w <= 0) w = (int)(((float)informationPtr->width / (float)informationPtr->height) * (float)h);
            if (h <= 0) h = (int)(((float)informationPtr->height / (float)informationPtr->width) * (float)w);

            textureResize(array.data + (index * sizeof(struct TextureInformation)), w, h);
ResolutionResizeEnd:
            free(resolutionString);
        }

        struct TextureInformation information = array.data[index];
        if (information.format != RGBA) {
            fprintf(stderr, "Decoder didn't provide RGBA, exiting\n");
            goto CommandLineEnd;
        }
        const struct ImageBuffer imageBuffer = imageGenerate(imageContainer, information, preferredQuality);
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
                snprintf(outputFileName + length, 128 - length, " (%llu)", index + 1);
                length = strlen(outputFileName);
            }
            outputFileName[length] = '.';

            switch (imageContainer) {
                case PNG: { memcpy(outputFileName + length + 1, "png", 3); break; }
                case WebP: { memcpy(outputFileName + length + 1, "webp", 4); break; }
                case AVIF: { memcpy(outputFileName + length + 1, "avif", 4); break; }
                default: break;
            }

            fopen_s(&file, outputFileName, "wb");
            fwrite(imageBuffer.buffer, imageBuffer.size, 1, file);
            fclose(file);

            if (verbose) printf("Exported texture %llu to %s\n", index + 1, outputFileName);
        }
        imageBufferFree(imageBuffer);
    }

CommandLineEnd:

    textureArrayFree(&array);
    textureLoadImplementationsFree(&implementations);

    free(individualArguments);

    free(buffer);
    if (file != NULL)
        fclose(file);

    return 0;
}