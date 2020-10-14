#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "Threads.h"
#include "Error.h"

static int readLine(char* buffer);
static void copyLine(char* buffer, char* str, int len);
static void copyLineToQueue(Reader* reader, char* buffer);
static void signalEndOfExecutionByReader(Reader* reader, char* buffer, int freeBuffer);
static void replaceSpaceWithAsterisk(char* str);
static void convertLowerToUpperCase(char* str);


Reader* CreateReader(Queue* outputQueue){
    Reader* reader = malloc(sizeof(Reader));
    if(reader == NULL) PrintMallocErrorAndExit(THREADS_MODULE, READER, "CreateReader");
    reader->outputQueue = outputQueue;
    return reader;
}

Munch1* CreateMunch1(Queue* inputQueue, Queue* outputQueue){
    Munch1* munch1 = malloc(sizeof(Munch1));
    if(munch1 == NULL) PrintMallocErrorAndExit(THREADS_MODULE, MUNCH1, "CreateMunch1");
    munch1->inputQueue = inputQueue;
    munch1->outputQueue = outputQueue;
    return munch1;
}

Munch2* CreateMunch2(Queue* inputQueue, Queue* outputQueue){
    Munch2* munch2 = malloc(sizeof(Munch2));
    if(munch2 == NULL) PrintMallocErrorAndExit(THREADS_MODULE, MUNCH2, "CreateMunch2");
    munch2->inputQueue = inputQueue;
    munch2->outputQueue = outputQueue;
    return munch2;
}

Writer* CreateWriter(Queue* inputQueue){
    Writer* writer = malloc(sizeof(Writer));
    if(writer == NULL) PrintMallocErrorAndExit(THREADS_MODULE, WRITER, "CreateWriter");
    writer->inputQueue = inputQueue;
    writer->stringsProcessedCount = 0;
    return writer;
}

void* StartReader(void* ptr){
    Reader* reader = (Reader*) ptr;

    while(1){
        char* buffer = malloc(sizeof(char) * MAX_BUFFER_SIZE);
        if(buffer == NULL) PrintMallocErrorAndExit(THREADS_MODULE, READER, "Buffer");
        int len = readLine(buffer);

        if(len == -1){
            free(buffer);
            continue;
        } else if(len == -2){
            signalEndOfExecutionByReader(reader, buffer, 1);
            break;
        } else if(len == -3){
            copyLineToQueue(reader, buffer);
            signalEndOfExecutionByReader(reader, buffer, 0);
            break;
        } else if(len == -4){
            signalEndOfExecutionByReader(reader, buffer, 1);
            break;
        }
        copyLineToQueue(reader, buffer);
    }

    pthread_exit(NULL);
}

void* StartMunch1(void* ptr){
    Munch1* munch1 = (Munch1*) ptr;

    while(1){
        char* str = DequeueString(munch1->inputQueue);
        if(str == NULL){
            EnqueueString(munch1->outputQueue, NULL);
            break;
        }
        replaceSpaceWithAsterisk(str);
        EnqueueString(munch1->outputQueue, str);
    }

    pthread_exit(NULL);
}

void* StartMunch2(void* ptr){
    Munch2* munch2 = (Munch2*) ptr;

    while(1){
        char* str = DequeueString(munch2->inputQueue);
        if(str == NULL){
            EnqueueString(munch2->outputQueue, NULL);
            break;
        }
        convertLowerToUpperCase(str);
        EnqueueString(munch2->outputQueue, str);
    }

    pthread_exit(NULL);
}

void* StartWriter(void* ptr){
    Writer* writer = (Writer*) ptr;

    int retVal;
    while(1){
        char* str = DequeueString(writer->inputQueue);
        if(str == NULL){
            retVal = printf("Writer processed %d strings!\n\n",writer->stringsProcessedCount);
            if(retVal < 0) PrintOutputPrintErrorAndExit(THREADS_MODULE, WRITER, "Processed Count");

            break;
        }
        retVal = printf("%s\n",str);
        if(retVal < 0) PrintOutputPrintErrorAndExit(THREADS_MODULE, WRITER, "Processed-String");
        writer->stringsProcessedCount = writer->stringsProcessedCount + 1;
        free(str);
    }

    pthread_exit(NULL);
}

/**
 * A return value of -1 means that the buffer was flushed due to length of line exceeding the max size
 * A return value of -2 means that EOF has been reached with empty buffer
 * A return value of -3 means that EOF has been reached with some value in buffer
 * A return value of -4 means that EOF was reached after line length exceeded Max length
 * */
static int readLine(char* buffer){
    int len = 0, eof = 0, retVal = 0;
    char ch;
    while(1){
        ch = fgetc(stdin);
        if(len > MAX_BUFFER_SIZE){
            if(ch == EOF) {
                eof = 1;
                break;
            }
            else if(ch == '\n') {
                break;
            }
        } else {
            if(ch == EOF){
                if(len == 0){
                    len = -2;
                } else {
                    len = -3;
                }
                break;
            } else if(ch == '\n'){
                break;
            }
            buffer[len++] = ch;
        }
    }

    if(len > MAX_BUFFER_SIZE){
        if(eof == 0)
            len = -1;
        else
            len = -4;
        retVal = fprintf(stderr, "Current line's length exceeded the max size of buffer. Skipping it.\n");
        if(retVal < 0) PrintOutputPrintErrorAndExit(THREADS_MODULE, READER, "STDERR-Buffer-Exceeded");
    }

    return len;
}

static void copyLine(char* buffer, char* str, int len){
    for(int i =0; i < len; i++){
        str[i] = buffer[i];
    }
}

static void copyLineToQueue(Reader* reader, char* buffer){
    int len = strlen(buffer);
    char* str = calloc(len, sizeof(char));
    if(str == NULL) PrintMallocErrorAndExit(THREADS_MODULE, READER, "calloc-reallocateString");
    copyLine(buffer, str, len);
    free(buffer);
    EnqueueString(reader->outputQueue, str);
}

static void signalEndOfExecutionByReader(Reader* reader, char* buffer, int freeBuffer){
    if(freeBuffer){
        free(buffer);
    }
    EnqueueString(reader->outputQueue, NULL);
}

static void replaceSpaceWithAsterisk(char* str){
    int index, len = strlen(str);
    for(index = 0; index < len; index++){
        char ch = str[index];
        if(ch == ' '){
            str[index] = '*';
        }
    }
}

static void convertLowerToUpperCase(char* str){
    int index, len = strlen(str);
    for(index = 0; index < len; index++){
        char ch = str[index];
        if(islower(ch) > 0){
            str[index] = toupper(str[index]);
        }
    }
}
