#ifndef HammingEncDec_H
#define HammingEncDec_H

uint8_t hamming_hbyte_encoder(uint8_t in);
uint8_t hamming_hbyte_decoder(uint8_t in);
extern uint16_t hamming_byte_encoder(uint8_t input);
extern uint8_t hamming_byte_decoder(uint8_t lower, uint8_t upper);
extern uint16_t get_error_mask(void);
#endif
