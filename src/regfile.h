#ifndef REGFILE_H_
#define REGFILE_H_

#include <stdint.h>

#include "regfile_declare.h"

void structs_check_size();
regf_struct *regf_init(regf_struct *s);
void set_data(uint8_t *data_);

hbin_struct *hbin_init(uint32_t ptr);

nk_struct *nk_init(uint32_t ptr);
void nk_print_name(nk_struct *s);
void nk_print_class(nk_struct *s);
void nk_print_sk(nk_struct *s);
void vk_print_name(vk_struct *s);
void nk_ls_params(nk_struct *s);
void nk_ls_childs(nk_struct *s);
void nk_print_pwd(nk_struct *s);
void nk_print_verbose(nk_struct *s);
void nk_recur(nk_struct *s);
nk_struct *nk_cd(nk_struct *s, const char *path);

#endif /* REGFILE_H_ */
