#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>

#include "ss.h"
#include "numtheory.h"
#include "randstate.h"

//miles
uint64_t random_number_btw(uint64_t lower, uint64_t upper) {
    uint64_t range = upper - lower;
    uint64_t random_num = (uint64_t) random();
    return lower + (random_num % range);
}

//
// Generates the components for a new SS key.
//
// Provides:
//  p:  first prime
//  q: second prime
//  n: public modulus/exponent
//
// Requires:
//  nbits: minimum # of bits in n
//  iters: iterations of Miller-Rabin to use for primality check
//  all mpz_t arguments to be initialized
//
void ss_make_pub(mpz_t p, mpz_t q, mpz_t n, uint64_t nbits, uint64_t iters) {
    mpz_t p_value, q_value, p_minus_1, q_minus_1;
    mpz_inits(p_value, q_value, p_minus_1, q_minus_1, NULL);

    bool p_flag = true;
    bool q_flag = true;

    //generate a random number btw certain range for p
    uint64_t p_bits = random_number_btw(nbits / 5, (2 * nbits) / 5);
    //the bits from p will be contributed to n twice, the remaining bits will go to q
    uint64_t q_bits = nbits - (p_bits * 2);

    //either p or q flag is true, keep looping
    while (p_flag == true || q_flag == true) {
        make_prime(p_value, p_bits, iters);
        make_prime(q_value, q_bits, iters);

        //Check p doesn't divide q-1
        mpz_sub_ui(p_minus_1, p_value, 1);
        if (mpz_divisible_p(p_minus_1, q)) {
            continue;
        }
        //Check q doesn't divide p-1
        mpz_sub_ui(q_minus_1, q_value, 1);
        if (mpz_divisible_p(q_minus_1, p)) {
            continue;
        }

        //once found, set flags to false to exit.
        p_flag = false;
        q_flag = false;
    }
    //find value of n
    mpz_mul(n, p_value, p_value);
    mpz_mul(n, n, q_value);

    // Copy the values of p and q to the output parameters.
    mpz_set(p, p_value);
    mpz_set(q, q_value);

    mpz_clears(p_value, q_value, p_minus_1, q_minus_1, NULL);
}

//
// Generates components for a new SS private key.
//
// Provides:
//  d:  private exponent
//  pq: private modulus
//
// Requires:
//  p:  first prime number
//  q: second prime number
//  all mpz_t arguments to be initialized
//
void ss_make_priv(mpz_t d, mpz_t pq, mpz_t p, mpz_t q) {
    //To compute d, simply compute
    //the inverse of n modulo λ(pq) = lcm(p − 1,q − 1).

    mpz_t n, p_minus_1, q_minus_1, phi_pq, gcd_pq, lamda_n;
    mpz_inits(n, p_minus_1, q_minus_1, phi_pq, gcd_pq, lamda_n, NULL);

    //make pq
    mpz_mul(pq, p, q);

    //make p - 1 and q - 1
    mpz_sub_ui(p_minus_1, p, 1);
    mpz_sub_ui(q_minus_1, q, 1);

    //ϕ(pq) = (p − 1)(q − 1)
    mpz_mul(phi_pq, p_minus_1, q_minus_1);
    //gcd of p - 1 amd q - 1
    gcd(gcd_pq, p_minus_1, q_minus_1);

    //λ(n) = ϕ(pq) / gcd of p - 1 amd q - 1
    mpz_fdiv_q(lamda_n, phi_pq, gcd_pq);

    //find n using pq
    mpz_mul(n, p, pq);

    //inverse of n modulo λ(pq) AKA lamda_n
    mod_inverse(d, n, lamda_n);
    mpz_clears(n, p_minus_1, q_minus_1, phi_pq, gcd_pq, lamda_n, NULL);
}

//
// Export SS public key to output stream
//
// Requires:
//  n: public modulus/exponent
//  username: login name of keyholder ($USER)
//  pbfile: open and writable file stream
//
void ss_write_pub(mpz_t n, char username[], FILE *pbfile) {
    // n written as a hexstring
    // https://gmplib.org/manual/Formatted-Output-Strings
    gmp_fprintf(pbfile, "%ZX\n%s\n", n, username);
}

//
// Export SS private key to output stream
//
// Requires:
//  pq: private modulus
//  d:  private exponent
//  pvfile: open and writable file stream
//
void ss_write_priv(mpz_t pq, mpz_t d, FILE *pvfile) {
    // pq and d written as a hexstring
    gmp_fprintf(pvfile, "%ZX\n%ZX\n", pq, d);
}

//
// Import SS public key from input stream
//
// Provides:
//  n: public modulus
//  username: $USER of the pubkey creator
//
// Requires:
//  pbfile: open and readable file stream
//  username: requires sufficient space
//  all mpz_t arguments to be initialized
//
void ss_read_pub(mpz_t n, char username[], FILE *pbfile) {
    gmp_fscanf(pbfile, "%ZX\n%s\n", n, username);
}

//
// Import SS private key from input stream
//
// Provides:
//  pq: private modulus
//  d:  private exponent
//
// Requires:
//  pvfile: open and readable file stream
//  all mpz_t arguments to be initialized
//
void ss_read_priv(mpz_t pq, mpz_t d, FILE *pvfile) {
    gmp_fscanf(pvfile, "%ZX\n%ZX\n", pq, d);
}

//
// Encrypt number m into number c
//
// Provides:
//  c: encrypted integer
//
// Requires:
//  m: original integer
//  n: public exponent/modulus
//  all mpz_t arguments to be initialized
//
void ss_encrypt(mpz_t c, mpz_t m, mpz_t n) {
    pow_mod(c, m, n, n);
}

//
// Encrypt an arbitrary file
//
// Provides:
//  fills outfile with the encrypted contents of infile
//
// Requires:
//  infile: open and readable file stream
//  outfile: open and writable file stream
//  n: public exponent and modulus
//
void ss_encrypt_file(FILE *infile, FILE *outfile, mpz_t n) {

    mpz_t sqrt_n;
    mpz_init(sqrt_n);

    uint64_t k;

    // Calculate the block size k.
    // find square root of n
    // https://gmplib.org/manual/Integer-Roots
    mpz_sqrt(sqrt_n, n);
    k = (mpz_sizeinbase(sqrt_n, 2) - 1) / 8;

    // Dynamically allocate an array that can hold k bytes.
    // This array should be of type (uint8_t *) and
    // will serve as the block.
    uint8_t *block = (uint8_t *) malloc(k * sizeof(uint8_t));

    // Set the zeroth byte of the block to 0xFF.
    // This effectively prepends the workaround byte that we need.
    block[0] = 0xFF;

    // While there are still unprocessed bytes in infile:
    while (!feof(infile)) {
        // Read at most k−1 bytes in from infile, and let j be the number of bytes actually read.
        // Place the read bytes into the allocated block starting
        // from index 1 so as to not overwrite the 0xFF.
        // starting from block[1],
        // fread(void *ptr, size_t size, size_t count, FILE *stream);
        uint64_t j = fread(&block[1], sizeof(uint8_t), k - 1, infile);

        // Using mpz_import(), convert the read bytes, including the prepended 0xFF into an mpz_t m.
        // You will want to set the order parameter of mpz_import() to 1 for most significant word
        // first, 1 for the endian parameter, and 0 for the nails parameter.
        mpz_t m, c;
        mpz_inits(m, c, NULL);
        // mpz_import(rop, count, order, size, endian, nails, limbs);
        mpz_import(m, j + 1, 1, sizeof(uint8_t), 1, 0, block);

        // Encrypt m with ss_encrypt(), then write the encrypted number to outfile as a hexstring
        // followed by a trailing newline.
        ss_encrypt(c, m, n);
        gmp_fprintf(outfile, "%ZX\n", c);

        mpz_clears(m, c, NULL);
    }

    // Clean up
    mpz_clear(sqrt_n);
    free(block);
}

//
// Decrypt number c into number m
//
// Provides:
//  m: decrypted/original integer
//
// Requires:
//  c: encrypted integer
//  d: private exponent
//  pq: private modulus
//  all mpz_t arguments to be initialized
//
void ss_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t pq) {
    pow_mod(m, c, d, pq);
}

//
// Decrypt a file back into its original form.
//
// Provides:
//  fills outfile with the unencrypted data from infile
//
// Requires:
//  infile: open and readable file stream to encrypted data
//  outfile: open and writable file stream
//  d: private exponent
//  pq: private modulus
//
void ss_decrypt_file(FILE *infile, FILE *outfile, mpz_t d, mpz_t pq) {

    mpz_t c, m;
    mpz_inits(c, m, NULL);

    uint64_t k;

    // Calculate the block size k.
    k = (mpz_sizeinbase(pq, 2) - 1) / 8;

    // Dynamically allocate an array that can hold k bytes.
    // This array should be of type (uint8_t *) and
    // will serve as the block.
    uint8_t *block = (uint8_t *) malloc(k * sizeof(uint8_t));

    // Iterating over the lines in infile:
    // && gmp_fscanf(infile, "%ZX\n", c)
    while (!feof(infile)) {
        // Scan in a hexstring, saving the hexstring as a mpz_t c.
        // Remember, each block is written as a hexstring with a trailing newline when encrypting a file.
        gmp_fscanf(infile, "%ZX\n", c);

        // First decrypt c back into its original value m.
        ss_decrypt(m, c, d, pq);
        // Then using mpz_export(), convert m back into bytes, storing them in the allocated block.
        // Let j be the number of bytes actually converted.
        // You will want to set the order parameter of mpz_export() to 1 for most significant word first,
        // 1 for the endian parameter, and 0 for the nails parameter.
        // mpz_export (*rop, *countp, order, size, endian, nails, const mpz_t op)
        uint64_t j;
        mpz_export(&block[0], &j, 1, sizeof(uint8_t), 1, 0, m);

        // Write out j − 1 bytes starting from index 1 of the block to outfile.
        // This is because index 0 must be prepended 0xFF. Do not output the 0xFF.
        // size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
        fwrite(&block[1], sizeof(uint8_t), j - 1, outfile);
    }

    mpz_clears(c, m, NULL);
    free(block);
}

// int main(void) {
// 	randstate_init(1234);

// 	mpz_t p, q, n, d, pq, c, m, m1;
// 	mpz_inits(p, q, n, d, pq, c, m, m1, NULL);

// 	ss_make_pub(p, q, n, 256, 50);

// 	gmp_printf(" p (%d) is %Zd,\n q (%d) is %Zd,\n n (%d) is %Zd\n", mpz_sizeinbase(p, 2), p, mpz_sizeinbase(q, 2), q, mpz_sizeinbase(n, 2), n);

// 	ss_make_priv(d, pq, p, q);

// 	gmp_printf(" d (%d) is %Zd,\n pq (%d) is %Zd,\n", mpz_sizeinbase(d, 2), d, mpz_sizeinbase(pq, 2), pq);

// 	// mpz_set_ui(m, 12345);
// 	// gmp_printf("before c is %Zd,\n m is %Zd,\n", c, m);
// 	// ss_encrypt(c, m, n);
// 	// gmp_printf("after encrypted c is %Zd,\n m is %Zd,\n", c, m);

// 	// gmp_printf("before c is %Zd,\n m1 is %Zd,\n", c, m1);
// 	// ss_decrypt(m1, c, d, pq);
// 	// gmp_printf("after decrypted c is %Zd,\n m1 is %Zd,\n", c, m1);

// 	FILE *input = stdin;
//     FILE *output = stdout;

//     ss_encrypt_file(input, output, n);
//     ss_decrypt_file(input, output, d, pq);

// 	mpz_clears(p, q, n, d, pq, c, m, m1, NULL);
// 	randstate_clear();
// 	return 0;
// }
