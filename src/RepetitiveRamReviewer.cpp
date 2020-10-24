/*
 * RepetitiveRamReviewer
 * Copyright (C) 2020 Frank Mueller
 *
 * SPDX-License-Identifier: MIT
 */

#include "RepetitiveRamReviewer.h"
#include <Arduino.h>

void dumpUInt16(uint16_t val, uint8_t base = 10, char fill = ' ', uint8_t width = 4);

extern void *__brkval;
extern uint8_t __heap_start;
extern uint8_t __stack;

RepetitiveRamReviewer::RepetitiveRamReviewer(uint8_t pattern)
  : m_pattern(pattern)
  , m_initDone(false)
  , m_maxUsedHeap(0)
  , m_maxUsedStack(0xFFFF)
  , m_lastReview((uint16_t)millis())
{}

bool myCheck = false;

void RepetitiveRamReviewer::begin() {
  // create all vars we need first (on stack)
  void *heap;
  void *stack;
  uint16_t free;
  uint16_t cur;
  
  if ((uint16_t)__brkval == 0) {
    heap = __malloc_heap_start;
  } else {
    heap = __brkval;
  }
  stack = SP;
  free = (uint16_t)stack - (uint16_t)heap;

  m_maxUsedHeap = (uint16_t)heap;
  m_maxUsedStack = (uint16_t)stack;
  if (m_maxUsedStack > m_maxUsedHeap) {
    for (cur = m_maxUsedHeap; cur < m_maxUsedStack; ++cur) {
      *(uint8_t*)(cur) = m_pattern;
    }
  }
  m_initDone = true;
}

bool RepetitiveRamReviewer::reviewRam() {
  if (!m_initDone) { begin(); }
  bool changed = false;
  
  // check current stack pointer
  if (SP < m_maxUsedStack) {
    m_maxUsedStack = SP;
    changed = true;
  }
  
  // check currend heap end
  if (__brkval > m_maxUsedHeap) {
    m_maxUsedHeap = __brkval;
    changed = true;
  }
  
  // check stack data first
  while ((*(uint8_t*)(m_maxUsedStack) != m_pattern) && (m_maxUsedStack > m_maxUsedHeap)) {
    --m_maxUsedStack;
    changed = true;
  }
  
  // than check heap data
  while ((*(uint8_t*)(m_maxUsedHeap) != m_pattern) && (m_maxUsedStack > m_maxUsedHeap)) {
    ++m_maxUsedHeap;
    changed = true;
  }
  
  uint16_t middle = (m_maxUsedHeap + m_maxUsedStack) / 2;
  
  // deeper check for inner changes start in the middle going to stack,
  // first byte not maching the pattern is count to stack
  for (uint16_t cur = middle; cur < m_maxUsedStack; ++cur) {
    if (*(uint8_t*)(cur) != m_pattern) {
      m_maxUsedStack = cur;
      changed = true;
      break;
    }
  }
  
  // deeper check for inner changes start in the middle going to heap
  // first byte not maching the pattern is count to heap
  for (uint16_t cur = middle-1; cur > m_maxUsedHeap; --cur) {
    if (myCheck) {
      Serial.print(" C:");
      dumpUInt16(cur, 16, '0');
    }
    if (*(uint8_t*)(cur) != m_pattern) {
      m_maxUsedHeap = cur;
      changed = true;
      if (myCheck) {
        Serial.println(" found!");
      }
      break;
    }
    if (myCheck && (cur % 16 == 0)) {
      Serial.println(" nothing!");
    }
  }

  return changed;
}

bool RepetitiveRamReviewer::reviewRamAndDump() {
  bool changed = reviewRam();
  dumpToSerial();
  return changed;
}

void RepetitiveRamReviewer::loop(uint16_t cylce, bool dump, bool onlyChanged) {
  uint16_t cur = (uint16_t)millis();
  if ((cur - m_lastReview) >= cylce) {
    m_lastReview += cylce;
    
    bool changed = reviewRam();
    
    if (dump && (changed || !onlyChanged)) {
      dumpToSerial();
    }
  }
}

uint16_t RepetitiveRamReviewer::getCurFree() {
    uint16_t free;

    if((uint16_t)__brkval == 0)
        free = ((uint16_t)&free) - ((uint16_t)&__malloc_heap_start);
    else
        free = ((uint16_t)&free) - ((uint16_t)__brkval);

    return free;
}

uint16_t RepetitiveRamReviewer::getMinFree() {
  if (m_maxUsedStack > m_maxUsedHeap) {
    return m_maxUsedStack - m_maxUsedHeap;
  } else {
    return 0;
  }
}

void dumpUInt16(uint16_t val, uint8_t base = 10, char fill = ' ', uint8_t width = 4) {
  if (base < 10) return; // we only need 10 and 16
  // "012345678"
  // "<<<<x>>>>"
  // "    65563"
  char tmp[10];
  char *str = itoa(val, tmp+4, base);
  uint8_t len = strlen(str);
  while ((len < width) && (str > tmp)) {
    --str;
    str[0] = fill;
    ++len;
  }
  Serial.print(str);
}


// something like  "|[1234]==( 123)=>|[1234]<=( 123)=>|[1234]<=( 123)==[1234]|"
void RepetitiveRamReviewer::dumpToSerial() {
  //Serial.println(F("|      HEAP      |      free      |        STACK         |"));
  Serial.print(F("|["));
  dumpUInt16((int16_t)__malloc_heap_start, 16, '0');
  Serial.print(F("]==("));
  dumpUInt16(m_maxUsedHeap - (int16_t)__malloc_heap_start);
  Serial.print(F(")=>|["));
  dumpUInt16(m_maxUsedHeap, 16, '0');
  Serial.print(F("]=>("));
  dumpUInt16(getMinFree());
  Serial.print(F(")<=|["));
  dumpUInt16(m_maxUsedStack, 16, '0');
  Serial.print(F("]<=("));
  dumpUInt16((uint16_t)(&__stack) - m_maxUsedStack);
  Serial.print(F(")==["));
  dumpUInt16((uint16_t)(&__stack), 16, '0');
  Serial.println(F("]|"));
}

