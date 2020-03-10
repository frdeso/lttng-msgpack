#include "msgpack.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define RUN_TEST(func, buf, values, nb_values, expected)                \
	do {                                                            \
		memset(buf, 0, 4096);                                   \
		func(buf, values, nb_values);                           \
		int _ret = memcmp(buf, expected, sizeof(expected));     \
		if (_ret == 0)                                          \
			printf("Test %s(%s) passed\n", #func, #values); \
		else                                                    \
			printf("Test %s(%s) failed\n", #func, #values); \
	} while (0);

#define BUFFER_SIZE 4096
static void string_test(uint8_t *buf, const char *value, int unused2)
{
	struct lttng_msgpack_writer writer;

	lttng_msgpack_writer_init(&writer, buf, BUFFER_SIZE);
	lttng_msgpack_write_str(&writer, value);
	lttng_msgpack_writer_fini(&writer);
}

static void int_test(uint8_t *buf, int64_t value, int unused2)
{
	struct lttng_msgpack_writer writer;

	lttng_msgpack_writer_init(&writer, buf, BUFFER_SIZE);
	lttng_msgpack_write_i64(&writer, value);

	lttng_msgpack_writer_fini(&writer);
}

static void uint_test(uint8_t *buf, uint64_t value, int unused2)
{
	struct lttng_msgpack_writer writer;

	lttng_msgpack_writer_init(&writer, buf, BUFFER_SIZE);
	lttng_msgpack_write_u64(&writer, value);
	lttng_msgpack_writer_fini(&writer);
}

static void float_test(uint8_t *buf, double value, int unused2)
{
	struct lttng_msgpack_writer writer;

	lttng_msgpack_writer_init(&writer, buf, BUFFER_SIZE);
	lttng_msgpack_write_f64(&writer, value);
	lttng_msgpack_writer_fini(&writer);
}

static void array_double_test(uint8_t *buf, double *values, size_t nb_values)
{
	int i = 0;
	struct lttng_msgpack_writer writer;

	lttng_msgpack_writer_init(&writer, buf, BUFFER_SIZE);
	lttng_msgpack_begin_array(&writer, nb_values);

	for (i = 0; i < nb_values; i++) {
		lttng_msgpack_write_f64(&writer, values[i]);
	}

	lttng_msgpack_end_array(&writer);
	lttng_msgpack_writer_fini(&writer);
}

static void simple_capture_example(uint8_t *buf, int unused, int unused2)
{
	/*
	 * This testcase tests the following json representation:
	 * {"id":17,"captures":["meow mix",18, null,14.197]}
	 */
	struct lttng_msgpack_writer writer;

	lttng_msgpack_writer_init(&writer, buf, BUFFER_SIZE);
	lttng_msgpack_begin_map(&writer, 2);

	lttng_msgpack_write_str(&writer, "id");
	lttng_msgpack_write_u64(&writer, 17);

	lttng_msgpack_write_str(&writer, "captures");
	lttng_msgpack_begin_array(&writer, 4);

	lttng_msgpack_write_str(&writer, "meow mix");
	lttng_msgpack_write_u64(&writer, 18);
	lttng_msgpack_write_nil(&writer);
	lttng_msgpack_write_f64(&writer, 14.197);

	lttng_msgpack_end_array(&writer);

	lttng_msgpack_end_map(&writer);
	lttng_msgpack_writer_fini(&writer);
}

static void nil_test(uint8_t *buf, int unused, int unused2)
{
	struct lttng_msgpack_writer writer;

	lttng_msgpack_writer_init(&writer, buf, BUFFER_SIZE);
	lttng_msgpack_write_nil(&writer);
	lttng_msgpack_writer_fini(&writer);
}

int main(int argc, char *argv[])
{
	uint8_t buf[BUFFER_SIZE] = {0};
	double arr_double[] = {1.1, 2.3, -12345.2};

	RUN_TEST(nil_test, buf, 0, 0, "\xc0");
	RUN_TEST(string_test, buf, "bye", 1, "\xa3\x62\x79\x65");
	RUN_TEST(uint_test, buf, 1337, 1,
			"\xcf\x00\x00\x00\x00\x00\x00\x05\x39");
	RUN_TEST(int_test, buf, -4242, 1,
			"\xd3\xff\xff\xff\xff\xff\xff\xef\x6e");
	RUN_TEST(float_test, buf, 0.0, 1,
			"\xcb\x00\x00\x00\x00\x00\x00\x00\x00");
	RUN_TEST(float_test, buf, 3.14159265, 1,
			"\xcb\x40\x09\x21\xfb\x53\xc8\xd4\xf1");
	RUN_TEST(float_test, buf, -3.14159265, 1,
			"\xcb\xc0\x09\x21\xfb\x53\xc8\xd4\xf1");

	RUN_TEST(array_double_test, buf, arr_double,
			sizeof(arr_double) / sizeof(arr_double[0]),
			"\x93" // fixarray size 3
			"\xcb\x3f\xf1\x99\x99\x99\x99\x99\x9a"
			"\xcb\x40\x02\x66\x66\x66\x66\x66\x66"
			"\xcb\xc0\xc8\x1c\x99\x99\x99\x99\x9a");

	RUN_TEST(simple_capture_example, buf, 0, 0,
			"\x82" // fixmap size 2
			"\xa2\x69\x64" // fixstr size2
			"\xcf\x00\x00\x00\x00\x00\x00\x00\x11"
			"\xa8\x63\x61\x70\x74\x75\x72\x65\x73"
			"\x94" // fixarray size 4
			"\xa8\x6d\x65\x6f\x77\x20\x6d\x69\x78"
			"\xcf\x00\x00\x00\x00\x00\x00\x00\x12"
			"\xc0"
			"\xcb\x40\x2c\x64\xdd\x2f\x1a\x9f\xbe");

	//	fwrite(buf, 1, BUFFER_SIZE, stdout);

	return 0;
}
