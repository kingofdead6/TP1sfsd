#ifndef FUNCTIONS_H
#define FUNCTIONS_H


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <windows.h>
#include <conio.h>

#ifdef _WIN32
    #define CLEAR_SCREEN "cls"
#else
    #define CLEAR_SCREEN "clear"
#endif


#define BLOCK_SIZE 256
#define DELIMITER "|"

// Record structure
typedef struct {
    int key;
    char First_Name[20];
    char Last_Name[20];
    char Description[100];
    bool Eraser;
} Record;

// Block structure
typedef struct {
    char data[BLOCK_SIZE];
    int Byte_Used;
    int Number_of_records;
} Block;

// Header structure
typedef struct {
    int Number_of_Blocks;
    int Number_of_Records;
} Header;

// File structure 
typedef struct {
    FILE *file;
    Header header;
} File;


//                   ABSTRACT MACHINE 


Block *AllocBlock(File *file) ;
int readBlock(FILE *file, int blockNumber, Block *block);
void writeBlock(FILE *file, int blockNumber, Block *block);
void setHeader(FILE *file, Header *header);
Header getHeader(FILE *file);
void Record_to_String(Record rec, char *s);
void String_to_Record(const char *s, Record *rec);
File *Open(const char *filename, const char *mode);
void Close(File *file) ;

//                TNOVS FUNCTIONS

void insertRecord_TnOVS(File *file, Record rec);
void initialLoad_TnOVS(File *file, int rate);
void Display_Header_TnOVS(File *file);
void display_File_TnOVS(File *file) ;
void Display_block_TnOVS(File *file, int num_of_block);
void display_Overlapping_TnOVS(File *file);
void search_TnOVS(File *file, int key);
void logicalDelete_TnOVS(File *file, int key);
void physicalDelete_TnOVS(File *file, int key);


//                TOVS FUNCTIONS :


void initialLoad_TOVS(File *file, int rate);
void Display_Header_TOVS(File *file);
void display_File_TOVS(File *file) ;
void Display_block_TOVS(File *file, int num_of_block);
void display_Overlapping_TOVS(File *file);
void search_TOVS(File *file, int key);
void logicalDelete_TOVS(File *file, int key);
void physicalDelete_TOVS(File *file, int key);



//                 DISPLAY FUNCTIONS 
void welcome ();
void setColor(int color);
void resetColor();
void printCentered(const char *text);
void display_TnOVS_Menu(int choice) ;
void display_Main_Menu (int choice) ;
void Before(char *filename) ;

//              MAIN TNOVS FUNCTION  
void TnOVS();
void TOVS();

#endif