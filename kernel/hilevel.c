/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"
#include <stdlib.h>

/* We assume there will be two user processes, stemming from execution of the 
 * two user programs P1 and P2, and can therefore
 * 
 * - allocate a fixed-size process table (of PCBs), and then maintain an index 
 *   into it to keep track of the currently executing process, and
 * - employ a fixed-case of round-robin scheduling: no more processes can be 
 *   created, and neither can be terminated, so assume both are always ready
 *   to execute.
 */

pcb_t* procTab[ PROC_TABLE_SIZE ]; pcb_t* executing = NULL;

void dispatch( ctx_t* ctx, pcb_t* prev, pcb_t* next ) {
  char prev_pid = '?', next_pid = '?';

  if( NULL != prev ) {
    memcpy( &prev->ctx, ctx, sizeof( ctx_t ) ); // preserve execution context of P_{prev}
    prev_pid = '0' + prev->pid;
  }
  if( NULL != next ) {
    memcpy( ctx, &next->ctx, sizeof( ctx_t ) ); // restore  execution context of P_{next}
    next_pid = '0' + next->pid;
  }

    PL011_putc( UART0, '[',      true );
    PL011_putc( UART0, prev_pid, true );
    PL011_putc( UART0, '-',      true );
    PL011_putc( UART0, '>',      true );
    PL011_putc( UART0, next_pid, true );
    PL011_putc( UART0, ']',      true );

    executing = next;                           // update   executing process to P_{next}

  return;
}

void increasePriorities(ctx_t* ctx){
  for(int i=0;i<PROC_TABLE_SIZE;i++){
    if(procTab[ i]->status == STATUS_READY){
      procTab[i]->priority++;
    }
  }
  return;
}

int CURRENT_PROCS=0;

void schedule( ctx_t* ctx ) {
  
  //schedule next only if process ready
 pcb_t* highestProc;
 int highestPrio=-1;
  

  

  for(int i=0;i<CURRENT_PROCS;i++){     //loop through whole process table
    if((procTab[i]->status==STATUS_READY)&&(procTab[i]->priority>highestPrio)){    //check if each process is ready to execute and if it's priority is higher than the previous highest
      highestProc=procTab[i];  //update highest priority process
      highestPrio=procTab[i]->priority; //update highest priority
    }
  }

  if(executing->status==STATUS_TERMINATED){
    highestProc->status=STATUS_EXECUTING;
    dispatch(ctx,executing,highestProc);
  }
  else if(highestPrio>executing->priority){
    executing->status=STATUS_READY;
    highestProc->status=STATUS_EXECUTING;
    dispatch(ctx,executing,highestProc);
  }

  



  return;  //current process continues to execute is there is no other ready process
}

extern void     main_P1(); 
extern uint32_t tos_P1;
extern void     main_P2(); 
extern uint32_t tos_P2;

extern void main_console();
extern uint32_t tos_console;


pcb_t *procTab[PROC_TABLE_SIZE];

void hilevel_handler_rst( ctx_t* ctx              ) { 
  //enable timer


  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

 





  /* Invalidate all entries in the process table, so it's clear they are not
   * representing valid (i.e., active) processes.
   */

 

  /* Automatically execute the user programs P1 and P2 by setting the fields
   * in two associated PCBs.  Note in each case that
   *    
   * - the CPSR value of 0x50 means the processor is switched into USR mode, 
   *   with IRQ interrupts enabled, and
   * - the PC and SP values match the entry point and top of stack. 
   */



 

//initialise console process table entry


procTab[0]=(pcb_t*)malloc(sizeof(pcb_t));


procTab[ 0 ]->status = STATUS_INVALID;




//memset(&procTab[0]->ctx, 0, sizeof(ctx_t) ); // initialise 0-th PCB = P_1

  procTab[ 0 ]->pid      = 0;
  procTab[ 0 ]->status   = STATUS_EXECUTING;
  procTab[ 0 ]->tos      = ( uint32_t )( &tos_console  );
  procTab[0]->priority=0;
  procTab[ 0 ]->ctx.cpsr = 0x50;
  procTab[ 0 ]->ctx.pc   = ( uint32_t )( &main_console );
  procTab[ 0 ]->ctx.sp   = procTab[ 0 ]->tos;


  CURRENT_PROCS++;





  /* Once the PCBs are initialised, we arbitrarily select the 0-th PCB to be 
   * executed: there is no need to preserve the execution context, since it 
   * is invalid on reset (i.e., no process was previously executing).
   */
 int_enable_irq();
  dispatch( ctx, NULL, procTab[ 0 ] );

  return;



















}


void hilevel_handler_irq(ctx_t* ctx) {
  // Step 2: read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;

  // Step 4: handle the interrupt, then clear (or reset) the source.

  if( id == GIC_SOURCE_TIMER0 ) {
    PL011_putc( UART0, 'T', true ); 
    increasePriorities(ctx);
    schedule(ctx);
    PL011_putc( UART0, 'T', true );
    TIMER0->Timer1IntClr = 0x01;
    
  }

  // Step 5: write the interrupt identifier to signal we're done.

  GICC0->EOIR = id;

  return;
}


void hilevel_handler_svc( ctx_t* ctx, uint32_t id ) { 
  /* Based on the identifier (i.e., the immediate operand) extracted from the
   * svc instruction, 
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call, then
   * - write any return value back to preserved usr mode registers.
   */

  switch( id ) {
    case 0x00 : { // 0x00 => yield()
      schedule( ctx );

      break;
    }

    case 0x01 : { // 0x01 => write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );  
      char*  x = ( char* )( ctx->gpr[ 1 ] );  
      int    n = ( int   )( ctx->gpr[ 2 ] ); 

      for( int i = 0; i < n; i++ ) {
        PL011_putc( UART0, *x++, true );
      }
      
      ctx->gpr[ 0 ] = n;

      break;
    }


    case 0x02:{  //read



    }

    case 0x03:{  //fork
      //realloc(&procTab,CURRENT_PROCS+1);
      procTab[CURRENT_PROCS]=(pcb_t*)malloc(sizeof(pcb_t));  
      memcpy(procTab[CURRENT_PROCS],executing,sizeof(pcb_t));
      procTab[CURRENT_PROCS]->status=STATUS_READY;
      memcpy(&procTab[CURRENT_PROCS]->ctx,ctx,sizeof(ctx_t));
      procTab[CURRENT_PROCS]->ctx.gpr[0]=0;
      procTab[CURRENT_PROCS]->ctx.sp= (procTab[CURRENT_PROCS-1]->ctx.sp)-0x00001000;
      ctx->gpr[0]=CURRENT_PROCS;
      procTab[CURRENT_PROCS]->pid=CURRENT_PROCS;
      CURRENT_PROCS++;
      break;
    }

    case 0x04:{ //exit

      executing->status=STATUS_TERMINATED;
      schedule(ctx);
      break;
    }

    case 0x05:{//exec
      ctx->pc=ctx->gpr[0];
      break;

    }

    case 0x06:{}

    case 0x07:{}

    


    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}