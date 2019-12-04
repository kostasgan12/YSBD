#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "AM.h"
#include "bf.h"
#include "defn.h"

#define MAX_RECORDS_PER_BLOCK BF_BLOCK_SIZE/sizeof(Record)

#define CALL_BF(call)         \
{                             \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      return AME_ERROR;        \
    }                         \
}

int AM_errno = AME_OK;

struct File {					//Πληροφορίες για το κάθε αρχείο που θα χρειαστούμε.
    int fileDesc;				//Η διεύθυνση του αρχείου.
    char fileName[30];			//Το όνομα του αρχείου.
    char attrType1;				//Ο τύπος του πρώτου πεδίου.
    int attrLength1;			//Το μήκος του πρώτου πεδίου.
    char attrType2;				//Ο τύπος του δεύτερου πεδίου.
    int attrLength2;			//Το μήκος του δεύτερου πεδίου.

};


struct File filesArray[MAX_OPEN_FILES];

int AM_Init() {


    CALL_BF(BF_Init(LRU));

    for (int i = 0; i < MAX_OPEN_FILES; i++){
        filesArray[i].fileDesc = -1;
        strcpy(filesArray[i].fileName, "");
        filesArray[i].attrType1 = ' ';
        filesArray[i].attrLength1 = -1;
        filesArray[i].attrType2 = ' ';
        filesArray[i].attrLength2 = -1;
    }

    return 0;
}


int AM_CreateIndex(char *fileName,
                   char attrType1,
                   int attrLength1,
                   char attrType2,
                   int attrLength2) {

    CALL_BF(BF_CreateFile(fileName));

    int fileDesc;

    BF_Block *block;
    BF_Block_Init(&block);

    CALL_BF(BF_OpenFile(fileName, &fileDesc));		//Ανοίγουμε το αρχείο που δημιουργήσαμε.

    char* data;
    char* metadata = "AMFile";

    CALL_BF(BF_AllocateBlock(fileDesc, block));		//Φτιάχνουμε το μπλοκ που θα καταχωρήσουμε τις πληροφορίες του αρχείου.
    data = BF_Block_GetData(block);

    memcpy(data, metadata, sizeof(metadata));		//Καταχωρούμε τις απαραίτητες πληροφορίες στο πρώτο μπλοκ.
    memcpy(data + sizeof(metadata), &attrType1, sizeof(char));
    memcpy(data + sizeof(metadata) + sizeof(char), &attrLength1, sizeof(int));
    memcpy(data + sizeof(metadata) + sizeof(char) + sizeof(int), &attrType2, sizeof(char));
    memcpy(data + sizeof(metadata) + 2 * sizeof(char) + sizeof(int), &attrLength2, sizeof(int));

    BF_Block_SetDirty(block);

    CALL_BF(BF_UnpinBlock(block));

    BF_Block_Destroy(&block);

    CALL_BF(BF_CloseFile(fileDesc));

    return AME_OK;
}


int AM_DestroyIndex(char *fileName) {

    int found = 0;											//Flag για την εύρεση ανοιχτού αρχείου.

    for (int i = 0; i < MAX_OPEN_FILES; i++){				//Ψάχνουμε όλο τον πίνακα για το αρχείο.
        if (strcmp(filesArray[i].fileName, fileName) == 0){ //Αν το βρούμε, σταματάμε την αναζήτηση.
            found = 1;
            break;
        }
    }

    if(found == 0){											//Αν το flag είναι 0 σημαίνει ότι το αρχείο δεν
        remove(fileName);									//είναι ανοιχτό άρα μπορεί να διαγραφεί.
        return AME_OK;
    }
    else{													//Αλλιώς εμφανίζουμε ένα μήνυμα λάθους και
        printf("The file is open! Cannot delete!\n");		//επιστρέφουμε τον κωδικό λάθους.
        return AME_ERROR;
    }


}


int AM_OpenIndex (char *fileName) {

    int fileDesc;
    int capacity = 0;										//Για τον έλεγχο της χωρητικότητας του πίνακα.
    int desc;

    CALL_BF(BF_OpenFile(fileName, &fileDesc));

    for(desc = 0; desc < MAX_OPEN_FILES; desc++){           //Ελέγχω αν ο πίνακας μου είναι πλήρης.
        if(filesArray[desc].fileDesc == -1){               	//Αν δεν είναι κάνω εισαγωγή του νέου αρχείου.
            filesArray[desc].fileDesc = fileDesc;
            break;
        }
        else{
            capacity++;
        }
    }

    if(capacity == MAX_OPEN_FILES){
        printf("Maximum capacity reached!\n");              //Αλλιώς εμφανίζω Error.
        return AME_ERROR;
    }

    BF_Block *block;
    BF_Block_Init(&block);

    char* data;
    char* metadata = "AMFile";
    char metadataCheck[8];

    CALL_BF(BF_GetBlock(fileDesc, 0, block));
    data = BF_Block_GetData(block);
    memcpy(metadataCheck, data, sizeof(metadata));

    if (strcmp(metadataCheck, metadata) != 0){              //Ελέγχω αν το αρχείο είναι αρχείο ευρετηρίου.
        printf("Error: File %s is not a hash file\n", fileName);
        return AME_ERROR;
    }
    //Γράφω στον πίνακα τα απαραίτητα στοιχεία.
    memcpy(&filesArray[desc].attrType1, data + sizeof(metadata), sizeof(char));
    memcpy(&filesArray[desc].attrLength1, data + sizeof(metadata) + sizeof(char), sizeof(int));
    memcpy(&filesArray[desc].attrType2, data + sizeof(metadata) + sizeof(char) + sizeof(int), sizeof(char));
    memcpy(&filesArray[desc].attrLength2, data + sizeof(metadata) + 2 * sizeof(char) + sizeof(int), sizeof(int));

    CALL_BF(BF_UnpinBlock(block));

    BF_Block_Destroy(&block);

    return desc;
}


int AM_CloseIndex (int fileDesc) {
    return AME_OK;
}


int AM_InsertEntry(int fileDesc, void *value1, void *value2) {
    return AME_OK;
}


int AM_OpenIndexScan(int fileDesc, int op, void *value) {
    return AME_OK;
}


void *AM_FindNextEntry(int scanDesc) {

}


int AM_CloseIndexScan(int scanDesc) {
    return AME_OK;
}


void AM_PrintError(char *errString) {

    if( AM_errno == AM_OPEN_FILE ){
        printf("ERROR (%d): Something went wrong while opening a file!\n", AM_errno);
    }
    else if( AM_errno == AM_OPEN_SCAN ){
        printf("ERROR (%d): Something went wrong while opening a scan file!\n", AM_errno);
    }
    else if( AM_errno == AM_CLOSE_FILE ){
        printf("ERROR (%d): Trying to close a file while scanning!\n", AM_errno);
    }
}

void AM_Close() {

    BF_Close();
}