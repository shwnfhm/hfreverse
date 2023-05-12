//================================================
//
// Shawn Fahimi
//
// Project 1 - HF-reverse
// Operating Systems
//
//================================================

//================================================
// includes and defines
//================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

//================================================
// function prototypes
//================================================

//================================================
// helper function that takes in a string (str),
// reverses it, and returns the reversed string.
// used to reverse each line of the input file as it
// is read in backwards (rightmost character
// to leftmost character) from the end of the
// input file towards the beginning of the file.
//================================================
char *strrev(char *str);

//================================================
// reads a single line from a FILE (inp) into a string 
// (str), assuming that the FILE pointer starts 
// at the end of the file on the first function
// call, and reads characters from the end of the
// file towards the start of the file, until a newline 
// is hit, thereby storing an entire line in str. 
// Parameter 'size' is used to keep track of the 
// size of the dynamically allocated string (str).
// Parameter (char **str) is NOT an array of strings,
// rather it is a pointer to a single string, so that
// successful reallocation using realloc can be done
// if necessary, as we cannot assume the length of
// file lines.
// returns 0 if a line is successfully retrieved
// returns -1 if otherwise or realloc fails
//================================================
int reverseGets(char **str, FILE *inp, int *size);

//================================================
// outputs the lines in the input FILE (inp) into
// the output FILE (outp) in reverse order by line
//================================================
void processFiles(FILE *inp, FILE *outp);

char* strrev(char *str){

    int i;  //index for traversing string
    char temp;  //temp for holding characters while swapping
    int len = strlen(str); //length of string (str)
    
    //only need to process the first half of the string
    //in order to reverse it
    for (i = 0; i < len/2; i++)  {  

        //swap characters at positions i and (len - i - 1)
        temp = str[i];  
        str[i] = str[len - i - 1];  
        str[len - i - 1] = temp;  

    }
    return str;  
}

//NOTE: char **str is NOT an array of strings,
//rather, it is a pointer to a single string,
//so that calls to realloc can be made 
//without causing malloc errors when memory
//gets freed
int reverseGets(char **str, FILE *inp, int *size){

    //checks if we are at the start of the file
    if(ftell(inp) == 0){
      return -1;  //no line retrieved
    }

    int i = 0;  //keeps track of the number of characters read 
  
    //while we haven't hit the beginning ("end") of the input file
    while(ftell(inp) != 0){
      
      //move back one character
      fseek(inp, -1, SEEK_CUR);

      //if the number of characters we have read exceeds the size
      //of the dynamically allocated string
      if(i >= *size){

        //reallocate memory and update string size variable
        *size = (*size)*2;
        char* temp = realloc(*str, (*size)*2);
        

        //check if realloc failed
        if(temp == NULL){
          return -1;
        }
        *str = temp;
      }
      
      //get the current character in the file
      //being pointed to
      char c = (char)fgetc(inp);

      //place this character in the string
      (*str)[i] = c;

      //checks if we have hit the end of the previous line
      if(c == '\n'){
        break;
      }

      //move back the file pointer
      //back by one byte, since fgetc
      //advances the file pointer
      fseek(inp, -1, SEEK_CUR);

      //increment the number of characters read
      i++;
    }

    //undo the most recent fgetc call
    //by moving back the file pointer
    //1 byte
    fseek(inp, -1, SEEK_CUR);

    //null terminate the retrieved string
    (*str)[i] = '\0';

    //reverse the string's characters using a helper function,
    //as we are traversing backwards in the file
    *str = strrev(*str);
    return 0;
}

void processFiles(FILE *inp, FILE *outp){

    int size = 1024;  //initial string size
    fseek(inp, 0, SEEK_END);  //seek to the end ("start") of the input file
    int check = 0;  //used to determine if we are reading trailing/empty newlines
    char *str = (char *)malloc(size); //dynamically allocate string of size 'size'
    
    if(str == NULL){  //check for malloc fail
        fprintf(stderr, "malloc failed\n");
        fclose(inp);
        fclose(outp);
        exit(1);
    }

    //while lines are getting successfully
    //read from FILE inp into string str
    while(reverseGets(&str, inp, &size) >= 0){ 

      //if we are reading a trailing newline
      if((strcmp(str, "") == 0) && check == 0){
        continue;
      }

      //otherwise, we have hit the first meaningful line
      check = 1;

      //print the line to the output FILE (outp)
      fprintf(outp, "%s\n", str);

      //free and reallocate the dynamically
      //allocated string
      free(str);
      str = NULL;
      size = 1024;
      str = (char *)malloc(size);

      //check if malloc failed
      if(str == NULL){
        fprintf(stderr, "error: malloc failed\n");
        fclose(inp);
        fclose(outp);
        exit(1);
      }
    }

    //check if we exited the while loop due to a 
    //realloc error in reverseGets
    if(str == NULL){
      fprintf(stderr, "error: realloc failed\n");
      fclose(inp);
      fclose(outp);
      exit(1);
    }

    //free dynamically allocated memory
    str = NULL;
    free(str);
    
}

//================================================
// main function
//================================================
int main(int argc, char *argv[]){

  // ./hfreverse <input file>
  if(argc == 2){
    FILE *ip;

    //making sure we are not reading/writing from/to
    //the same file 
    if(strcmp(argv[1], "stdout") == 0){
      fprintf(stderr, "error: input and output file must differ\n");
      exit(1);
    }

    //if we can open the provided input file...
    if((ip = (fopen(argv[1], "r")))){

      //call processFiles to print lines 
      //in FILE ip to standard output, in 
      //reverse order by line
      processFiles(ip, stdout);

      //close the input file
      fclose(ip);
    }

    // if we could not open the provided input file
    else{

      //print error message to stderr
      fprintf(stderr, "error: cannot open file \'%s\'\n", argv[1]);
      exit(1);
    }
  }

  // ./hfreverse <input file> <output file>
  else if(argc == 3){
    FILE *ip, *opcheck, *op;  //input file, output check file (to use for hard-link checks), and output file
    int fd1, fd2; //used to store fd numbers of FILE *ip, *opcheck
    struct stat file1_stat, file2_stat; //used to check for hard-linked files
    
    // if we are able to open the provided input file
    if((ip = (fopen(argv[1], "r")))){
    
      //first use opcheck to open the output file
      //so that if the files are hard-linked, the
      //output file isn't truncated
      if((opcheck = (fopen(argv[2], "a+")))){

        //check if the files are hardlinked
        fd1 = fileno(ip);
        fd2 = fileno(opcheck);
        fstat(fd1, &file1_stat);
        fstat(fd2, &file2_stat);
        if(file1_stat.st_ino == file2_stat.st_ino){

          //print error to stderr
          fprintf(stderr, "error: input and output file must differ\n");

          //close files and exit
          fclose(ip);
          fclose(opcheck);
          exit(1);
        }
        else{ 

          //close output check file
          fclose(opcheck);

          //try to open the output file for writing
          if((op = (fopen(argv[2], "w+")))){

            //call processFiles to print the lines 
            //present in FILE ip to FILE op, in 
            //reverse order by line
            processFiles(ip, op);

            //close files
            fclose(ip);
            fclose(op);
          }
          else{
            //print error message to stderr
            fprintf(stderr, "error: cannot open file \'%s\'\n", argv[2]);
            exit(1);
          }
        }
      }
      else{

        //print error message to stderr
        fprintf(stderr, "error: cannot open file \'%s\'\n", argv[2]);
        exit(1);
      }
    }

    // if we cannot open the input file
    else{

      //print error message to stderr
      fprintf(stderr, "error: cannot open file \'%s\'\n", argv[1]);
      exit(1);
    }
  }

  // improper usage (too many or too few arguments)
  else{

    //print error message to stderr
    fprintf(stderr, "usage: hfreverse <input> <output>\n");
    exit(1);
  }
  return(0);
}