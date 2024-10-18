#ifndef CONFIG_H
  #define CONFIG_H
  typedef struct {
    volatile unsigned b0 : 1;
    volatile unsigned b1 : 1;
    volatile unsigned b2 : 1;
    volatile unsigned b3 : 1;
    volatile unsigned b4 : 1;
    volatile unsigned b5 : 1;
    volatile unsigned b6 : 1;
    volatile unsigned b7 : 1;
  } bitfield_s;

  typedef union {
    volatile uint8_t *REG;
    bitfield_s *bits;

  } bitreg_u;

  extern bitreg_u PORTb_bitreg;
  #define PORTb *(PORTb_bitreg.REG)
  #define PORTbBits (*(PORTb_bitreg.bits))
  extern bitreg_u PINb_bitreg;
  #define PINb *(PINb_bitreg.REG)
  #define PINbBits (*(PINb_bitreg.bits))

  extern bitreg_u PORTc_bitreg;
  #define PORTc *(PORTc_bitreg.REG)
  #define PORTcBits (*(PORTc_bitreg.bits))
  extern bitreg_u PINc_bitreg;
  #define PINc *(PINc_bitreg.REG)
  #define PINcBits (*(PINc_bitreg.bits))

  extern bitreg_u PORTd_bitreg;
  #define PORTd *(PORTd_bitreg.REG)
  #define PORTdBits (*(PORTd_bitreg.bits))
  extern bitreg_u PINd_bitreg;
  #define PINd *(PINd_bitreg.REG)
  #define PINdBits (*(PINd_bitreg.bits))
  #endif
