#ifndef SECURITY_DESCRIPTOR_H_
#define SECURITY_DESCRIPTOR_H_

#include <stdint.h>

void secstructs_check_size();
void sec_data_parse(uint8_t *sec_data, uint32_t sec_data_size);

#endif /* SECURITY_DESCRIPTOR_H_ */
