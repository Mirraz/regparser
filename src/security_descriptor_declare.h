#ifndef SECURITY_DESCRIPTOR_DECLARE_H_
#define SECURITY_DESCRIPTOR_DECLARE_H_

#include <stdint.h>

#include "parse_common.h"

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
#define sd_flags_count 16

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

typedef enum {
	ACCESS_ALLOWED_ACE_TYPE =				0x00,
	ACCESS_DENIED_ACE_TYPE =				0x01,
	SYSTEM_AUDIT_ACE_TYPE =					0x02,
	SYSTEM_ALARM_ACE_TYPE =					0x03,
	ACCESS_ALLOWED_COMPOUND_ACE_TYPE =		0x04,
	ACCESS_ALLOWED_OBJECT_ACE_TYPE =		0x05,
	ACCESS_DENIED_OBJECT_ACE_TYPE =			0x06,
	SYSTEM_AUDIT_OBJECT_ACE_TYPE =			0x07,
	SYSTEM_ALARM_OBJECT_ACE_TYPE =			0x08,
	ACCESS_ALLOWED_CALLBACK_ACE_TYPE =		0x09,
	ACCESS_DENIED_CALLBACK_ACE_TYPE =		0x0A,
	ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE=0x0B,
	ACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE=	0x0C,
	SYSTEM_AUDIT_CALLBACK_ACE_TYPE =		0x0D,
	SYSTEM_ALARM_CALLBACK_ACE_TYPE =		0x0E,
	SYSTEM_AUDIT_CALLBACK_OBJECT_ACE_TYPE =	0x0F,
	SYSTEM_ALARM_CALLBACK_OBJECT_ACE_TYPE =	0x10,
	SYSTEM_MANDATORY_LABEL_ACE_TYPE =		0x11,
	SYSTEM_RESOURCE_ATTRIBUTE_ACE_TYPE =	0x12,
	SYSTEM_SCOPED_POLICY_ID_ACE_TYPE =		0x13
} ace_types;
#define ace_types_count 20

typedef struct {
	uint8_t value;
	const char*const name;
} enum8_desc;

#define ace_types_desc_value { \
		enum_desc_item(ACCESS_ALLOWED_ACE_TYPE), \
		enum_desc_item(ACCESS_DENIED_ACE_TYPE), \
		enum_desc_item(SYSTEM_AUDIT_ACE_TYPE), \
		enum_desc_item(SYSTEM_ALARM_ACE_TYPE), \
		enum_desc_item(ACCESS_ALLOWED_COMPOUND_ACE_TYPE), \
		enum_desc_item(ACCESS_ALLOWED_OBJECT_ACE_TYPE), \
		enum_desc_item(ACCESS_DENIED_OBJECT_ACE_TYPE), \
		enum_desc_item(SYSTEM_AUDIT_OBJECT_ACE_TYPE), \
		enum_desc_item(SYSTEM_ALARM_OBJECT_ACE_TYPE), \
		enum_desc_item(ACCESS_ALLOWED_CALLBACK_ACE_TYPE), \
		enum_desc_item(ACCESS_DENIED_CALLBACK_ACE_TYPE), \
		enum_desc_item(ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE), \
		enum_desc_item(ACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE), \
		enum_desc_item(SYSTEM_AUDIT_CALLBACK_ACE_TYPE), \
		enum_desc_item(SYSTEM_ALARM_CALLBACK_ACE_TYPE), \
		enum_desc_item(SYSTEM_AUDIT_CALLBACK_OBJECT_ACE_TYPE), \
		enum_desc_item(SYSTEM_ALARM_CALLBACK_OBJECT_ACE_TYPE), \
		enum_desc_item(SYSTEM_MANDATORY_LABEL_ACE_TYPE), \
		enum_desc_item(SYSTEM_RESOURCE_ATTRIBUTE_ACE_TYPE), \
		enum_desc_item(SYSTEM_SCOPED_POLICY_ID_ACE_TYPE), \
		{-1, "UNKNOWN TYPE"} \
}

typedef enum {
	CONTAINER_INHERIT_ACE =		0x02,
	FAILED_ACCESS_ACE_FLAG =	0x80,
	INHERIT_ONLY_ACE =			0x08,
	INHERITED_ACE =				0x10,
	NO_PROPAGATE_INHERIT_ACE =	0x04,
	OBJECT_INHERIT_ACE =		0x01,
	SUCCESSFUL_ACCESS_ACE_FLAG=	0x40
} ace_flags;
#define ace_flags_count 7

#define ace_flags_desc_value { \
		enum_desc_item(CONTAINER_INHERIT_ACE), \
		enum_desc_item(FAILED_ACCESS_ACE_FLAG), \
		enum_desc_item(INHERIT_ONLY_ACE), \
		enum_desc_item(INHERITED_ACE), \
		enum_desc_item(NO_PROPAGATE_INHERIT_ACE), \
		enum_desc_item(OBJECT_INHERIT_ACE), \
		enum_desc_item(SUCCESSFUL_ACCESS_ACE_FLAG) \
}

#pragma pack(push,1)
typedef struct {
	uint8_t type;
	uint8_t flags;
	uint16_t size;
	uint32_t mask;
	uint8_t sid[];
} __attribute__ ((__packed__)) ace_mask_sid_struct;
#pragma pack(pop)
#define ace_mask_sid_struct_size 0x08

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
#define access_mask_flags_count 11

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
