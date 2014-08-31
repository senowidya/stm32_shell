#include "srcconf.h"

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

/*
 * Shell responses
 */

static void cmd_input(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  uint16_t num=0;
  if (argc != 1) {
    chprintf(chp, "Usage: input [16bit_value]\r\n");
    return;
  }
  num = atoi(argv[0]);
  save_mem(nilai,num);
  return;
}

static void cmd_output(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  uint16_t num=0;
  if (argc > 0) {
    chprintf(chp, "Usage: output\r\n");
    return;
  }
  
  num=read_mem(num);
  chprintf(chp, "num= %i\r\n",num);
  return;
}

static void cmd_start(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  if(argc>0){
    chprintf(chp,"Serial Shell Connected!\r\n");
    return;
  };
  chprintf(chp,"Serial Shell Connected!\r\n");
}

/*
 * Shell Class
 */

static const ShellCommand commands[] = {
  {"input",cmd_input},
  {"output",cmd_output},
  {"ok",cmd_start},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SD1,
  commands
};

/*
 * App entry
 */

int main(void){
  
  halInit();
  chSysInit();
  
  MEM_Setup();
  
  palSetPadMode(GPIOA,9,PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOA,10,PAL_MODE_INPUT);

  sdStart(&SD1,NULL);
  shellInit();
  Thread *shelltp = NULL;
  
  while (TRUE){
    
    if (!shelltp)
      shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    else if (chThdTerminated(shelltp)) {
      chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
      shelltp = NULL;           /* Triggers spawning of a new shell.        */
    }
    chThdSleepMilliseconds(500);
    
  }
}