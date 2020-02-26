/*
 * Send bits 
 */
void send_tpms( bitArray_t *bits)
{
  bool pinState;
  bitLength_t bitno;
  
  cli();
    
  pinState = true;
  digitalWrite( TXPin, pinState);

  setTxState();
  delayMicroseconds( 1000); // Wait for oscillator calibration. Takes 799 usec.

  pinState = !pinState;
  digitalWrite( TXPin, pinState);

  for( bitno = 0; bitno < bits->length; bitno++) {
    if( bitno < bits->length-1 && (get_bit( bits, bitno) == get_bit( bits, bitno+1))) {
      // long pulse worth two bits
      delayMicroseconds( 95);
      bitno++;
    } else {
      // short pulse
      delayMicroseconds( 45);
    }

    pinState = !pinState;
    digitalWrite( TXPin, pinState);
  }
    
  delayMicroseconds( 45);

  pinState = !pinState;
  digitalWrite( TXPin, pinState);

  setIdleState();

  sei();
}
