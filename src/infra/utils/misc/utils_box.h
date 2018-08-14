#ifndef _UTILS_BOX_H_
#define _UTILS_BOX_H_

#ifndef _IN_
    #define _IN_
#endif

#ifndef _OU_
    #define _OU_
#endif

/**
 * @brief Generate a random string with specified length
 *        This function generates a random string with specified length.
 *
 * @param random. The character array for storing random string generated
 * @param length. The length of random
 *
 * @warning The length of character array must be length + 1
 * @return success or fail.
 *
 */
int ubox_random_string(_IN_ char *random, _IN_ int length);

#endif
