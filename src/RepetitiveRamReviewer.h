/*
 * RepetitiveRamReviewer
 * Copyright (C) 2020 Frank Mueller
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef REPETITIVERAMREVIEWER_H
#define REPETITIVERAMREVIEWER_H

#include <stdint.h>

class RepetitiveRamReviewer {
  public:
    RepetitiveRamReviewer(uint8_t pattern = 0xA5);

    /** Prepare for messuring. */
    void begin();

    /** 
     * Do a cyclic check every n milliseconds.
     * @param  cylce  Count of milliseconds between two checks.
     */
    void loop(uint16_t cylce = 10000) {
      loop(cylce, false, false);
    }

    /** Like loop() but also dump values after check. */
    void loopAndDump(uint16_t cylce = 10000) {
      loop(cylce, true, false);
    }

    /**
     * Like loopAndDump() but only dump if something change.
     * If you use this function you should not run reviewRam() by your self.
     */
    void loopAndDumpChanges(uint16_t cylce = 10000) {
      loop(cylce, true, true);
    }

    /**
     * Do a single check for current RAM usage.
     * @return  true if more RAM was used since last check.
     */
    bool reviewRam();
    
    /** Like reviewRam() but also dump values after measuring. */
    bool reviewRamAndDump();

    /** Reset maximum reviwed RAM usage to current RAM usage. */
    void resetView() {
      begin();
    }

    /** Get current free (unused) RAM. */
    uint16_t getCurFree();

    /** Get minimal messured value of free (unused) RAM. */
    uint16_t getMinFree();

    /** Print mesured values to serial. */
    void dumpToSerial();

  private:

    void loop(uint16_t cylce, bool dump, bool onlyChanged);

    /** This value is stored in free RAM so I can compare it later to check if someone used it. */
    const uint8_t m_pattern;

    bool     m_initDone;
    uint16_t m_maxUsedHeap;
    uint16_t m_maxUsedStack;
    uint16_t m_lastReview;
};

#endif // REPETITIVERAMREVIEWER_H
