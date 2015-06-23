/*
 * persistence.c
 *
 *  Created on: 11.07.2014
 *      Author: zaphod
 */

#include "persistence.h"

SchlangomatPersistenceData persData = {0};

void persistence_restore() {
	cli();
	eeprom_read_block((void*)&persData, 0, sizeof(persData));
	sei();

	if(persData.magic == PERSISTENCE_EEPROM_MAGIC) {
		rules_set_rule_volatile(1, persData.rule1);
		rules_set_rule_volatile(2, persData.rule2);
		rules_set_rule_volatile(3, persData.rule3);
		rules_set_rule_volatile(4, persData.rule4);
	}
	else {
		rules_set_rule_volatile(1, "");
		rules_set_rule_volatile(2, "");
		rules_set_rule_volatile(3, "");
		rules_set_rule_volatile(4, "");
	}
}

void persistence_persist() {
	persData.magic = PERSISTENCE_EEPROM_MAGIC;

	rules_print_rule(1, persData.rule1);
	rules_print_rule(2, persData.rule2);
	rules_print_rule(3, persData.rule3);
	rules_print_rule(4, persData.rule4);

	cli();
	eeprom_write_block((void*)&persData, 0, sizeof(persData));
	sei();
}
