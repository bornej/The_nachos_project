#include "userthread.h"
#include "thread.h"
#include "system.h"
#include <string>
#include <cstring>
#include <algorithm>

/*
 * Lance un thread utilisateur qui exécute la fonction UserThreadParams->f et prenant comme
 * arguments UserThreadparams->arg.
 */
 static void StartUserThread(int threadParams) {
     int maxAddr, stackPtrOffset;
     UserThreadParams *userThreadParams = (UserThreadParams*) threadParams;


     currentThread->space->InitRegisters();
     currentThread->space->RestoreState();

     /* calcul de la dernière adresse de l'espace mémoire */
     maxAddr = (currentThread->space->getNumPages() * PageSize);
     /*
      * Calcul du décalage du pointeur de pile du thread courant par rapport à l'adresse max.
      * On a getThreadID correspond à l'index dans stackBitmap
      */
     stackPtrOffset = (currentThread->getStackBitmapIndex() * NumPagesPerStack * PageSize);
     machine->WriteRegister (StackReg, maxAddr - stackPtrOffset);

     /* On place l'addresse de la fonction UserThreadexit dans le registre RetAddrReg,
        pour qu'elle soit appellée à la terminaison de la fonction f*/
     machine->WriteRegister (RetAddrReg, userThreadParams->addrExit);
     /* Place dans PC la prochaine instruction à exécuter -> f */
     machine->WriteRegister (PCReg, userThreadParams->f);
     /* Place dans NextPcreg l'instruction suivante */
     machine->WriteRegister (NextPCReg, userThreadParams->f+4);
     /* Place les paramètres de la fonction dans le registre 4. */
     machine->WriteRegister (4, userThreadParams->arg);

     /* Libère l'espace mémoire allouée à la structure threadparams. */
     free(userThreadParams);
     /* Lance l'interpréteur mips -> branchement vers f. */
     machine->Run();
 }

/*
 * Crée un thread noyau (propulseur) qui permet le lancement d'un thread utilisateur exécutant la fonction
 * dont l'adresse est f et dont les paramètres sont fournis à l'adresse arg.
 * la fonction f et les paramètres arg sont passés à l'appel Fork à par le biais d'une structure
 * de donnée de type UserThreadParams.
 */
extern void do_UserThreadCreate() {
    Thread *t;
    int tid;
    if ((t = new Thread ("UserThread")) == NULL) {
      tid = -1;
    }

    UserThreadParams *threadParams = (UserThreadParams*) malloc(sizeof(UserThreadParams));
    threadParams->f = machine->ReadRegister(4);
    threadParams->arg = machine->ReadRegister(5);
    threadParams->addrExit = machine->ReadRegister(6);

    t->Fork(StartUserThread,(int) threadParams);
    // /!\ ATTENTION TRAITER LE CAS OU LE THREADID = -1
    tid = t->getTid();
    machine->WriteRegister(2,tid);
}

/*
 * Spécification: extern void do_UserThreadExit()
 * Sémantique:
 * Lorsqu'un Thread T termine, il regarde si des Threads Ti l'attendent.
 * Avant de quitter le thread courant remet les éventuels Ti en sommeil dans la ready list du scheduler.
 */
extern void do_UserThreadExit() {
    std::map<int, std::list<Thread*>* >::iterator it = currentThread->space->joinMap->find(currentThread->getTid());
    if (it != currentThread->space->joinMap->end()){
        std::list<Thread*>* listThread = it->second;
        for (std::list<Thread*>::const_iterator iterator = listThread->begin(), end = listThread->end(); iterator != end; ++iterator) {
            scheduler->ReadyToRun(*iterator);
        }
        currentThread->space->joinMap->erase(currentThread->getTid());
    }
    currentThread->Finish();
}

/*
 * Spécification: extern int do_UserThreadJoin(int tid)
 * Sémantique: Lorsqu'un Thread T1 fait un join sur un Thread T2 on ajoute dans la map
 * T1->space->joinMap l'association (T2.Tid, [T1]).
 * Ainsi on sait que T2 est attendu par T1.
 * Un thread ne peut join un autre thread que si il est vivant (présent dans threadList de l'addrspace)
 */
extern void do_UserThreadJoin(){

    int tid = machine->ReadRegister(4);
    int error;
    if (tid == currentThread->getTid() || std::find(currentThread->space->threadList->begin(), currentThread->space->threadList->end(), tid) == currentThread->space->threadList->end()){
        error =  -1;
    }
    interrupt->SetLevel (IntOff);
    std::map<int, std::list<Thread*>* >::iterator it = currentThread->space->joinMap->find(tid);
    std::list<Thread*>* tempThreadList;
    if (it == currentThread->space->joinMap->end()){
        tempThreadList = new std::list<Thread*>();
        currentThread->space->joinMap->insert(std::make_pair(tid, tempThreadList));
    } else {
        tempThreadList = it->second;
    }
    tempThreadList->push_back(currentThread);
    currentThread->Sleep();
    error = 0;
    machine->WriteRegister(2,error);
}
