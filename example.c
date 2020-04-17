/* Example use of Reed-Solomon library 
 *
 * Copyright Henry Minsky (hqm@alum.mit.edu) 1991-2009
 *
 * This software library is licensed under terms of the GNU GENERAL
 * PUBLIC LICENSE
 *
 * RSCODE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RSCODE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rscode.  If not, see <http://www.gnu.org/licenses/>.

 * Commercial licensing is available under a separate license, please
 * contact author for details.
 *
 * This same code demonstrates the use of the encodier and 
 * decoder/error-correction routines. 
 *
 * We are assuming we have at least four bytes of parity (NPAR >= 4).
 * 
 * This gives us the ability to correct up to two errors, or 
 * four erasures. 
 *
 * In general, with E errors, and K erasures, you will need
 * 2E + K bytes of parity to be able to correct the codeword
 * back to recover the original message data.
 *
 * You could say that each error 'consumes' two bytes of the parity,
 * whereas each erasure 'consumes' one byte.
 *
 * Thus, as demonstrated below, we can inject one error (location unknown)
 * and two erasures (with their locations specified) and the 
 * error-correction routine will be able to correct the codeword
 * back to the original message.
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include "ecc.h"
 
//unsigned char msg[] = "Nervously I loaded the twin ducks aboard the revolving platform.";
unsigned char msg[] = {0xca, 0x17, 0x02, 0x00, 0x2c, 0x96, 0x32, 0xff, 0x02, 0xdc, 0xdc, 0x51, 0x58, 0x57, 0x57, 0x28, 0x52, 0xf0, 0x87, 0xab, 0xb2, 0x02, 0xa0, 0x0f, 0xde, 0xdc, 0x56, 0x58, 0x57, 0x57, 0x27, 0x54, 0xf0, 0xe2, 0x11, 0x31, 0x87, 0x24, 0x2a, 0x1e, 0xd5, 0x2d, 0x17, 0x1c, 0xa6, 0x34, 0x80, 0x7f, 0x7b, 0xc9, 0x17, 0x02, 0x00, 0x0f, 0xde, 0xde, 0x5b, 0x58, 0x57, 0x1f, 0x2a, 0x55, 0xf0, 0x10, 0xe6, 0x33, 0x35, 0x1b, 0x5a, 0x1e, 0x5f, 0x30, 0x8f, 0x27, 0x1f, 0x07, 0x80, 0x7f, 0x7b, 0x51, 0x17, 0x02, 0x00, 0x0f, 0xde, 0xdf, 0x56, 0x58, 0x56, 0x57, 0x29, 0x54, 0xb8, 0xe2, 0x3e, 0x33, 0xe8, 0x2d, 0x77, 0x1c, 0x5c, 0x24, 0x8e, 0x29, 0x7e, 0x22, 0x7f, 0x7f, 0x7b, 0xd9, 0x16, 0x02, 0x00, 0x0f, 0xe0, 0xdd, 0x5d, 0x58, 0x57, 0x58, 0x2a, 0x55, 0x24, 0xe2, 0x7f, 0x32, 0x5d, 0x2f, 0xc8, 0x1c, 0x8a, 0x1d, 0x33, 0x23, 0x16, 0x2f, 0x7f, 0x7f, 0x2a, 0x61, 0x16, 0x02, 0x00, 0x0f, 0xdd, 0xde, 0x56, 0x58, 0x57, 0x58, 0x29, 0x54, 0xf0, 0xe2, 0xe7, 0x32, 0x59, 0x26, 0xe8, 0x1c, 0xc0, 0x28, 0xe8, 0x1c, 0x3d, 0x2f, 0x7f, 0x7f, 0x7b, 0xe9, 0x15, 0xc9, 0xe5, 0x0e, 0x01, 0x62, 0xb5, 0x4a, 0x62, 0xbb, 0x49, 0x62, 0x9b, 0x29, 0xa3, 0xb5, 0x49, 0xac, 0x9b, 0x2f, 0xcf, 0x9b, 0x29, 0xcf, 0x9c, 0x29, 0xa3, 0x9c, 0x29, 0xcf, 0xd1, 0x3d, 0xcf, 0x1c, 0x06, 0x62, 0x1c, 0x05, 0x62, 0x1c, 0x04, 0x62, 0x1c, 0x03, 0x4b, 0x00, 0x00, 0x7c, 0x6d, 0xff, 0x5d, 0xb7, 0xac, 0x41, 0x0e, 0xe6, 0xa0, 0xf7, 0x0e, 0xb1, 0xeb, 0x88, 0xd6, 0xcd, 0xb3, 0xd4, 0xc5, 0xf0, 0xf6, 0xfb, 0x30, 0xf6, 0xb6, 0x7b, 0x02, 0xaa, 0xb0, 0x25, 0xbd};
unsigned char codeword[256];
 
/* Some debugging routines to introduce errors or erasures
   into a codeword. 
   */

/* Introduce a byte error at LOC */
void
byte_err (int err, int loc, unsigned char *dst)
{
  printf("Adding Error at loc %d, data %#x\n", loc, dst[loc-1]);
  dst[loc-1] ^= err;
}

/* Pass in location of error (first byte position is
   labeled starting at 1, not 0), and the codeword.
*/
void
byte_erasure (int loc, unsigned char dst[], int cwsize, int erasures[]) 
{
  printf("Erasure at loc %d, data %#x\n", loc, dst[loc-1]);
  dst[loc-1] = 0;
}


int
main (int argc, char *argv[])
{
#define ML (sizeof(msg))
  initialize_ecc ();
  for (int i = 0; i < 30000; i++) {
    decode_data(msg, ML);
    int erasures[] = {(249 * i) % 15, (249 * i) % 16};
    if (check_syndrome() != 0) {
      correct_errors_erasures(msg, ML, 2, erasures);
    }
    encode_data(msg, ML, codeword);
  }
  exit(0);
}
//  int erasures[16];
//  int nerasures = 0;
//
//  /* Initialization the ECC library */
// 
//  initialize_ecc ();
// 
//  /* ************** */
// 
//  /* Encode data into codeword, adding NPAR parity bytes */
//  encode_data(msg, sizeof(msg), codeword);
//
//  printf("parity bytes:\n");
//  for (int i = 0; i < NPAR; i++) {
//      printf("%x ", codeword[sizeof(msg) + i]);
//      if (i % 8 == 7) {
//          printf("\n");
//      }
//  }
//  exit(0);
//}
//
// 
//  printf("Encoded data is: \"%s\"\n", codeword);
// 
//#define ML (sizeof (msg) + NPAR)
//
//
//  /* Add one error and two erasures */
//  byte_err(0x35, 3, codeword);
//
//  byte_err(0x23, 17, codeword);
//  byte_err(0x34, 19, codeword);
//
//
//  printf("with some errors: \"%s\"\n", codeword);
//
//  /* We need to indicate the position of the erasures.  Eraseure
//     positions are indexed (1 based) from the end of the message... */
//
//  erasures[nerasures++] = ML-17;
//  erasures[nerasures++] = ML-19;
//
// 
//  /* Now decode -- encoded codeword size must be passed */
//  decode_data(codeword, ML);
//
//  /* check if syndrome is all zeros */
//  if (check_syndrome () != 0) {
//    correct_errors_erasures (codeword, 
//			     ML,
//			     nerasures, 
//			     erasures);
// 
//    printf("Corrected codeword: \"%s\"\n", codeword);
//  }
// 
//  exit(0);
//}
//
