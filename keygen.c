#include <stdio.h>
#include <stdlib.h> //atof
#include <unistd.h> //getopt().
#include <time.h>
#include <gmp.h>
#include <sys/stat.h>

#include "ss.h"
#include "numtheory.h"
#include "randstate.h"

#define OPTIONS "b:i:n:d:s:hv"

int main(int argc, char **argv) {
    int opt = 0;

    // setting default bits and iterations
    uint32_t bits = 256, iters = 50;

    // disable verbose by default
    int verbose = 0;

    // file steams
    FILE *pub_key_file;
    FILE *priv_key_file;

    // default names for files
    char *pub_key_name = "ss.pub";
    char *priv_key_name = "ss.priv";

    uint32_t seed = time(NULL);

    // help_message
    const char *help_message
        = "SYNOPSIS\n"
          "   Generates an SS public/private key pair.\n"
          "\n"
          "USAGE\n"
          "   ./keygen [OPTIONS]\n"
          "\n"
          "OPTIONS\n"
          "   -h              Display program help and usage.\n"
          "   -v              Display verbose program output.\n"
          "   -b bits         Minimum bits needed for public key n (default: 256).\n"
          "   -i iterations   Miller-Rabin iterations for testing primes (default: 50).\n"
          "   -n pbfile       Public key file (default: ss.pub).\n"
          "   -d pvfile       Private key file (default: ss.priv).\n"
          "   -s seed         Random seed for testing.\n";

    // 1. Parse command-line options using getopt() and handle them accordingly.
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'b': bits = atoi(optarg); break;
        case 'i': iters = atoi(optarg); break;
        case 'n': pub_key_name = optarg; break;
        case 'd': priv_key_name = optarg; break;
        case 's': seed = atoi(optarg); break;
        case 'v': verbose = 1; break;
        case 'h': printf("%s", help_message); return 1;
        default:
            fprintf(stderr,
                "Usage: %s [-b bits] [-i iterations] [-n pbfile] [-d pvfile] [-s seed] [-v] [-h]\n",
                argv[0]);
            exit(1);
        }
    }

    // 2. Open the public and private key files using fopen().

    // Open public key file
    pub_key_file = fopen(pub_key_name, "w");
    if (pub_key_file == NULL) {
        fprintf(stderr, "Error: unable to open public key file -- '%s'\n", pub_key_name);
        exit(1);
    }

    // Open private key file
    priv_key_file = fopen(priv_key_name, "w");
    if (priv_key_file == NULL) {
        fprintf(stderr, "Error: unable to open private key file -- '%s'\n", priv_key_name);
        exit(1);
    }

    // https://pubs.opengroup.org/onlinepubs/009604599/functions/fileno.html
    // https://www.mkssoftware.com/docs/man3/chmod.3.asp#:~:text=The%20fchmod()%20function%20has,the%20file%2C%20and%20all%20others.
    // 3. Using fchmod() and fileno(), make sure that the private key file permissions are set to 0600
    int file_descriptor = fileno(priv_key_file);
    fchmod(file_descriptor, 0600); //Ben Grant

    // 4. Initialize the random state using randstate_init(), using the set seed.
    randstate_init(seed);

    // 5. Make the public and private keys using ss_make_pub() and ss_make_priv(), respectively.
    mpz_t p, q, n, d, pq;
    mpz_inits(p, q, n, d, pq, NULL);

    ss_make_pub(p, q, n, bits, iters);
    ss_make_priv(d, pq, p, q);

    // 6. Get the current user’s name as a string. You will want to use getenv().
    char *username = getenv("USER");

    // 7. Write the computed public and private key to their respective files.
    ss_write_pub(n, username, pub_key_file);
    ss_write_priv(pq, d, priv_key_file);

    // 8. If verbose output is enabled print the following, each with a trailing newline, in order:
    if (verbose) {
        // (a) username
        gmp_printf("user = %s\n", username);
        // (b) the first large prime p
        gmp_printf("p  (%d bits) = %Zd\n", mpz_sizeinbase(p, 2), p);
        // (c) the second large prime q
        gmp_printf("q  (%d bits) = %Zd\n", mpz_sizeinbase(q, 2), q);
        // (d) the public key n
        gmp_printf("n  (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        // (e) the private exponent d
        gmp_printf("d  (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
        // (f) the private modulus pq
        gmp_printf("pq (%d bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq);
    }

    mpz_clears(p, q, n, d, pq, NULL);
    fclose(pub_key_file);
    fclose(priv_key_file);
}
