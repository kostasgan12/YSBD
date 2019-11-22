#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20

#define MAX_RECORDS_SUM (BF_BLOCK_SIZE - sizeof(int))/sizeof(Record)

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}


int hash(int key, int buckets){
    return key%buckets;
}

HT_ErrorCode HT_Init() {

    //insert code here
  return HT_OK;
}

HT_ErrorCode HT_CreateIndex(const char *filename, int buckets) {
  //insert code here
    BF_Block *block;
    BF_Block_Init(&block);
    char* data;
    int fd;

    CALL_BF(BF_CreateFile(filename));
    CALL_BF(BF_OpenFile(filename, &fd));

    //BF_Block_Init(&block);

//    CALL_BF(BF_AllocateBlock(fd, block));
//    data = BF_Block_GetData(block);

//    metadata meta;
//    meta->hashFlag = "This Is A Hash File";
//    meta->bucketSum = buckets;
//    memcpy(data, meta, sizeof(meta));

    int blockSum;
    for(int i = 0; i < buckets; i++){                                                           //initiallizing buckets blocks. we already know how many we have initially
        CALL_BF(BF_AllocateBlock(fd, block));
        data = BF_Block_GetData(block);
        CALL_BF(BF_GetBlockCounter(fd, &blockSum));

        metadata meta;

        if( blockSum == 0 || blockSum == NULL ){
//            CALL_BF(BF_AllocateBlock(fd, block));
//            data = BF_Block_GetData(block);
//            metadata meta;

            meta->hashFlag = "This Is A Hash File";
            meta->bucketSum = buckets;

//            memcpy(data, meta, sizeof(meta));
        }else if(blockSum > 1){

            meta->hashFlag = "This Is A Bucket Index";
            meta->bucketMaxLoad = MAX_RECORDS_SUM;
            meta->bucketLoad = 0;
        }
        memcpy(data, meta, sizeof(meta));
    }


    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    CALL_BF(BF_CloseFile(fd));

    return HT_OK;
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
  //insert code here

    BF_Block *block;
    char* data;

    CALL_BF(BF_OpenFile(fileName, indexDesc));

    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(*indexDesc, 0, block));
    data = BF_Block_GetData(block);

    if (memcmp(data, "This Is A Hash File", sizeof("This Is A Hash File"))){      // 0 means they're equal, and that the file we have opened is actually a Hash File
        CALL_BF(BF_UnpinBlock(block));                                            // entering the if block means they aren't equal and that we have opened the wrong file
        BF_Block_Destroy(&block);
        return HP_ERROR;
    }

    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);

    return HT_OK;
}

HT_ErrorCode HT_CloseFile(int indexDesc) {
  //insert code here

    CALL_BF(BF_CloseFile(indexDesc));                                              // closing our file
    return HT_OK;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  //insert code here
    BF_Block *block;
    BF_Block_Init(&block);
    char *data;
    int recordCounter = 0;                                        // keeps track of the number of records kept in a block



    return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}

HT_ErrorCode HT_DeleteEntry(int indexDesc, int id) {
  //insert code here
  return HT_OK;
}
