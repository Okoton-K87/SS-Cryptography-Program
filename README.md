# SS Cryptography Program

## Description:
This program provides SS encryption and decryption functionality. It includes the following components:

- `keygen`: Generates an SS public/private key pair.
- `encrypt`: Encrypts data using SS encryption.
- `decrypt`: Decrypts data using SS decryption.

## Makefile Usage:
### The following commands will build the keygen, encrypt, decrypt executable together.
```
make
```
```
make all
```
### The following commands will build the keygen, encrypt, decrypt executable respectively, along with their required object files.
```
make keygen
```
```
make encrypt
```
```
make decrypt
```

### The following command will remove all files that are compiler generated.
```
make clean
```

### The following command will format all source code, including the header files.
```
make format
```


## Running with Command-Line Options
### `keygen`
SYNOPSIS
Generates an SS public/private key pair.

USAGE
./keygen [OPTIONS]

OPTIONS
1. -h Display program help and usage.
2. -v Display verbose program output.
3. -b bits Minimum bits needed for public key n (default: 256).
4. -i iterations Miller-Rabin iterations for testing primes (default: 50).
5. -n pbfile Public key file (default: ss.pub).
6. -d pvfile Private key file (default: ss.priv).
7. -s seed Random seed for testing.

### `encrypt`
SYNOPSIS
Encrypts data using SS encryption.
Encrypted data is decrypted by the decrypt program.

USAGE
./encrypt [OPTIONS]

OPTIONS
1. -h Display program help and usage.
2. -v Display verbose program output.
3. -i infile Input file of data to encrypt (default: stdin).
4. -o outfile Output file for encrypted data (default: stdout).
5. -n pbfile Public key file (default: ss.pub).

### `decrypt`
SYNOPSIS
Decrypts data using SS decryption.
Encrypted data is encrypted by the encrypt program.

USAGE
./decrypt [OPTIONS]

OPTIONS
1. -h Display program help and usage.
2. -v Display verbose program output.
3. -i infile Input file of data to decrypt (default: stdin).
4. -o outfile Output file for decrypted data (default: stdout).
5. -n pvfile Private key file (default: ss.priv).



