#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"

static Semaphore *readAvail;
static Semaphore *writeDone;

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

SynchConsole::SynchConsole(char *readFile, char *writeFile) {
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    console = new Console(readFile, writeFile, ReadAvail, WriteDone, 0);
}

SynchConsole::~SynchConsole() {
    delete console;
    delete writeDone;
    delete readAvail;
}

void SynchConsole::SynchPutChar(const char c) {
    console->PutChar(c);
    writeDone->P();
}

char SynchConsole::SynchGetChar() {
    readAvail->P();
    return console->GetChar();
}

void SynchConsole::SynchPutString(const char s[]) {
    int i=0;
    while(s[i] != '\0'){
        SynchPutChar(s[i++]);
    }

}

void SynchConsole::SynchGetString(char *s, int n) {
    int i;
    char c;
    for(i=0; i<(n-1); i++){
        c = SynchGetChar();
        if (c == EOF || c == 04 || c == '\n'){
            s[i] = '\0';
            return;
        }
        s[i] = c;
    }
    if (i == (n-1)){
        s[i] = '\0';
    }
}