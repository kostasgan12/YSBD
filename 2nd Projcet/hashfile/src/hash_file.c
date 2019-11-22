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

Open_File openFiles[MAX_OPEN_FILES];

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

    CALL_BF(BF_AllocateBlock(fd, block));
    data = BF_Block_GetData(block);

    memcpy(data, "This Is A Hash File", sizeof("This Is A Hash File"));                         //1st block has Hash flag
    data += sizeof("This Is A Hash File");

    memcpy(data, buckets, sizeof(int));
    data += sizeof(int);

    int root = 1 ;
    memcpy(data, &root, sizeof(int));


//    metadata meta;
//    meta->hashFlag = "This Is A Hash File";
//    meta->bucketSum = buckets;
//    memcpy(data, meta, sizeof(meta));

    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));



    CALL_BF(BF_AllocateBlock(fd, block));                                                       //allocating space for 2nd block
    data = BF_Block_GetData(block);
    int hMap[buckets];                                                                          //creating a Hash Map
    memcpy(data, hMap, sizeof(hMap));                                                           //copying Hash Map to 2nd block

//////////////////////////////////////////////////////////////////////////////////////////////
//    int blockSum;
//    for(int i = 0; i < buckets; i++){                                                           //initiallizing buckets blocks. we already know how many we have initially
//        CALL_BF(BF_AllocateBlock(fd, block));
//        data = BF_Block_GetData(block);
//        CALL_BF(BF_GetBlockCounter(fd, &blockSum));
//
//        metadata meta;
//
//        if( blockSum == 0 || blockSum == NULL ){
////            CALL_BF(BF_AllocateBlock(fd, block));
////            data = BF_Block_GetData(block);
////            metadata meta;
//
//            meta->hashFlag = "This Is A Hash File";
//            meta->bucketSum = buckets;
//
////            memcpy(data, meta, sizeof(meta));
//        }else if(blockSum > 1){
//
//            meta->hashFlag = "This Is A Bucket Index";
//            meta->bucketMaxLoad = MAX_RECORDS_SUM;
//            meta->bucketLoad = 0;
//        }
//        memcpy(data, meta, sizeof(meta));
//    }
//////////////////////////////////////////////////////////////////////////////////////////////
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
    int fd;
    CALL_BF(BF_OpenFile(fileName, &fd));

    BF_Block_Init(&block);

    CALL_BF(BF_GetBlock(*indexDesc, 0, block));
    data = BF_Block_GetData(block);

    if (memcmp(data, "This Is A Hash File", sizeof("This Is A Hash File"))){            // 0 means they're equal, and that the file we have opened is actually a Hash File
        CALL_BF(BF_UnpinBlock(block));                                                  // entering the if block means they aren't equal and that we have opened the wrong file
        BF_Block_Destroy(&block);
        return HT_ERROR;
    }

//    char fileArray[MAX_OPEN_FILES][30];                                                 //30 is the length of each row, so we have 30 spaces to enter info

    int bucketSum;
    data += sizeof("This Is A Hash File");                                              //move the data pointer to where the bucket sum is located
    memcpy(bucketSum, data, sizeof(int));

    *indexDesc = hash(fd, bucketSum);
    strcpy(openFiles[*indexDesc].filename, fileName);
    openFiles[*indexDesc].filedesc = fd;

    data += sizeof(int);
    memcpy(openFiles[*indexDesc].blockRoot, data, sizeof(int));


    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);

    return HT_OK;
}

HT_ErrorCode HT_CloseFile(int indexDesc) {
  //insert code here

    if(openFiles[indexDesc].filedesc == NULL){
        printf("!ERROR! File Not Found")
        return HT_ERROR;
    }


    CALL_BF(BF_CloseFile(indexDesc));                                                       // closing our file
    openFiles[indexDesc].filedesc = NULL;
    return HT_OK;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  //insert code here
    BF_Block *block;
    BF_Block_Init(&block);
    char *data;
    int fd;
    int recordCounter = 0;                                                                  // keeps track of the number of records kept in a block
    int bucketSum =0;                                                                       // keeps track of the number of hash buckets
    int blockSum = 0;

    int bucketMAX;                                                                          //MAX number of buckets
    CALL_BF(BF_GetBlock(indexDesc, 0, block));
    data = BF_Block_GetData(block);
    data += sizeof("This Is A Hash File");                                                  //move the data pointer to where the bucket sum is located
    memcpy(bucketMAX, data, sizeof(int));

//    int index = hash(indexDesc, bucketMAX);

    fd = openFiles[indexDesc].filedesc;

    CALL_BF(BF_GetBlockCounter(fd, &blockSum));

    if(blockSum == 2){                                                                      //this means the block is empty, because block #1 has metadata and #2 has openFiles array
        CALL_BF(BF_AllocateBlock(fd, block));                                        //Generating an Index block for our first Bucket
        data = BF_Block_GetData(block);

        memcpy(data, "H", sizeof("H"));                                                     //Index blocks start with H for Hash so we can recognise them
        data += sizeof("H");

        memcpy(data, hash(record.id, bucketMAX), sizeof(int));                              //Copying the generated hash code so we know what hash code we find in this bucket
        data += sizeof(int);

        bucketSum = 1;
        memcpy(data, &bucketSum, sizeof(int));
        data += sizeof(int);

        BF_Block_SetDirty(block);                                                           //We dont need this block as we're going to create another one next
        checkBF(BF_UnpinBlock(block));                                                      //so we set it as dirty and we Unpin it


        CALL_BF(BF_AllocateBlock(fd, block));                                        //Generating a data block for our first record element
        data = BF_Block_GetData(block);

        memcpy(data, "D", sizeof("D"));                                                     //Data blocks start with D for Data so we can recognise them
        data += sizeof("D");

        recordCounter = 1;
        memcpy(data, recordCounter, sizeof(int));                                           //Copying our record counter for this bucket
        data += sizeof(int);

        memcpy(data, &record, sizeof(Record));                                              //Inserting our 1st record in our bucket
        data += sizeof(Record);

        BF_Block_SetDirty(block);
        checkBF(BF_UnpinBlock(block));

    }else if(blockSum == 3){                                                                //In this case we have 1 block that containts buckets which narrows our search
        CALL_BF(BF_GetBlock(fd, openFiles[indexDesc].blockRoot , block));
        data = BF_Block_GetData(block);
        data += sizeof("H");
        int tmpCounter = 0;
        memcpy(&tmpCounter, data, sizeof(int));
        data += sizeof(int);

        int recordHash = hash((record.id, bucketMAX));                                      //



    }


    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);

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
