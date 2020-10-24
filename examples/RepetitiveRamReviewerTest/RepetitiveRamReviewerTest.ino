/*
 * Example to test and demonstrate RepetitiveRamReviewer
 * 
 * by Frank Müller
 * 
 * This is distributed under the MIT License.
 */

#include <RepetitiveRamReviewer.h>

RepetitiveRamReviewer rrr;

void setup() {
  // Don't forget to set the same baud rate in serial monitor
  Serial.begin(115200);

  rrr.begin();
  
  // We are inside a function, so we still use stack
  measureAndDump(F("In setup()"));
}

void loop() {
  // Vars in this function are defined below but they are still reserved on stack
  measureAndDump(F("In loop()"));

  // Calling a function will always use the stack, but if an other function was
  // still called before it may not increase the max used stack.
  // The fallowing call is recursive so its call it self inside until value is calculated.
  int f = fibonacci(5);
  Serial.println();
  Serial.print(F("fibonacci(5): "));
  Serial.println(f);
  measureAndDump();

  // This will do more recursive calls so more stack is used.
  f = fibonacci(23);
  Serial.println();
  Serial.print(F("fibonacci(23): "));
  Serial.println(f);
  measureAndDump();

  // Now test the heap

  size_t len = 500;
  // volatile => do not optimize useless code
  volatile char * data = malloc(len);
  // Normally you should check the result, we just store the address.
  uint16_t addr = (uint16_t)data;
  free(data);
  // There was no messure between malloc and free, so we can not see it,
  // but malloc writes some organisation data on heap we can see.
  measureAndDump(F("After malloc and free: "), (uint16_t)data);

  data = malloc(len);
  for (size_t i = 0; i < len; ++i) {
    data[i] = 0xA5;
  }
  free(data);
  // We have write data into heap but we can not see it because no byte changed
  measureAndDump(F("After malloc, writing 0xA5 and free"), (uint16_t)data);

  data = malloc(len);
  for (size_t i = 0; i < (len/2); ++i) {
    data[i] = 0x5A;
  }
  free(data);
  // Now some bytes are changed but not all.
  measureAndDump(F("After writing 0x5A first half"), (uint16_t)data);

  data = malloc(len);
  data[len/2 + 2] = 0x5A;
  free(data);
  // Even if we change some byte enywhere between heap and stack, we can detect it.
  measureAndDump(F("After skip a byte"), (uint16_t)data);

  data = malloc(len);
  // We can check the current heap size even if data do not change,
  // but only if free is not yet called.
  measureAndDump(F("Just malloc no free"), (uint16_t)data);
  
  free(data);
  // Now some free memory is available but we still dump the wort case.
  measureAndDump(F("After free"), (uint16_t)data);

  Serial.println();
  Serial.println(F("### thats all ###"));
  Serial.println();
  
  // After 30s reset to current memory usage and start loop again.
  // Compare new values with the old one. Some may change.
  // For example interrupt can happen at any possible time and they also
  // uses stack. So it may happen while little current stack usage or while huge usage.
  delay(30000);
  rrr.resetView();
}

int fibonacci(int n) {
  if (n < 3) {
    return 1;
  } else {
    return fibonacci(n-1) + fibonacci(n-2);
  }
}

void measureAndDump() {
  bool changed = rrr.reviewRamAndDump();
  if (changed) { Serial.println(F("Changed")); }
  else         { Serial.println(F("No change")); }
  Serial.print(F("cur free: "));
  Serial.println(rrr.getCurFree());
}

void measureAndDump(__FlashStringHelper *str) {
  Serial.println();
  Serial.println(str);
  measureAndDump();
}

void measureAndDump(__FlashStringHelper *str, uint16_t addr) {
  measureAndDump(str);
  Serial.print(F("addr: "));
  Serial.println(addr, HEX);
}
