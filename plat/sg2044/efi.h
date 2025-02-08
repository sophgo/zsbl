#ifndef __EFI_H__
#define __EFI_H__

#include <stdint.h>
#include <wchar.h>

#pragma pack(1)

#define EFI_FV_FILE_ATTRIB_ALIGNMENT		0x0000001F
#define EFI_FV_FILE_ATTRIB_FIXED		0x00000100
#define EFI_FV_FILE_ATTRIB_MEMORY_MAPPED	0x00000200

//
// Attributes bit definitions
//
#define EFI_FVB2_READ_DISABLED_CAP		0x00000001
#define EFI_FVB2_READ_ENABLED_CAP		0x00000002
#define EFI_FVB2_READ_STATUS			0x00000004
#define EFI_FVB2_WRITE_DISABLED_CAP		0x00000008
#define EFI_FVB2_WRITE_ENABLED_CAP		0x00000010
#define EFI_FVB2_WRITE_STATUS			0x00000020
#define EFI_FVB2_LOCK_CAP			0x00000040
#define EFI_FVB2_LOCK_STATUS			0x00000080
#define EFI_FVB2_STICKY_WRITE			0x00000200
#define EFI_FVB2_MEMORY_MAPPED			0x00000400
#define EFI_FVB2_ERASE_POLARITY			0x00000800
#define EFI_FVB2_READ_LOCK_CAP			0x00001000
#define EFI_FVB2_READ_LOCK_STATUS		0x00002000
#define EFI_FVB2_WRITE_LOCK_CAP			0x00004000
#define EFI_FVB2_WRITE_LOCK_STATUS		0x00008000
#define EFI_FVB2_ALIGNMENT			0x001F0000
#define EFI_FVB2_ALIGNMENT_1			0x00000000
#define EFI_FVB2_ALIGNMENT_2			0x00010000
#define EFI_FVB2_ALIGNMENT_4			0x00020000
#define EFI_FVB2_ALIGNMENT_8			0x00030000
#define EFI_FVB2_ALIGNMENT_16			0x00040000
#define EFI_FVB2_ALIGNMENT_32			0x00050000
#define EFI_FVB2_ALIGNMENT_64			0x00060000
#define EFI_FVB2_ALIGNMENT_128			0x00070000
#define EFI_FVB2_ALIGNMENT_256			0x00080000
#define EFI_FVB2_ALIGNMENT_512			0x00090000
#define EFI_FVB2_ALIGNMENT_1K			0x000A0000
#define EFI_FVB2_ALIGNMENT_2K			0x000B0000
#define EFI_FVB2_ALIGNMENT_4K			0x000C0000
#define EFI_FVB2_ALIGNMENT_8K			0x000D0000
#define EFI_FVB2_ALIGNMENT_16K			0x000E0000
#define EFI_FVB2_ALIGNMENT_32K			0x000F0000
#define EFI_FVB2_ALIGNMENT_64K			0x00100000
#define EFI_FVB2_ALIGNMENT_128K			0x00110000
#define EFI_FVB2_ALIGNMENT_256K			0x00120000
#define EFI_FVB2_ALIGNMENT_512K			0x00130000
#define EFI_FVB2_ALIGNMENT_1M			0x00140000
#define EFI_FVB2_ALIGNMENT_2M			0x00150000
#define EFI_FVB2_ALIGNMENT_4M			0x00160000
#define EFI_FVB2_ALIGNMENT_8M			0x00170000
#define EFI_FVB2_ALIGNMENT_16M			0x00180000
#define EFI_FVB2_ALIGNMENT_32M			0x00190000
#define EFI_FVB2_ALIGNMENT_64M			0x001A0000
#define EFI_FVB2_ALIGNMENT_128M			0x001B0000
#define EFI_FVB2_ALIGNMENT_256M			0x001C0000
#define EFI_FVB2_ALIGNMENT_512M			0x001D0000
#define EFI_FVB2_ALIGNMENT_1G			0x001E0000
#define EFI_FVB2_ALIGNMENT_2G			0x001F0000
#define EFI_FVB2_WEAK_ALIGNMENT			0x80000000

struct efi_guid {
	uint32_t d0;
	uint16_t d1;
	uint16_t d2;
	uint8_t d3[8];
};

struct block_map_entry {
	///
	/// The number of sequential blocks which are of the same size.
	///
	uint32_t blocks;
	///
	/// The size of the blocks.
	///
	uint32_t len;
};

///
/// Describes the features and layout of the firmware volume.
///
struct efi_firmware_volume_header {
	///
	/// The first 16 bytes are reserved to allow for the reset vector of
	/// processors whose reset vector is at address 0.
	///
	uint8_t zero[16];
	///
	/// Declares the file system with which the firmware volume is formatted.
	///
	struct efi_guid guid;
	///
	/// Length in bytes of the complete firmware volume, including the header.
	///
	uint64_t volume_len;
	///
	/// Set to EFI_FVH_SIGNATURE
	///
	uint32_t sig;
	///
	/// Declares capabilities and power-on defaults for the firmware volume.
	///
	uint32_t attr;
	///
	/// Length in bytes of the complete firmware volume header.
	///
	uint16_t header_len;
	///
	/// A 16-bit checksum of the firmware volume header. A valid header sums to zero.
	///
	uint16_t checksum;
	///
	/// Offset, relative to the start of the header, of the extended header
	/// (EFI_FIRMWARE_VOLUME_EXT_HEADER) or zero if there is no extended header.
	///
	uint16_t ext_hdr_off;
	///
	/// This field must always be set to zero.
	///
	uint8_t reserved[1];
	///
	/// Set to 2. Future versions of this specification may define new header fields and will
	/// increment the Revision field accordingly.
	///
	uint8_t version;
	///
	/// An array of run-length encoded FvBlockMapEntry structures. The array is
	/// terminated with an entry of {0,0}.
	///
	struct block_map_entry block_map[1];
};

#define SIGNATURE_16(A, B)		((A) | (B << 8))
#define SIGNATURE_32(A, B, C, D)	(SIGNATURE_16 (A, B) | (SIGNATURE_16 (C, D) << 16))

#define EFI_FVH_SIGNATURE SIGNATURE_32('_', 'F', 'V', 'H')

///
/// Firmware Volume Header Revision definition
///
#define EFI_FVH_REVISION 0x02

#define EFI_SYSTEM_NVDATA_FV_GUID					\
	{								\
		0xFFF12B8D, 0x7696, 0x4C8B,				\
		{							\
			0xA9, 0x85, 0x27, 0x47, 0x07, 0x5B, 0x4F, 0x50	\
		}							\
	}

#define EFI_VARIABLE_GUID                                              \
	{                                                              \
		0xddcf3616, 0x3275, 0x4164,                            \
		{                                                      \
			0x98, 0xb6, 0xfe, 0x85, 0x70, 0x7f, 0xfe, 0x7d \
		}                                                      \
	}

#define EFI_AUTHENTICATED_VARIABLE_GUID                                \
	{                                                              \
		0xaaf32c78, 0x947b, 0x439a,                            \
		{                                                      \
			0xa1, 0x80, 0x2e, 0x14, 0x4e, 0xc3, 0x77, 0x92 \
		}                                                      \
	}

///
/// Alignment of variable name and data, according to the architecture:
/// * For IA-32 and Intel(R) 64 architectures: 1.
///
#define ALIGNMENT 1

//
// GET_PAD_SIZE calculates the miminal pad bytes needed to make the current pad size satisfy the alignment requirement.
//
#if (ALIGNMENT == 1)
#define GET_PAD_SIZE(a) (0)
#else
#define GET_PAD_SIZE(a) (((~a) + 1) & (ALIGNMENT - 1))
#endif

///
/// Alignment of Variable Data Header in Variable Store region.
///
#define HEADER_ALIGNMENT 4
#define HEADER_ALIGN(Header) \
	(((uint64_t)(Header) + HEADER_ALIGNMENT - 1) & (~(HEADER_ALIGNMENT - 1)))

///
/// Status of Variable Store Region.
///
enum variable_store_status { EfiRaw, EfiValid, EfiInvalid, EfiUnknown };


#define VARIABLE_STORE_SIGNATURE EFI_VARIABLE_GUID
#define AUTHENTICATED_VARIABLE_STORE_SIGNATURE EFI_AUTHENTICATED_VARIABLE_GUID

///
/// Variable Store Header Format and State.
///
#define VARIABLE_STORE_FORMATTED	0x5a
#define VARIABLE_STORE_HEALTHY		0xfe

///
/// Variable Store region header.
///
struct efi_variable_store_header {
	///
	/// Variable store region signature.
	///
	struct efi_guid guid;
	///
	/// Size of entire variable store,
	/// including size of variable store header but not including the size of FvHeader.
	///
	uint32_t size;
	///
	/// Variable region format state.
	///
	uint8_t format;
	///
	/// Variable region healthy state.
	///
	uint8_t status;
	uint16_t reserved0;
	uint32_t reserved1;
};

///
/// Variable data start flag.
///
#define VARIABLE_DATA 0x55AA

///
/// Variable State flags.
///
#define VAR_IN_DELETED_TRANSITION	0xfe ///< Variable is in obsolete transition.
#define VAR_DELETED			0xfd ///< Variable is obsolete.
#define VAR_HEADER_VALID_ONLY		0x7f ///< Variable header has been valid.
#define VAR_ADDED			0x3f ///< Variable has been completely added.

/////
/// Single Variable Data Header Structure.
///
struct efi_variable_header {
	///
	/// Variable Data Start Flag.
	///
	uint16_t id;
	///
	/// Variable State defined above.
	///
	uint8_t status;
	uint8_t reserved;
	///
	/// Attributes of variable defined in UEFI specification.
	///
	uint32_t attr;
	///
	/// Size of variable null-terminated Unicode string name.
	///
	uint32_t name_size;
	///
	/// Size of the variable data without this header.
	///
	uint32_t data_size;
	///
	/// A unique identifier for the vendor that produces and consumes this varaible.
	///
	struct efi_guid vendor_guid;
};

/// EFI Time Abstraction:
///  Year:       1900 - 9999
///  Month:      1 - 12
///  Day:        1 - 31
///  Hour:       0 - 23
///  Minute:     0 - 59
///  Second:     0 - 59
///  Nanosecond: 0 - 999,999,999
///  TimeZone:   -1440 to 1440 or 2047
///
struct efi_time {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t pad0;
	uint32_t nanosecond;
	int16_t time_zone;
	uint8_t daylight;
	uint8_t pad1;
};

///
/// Single Authenticated Variable Data Header Structure.
///
struct efi_authenticated_variable_header {
	///
	/// Variable Data Start Flag.
	///
	uint16_t id;
	///
	/// Variable State defined above.
	///
	uint8_t status;
	uint8_t reserved;
	///
	/// Attributes of variable defined in UEFI specification.
	///
	uint32_t attr;
	///
	/// Associated monotonic count value against replay attack.
	///
	uint64_t monotonic_count;
	///
	/// Associated TimeStamp value against replay attack.
	///
	struct efi_time time_stamp;
	///
	/// Index of associated public key in database.
	///
	uint32_t pub_key_index;
	///
	/// Size of variable null-terminated Unicode string name.
	///
	uint32_t name_size;
	///
	/// Size of the variable data without this header.
	///
	uint32_t data_size;
	///
	/// A unique identifier for the vendor that produces and consumes this varaible.
	///
	struct efi_guid vendor_guid;
};

#pragma pack()

struct variable_zone {
	struct efi_firmware_volume_header *fv;
	struct efi_variable_store_header *vs;
	struct efi_variable_header *var;
	struct efi_variable_header *end;
	int auth;
};

int efi_vz_init(struct variable_zone *vz, void *fv);

struct efi_variable_header *
efi_vz_find_variable(struct variable_zone *vz,
		     const struct efi_guid *vendor_guid, const wchar_t *name);
wchar_t *efi_var_get_name(struct efi_variable_header *var);
void *efi_var_get_data(struct efi_variable_header *var);

#endif
