#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef uint32_t u32;
typedef uint64_t u64;

#define __packed __attribute__((packed))
#define __counted_by(member)
#define GENMASK(high, low) 0U

#include "../drivers/accel/amdxdna/aie2_msg_priv.h"

static void test_supported_firmware_is_exactly_pinned(void)
{
	assert(aie2_phoenix_legacy_rw_mem_supported(5, 8, 1, 5, 5, 391));
	assert(!aie2_phoenix_legacy_rw_mem_supported(5, 7, 1, 5, 5, 391));
	assert(!aie2_phoenix_legacy_rw_mem_supported(5, 8, 1, 5, 6, 399));
	assert(!aie2_phoenix_legacy_rw_mem_supported(6, 24, 1, 5, 5, 391));
}

static void test_memory_request_uses_physical_row_column_layout(void)
{
	static const unsigned char expected[] = {
		0x00, 0x00, 0x00, 0x00,
		0x01, 0x03, 0x00, 0x00,
		0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,
		0xcc, 0xbb, 0xaa, 0x99,
		0x08, 0x00, 0x00, 0x00,
	};
	struct phoenix_aie_rw_access_req req;

	aie2_init_phoenix_rw_mem_req(&req, AIE2_ACCESS_TYPE_MEM_READ,
				      1, 1, 2, 0x99aabbcc,
				      0x1122334455667788ULL, 8);

	assert(sizeof(req) == sizeof(expected));
	assert(memcmp(&req, expected, sizeof(expected)) == 0);

	aie2_init_phoenix_rw_mem_req(&req, AIE2_ACCESS_TYPE_MEM_WRITE,
				      2, 1, 0, 0x20, 0x10010000, 16);
	assert(req.type == AIE2_ACCESS_TYPE_MEM_WRITE);
	assert(req.row == 2);
	assert(req.col == 1);
}

int main(void)
{
	test_supported_firmware_is_exactly_pinned();
	test_memory_request_uses_physical_row_column_layout();
	return 0;
}
