#include <signal.h>
#include <stddef.h>

/***
 * A function to set the signal behavior of the program as wanted :
 * ignore SIGINT and SIGTERM
 */
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

/***
 * A function to reset the signal behavior of child process for external commands.
 */
void defaultSignals(){
  for(int i = 1; i <= 31; i++){
    struct sigaction sa = {0};
    sa.sa_handler = SIG_DFL;
    sigaction(i,&sa,NULL);
  }
}
