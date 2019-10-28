#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "heap_file.h"

#define MAX_RECORDS_SUM (BF_BLOCK_SIZE - sizeof(int))/sizeof(Record)

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

HP_ErrorCode HP_Init() {
//  BF_Block *block;
//  BF_Block_Init(&block);
//
//  CALL_OR_DIE(BF_Init(LRU));

  return HP_OK;
}

HP_ErrorCode HP_CreateFile(const char *filename) {
  //insert code here
  BF_Block *block;
  BF_Block_Init(&block);
  int fd;

  CALL_BF(BF_CreateFile(filename));
  CALL_BF(BF_OpenFile(filename, &fd));

  BF_Block_Init(&block);
  CALL_BF(BF_AllocateBlock(fd, block));
  char* data = BF_Block_GetData(block);

  memcpy(data, "This Is A Heap File", sizeof("This Is A Heap File"));

  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);
  CALL_BF(BF_CloseFile(fd));

  return HP_OK;
}

HP_ErrorCode HP_OpenFile(const char *fileName, int *fileDesc){
  //insert code here
  BF_Block *block;

  CALL_BF(BF_OpenFile(fileName, fileDesc));

  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(*fileDesc, 0, block));
  char* data = BF_Block_GetData(block);

  if (memcmp(data, "This Is A Heap File", sizeof("This Is A Heap File"))){      // 0 means they're equal, and that the file we have opened is actually a Heap File
      CALL_BF(BF_UnpinBlock(block));                                            // entering the if block means they aren't equal and that we have opened the wrong file
      BF_Block_Destroy(&block);
      return HP_ERROR;
  }

  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);

  return HP_OK;                                                                 // we have correctly opened a Heap File
}

HP_ErrorCode HP_CloseFile(int fileDesc) {
  //insert code here
  CALL_BF(BF_CloseFile(fileDesc));                                              // closing our file
  return HP_OK;
}

HP_ErrorCode HP_InsertEntry(int fileDesc, Record record) {
  //insert code here
  BF_Block *block;
  BF_Block_Init(&block);

  int recordCounter = 0;                                        // keeps track of the number of records kept in a block

  int blockSum = 0;
  CALL_BF(BF_GetBlockCounter(fileDesc, &blockSum));

  if( blockSum == 1) {                                          // 1 means the file is empty of records, because the only file in there is the Heap flag that we inserted when creating it
    CALL_BF(BF_AllocateBlock(fileDesc, block));

    char *data = BF_Block_GetData(block);
    recordCounter++;
    memcpy(data, &recordCounter, sizeof(int));

    data += sizeof(int);                                        // move data pointer by sizeof(int) (4) bytes, so that we can add our next item

    memcpy(data, &record, sizeof(Record));                      // adding the record to our Heap file

    BF_Block_SetDirty(block);                                   //SetDirty and Unpin because we altered the Heap file (as instructed)
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);

  }else if (blockSum >= 2){                                     // 2 or more means that the file already contains blocks with records
    CALL_BF(BF_GetBlock(fileDesc, blockSum-1, block));          //blockSum-1 because we want to add at the end of the current block (if it fits)

    char* data = BF_Block_GetData(block);
    memcpy(&recordCounter, data, sizeof(int));                  // copying counter from data to see how many records are currently in the block
    if(recordCounter < MAX_RECORDS_SUM) {
        recordCounter++;
        memcpy(data, &recordCounter, sizeof(int));              // update the Record Counter

        data += sizeof(int);
        data += ((recordCounter - 1) * sizeof(Record));         // move the data pointer to the first available slot

        //memcpy(data, &record, sizeof(Record));                // insert record

    }else {                                                     // if the current block contains maximum amount of records we need to create a new one
        CALL_BF(BF_UnpinBlock(block));                          // Unpin the current block, as its unnecessary

        CALL_BF(BF_AllocateBlock(fileDesc, block));

        char *data = BF_Block_GetData(block);
        recordCounter++;
        memcpy(data, &recordCounter, sizeof(int));

        data += sizeof(int);                                    // move data pointer by sizeof(int) (4) bytes only as its the first record, so that we can add our next item
        //memcpy(data, &record, sizeof(Record));                    // insert record
    }
    memcpy(data, &record, sizeof(Record));                      // insert record

    BF_Block_SetDirty(block);                                   //SetDirty and Unpin because we altered the Heap file (as instructed)
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
  }

  return HP_OK;
}

HP_ErrorCode HP_PrintAllEntries(int fileDesc, char *attrName, void* value) {
  //insert code here

  return HP_OK;
}

HP_ErrorCode HP_GetEntry(int fileDesc, int rowId, Record *record) {
  //insert code here
  return HP_OK;
}
