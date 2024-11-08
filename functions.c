#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <conio.h>
#include <windows.h>
#include <unistd.h> 
#include "functions.h"





//                                        ABSTRACT MACHINE FUNCTIONS 

// Allocate a new block and initialize it
Block *AllocBlock(File *file) {
    Block *block = (Block*)malloc(sizeof(Block));
    if (!block) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }
    memset(block->data, 0, BLOCK_SIZE);
    block->Number_of_records = 0;
    block->Byte_Used = 0;
    file->header.Number_of_Blocks++;
    return block;
}

// Read a block from file
int readBlock(FILE *file, int blockNumber, Block *block) {
    fseek(file, sizeof(Header) + blockNumber * sizeof(Block), SEEK_SET);
    return fread(block, sizeof(Block), 1, file);
}

// Write a block to file
void writeBlock(FILE *file, int blockNumber, Block *block) {
    fseek(file, sizeof(Header) + blockNumber * sizeof(Block), SEEK_SET);
    fwrite(block, sizeof(Block), 1, file);
    printf("Block %d written with %d bytes used.\n", blockNumber, block->Byte_Used); 
}

// Set header in file
void setHeader(FILE *file, Header *header) {
    fseek(file, 0, SEEK_SET);
    fwrite(header, sizeof(Header), 1, file);
}

// Get header from file
Header getHeader(FILE *file) {
    Header header;
    fseek(file, 0, SEEK_SET);
    fread(&header, sizeof(Header), 1, file);
    return header;
}

// Convert a record to a delimited string
void Record_to_String(Record rec, char *s) {
    sprintf(s, "%d,%s,%s,%s,%d%s",
            rec.key, rec.First_Name, rec.Last_Name, rec.Description,
            rec.Eraser ? 1 : 0, DELIMITER);
   
}

// Convert a string to a Record
void String_to_Record(const char *s, Record *rec) {
    sscanf(s, "%d,%20[^,],%20[^,],%100[^,],%d", 
           &rec->key, rec->First_Name, rec->Last_Name,
           rec->Description, (int*)&rec->Eraser);
}


// Open or create a TOVS file
File *Open(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (!file && mode[0] == 'r') {
        file = fopen(filename, "wb+");
        if (file) {
            Header header = {0, 0};
            setHeader(file, &header);
            printCentered("File Created Successfully");
        }
        if(!file){
            printCentered("File was not created");
        }
    }
    File *FIle = (File*)malloc(sizeof(File));
    FIle->file = file;
    FIle->header = getHeader(file);
    printCentered("File is open");
    return FIle;
}

// Close the TOVS file
void Close(File *file) {
    setHeader(file->file, &file->header);  
    if (file->file) fclose(file->file);
    free(file);
}



//                                                        TNOVS FUNCTIONS 


void insertRecord_TnOVS(File *file, Record rec) {
    char recordStr[BLOCK_SIZE * 2]; 
    Record_to_String(rec, recordStr);
    int recordLen = strlen(recordStr); 

    if (recordLen > BLOCK_SIZE) {
        printf("Error: Record size exceeds block size.\n");
        return;
    }

    Block block;
    bool isDuplicate = false; 

    for (int blockNumber = 0; blockNumber < file->header.Number_of_Blocks; blockNumber++) {
        readBlock(file->file, blockNumber, &block); 

        char *token = strtok(block.data, DELIMITER);
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord); 

            if (!existingRecord.Eraser && existingRecord.key == rec.key) {
                isDuplicate = true;
                break; 
            }

            token = strtok(NULL, DELIMITER); 
        }
        if (isDuplicate) break; 
    }

    if (isDuplicate) {
        printf("Duplicate record with key %d found. Insertion skipped.\n", rec.key);
        return;
    }

    int blockNumber = file->header.Number_of_Blocks > 0 ? file->header.Number_of_Blocks - 1 : 0;
    readBlock(file->file, blockNumber, &block); 

    if (block.Byte_Used >= BLOCK_SIZE || file->header.Number_of_Blocks == 0) {
        blockNumber = file->header.Number_of_Blocks; 
        memset(&block, 0, sizeof(Block)); 
    }

    int remainingBytes = recordLen;
    char *recordPointer = recordStr;

    while (remainingBytes > 0) {
        int availableSpace = BLOCK_SIZE - block.Byte_Used;

        if (remainingBytes > availableSpace) {
            strncat(block.data, recordPointer, availableSpace);
            block.Byte_Used += availableSpace;
            block.Number_of_records++;
            writeBlock(file->file, blockNumber++, &block);

            memset(&block, 0, sizeof(Block));
            recordPointer += availableSpace;
            remainingBytes -= availableSpace;
        } else {
            strcat(block.data, recordPointer);
            block.Byte_Used += remainingBytes;
            block.Number_of_records++;
            remainingBytes = 0;
        }
    }

    writeBlock(file->file, blockNumber, &block);

    file->header.Number_of_Blocks = blockNumber + 1;
    file->header.Number_of_Records++;
    setHeader(file->file, &file->header); 
}

void initialLoad_TnOVS(File *file, int rate) {
    for (int i = 1; i <= rate; i++) {
        Record rec;
        rec.key = i;
        snprintf(rec.First_Name, sizeof(rec.First_Name), "First%d", i);
        snprintf(rec.Last_Name, sizeof(rec.Last_Name), "Last%d", i);
        snprintf(rec.Description, sizeof(rec.Description), "Record number %d", i);
        rec.Eraser = false;

        insertRecord_TnOVS(file, rec);
    }
    printf("Initial load is completed with %d records.\n", rate);
}

void Display_Header_TnOVS(File *file) {
    printf("The number of blocks here is  : %d \n", file->header.Number_of_Blocks);
    printf("The number of records here is  : %d \n", file->header.Number_of_Records);
}

void display_File_TnOVS(File *file) {
    Display_Header_TnOVS(file);
    Block block;
    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        readBlock(file->file, i, &block);
        char *token = strtok(block.data, DELIMITER);
        
        printf("Block %d:\n", i);
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);

            if (!existingRecord.Eraser) {
                printf("  %s\n", token);
            }
            token = strtok(NULL, DELIMITER);
        }
    }
}

void Display_block_TnOVS(File *file, int num_of_block) {
    Block block;
    readBlock(file->file, num_of_block, &block);
    char *token = strtok(block.data, DELIMITER);
    
    printf("Contents of Block %d:\n", num_of_block);
    while (token != NULL) {
        Record existingRecord;
        String_to_Record(token, &existingRecord);

        if (!existingRecord.Eraser) {
            printf("  %s\n", token);
        }
        token = strtok(NULL, DELIMITER);
    }
}

void display_Overlapping_TnOVS(File *file) {
    Block block;
    bool foundOverlap = false;

    for (int blockNumber = 0; blockNumber < file->header.Number_of_Blocks - 1; blockNumber++) {
        readBlock(file->file, blockNumber, &block);

        if (block.Byte_Used > 0 && block.data[block.Byte_Used - 1] != '\0') {
            foundOverlap = true;

            char *lastDelimiter = strrchr(block.data, DELIMITER[0]);
            if (lastDelimiter != NULL && (lastDelimiter - block.data) < BLOCK_SIZE - 1) {
                char partialRecordStr[BLOCK_SIZE + 1] = {0};
                strncpy(partialRecordStr, lastDelimiter + 1, BLOCK_SIZE - (lastDelimiter - block.data + 1));
                partialRecordStr[BLOCK_SIZE] = '\0'; 

                
                Record overlappingRecord;
                String_to_Record(partialRecordStr, &overlappingRecord);

                printf("Overlapping Record Detected: Key %d starts in Block %d and continues to Block %d.\n",
                       overlappingRecord.key, blockNumber, blockNumber + 1);
            } else {
                printf("Error: Could not find a valid starting point for the overlapping record.\n");
            }
        }
    }

    if (!foundOverlap) {
        printf("No overlapping records across blocks found in the entire file.\n");
    }
}

void search_TnOVS(File *file, int key) {
    Block block;
    bool found = 0 ;
    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        readBlock(file->file, i, &block);
        char *token = strtok(block.data, DELIMITER);

        int pos = 0;
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);

            if (!existingRecord.Eraser && existingRecord.key == key) {
                found = 1 ;
                printf("Record with key %d was found in Block %d at Position %d\n",key, i, pos+1);
            }

            token = strtok(NULL, DELIMITER);
            pos++;
        }
    }
    if(!found){
        printf("Record with key %d was not found.\n" ,key);         
    }
}

void logicalDelete_TnOVS(File *file, int key) {
    Block block;
    bool found = false;

    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        readBlock(file->file, i, &block);
        char *token = strtok(block.data, DELIMITER);
        char newBlockData[BLOCK_SIZE] = {0};
        int pos = 0;

        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);

            if (existingRecord.key == key && !existingRecord.Eraser) {
                // Logically delete the record by setting Eraser to true
                existingRecord.Eraser = true;
                found = true;
            } else {
                // Only append non-deleted records
                char recordStr[BLOCK_SIZE];
                Record_to_String(existingRecord, recordStr);
                if (pos > 0) strcat(newBlockData, DELIMITER);
                strcat(newBlockData, recordStr);
                pos++;
            }

            token = strtok(NULL, DELIMITER);
        }

        if (found) {
            // Update block data only once when a record is deleted
            strncpy(block.data, newBlockData, BLOCK_SIZE);
            writeBlock(file->file, i, &block);
            printf("Record with key %d logically deleted.\n", key);
            break; // Exit after deleting the record
        }
    }

    if (!found) {
        printf("Record with key %d not found.\n", key);
    }
}

void physicalDelete_TnOVS(File *file, int key) {
    Block block;
    bool found = false;

    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        readBlock(file->file, i, &block);
        char *token = strtok(block.data, DELIMITER);
        char newBlockData[BLOCK_SIZE] = {0};
        int pos = 0;

        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);

            // Skip this record entirely if it matches the key (physically deleting it)
            if (existingRecord.key == key) {
                found = true;
                token = strtok(NULL, DELIMITER);
                continue;
            }

            // Add non-deleted records back to `newBlockData`
            char recordStr[BLOCK_SIZE];
            Record_to_String(existingRecord, recordStr);
            if (pos > 0) strcat(newBlockData, DELIMITER);
            strcat(newBlockData, recordStr);
            pos++;

            token = strtok(NULL, DELIMITER);
        }

        if (found) {
            strncpy(block.data, newBlockData, BLOCK_SIZE);
            writeBlock(file->file, i, &block);
            printf("Record with key %d physically deleted.\n", key);
            break;
        }
    }

    if (!found) {
        printf("Record with key %d not found.\n", key);
    }
}

//                          TOVS FUNCTIONS : 


void insertRecord_TOVS(File *file, Record rec) {
    char recordStr[BLOCK_SIZE * 2];
    Record_to_String(rec, recordStr);
    int recordLen = strlen(recordStr);

    if (recordLen > BLOCK_SIZE) {
        printf("Error: Record size exceeds block size.\n");
        return;
    }

    Block block;
    bool isDuplicate = false;
    bool inserted = false;
    int blockNumberToInsert = -1;
    int positionToInsert = -1;

    for (int blockNumber = 0; blockNumber < file->header.Number_of_Blocks; blockNumber++) {
        readBlock(file->file, blockNumber, &block);

        char tempBlockData[BLOCK_SIZE];
        strcpy(tempBlockData, block.data);

        char *token = strtok(tempBlockData, DELIMITER);
        int position = 0;

        while (token != NULL) {
            Record existingRecord;
            sscanf(token, "%d,%20[^,],%20[^,],%100[^,],%d",
                   &existingRecord.key, existingRecord.First_Name,
                   existingRecord.Last_Name, existingRecord.Description,
                   (int *)&existingRecord.Eraser);

            if (!existingRecord.Eraser) {
                // Check for duplicate key
                if (existingRecord.key == rec.key) {
                    isDuplicate = true;
                    break;
                }

                // Find the correct position to insert based on the key
                if (existingRecord.key > rec.key && !inserted) {
                    // Mark this block and position for insertion
                    blockNumberToInsert = blockNumber;
                    positionToInsert = position;
                    inserted = true;
                }
            }

            position += strlen(token) + strlen(DELIMITER);
            token = strtok(NULL, DELIMITER);
        }

        if (isDuplicate) break;
    }

    if (isDuplicate) {
        printf("Duplicate record with key %d found. Insertion skipped.\n", rec.key);
        return;
    }

    if (inserted) {
        // Insert the record at the identified block and position
        readBlock(file->file, blockNumberToInsert, &block);
        char newData[BLOCK_SIZE * 2];

        // Split the block data to insert the new record in order
        strncpy(newData, block.data, positionToInsert);
        newData[positionToInsert] = '\0';
        strcat(newData, recordStr);
        strcat(newData, DELIMITER);
        strcat(newData, block.data + positionToInsert);

        // Update the block with the new ordered data
        strncpy(block.data, newData, BLOCK_SIZE);
        block.Byte_Used += recordLen + strlen(DELIMITER);
        block.Number_of_records++;
        writeBlock(file->file, blockNumberToInsert, &block);
    } else {
        // If the record is not inserted, append to a new block
        int blockNumber = file->header.Number_of_Blocks > 0 ? file->header.Number_of_Blocks - 1 : 0;
        readBlock(file->file, blockNumber, &block);

        if (block.Byte_Used >= BLOCK_SIZE || file->header.Number_of_Blocks == 0) {
            blockNumber = file->header.Number_of_Blocks;
            memset(&block, 0, sizeof(Block));
        }

        int remainingBytes = recordLen;
        char *recordPointer = recordStr;
        bool finalWrite = false;

        while (!finalWrite) {
            int availableSpace = BLOCK_SIZE - block.Byte_Used;

            if (remainingBytes > availableSpace) {
                strncat(block.data, recordPointer, availableSpace);
                block.Byte_Used += availableSpace;
                block.Number_of_records++;
                writeBlock(file->file, blockNumber++, &block);

                memset(&block, 0, sizeof(Block));
                recordPointer += availableSpace;
                remainingBytes -= availableSpace;
            } else {
                strcat(block.data, recordPointer);
                block.Byte_Used += remainingBytes;
                block.Number_of_records++;
                remainingBytes = 0;
                finalWrite = true;
            }
        }

        writeBlock(file->file, blockNumber, &block);

        file->header.Number_of_Blocks = blockNumber + 1;
        file->header.Number_of_Records++;
    }

    setHeader(file->file, &file->header);
}

void initialLoad_TOVS(File *file, int rate) {
    for (int i = 1; i <= rate; i++) {
        Record rec;
        rec.key = i;
        snprintf(rec.First_Name, sizeof(rec.First_Name), "First%d", i);
        snprintf(rec.Last_Name, sizeof(rec.Last_Name), "Last%d", i);
        snprintf(rec.Description, sizeof(rec.Description), "Record number %d", i);
        rec.Eraser = false;

        insertRecord_TOVS(file, rec);
    }
    printf("Initial load is completed with %d records.\n", rate);
}

void Display_Header_TOVS(File *file) {
    printf("The number of blocks here is  : %d \n", file->header.Number_of_Blocks);
    printf("The number of records here is  : %d \n", file->header.Number_of_Records);
}

void display_File_TOVS(File *file) {
    Display_Header_TOVS(file);
    Block block;
    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        readBlock(file->file, i, &block);
        char *token = strtok(block.data, DELIMITER);
        
        printf("Block %d:\n", i);
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);

            if (!existingRecord.Eraser) {
                printf("  %s\n", token);
            }
            token = strtok(NULL, DELIMITER);
        }
    }
}

void Display_block_TOVS(File *file, int num_of_block) {
    Block block;
    readBlock(file->file, num_of_block, &block);
    char *token = strtok(block.data, DELIMITER);
    
    printf("Contents of Block %d:\n", num_of_block);
    while (token != NULL) {
        Record existingRecord;
        String_to_Record(token, &existingRecord);

        if (!existingRecord.Eraser) {
            printf("  %s\n", token);
        }
        token = strtok(NULL, DELIMITER);
    }
}

void display_Overlapping_TOVS(File *file) {
    Block block;
    bool foundOverlap = false;

    for (int blockNumber = 0; blockNumber < file->header.Number_of_Blocks - 1; blockNumber++) {
        readBlock(file->file, blockNumber, &block);

        if (block.Byte_Used > 0 && block.data[block.Byte_Used - 1] != '\0') {
            foundOverlap = true;

            char *lastDelimiter = strrchr(block.data, DELIMITER[0]);
            if (lastDelimiter != NULL && (lastDelimiter - block.data) < BLOCK_SIZE - 1) {
                char partialRecordStr[BLOCK_SIZE + 1] = {0};
                strncpy(partialRecordStr, lastDelimiter + 1, BLOCK_SIZE - (lastDelimiter - block.data + 1));
                partialRecordStr[BLOCK_SIZE] = '\0'; 

                
                Record overlappingRecord;
                String_to_Record(partialRecordStr, &overlappingRecord);

                printf("Overlapping Record Detected: Key %d starts in Block %d and continues to Block %d.\n",
                       overlappingRecord.key, blockNumber, blockNumber + 1);
            } else {
                printf("Error: Could not find a valid starting point for the overlapping record.\n");
            }
        }
    }

    if (!foundOverlap) {
        printf("No overlapping records across blocks found in the entire file.\n");
    }
}

void search_TOVS(File *file, int key) {
    Block block;
    bool found = 0 ;
    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        readBlock(file->file, i, &block);
        char *token = strtok(block.data, DELIMITER);

        int pos = 0;
        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);

            if (!existingRecord.Eraser && existingRecord.key == key) {
                found = 1 ;
                printf("Record with key %d was found in Block %d at Position %d\n",key, i, pos+1);
            }

            token = strtok(NULL, DELIMITER);
            pos++;
        }
    }
    if(!found){
        printf("Record with key %d was not found.\n" ,key);         
    }
}

void logicalDelete_TOVS(File *file, int key) {
    Block block;
    bool found = false;

    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        readBlock(file->file, i, &block);
        char *token = strtok(block.data, DELIMITER);
        char newBlockData[BLOCK_SIZE] = {0};
        int pos = 0;

        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);

            if (existingRecord.key == key && !existingRecord.Eraser) {
                // Logically delete the record by setting Eraser to true
                existingRecord.Eraser = true;
                found = true;
            } else {
                // Only append non-deleted records
                char recordStr[BLOCK_SIZE];
                Record_to_String(existingRecord, recordStr);
                if (pos > 0) strcat(newBlockData, DELIMITER);
                strcat(newBlockData, recordStr);
                pos++;
            }

            token = strtok(NULL, DELIMITER);
        }

        if (found) {
            // Update block data only once when a record is deleted
            strncpy(block.data, newBlockData, BLOCK_SIZE);
            writeBlock(file->file, i, &block);
            printf("Record with key %d logically deleted.\n", key);
            break; // Exit after deleting the record
        }
    }

    if (!found) {
        printf("Record with key %d not found.\n", key);
    }
}

void physicalDelete_TOVS(File *file, int key) {
    Block block;
    bool found = false;

    for (int i = 0; i < file->header.Number_of_Blocks; i++) {
        readBlock(file->file, i, &block);
        char *token = strtok(block.data, DELIMITER);
        char newBlockData[BLOCK_SIZE] = {0};
        int pos = 0;

        while (token != NULL) {
            Record existingRecord;
            String_to_Record(token, &existingRecord);

            // Skip this record entirely if it matches the key (physically deleting it)
            if (existingRecord.key == key) {
                found = true;
                token = strtok(NULL, DELIMITER);
                continue;
            }

            // Add non-deleted records back to `newBlockData`
            char recordStr[BLOCK_SIZE];
            Record_to_String(existingRecord, recordStr);
            if (pos > 0) strcat(newBlockData, DELIMITER);
            strcat(newBlockData, recordStr);
            pos++;

            token = strtok(NULL, DELIMITER);
        }

        if (found) {
            strncpy(block.data, newBlockData, BLOCK_SIZE);
            writeBlock(file->file, i, &block);
            printf("Record with key %d physically deleted.\n", key);
            break;
        }
    }

    if (!found) {
        printf("Record with key %d not found.\n", key);
    }
}



//                          DISPLAY FUNCTIONS


void welcome (){
    system("cls");
    printf("\n\n");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),3);
    printf("     %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",201,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,203,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,187);
    printf("     %c .88d88b.  .d88888b    dP   %c     KAHLOUCHE YOUCEF   %c\n",186,186,186);
    printf("     %c 88.  `88  88.    \"'        %c                        %c\n",186,186,186);
    printf("     %c 888d888;  ~Y88888b.   88   %c     GROUPE  : 02       %c\n",186,186,186);
    printf("     %c 88.             `8b   88   %c     TP N:1             %c\n",186,186,186);
    printf("     %c  Y88888    Y88888P    dP   %c     TOVS TnOVS         %c\n",186,186,186);
    printf("     %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",204,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,202,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,185);
    printf("     %c   HIGHER   SCHOOL   OF   COMPUTER   SIENCE          %c\n",186,186);
    printf("     %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",200,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,188);
    printf("\n");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),7);
    printf("        THANKS FOR CHOSING OUR APPLICATION! ");
    printf(" \n\n        press ENTER to continue ! ");
    getchar();

}
//-------------------------------------------------------------------------------------------------------------------------------------------------

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetColor() {
    setColor(7);
}

void printCentered(const char *text) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    int textLength = strlen(text);
    int space = (consoleWidth - textLength) / 2;

    for (int i = 0; i < space; i++) {
        printf(" ");
    }
    printf("%s\n", text);
}

void display_TnOVS_Menu(int choice) {
    system("cls");
    printf("\033[32m");
    printCentered(" ____________________________________________ ");
    printCentered("|                                            |");
    printCentered("|                    MENU                    |");
    printCentered("|____________________________________________|");
    printf("\033[0m");
    printf("\n\n");
    const char *options[10] = {
        "1: Give an initial load",
        "2: Insert a Record",
        "3: Delete Logically a Record",
        "4: Delete Physically a Record",
        "5: Search about a Record",
        "6: Display the Header",
        "7: Display a Certain Block",
        "8: Display the Overlapping Records",
        "9: Display the Whole File",
        "0: Exit"
    };

    for (int i = 0; i < 10; i++) {
        if (i == choice) {
            setColor(4); // Yellow for highlighted option
            printCentered(options[i]);
            resetColor();
        } else {
            printCentered(options[i]);
        }
    }
}

void display_Main_Menu (int choice) {
    system("cls");
    printf("\033[31m");
    printCentered(" ____________________________________________ ");
    printCentered("|                                            |");
    printCentered("|               MAIN MENU                    |");
    printCentered("|____________________________________________|");
    printf("\033[0m");
    printf("\n\n");
    const char *options[3] = {
        "1: TOVS",
        "2: TNOVS",
        "0: Exit"
    };

    for (int i = 0; i < 3; i++) {
        if (i == choice) {
            setColor(6); // Yellow for highlighted option
            printCentered(options[i]);
            resetColor();
        } else {
            printCentered(options[i]);
        }
    }
}

void Before(char *filename) {
    printf("\033[31m");
    printCentered("______________________________________________________________");
    printCentered("|                                                             |");
    printCentered("|                                                             |");
    printCentered("|        BEFORE WE START YOU SHOULD ENTER THE FILE NAME       |");
    printCentered("|                                                             |");
    printCentered("|                                                             |");
    printCentered("|_____________________________________________________________|\n");
    printf("\033[0m");

    printf("\033[3A\033[50C");  

    scanf("%19s", filename);  

    printf("\n\n\n");
}



//        MAIN TNOVS FUNCTION 


void TnOVS(){
    int key   ;
    int choice = 0; 
    char filename[24];  
    Before(filename);

    strcat(filename, ".dat");

    File *tnovsFile = Open(filename, "rb+");
    sleep(3);
    
    while (1) {
        display_TnOVS_Menu(choice);

        key = getch();

        if (key == 224) {
            key = getch();
            switch (key) {
                case 72: 
                    choice = (choice - 1 + 10) % 10; 
                    break;
                case 80: 
                    choice = (choice + 1) % 10; 
                    break;
            }
        } else if (key == 13) { 
            switch (choice) {
                                    
                case 0:
                    system("cls");
                    int rate;
                    printCentered("Enter the number of records that you want to insert as an initial load : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &rate);
                    initialLoad_TnOVS(tnovsFile ,rate);
                    break;
                case 1:
                    system("cls");
                    Record rec ;
                    printCentered("Enter the informations of the record that you want to insert : \n");
                    printCentered("key : ");
                    printf("\033[1A\033[65C"); 
                    scanf("%d", &rec.key);
                    printCentered("  First Name : ");
                    printf("\033[0A\033[66C"); 
                    scanf("%20s",&rec.First_Name);
                    printCentered("  Last Name : ");
                    printf("\033[0A\033[66C"); 
                    scanf("%20s",&rec.Last_Name);
                    printCentered("  Description : ");
                    printf("\033[0A\033[68C"); 
                    scanf("%100s",&rec.Description);
                    rec.Eraser=false ;
                    insertRecord_TnOVS(tnovsFile , rec);
                    break;
                case 2:
                    int reckey ;
                    system("cls");
                    printCentered("Enter the key of the record that you want to logically delete : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &reckey);
                    logicalDelete_TnOVS(tnovsFile ,reckey);
                    break;
                case 3:
                    int reckey2 ;
                    system("cls");
                    printCentered("Enter the key of the record that you want to phisically delete : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &reckey2);
                    physicalDelete_TnOVS(tnovsFile ,reckey2);
                    break;
                case 4:
                    int reckey3 ;
                    system("cls");
                    printCentered("Enter the key of the record that you want to Search about : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &reckey3);
                    search_TnOVS(tnovsFile ,reckey3);
                    break;
                case 5:
                    system("cls");
                    printCentered("These are the informations of your header : \n");
                    Display_Header_TnOVS(tnovsFile);
                    break;
                case 6:
                    int Blocknum ;
                    system("cls");
                    printCentered("Enter the Number of the block that you want to display it's content : \n");
                    scanf("%d" , &Blocknum );
                    Display_block_TnOVS(tnovsFile , Blocknum);
                    break;
                case 7:
                    system("cls");
                    display_Overlapping_TnOVS(tnovsFile);
                    break;
                case 8:
                    system("cls");
                    display_File_TnOVS(tnovsFile);
                    break;
                case 9:
                    exit(0);
                    break ;
                default:
                    printf("Invalid option\n");
                    break;
            }
            printf("Press any key to continue...\n");
            getch(); 
        }
    }
}


//          MAIN TOVS FUNSTION 


void TOVS(){
    int key   ;
    int choice = 0; 
    char filename[24];  
    Before(filename);

    strcat(filename, ".dat");

    File *tovsFile = Open(filename, "rb+");
    sleep(3);
    
    while (1) {
        display_TnOVS_Menu(choice);

        key = getch();

        if (key == 224) {
            key = getch();
            switch (key) {
                case 72: 
                    choice = (choice - 1 + 10) % 10; 
                    break;
                case 80: 
                    choice = (choice + 1) % 10; 
                    break;
            }
        } else if (key == 13) { 
            switch (choice) {
                                    
                case 0:
                    system("cls");
                    int rate;
                    printCentered("Enter the number of records that you want to insert as an initial load : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &rate);
                    initialLoad_TOVS(tovsFile ,rate);
                    break;
                case 1:
                    system("cls");
                    Record rec ;
                    printCentered("Enter the informations of the record that you want to insert : \n");
                    printCentered("key : ");
                    printf("\033[1A\033[65C"); 
                    scanf("%d", &rec.key);
                    printCentered("  First Name : ");
                    printf("\033[0A\033[66C"); 
                    scanf("%20s",&rec.First_Name);
                    printCentered("  Last Name : ");
                    printf("\033[0A\033[66C"); 
                    scanf("%20s",&rec.Last_Name);
                    printCentered("  Description : ");
                    printf("\033[0A\033[68C"); 
                    scanf("%100s",&rec.Description);
                    rec.Eraser=false ;
                    insertRecord_TOVS(tovsFile , rec);
                    break;
                case 2:
                    int reckey ;
                    system("cls");
                    printCentered("Enter the key of the record that you want to logically delete : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &reckey);
                    logicalDelete_TOVS(tovsFile ,reckey);
                    break;
                case 3:
                    int reckey2 ;
                    system("cls");
                    printCentered("Enter the key of the record that you want to phisically delete : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &reckey2);
                    physicalDelete_TOVS(tovsFile ,reckey2);
                    break;
                case 4:
                    int reckey3 ;
                    system("cls");
                    printCentered("Enter the key of the record that you want to Search about : \n");
                    printf("\033[1A\033[63C"); 
                    scanf("%d" , &reckey3);
                    search_TOVS(tovsFile ,reckey3);
                    break;
                case 5:
                    system("cls");
                    printCentered("These are the informations of your header : \n");
                    Display_Header_TOVS(tovsFile);
                    break;
                case 6:
                    int Blocknum ;
                    system("cls");
                    printCentered("Enter the Number of the block that you want to display it's content : \n");
                    scanf("%d" , &Blocknum );
                    Display_block_TOVS(tovsFile , Blocknum);
                    break;
                case 7:
                    system("cls");
                    display_Overlapping_TOVS(tovsFile);
                    break;
                case 8:
                    system("cls");
                    display_File_TOVS(tovsFile);
                    break;
                case 9:
                    exit(0);
                    break ;
                default:
                    printf("Invalid option\n");
                    break;
            }
            printf("Press any key to continue...\n");
            getch(); 
        }
    }
}



