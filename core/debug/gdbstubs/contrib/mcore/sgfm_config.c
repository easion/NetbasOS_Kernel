typedef struct {
  const unsigned long  backdoor_word_1;
  const unsigned long  backdoor_word_2;
  
  const unsigned short block0_PESR;
  const unsigned short reserved0;
  const unsigned short block0_SUSR;
  const unsigned short block0_PDSR;
  
  const unsigned short block1_PESR;
  const unsigned short reserved1;
  const unsigned short block1_SUSR;
  const unsigned short block1_PDSR;
  
  const unsigned short block2_PESR;
  const unsigned short reserved2;
  const unsigned short block2_SUSR;
  const unsigned short block2_PDSR;
  
  const unsigned short block3_PESR;
  const unsigned short reserved3;
  const unsigned short block3_SUSR;
  const unsigned short block3_PDSR;
  
  const unsigned long  security_word;
  const unsigned long  space;
} sgfm_conf_t;


const sgfm_conf_t sgfm_conf = {
   0xFFFFFFFF,  /* 0x0000_0200 Backdoor Word #1 */
   0xFFFFFFFF,  /* 0x0000_0204 Backdoor Word #2 */

   0x0000,      /* 0x0000_0208 Block 0  Program/Erase Space Restrictions */
   0x0000,      /* 0x0000_020A Reserved */
   0x0000,      /* 0x0000_020C Block 0  Supervisor/User Space Restrictions */
   0x0000,      /* 0x0000_020E Block 0  Program/Data Space Restrictions */

   0x0000,      /* 0x0000_0210 Block 1  Program/Erase Space Restrictions */
   0x0000,      /* 0x0000_0212 Reserved */
   0x0000,      /* 0x0000_0214 Block 1  Supervisor/User Space Restrictions */
   0x0000,      /* 0x0000_0216 Block 1  Program/Data Space Restrictions */

   0x0000,      /* 0x0000_0218 Block 2  Program/Erase Space Restrictions */
   0x0000,      /* 0x0000_021A Reserved */
   0x0000,      /* 0x0000_021C Block 2  Supervisor/User Space Restrictions */
   0x0000,      /* 0x0000_021E Block 2  Program/Data Space Restrictions */

   0x0000,      /* 0x0000_0220 Block 3  Program/Erase Space Restrictions */
   0x0000,      /* 0x0000_0222 Reserved */
   0x0000,      /* 0x0000_0224 Block 3  Supervisor/User Space Restrictions */
   0x0000,      /* 0x0000_0226 Block 3  Program/Data Space Restrictions */

   0xFFFFFFFF,  /* 0x0000_0228 Security Word */
   0x00000000   /* Unused space */
};
