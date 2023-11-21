#include <stdio.h>
#include <stdlib.h> //atof
#include <unistd.h> //getopt().
#include <time.h>
#include <gmp.h>
#include <sys/stat.h>

#include "ss.h"
#include "numtheory.h"
#include "randstate.h"

#define OPTIONS "i:o:n:vh"

int main(int argc, char **argv) {
    int opt = 0;

    // disable verbose by default
    int verbose = 0;

    // file steams
    FILE *input = stdin;
    FILE *output = stdout;
    FILE *priv_key_file;

    // default names for files
    char *input_file_name = NULL;
    char *output_file_name = NULL;
    char *priv_key_name = "ss.priv";

    // help_message
    const char *help_message
        = "SYNOPSIS\n"
          "   Decrypts data using SS decryption.\n"
          "   Encrypted data is encrypted by the encrypt program.\n"
          "\n"
          "USAGE\n"
          "   ./decrypt [OPTIONS]\n"
          "\n"
          "OPTIONS\n"
          "   -h              Display program help and usage.\n"
          "   -v              Display verbose program output.\n"
          "   -i infile       Input file of data to decrypt (default: stdin).\n"
          "   -o outfile      Output file for decrypted data (default: stdout).\n"
          "   -n pvfile       Private key file (default: ss.priv).\n";

    // 1. Parse command-line options using getopt() and handle them accordingly.
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i':
            input_file_name = optarg;
            input = fopen(input_file_name, "r");
            if (input == NULL) {
                fprintf(stderr, "Error: unable to open input file -- '%s'\n", input_file_name);
                exit(1);
            }
            break;
        case 'o':
            output_file_name = optarg;
            output = fopen(output_file_name, "w");
            if (output == NULL) {
                fprintf(stderr, "Error: unable to open output file -- '%s'\n", output_file_name);
                exit(1);
            }
            break;
        case 'n': priv_key_name = optarg; break;
        case 'v': verbose = 1; break;
        case 'h': printf("%s", help_message); return 1;
        default:
            fprintf(stderr, "Usage: %s [-i infile] [-o outfile] [-n pbfile] [-v] [-h]\n", argv[0]);
            exit(1);
        }
    }

    // 2. Open the private key file using fopen(). Print a helpful error and exit the program in the event of failure
    priv_key_file = fopen(priv_key_name, "r");
    if (priv_key_file == NULL) {
        fprintf(stderr, "Error: unable to open private key file -- '%s'\n", priv_key_name);
        exit(1);
    }
    // 3. Read the public key from the opened public key file.
    mpz_t pq, d;
    mpz_inits(pq, d, NULL);
    ss_read_priv(pq, d, priv_key_file);

    // 4. If verbose output is enabled print the following, each with a trailing newline, in order:
    // (a) username
    // (b) the public key n
    // All of the mpz_t values should be printed with information about the number of bits that constitute
    // them, along with their respective values in decimal. See the reference encryptor program for an
    // example.
    if (verbose) {
        // (a) username
        gmp_printf("pq (%d bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq);
        // (b) the public key n
        gmp_printf("d  (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    // 5. Encrypt the file using ss_encrypt_file().
    ss_decrypt_file(input, output, d, pq);

    // 6. Close the public key file and clear any mpz_t variables you have used.
    mpz_clears(pq, d, NULL);
    fclose(input);
    fclose(output);
    fclose(priv_key_file);
}
