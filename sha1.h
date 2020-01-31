// sha1.h : header file
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
//
// Copyleft (GPLv3) 2020  Thomas Mielke
// 
// Dies ist freie Software; Sie dürfen sie unter den Bedingungen der 
// GNU General Public License, wie von der Free Software Foundation 
// veröffentlicht, weiterverteilen und/oder modifizieren; entweder gemäß 
// Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren Version.
//
// Das Entfernen des ursprünglichen Copyright-Hinweises von Brian Gladman ist
// jedoch ausgeschlossen (siehe vorhergehende Release-Info weiter unten).
//
// Diese Software wird in der Hoffnung weiterverbreitet, dass sie nützlich 
// sein wird, jedoch OHNE IRGENDEINE GARANTIE, auch ohne die implizierte 
// Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BESTIMMTEN ZWECK.
// Mehr Details finden Sie in der GNU Lesser General Public License.
//
// Sie sollten eine Kopie der GNU General Public License Version 3 zusammen mit 
// dieser Software erhalten haben; falls nicht, schreiben Sie an die Free 
// Software Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110, USA. 

/*
 ---------------------------------------------------------------------------
 Copyright (c) 2002, Dr Brian Gladman <brg@gladman.me.uk>, Worcester, UK.
 All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary 
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright 
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products 
      built using this software without specific written permission. 

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.
 
 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness 
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
Issue Date: 30/11/2002
*/

#ifndef _SHA1_H
#define _SHA1_H

#include <limits.h>

/* define an unsigned 32-bit type */

#if UINT_MAX == 0xffffffff
  typedef   unsigned int     sha1_32t;
#elif ULONG_MAX == 0xffffffff
  typedef   unsigned long    sha1_32t;
#else
#error Please define sha1_32t as an unsigned 32 bit type in sha2.h
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#define SHA1_BLOCK_SIZE  64
#define SHA1_DIGEST_SIZE 20
#define SHA2_GOOD         0
#define SHA2_BAD          1

/* type to hold the SHA256 context  */

typedef struct
{   sha1_32t count[2];
    sha1_32t hash[5];
    sha1_32t wbuf[16];
} sha1_ctx;

void sha1_compile(sha1_ctx ctx[1]);

void sha1_begin(sha1_ctx ctx[1]);
void sha1_hash(const unsigned char data[], unsigned int len, sha1_ctx ctx[1]);
void sha1_end(unsigned char hval[], sha1_ctx ctx[1]);
void sha1(unsigned char hval[], const unsigned char data[], unsigned int len);

#if defined(__cplusplus)
}
#endif

#include "sha1.cpp"

#endif
