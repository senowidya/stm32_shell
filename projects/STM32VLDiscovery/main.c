#include "srcconf.h"

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

unsigned char led3,led4;
unsigned char count;

/* Timer 
 * 
 */

static void gpt1cb(GPTDriver *gptp) {

  (void)gptp;
  count++;
  if((count==2)||(count==4)||(count==6)||(count==8)){
    palTogglePad(GPIOC, GPIOC_LED3);
  }
  
  if(count==8){
    chSysLockFromIsr();
    gptStartOneShotI(&GPTD2, 2000);   /* 0.02 second pulse.*/
    palSetPad(GPIOC,GPIOC_LED4);
    chSysUnlockFromIsr();
    count=0;
  }
}

static void gpt2cb(GPTDriver *gptp) {

  (void)gptp;
  palClearPad(GPIOC, GPIOC_LED4);
}

static const GPTConfig gpt1cfg = {
//   10000,    /* 10kHz timer clock.*/
  100000,    /* 100kHz timer clock.*/
  gpt1cb,   /* Timer callback.*/
  0
};

static const GPTConfig gpt2cfg = {
//   10000,    /* 10kHz timer clock.*/
  100000,    /* 100kHz timer clock.*/
  gpt2cb,   /* Timer callback.*/
  0
};

/*
 * Shell responses
 */

static void cmd_led3(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  if(argc!=1){
    chprintf(chp,"led3 on|off \r\n");
    return;
  };
  
  if(strcmp(argv[0],"on")==0){
    led3=1;
    chprintf(chp,"led3 activated\r\n");
  }
  else if(strcmp(argv[0],"off")==0){
    led3=0;
    chprintf(chp,"led3 deactivated\r\n");
  }
  else{
    chprintf(chp,"led3 on|off \r\n");
  }
  
//   chprintf(chp,"argv is %s \r\n",argv[0]);

}

static void cmd_led4(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  if(argc!=1){
    chprintf(chp,"led4 on|off \r\n");
    return;
  };
  
  if(strcmp(argv[0],"on")==0){
    led4=1;
    chprintf(chp,"led4 activated\r\n");
  }
  else if(strcmp(argv[0],"off")==0){
    led4=0;
    chprintf(chp,"led4 deactivated\r\n");
  }
  else{
    chprintf(chp,"led4 on|off \r\n");
  }
  
//   chprintf(chp,"argv is %s \r\n",argv[0]);

}

static void cmd_delay(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  int num=0;
  if (argc != 1) {
    chprintf(chp, "Usage: delay [integer in ms] from 1 to 60000\r\n");
    return;
  }
  num = atoi(argv[0]);
  
  if((num>60000)||(num<1)){
    chprintf(chp,"delay is not compatible\r\n");
    return;
  }
  
  gptChangeInterval(&GPTD1,num);
  chprintf(chp,"delay was set to %i\r\n",num);
}

static void cmd_start(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  if(argc>0){
    chprintf(chp,"Serial Shell Connected!\r\n");
    return;
  };
  chprintf(chp,"Serial Shell Connected!\r\n");
}

static void cmd_infoOS(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    usage(chp, "info");
    return;
  }

  chprintf(chp, "Kernel:       %s\r\n", CH_KERNEL_VERSION);
#ifdef CH_COMPILER_NAME
  chprintf(chp, "Compiler:     %s\r\n", CH_COMPILER_NAME);
#endif
  chprintf(chp, "Architecture: %s\r\n", CH_ARCHITECTURE_NAME);
#ifdef CH_CORE_VARIANT_NAME
  chprintf(chp, "Core Variant: %s\r\n", CH_CORE_VARIANT_NAME);
#endif
#ifdef CH_PORT_INFO
  chprintf(chp, "Port Info:    %s\r\n", CH_PORT_INFO);
#endif
#ifdef PLATFORM_NAME
  chprintf(chp, "Platform:     %s\r\n", PLATFORM_NAME);
#endif
#ifdef BOARD_NAME
  chprintf(chp, "Board:        %s\r\n", BOARD_NAME);
#endif
#ifdef __DATE__
#ifdef __TIME__
  chprintf(chp, "Build time:   %s%s%s\r\n", __DATE__, " - ", __TIME__);
#endif
#endif
}

/*
 * Shell Class
 */

static const ShellCommand commands[] = {
  {"info", cmd_infoOS},
  {"delay",cmd_delay},
  {"led3",cmd_led3},
  {"led4",cmd_led4},
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
  
  gptStart(&GPTD1, &gpt1cfg);
  gptStart(&GPTD2, &gpt2cfg);
  
  palSetPadMode(GPIOA,9,PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOA,10,PAL_MODE_INPUT);
  
  /*
   * On STM32F30x use this. why?
   */
//   palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7));
//   palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7));
  sdStart(&SD1,NULL);
  
  palSetPadMode(GPIOC,GPIOC_LED3,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOC,GPIOC_LED4,PAL_MODE_OUTPUT_PUSHPULL);
  
  palClearPad(GPIOC,GPIOC_LED3);
  palClearPad(GPIOC,GPIOC_LED4);
  
  shellInit();
  Thread *shelltp = NULL;
  
  gptStartContinuous(&GPTD1, 50000);
  
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