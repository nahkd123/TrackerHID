#ifndef _MACROS_H_
#define _MACROS_H_

#define __INT_SIZE_8 int8_t
#define __INT_SIZE_16 int16_t
#define __INT_SIZE_32 int32_t
#define INT_SIZE(size) __INT_SIZE_##size

#define TRY(last_error, x) \
	last_error = x; \
	if (last_error != ESP_OK) return last_error

#endif /* _MACROS_H_ */
