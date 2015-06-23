/*
 * persistence.h
 *
 *  Created on: 11.07.2014
 *      Author: zaphod
 */

#ifndef PERSISTENCE_H_
#define PERSISTENCE_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "rules.h"

#define PERSISTENCE_EEPROM_MAGIC	0xbeef
#define PERSISTENCE_NUM_SLOTS			6

typedef struct {
  uint16_t magic;
  char rule1[9];
  char rule2[9];
  char rule3[9];
  char rule4[9];
} SchlangomatPersistenceData;

void persistence_restore(void);
void persistence_persist(void);

#endif /* PERSISTENCE_H_ */
