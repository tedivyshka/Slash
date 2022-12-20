#include <signal.h>
#include <stddef.h>


void initSignals(){
  for(int i = 1; i <= 31; i++){
    struct sigaction sa = {0};

    switch(i){
      case SIGTERM:
      case SIGINT:
        sa.sa_handler = SIG_IGN;
        break;
      default:
        sa.sa_handler = SIG_DFL;
    }
    sigaction(i,&sa,NULL);
  }
}

void defaultSignals(){
  for(int i = 1; i <= 31; i++){
    struct sigaction sa = {0};
    sa.sa_handler = SIG_DFL;
    sigaction(i,&sa,NULL);
  }
}
