#include "syscall.h"

#define NB 10

/*
 * Programme de test créant un nombre de thread supérieur au nombre de thread possible.
 * Ici 7 Threads sont crées, puis au huitième thread, il n'y a plus assez de place pour placer la pile
 * de ce thread en mémoire. La bitMap est pleine, la création du thread renvoie -1.
 * Chaque thread affiche son identifiant.
 */

void g(void *arg) {
  int n = *(int*) arg;
  PutString("Je suis le thread num ");
  PutInt(n);
  PutChar('\n');
}


int main(){
  int tab[NB];
  int i, tid;

  for(i=0; i<NB; i++){
    tab[i] = i;
    tid = UserThreadCreate(g,(void*) (tab+i));
    PutString("ID du thread : ");
    PutInt(tid);
    PutChar('\n');
  }

  for(; i<8; i++){
    tab[i] = i;
    tid = UserThreadCreate(g,(void*) (tab+i));
    PutString("ID du thread : ");
    PutInt(tid);
    PutChar('\n');
  }

  return 0;
}
