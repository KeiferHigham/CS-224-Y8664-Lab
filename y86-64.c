#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

const int MAX_MEM_SIZE  = (1 << 13);

void fetchStage(int *icode, int *ifun, int *rA, int *rB, wordType *valC, wordType *valP) {
 
 wordType pc = getPC();
 byteType byte = getByteFromMemory(pc);
 *icode = (byte >> 4) & 0xf;
 *ifun = byte & 0xf;

 if(*icode == HALT) {
   *valP = pc + 1;
   setStatus(STAT_HLT);

 }
 if(*icode == NOP) {
   *valP = pc + 1;
 }
 if(*icode == IRMOVQ) {
   byte = getByteFromMemory(pc + 1);
   *rA = (byte >> 4) & 0xf;
   *rB = byte & 0xf;
   *valC = getWordFromMemory(pc + 2);
   *valP = pc + 10;
 }
 if(*icode == RRMOVQ) {
   byte = getByteFromMemory(pc + 1);
   *rA = (byte >> 4) & 0xf;
   *rB = byte & 0xf;
   *valP = pc + 2;
 }
 if(*icode == RMMOVQ) {
   byte = getByteFromMemory(pc + 1);
   *rA = (byte >> 4) & 0xf;
   *rB = byte & 0xf;
   *valC = getWordFromMemory(pc + 2);
   *valP = pc + 10;
 }
 if(*icode == MRMOVQ) {
   byte = getByteFromMemory(pc + 1);
   *rA = (byte >> 4) & 0xf;
   *rB = byte & 0xf;
   *valC = getWordFromMemory(pc + 2);
   *valP = pc + 10;
 }
 if(*icode == OPQ) {
   byte = getByteFromMemory(pc + 1);
   *rA = (byte >> 4) & 0xf;
   *rB = byte & 0xf;
   *valP = pc + 2;
 }
 if(*icode == JXX) {
   *valC = getWordFromMemory(pc + 1);
   *valP = pc + 9;

 }
 if(*icode == CALL) {
   *valC = getWordFromMemory(pc + 1);
   *valP = pc + 9;
 }
 if(*icode == PUSHQ) {
   byte = getByteFromMemory(pc + 1);
   *rA = (byte >> 4) & 0xf;
   *rB = byte & 0xf;
   *valP = pc + 2;
 }
 if(*icode == POPQ) {
   byte = getByteFromMemory(pc + 1);
   *rA = (byte >> 4) & 0xf;
   *rB = byte & 0xf;
   *valP = pc + 2;
 }
 if(*icode == CMOVXX) {
   byte = getByteFromMemory(pc + 1);
   *rA = (byte >> 4) & 0xf;
   *rB = byte & 0xf;
   *valP = pc + 2;
 }

}

void decodeStage(int icode, int rA, int rB, wordType *valA, wordType *valB) {
    
     if(icode == RRMOVQ) {
       *valA = getRegister(rA);
     }
     if(icode == RMMOVQ) {
       *valA = getRegister(rA);
       *valB = getRegister(rB);
     }
     if(icode ==MRMOVQ) {
       *valB = getRegister(rB);

     }
     if(icode == OPQ) {
       *valA = getRegister(rA);
       *valB = getRegister(rB);
     }
     if(icode == CALL) {
       *valB = getRegister(RSP);// need to read the stack pointer
     }
     if(icode == RET) {
       *valA = getRegister(RSP);
       *valB = getRegister(RSP);
     }
     if(icode == PUSHQ) {
       *valA =  getRegister(rA);
       *valB = getRegister(RSP);
     }
     if(icode == POPQ) {
       *valA = getRegister(RSP);
       *valB = getRegister(RSP);
     }
     if(icode == CMOVXX) {
       *valA = getRegister(rA);
       *valB = 0;
     }


}

void executeStage(int icode, int ifun, wordType valA, wordType valB, wordType valC, wordType *valE, bool *Cnd) {
  bool zf = FALSE;
  bool sf = FALSE;
  bool of = FALSE;
  
  if(icode == IRMOVQ) {
    *valE = 0 + valC;
  }
  if(icode == RRMOVQ) {
    *valE = valA + 0;
  }
  if(icode == RMMOVQ) {
    *valE = valB + valC;
  }
  if(icode == MRMOVQ) {
    *valE = valB + valC;
  }
  if(icode == OPQ) {
    byteType byte;
    wordType pc;
    pc = getPC();
    byte = getByteFromMemory(pc);
    icode = (byte >> 4) & 0xf;
    ifun = byte & 0xf;

    if(ifun == 0) {
      *valE = valA + valB;
      if(*valE < 0) {
        sf = TRUE;
      }
      if(*valE == 0) {
        zf = TRUE;
      }
      if(valA > 0 && valB > 0 && *valE <= 0) {  //if we add up two positive numbers and get a negative result it's overflow
        of = TRUE;
      }
      if(valA < 0 && valB < 0 && *valE >= 0) {
        of = TRUE;                                //if we add up two negative numbers and get postiti throw of flag
      }
    }
    
    if(ifun == 1) {
      *valE = valB - valA;
      if (*valE < 0) {
        sf = TRUE;
      }
      if(*valE == 0) {
        zf = TRUE;
      }
      if(valA > 0 && *valE >= valB) {
        of = TRUE;                            // if we subtract from a postive number the result should be less than original
      }
      if(valA < 0 && *valE <= valB) {
        of = TRUE;                            //if we subtract a negative number and get something smaller throw flag
      }

    }
    
    if(ifun == 2) {
     *valE = valB & valA;
     if(*valE < 0) {
       sf = TRUE;
     }
     if(*valE == 0) {
       zf = TRUE;
     }
     if(valB > 0 && valA > 0 && *valE <= 0) {
       of = TRUE;
     }
     if(valB < 0 && valA < 0 && *valE >=0) {
       of = TRUE;
     }

    }
    
    if(ifun == 3) {
      *valE = valB ^ valA;
     if(*valE < 0) {
       sf = TRUE;
     }
     if(*valE == 0) {
       zf = TRUE;
     }
     if(valB > 0 && valA > 0 && *valE <= 0) {
       of = TRUE;
     }
     if(valB < 0 && valA < 0 && *valE >= 0) {
       of = TRUE;
     }

    }

      setFlags(sf,zf,of);
  }

  if(icode == JXX) {
     byteType byte;
     wordType pc;
     pc = getPC();
    byte = getByteFromMemory(pc);
    icode = (byte >> 4) & 0xf;
    ifun = byte & 0xf;
     *Cnd = Cond(ifun);

  }
  if(icode == CALL) {
    *valE = valB + -8; // decrement the stackpointer
  }
  if(icode == RET) {
    *valE = valB + 8;
  }
  if(icode == PUSHQ) {
    *valE = valB + (-8);
  }
  if(icode == POPQ) {
    *valE = valB + 8;
  }
  if(icode == CMOVXX) {
   *valE = valB + valA;
   byteType byte;
     wordType pc;
     pc = getPC();
    byte = getByteFromMemory(pc);
    icode = (byte >> 4) & 0xf;
    ifun = byte & 0xf;
     *Cnd = Cond(ifun);
     


  }
  
}

void memoryStage(int icode, wordType valA, wordType valP, wordType valE, wordType *valM) {
 
 if(icode == RMMOVQ) {
   setWordInMemory(valE,valA);
 }
 if(icode == MRMOVQ) {
   *valM = getWordFromMemory(valE);
 }
 if(icode == RET) {
   *valM = getWordFromMemory(valA);
 }
 if(icode == PUSHQ) {
   setWordInMemory(valE,valA);
 }
 if(icode == POPQ) {
   *valM = getWordFromMemory(valA);
 }
 if(icode == CALL) {
   setWordInMemory(valE,valP);
 }
}

void writebackStage(int icode, wordType rA, wordType rB, wordType valE, wordType valM) {
 
 if(icode == IRMOVQ) {
   setRegister(rB,valE);
 }
 if(icode == RRMOVQ) {
   setRegister(rB,valE);
 }
 if(icode == MRMOVQ) {
   setRegister(rA,valM);
 }
 if(icode == OPQ) {
   setRegister(rB,valE);
 }
 if(icode == CALL) {
   setRegister(RSP,valE);
 }
 if(icode == RET) {
   setRegister(RSP,valE);
 }
 if(icode == PUSHQ) {
   setRegister(RSP, valE);
 }
 if(icode == POPQ) {
   setRegister(RSP,valE);
   setRegister(rA,valM);
 }
 if(icode == CMOVXX) {
   setRegister(rB,valE);
 }
}

void pcUpdateStage(int icode, wordType valC, wordType valP, bool Cnd, wordType valM) {
  
  if(icode == HALT || icode == NOP || icode == IRMOVQ || icode == RRMOVQ || icode == RMMOVQ || MRMOVQ || OPQ || PUSHQ || POPQ || CMOVXX) {
    setPC(valP);
  }
  if(icode == JXX) {
    if(Cnd == TRUE) {
      setPC(valC);
    }
    else {
      setPC(valP);
    }
  }
  if(icode == RET) {
    setPC(valM);
  }
  if(icode == CALL) {
    setPC(valC);
  }
}

void stepMachine(int stepMode) {
  /* FETCH STAGE */
  int icode = 0, ifun = 0;
  int rA = 0, rB = 0;
  wordType valC = 0;
  wordType valP = 0;
 
  /* DECODE STAGE */
  wordType valA = 0;
  wordType valB = 0;

  /* EXECUTE STAGE */
  wordType valE = 0;
  bool Cnd = 0;

  /* MEMORY STAGE */
  wordType valM = 0;

  fetchStage(&icode, &ifun, &rA, &rB, &valC, &valP);
  applyStageStepMode(stepMode, "Fetch", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);

  decodeStage(icode, rA, rB, &valA, &valB);
  applyStageStepMode(stepMode, "Decode", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  executeStage(icode, ifun, valA, valB, valC, &valE, &Cnd);
  applyStageStepMode(stepMode, "Execute", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  memoryStage(icode, valA, valP, valE, &valM);
  applyStageStepMode(stepMode, "Memory", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  writebackStage(icode, rA, rB, valE, valM);
  applyStageStepMode(stepMode, "Writeback", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  pcUpdateStage(icode, valC, valP, Cnd, valM);
  applyStageStepMode(stepMode, "PC update", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);

  incrementCycleCounter();
}

/** 
 * main
 * */
int main(int argc, char **argv) {
  int stepMode = 0;
  FILE *input = parseCommandLine(argc, argv, &stepMode);

  initializeMemory(MAX_MEM_SIZE);
  initializeRegisters();
  loadMemory(input);

  applyStepMode(stepMode);
  while (getStatus() != STAT_HLT) {
    stepMachine(stepMode);
    applyStepMode(stepMode);
#ifdef DEBUG
    printMachineState();
    printf("\n");
#endif
  }
  printMachineState();
  return 0;
}