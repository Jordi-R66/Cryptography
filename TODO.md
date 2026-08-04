# TODO - Implémentation Cryptographie Post-Quantique (ML-KEM & ML-DSA)

Ce document liste l'ensemble des tâches à accomplir pour implémenter les standards FIPS 203 (ML-KEM) et FIPS 204 (ML-DSA) en respectant la ségrégation des responsabilités entre la bibliothèque généraliste `myOwnCLib` et la bibliothèque métier `Cryptography`.

---

## 2. `Cryptography` : Primitives & Protocoles

C'est ici qu'on manipule des octets, des flux d'entropie, les hachages, et qu'on orchestre les protocoles standardisés.

### A. Famille de hachage Keccak / SHA-3 (Prérequis Absolu)
> Les algorithmes ML-KEM et ML-DSA reposent massivement sur les fonctions standardisées dans le [**FIPS 202 (SHA-3 Standard)**](https://doi.org/10.6028/NIST.FIPS.202) ([PDF direct](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.202.pdf)).
- [ ] Créer le dossier `Cryptography/hash/sha3/`.
- [ ] **Fonctions de Hash** : Implémenter `SHA3-256` et `SHA3-512`.
- [ ] **Fonctions XOF (Extendable-Output Functions)** : Implémenter `SHAKE128` et `SHAKE256` avec une API orientée "Streaming" (`Init`, `Absorb`, `Squeeze`).

### B. ML-KEM (FIPS 203) - Encapsulation de Clés
- [ ] Créer le dossier `Cryptography/cipher/asymmetric/ml_kem/`.
- [ ] **Sérialisation / Désérialisation** :
  - [ ] `BitsToBytes` / `BytesToBits`.
  - [ ] `ByteEncode_d` / `ByteDecode_d` (avec $d$ variant de 1 à 12).
  - [ ] Compression `Compress_d` / `Decompress_d`.
- [ ] **Échantillonnage (Sampling)** :
  - [ ] `SamplePolyCBD_eta` (Distribution binomiale centrée).
  - [ ] `SampleNTT` (Échantillonnage pseudo-aléatoire dans le domaine NTT via XOF).
- [ ] **Core PKE (Sous-protocole K-PKE)** :
  - [ ] `K-PKE.KeyGen`
  - [ ] `K-PKE.Encrypt`
  - [ ] `K-PKE.Decrypt`
- [ ] **Protocole principal ML-KEM** :
  - [ ] `ML-KEM.KeyGen` (et version interne).
  - [ ] `ML-KEM.Encaps` (et version interne).
  - [ ] `ML-KEM.Decaps` (et version interne, incluant le mécanisme d'*Implicit Rejection*).
- [ ] **Paramétrage** : Définir les profils `ML-KEM-512`, `ML-KEM-768`, `ML-KEM-1024`.

### C. ML-DSA (FIPS 204) - Signature Numérique
- [ ] Créer le dossier `Cryptography/cipher/asymmetric/ml_dsa/`.
- [ ] **Sérialisation Avancée** :
  - [ ] `SimpleBitPack` / `SimpleBitUnpack`.
  - [ ] `BitPack` / `BitUnpack`.
  - [ ] `HintBitPack` / `HintBitUnpack` (Formatage économe des indices de zéros/uns).
  - [ ] Algorithmes d'encodage de clés et signatures : `pkEncode/Decode`, `skEncode/Decode`, `sigEncode/Decode`, `w1Encode`.
- [ ] **Compression & Hints (Indicateurs d'arrondi)** :
  - [ ] `Power2Round` et `Decompose`.
  - [ ] `HighBits` et `LowBits`.
  - [ ] `MakeHint` (Création) et `UseHint` (Utilisation lors de la vérification).
- [ ] **Échantillonnage Complexe (Sampling)** :
  - [ ] `RejNTTPoly` et `RejBoundedPoly` (Rejection sampling sur XOF).
  - [ ] `SampleInBall` (Mélange de Fisher-Yates pour forcer le poids de Hamming $	au$).
  - [ ] `ExpandA`, `ExpandS`, `ExpandMask`.
- [ ] **Protocole principal ML-DSA** :
  - [ ] `ML-DSA.KeyGen` (et sa version interne).
  - [ ] `ML-DSA.Sign` (Version hachée aléatoire par défaut + version déterministe optionnelle. Inclut la boucle de "Rejection Sampling" de la réponse $z$).
  - [ ] `ML-DSA.Verify`.
- [ ] **Variante HashML-DSA (Pre-Hash)** :
  - [ ] Implémenter l'API `HashML-DSA.Sign` et `HashML-DSA.Verify` avec structuration OID + pré-hachage.
- [ ] **Paramétrage** : Définir les profils `ML-DSA-44`, `ML-DSA-65`, `ML-DSA-87`.
