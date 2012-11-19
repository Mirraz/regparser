#ifndef REGFILE_H_
#define REGFILE_H_

#include <stdint.h>

#include "regfile_declare.h"

void structs_check_size();
regf_struct *regf_init(regf_struct *s);
void set_data(uint8_t *data_);

nk_struct *nk_init(uint32_t ptr);
void nk_print_name(nk_struct *s);
void nk_print_class(nk_struct *s);
void nk_print_sk(nk_struct *s);
void vk_print_name(vk_struct *s);
void nk_ls_params(nk_struct *s);
void nk_ls_childs(nk_struct *s);
void nk_recur(nk_struct *s);

#endif /* REGFILE_H_ */
