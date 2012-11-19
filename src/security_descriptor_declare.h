#ifndef SECURITY_DESCRIPTOR_DECLARE_H_
#define SECURITY_DESCRIPTOR_DECLARE_H_

#include <stdint.h>

#pragma pack(push,1)
typedef struct {
	uint8_t revision;
	uint8_t sbz1;
	uint16_t control;
	uint32_t offset_owner;
	uint32_t offset_group;
	uint32_t offset_sacl;
	uint32_t offset_dacl;
	uint8_t data[];
} __attribute__ ((__packed__)) sd_struct;
#pragma pack(pop)
#define sd_struct_size 0x14

typedef struct {
	uint32_t flag;
	const char *const short_name;
	const char *const name;
} flag_desc;

typedef enum {
	SDF_SR = 0x8000,
	SDF_RM = 0x4000,
	SDF_PS = 0x2000,
	SDF_PD = 0x1000,
	SDF_SI = 0x0800,
	SDF_DI = 0x0400,
	SDF_SC = 0x0200,
	SDF_DC = 0x0100,
	SDF_DT = 0x0080,
	SDF_SS = 0x0040,
	SDF_SD = 0x0020,
	SDF_SP = 0x0010,
	SDF_DD = 0x0008,
	SDF_DP = 0x0004,
	SDF_GD = 0x0002,
	SDF_OD = 0x0001,
} sd_flags;

#define sd_flags_desc_value { \
		{SDF_SR, "SR", "Self-Relative"}, \
		{SDF_RM, "RM", "RM Control Valid"}, \
		{SDF_PS, "PS", "SACL Protected"}, \
		{SDF_PD, "PD", "DACL Protected"}, \
		{SDF_SI, "SI", "SACL Auto-Inherited"}, \
		{SDF_DI, "DI", "DACL Auto-Inherited"}, \
		{SDF_SC, "SC", "SACL Computed Inheritance Required"}, \
		{SDF_DC, "DC", "DACL Computed Inheritance Required"}, \
		{SDF_DT, "DT", "DACL Trusted"}, \
		{SDF_SS, "SS", "Server Security"}, \
		{SDF_SD, "SD", "SACL Defaulted"}, \
		{SDF_SP, "SP", "SACL Present"}, \
		{SDF_DD, "DD", "DACL Defaulted"}, \
		{SDF_DP, "DP", "DACL Present"}, \
		{SDF_GD, "GD", "Group Defaulted"}, \
		{SDF_OD, "OD", "Owner Defaulted"} \
}
#define sd_flags_desc_count (sizeof(sd_flags_desc) / sizeof(sd_flags_desc[0]))

#pragma pack(push,1)
typedef struct {
	uint8_t revision;
	uint8_t sbz1;
	uint16_t size;
	uint16_t ace_count;
	uint16_t sbz2;
} __attribute__ ((__packed__)) acl_struct;
#pragma pack(pop)
#define acl_struct_size 0x08

#pragma pack(push,1)
typedef struct {
	uint8_t type;
	uint8_t flags;
	uint16_t size;
} __attribute__ ((__packed__)) ace_struct;
#pragma pack(pop)
#define ace_struct_size 0x04

#pragma pack(push,1)
typedef struct {
	uint8_t type;
	uint8_t flags;
	uint16_t size;
	uint32_t mask;
	uint8_t sid[];
} __attribute__ ((__packed__)) ace_accall_struct;
#pragma pack(pop)
#define ace_accall_struct_size 0x08

typedef enum {
	AM_GR = 0x80000000,
	AM_GW = 0x40000000,
	AM_GX = 0x20000000,
	AM_GA = 0x10000000,
	AM_MA = 0x02000000,
	AM_AS = 0x01000000,
	AM_SY = 0x00100000,
	AM_WO = 0x00080000,
	AM_WD = 0x00040000,
	AM_RC = 0x00020000,
	AM_DE = 0x00010000
} access_mask_flags;

#define access_mask_flags_desc_value { \
		{AM_GR, "GR", "GENERIC_READ"}, \
		{AM_GW, "GW", "GENERIC_WRITE"}, \
		{AM_GX, "GX", "GENERIC_EXECUTE"}, \
		{AM_GA, "GA", "GENERIC_ALL"}, \
		{AM_MA, "MA", "MAXIMUM_ALLOWED"}, \
		{AM_AS, "AS", "ACCESS_SYSTEM_SECURITY"}, \
		{AM_SY, "SY", "SYNCHRONIZE"}, \
		{AM_WO, "WO", "WRITE_OWNER"}, \
		{AM_WD, "WD", "WRITE_DACL"}, \
		{AM_RC, "RC", "READ_CONTROL"}, \
		{AM_DE, "DE", "DELETE"} \
}
#define access_mask_flags_desc_count \
	(sizeof(access_mask_flags_desc) / sizeof(access_mask_flags_desc[0]))

#pragma pack(push,1)
typedef struct {
	uint8_t revision;
	uint8_t sub_authority_count;
	uint8_t identifier_authority[6];
	uint32_t sub_authority[];
} __attribute__ ((__packed__)) sid_struct;
#pragma pack(pop)
#define sid_struct_size 0x08

#endif /* SECURITY_DESCRIPTOR_DECLARE_H_ */
