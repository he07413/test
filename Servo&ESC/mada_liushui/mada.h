#ifndef __MADA_H__
#define __MADA_H__

#define ON 1
#define OFF 0

/*
#define MA_ON() {P0_3 = 1}
#define MA_OFF() {P0_3 = 0}
#define MB_ON() {P0_3 = 1}
#define MB_OFF() {P0_3 = 0}
#define MC_ON() {P0_3 = 1}
#define MC_OFF() {P0_3 = 0}
#define MD_ON() {P0_3 = 1}
#define MD_OFF() {P0_3 = 0}
*/

#define MADA_PIN_OUT(x,y,z) *(x) = (((*(x))&(~(1<<(y))))|((z)<<(y)))

typedef enum
{
  MA = 0,
  MB,
  MC,
  MD,
}__MADA_NUM;

typedef struct
{
  unsigned int Freq;
  unsigned int Duty;
  unsigned char Status;
}__MADA_PARAM;

typedef struct
{
  volatile int *Port;
  unsigned char PinNum;
}__MADA_PORT;

extern __MADA_PARAM Mada_param[];

extern unsigned char P1_1;//??
extern unsigned char P1_2;
extern unsigned char P1_5;
extern unsigned char P0_5;
extern unsigned char mode;

void Mada_execute(unsigned char mode);

#endif
