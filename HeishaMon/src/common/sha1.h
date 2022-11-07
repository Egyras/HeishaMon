/*******************************************************************************
 * Teeny SHA-1
 *
 * The below sha1digest() calculates a SHA-1 hash value for a
 * specified data buffer and generates a hex representation of the
 * result.  This implementation is a re-forming of the SHA-1 code at
 * https://github.com/jinqiangshou/EncryptionLibrary.
 *
 * Copyright (c) 2017 CTrabant
 *
 * License: MIT, see included LICENSE file for details.
 *
 * To use the sha1digest() function either copy it into an existing
 * project source code file or include this file in a project and put
 * the declaration (example below) in the sources files where needed.
 ******************************************************************************/

int sha1digest(uint8_t *digest, char *hexdigest, const uint8_t *data, size_t databytes);