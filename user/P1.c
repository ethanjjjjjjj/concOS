/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "P1.h"
#include "libc.h"
#include <stdlib.h>
void main_P1() {
  pipePointers *p =pipe(); 
  int pid=fork();
  
  
   
  
  if(pid!=0){

    char *x=(char*)malloc(41*4);
    read((uint32_t)p,x,21);
    write(STDOUT_FILENO,"read:",6);
    write(STDOUT_FILENO,x,41);
  }
  else if(pid==0){
    //write(STDOUT_FILENO,"writing",8);
    write(STDOUT_FILENO,"written P1",11);
    write((uint32_t)p,"P1P1P1P1P1P1P1P1P1P1P1P1P1P1P1P1P1P1P1P1", 41 );
  }
  

  exit( EXIT_SUCCESS );}
