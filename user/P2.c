/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "P2.h"

void main_P2() {
  while( 1 ) {
    write( STDOUT_FILENO, "P2", 2 );
    for(int i=0;i<1000000;i++){
      asm volatile("nop");
    }
  }

  exit( EXIT_SUCCESS );
}