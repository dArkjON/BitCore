# Bitcore BTX Component Inventory
# Version 0.90.9.10 → 0.18 Upgrade

**Datum:** 2025-12-18
**Baseline:** v0.90.9.10-baseline
**Ziel:** BTX 0.18 (v0.90.9.11)

---

## 1. BTX-SPEZIFISCHE CRYPTO-DATEIEN (KRITISCH)

### 1.1 PoW-Algorithmen (MUSS ERHALTEN BLEIBEN)

**Mega-BTX (Phase 3 - Aktuell):**
- `src/crypto/mega-btx.h` (348 Zeilen) - 16 Hash-Funktionen, dynamische Permutation

**Time Travel (Phase 2 - Historisch):**
- `src/crypto/hashblock.h` (250+ Zeilen) - 10 Hash-Funktionen

**Scrypt (Phase 1 - Historisch):**
- `src/crypto/scrypt.cpp`
- `src/crypto/scrypt.h`
- `src/crypto/scrypt-sse2.cpp`

### 1.2 SPH Hash Library (26 Dateien)

**Core Algorithmen:**
- sph_blake.h
- sph_bmw.h
- sph_groestl.h
- sph_jh.h
- sph_keccak.h
- sph_skein.h
- sph_luffa.h
- sph_cubehash.h
- sph_shavite.h
- sph_simd.h
- sph_echo.h
- sph_haval.h
- sph_tiger.h
- sph_sha2.c / .h
- sph_sha512.c
- sph_shabal.c / .h
- sph_whirlpool.c / .h

**Support Files:**
- sph_fugue.c / .h
- sph_hamsi.c / .h
- sph_hamsi_helper.c
- sph_types.h

### 1.3 Zusätzliche Hash-Algorithmen

- `src/crypto/gost_streebog.c` - GOST Hash
- `src/crypto/gost_streebog.h`
- `src/crypto/haval.c` - HAVAL Hash
- `src/crypto/haval_helper.c`

**Gesamt: 35 BTX-Crypto-Dateien**

---

## 2. MASTERNODE & GOVERNANCE-DATEIEN (KRITISCH - 30 Dateien)

### 2.1 Masternode Core (12 Dateien)

- src/activemasternode.cpp / .h (2)
- src/masternode.cpp / .h (2)
- src/masternodeman.cpp / .h (2)
- src/masternode-payments.cpp / .h (2)
- src/masternode-sync.cpp / .h (2)
- src/masternodeconfig.cpp / .h (2)

### 2.2 Governance System (16 Dateien)

- src/governance.cpp / .h (2)
- src/governance-classes.cpp / .h (2)
- src/governance-exceptions.h (1)
- src/governance-misc.h (1)
- src/governance-object.cpp / .h (2)
- src/governance-validators.cpp / .h (2)
- src/governance-vote.cpp / .h (2)
- src/governance-votedb.cpp / .h (2)
- src/governance-votedb.h (bereits in .h gezählt)

### 2.3 Spork System (4 Dateien)

- src/spork.cpp / .h (2)
- src/sporkdb.cpp / .h (2)

### 2.4 RPC-Module (2 Dateien)

- src/rpc/masternode.cpp
- src/rpc/governance.cpp

**Gesamt: 30 Masternode/Governance-Dateien**

---

## 3. CONSENSUS-KRITISCHE DATEIEN

### 3.1 Proof-of-Work

- **src/pow.cpp** (437 Zeilen) - DUAL_KGW3 Algorithmus
  - MUSS KOMPLETT ERHALTEN BLEIBEN
  - Ersetzt Bitcoin's Standard-PoW komplett

### 3.2 Block Primitives

- **src/primitives/block.h** - BTX-Erweiterungen:
  - `uint256 GetPoWHash() const` - Multi-Algo Selektor
  - `unsigned int GetAlgoEfficiency(int nBlockHeight) const`

- **src/primitives/block.cpp** (73 Zeilen):
  - GetPoWHash() Implementation (3-Phasen-Selektion)

### 3.3 Consensus Parameters

- **src/consensus/params.h** (123 Zeilen):
  - 40+ BTX-spezifische Parameter hinzugefügt
  - Masternode-Parameter (8 Felder)
  - Governance-Parameter (9 Felder)
  - Timing-Parameter (5 Felder)
  - Optimierungen (3 Felder)

- **src/consensus/consensus.h**:
  - MAX_BLOCK_SERIALIZED_SIZE = 20000000 (20MB vs Bitcoin 4MB)
  - MAX_BLOCK_WEIGHT = 20000000
  - MAX_BLOCK_SIGOPS_COST = 500000
  - COINBASE_MATURITY_2 = 576
  - COINBASE_MATURITY_3 = 4032
  - HASH_FORK_TIME_1 = 1598961600

### 3.4 Chain Parameters

- **src/chainparams.cpp** (526 Zeilen):
  - Genesis Block: 0x604148281e5c4b7f2487e5d03cd60d8e6f69411d613f6448034508cea52e9574
  - Network Magic: 0xf9, 0xbe, 0xb4, 0xd9
  - Port: 8555 (mainnet), 8666 (testnet)
  - Bech32 HRP: "btx1" (neu hinzuzufügen)
  - 11 Checkpoints

### 3.5 Validation

- **src/validation.cpp** (5142 Zeilen):
  - Masternode-Payment-Validierung (Integration erforderlich)
  - Superblock-Validierung
  - ca. 5 Includes für Masternode-Module

---

## 4. BUILD-SYSTEM-DATEIEN

### 4.1 Autoconf

- **configure.ac**:
  - Version: CLIENT_VERSION_MAJOR=0, MINOR=90, REVISION=9, BUILD=11
  - Package: bitcore
  - Binaries: bitcored, bitcore-cli, bitcore-tx, bitcore-qt

### 4.2 Makefile

- **src/Makefile.am**:
  - 35+ Referenzen für Masternode/Governance-Dateien
  - 35+ Referenzen für Crypto-Dateien
  - Binary-Namen: bitcored (daemon)

### 4.3 Client Version

- **src/clientversion.h**:
  - CLIENT_VERSION_BUILD = 11
  - User-Agent: "/BitCore:0.90.9.11/"

---

## 5. BTX-SPEZIFISCHE KONSTANTEN

### Network Parameters
- Block Time: 2.5 Minuten (150 Sekunden)
- Block Size: 20MB
- Retargeting: 1,000 Blöcke
- Halving: 210,000 Blöcke
- Minimum Subsidy: 0.001 BTX

### Masternode Parameters
- Collateral Min: 2,100 BTX
- Collateral Max: 21,000 BTX
- Confirmations: 15 Blöcke
- Payment Start: Block 50
- Instant Send Lock: 24 Blöcke

### Governance Parameters
- Budget Start: 99,999,999 (disabled)
- Superblock Start: 99,999,999 (disabled)
- Superblock Cycle: 10,958 Blöcke
- Governance Quorum: 10
- Governance Filter Elements: 20,000

### Timing Forks
- Fork Height: 21,000
- Hash Fork Time: 1598961600 (09/01/2020)
- TimeTravel Start: 1493124696
- Scrypt End: 1493124695

---

## 6. PRESERVATION CHECKLIST

### KRITISCH - MUSS EXAKT ERHALTEN BLEIBEN

**Algorithmen:**
- [x] src/crypto/mega-btx.h (Mega-BTX Algorithmus)
- [x] src/crypto/hashblock.h (TimeTravel Algorithmus)
- [x] src/crypto/scrypt.cpp (Scrypt Algorithmus)
- [x] Alle 26 SPH Hash-Dateien
- [x] src/primitives/block.cpp (GetPoWHash())
- [x] src/pow.cpp (DUAL_KGW3)

**Masternode-System:**
- [x] Alle 12 Masternode Core-Dateien
- [x] Alle 16 Governance-Dateien
- [x] Alle 4 Spork-Dateien
- [x] 2 RPC-Module

**Consensus:**
- [x] src/consensus/params.h (40+ BTX-Parameter)
- [x] src/consensus/consensus.h (Block-Size 20MB)
- [x] src/chainparams.cpp (Genesis, Magic, Ports)

### HOCH - KRITISCHE ANPASSUNGEN ERFORDERLICH

**Validation:**
- [ ] src/validation.cpp - Masternode-Payment-Hooks integrieren
- [ ] src/validation.cpp - Superblock-Validierung hinzufügen

**Init:**
- [ ] src/init.cpp - Masternode-System initialisieren
- [ ] src/init.cpp - Governance initialisieren
- [ ] src/init.cpp - Spork-System laden

**RPC Registration:**
- [ ] src/rpc/register.h - Masternode RPCs registrieren
- [ ] src/rpc/register.h - Governance RPCs registrieren
- [ ] src/rpc/register.h - Spork RPCs registrieren

**Build System:**
- [ ] configure.ac - Version 0.90.9.11 setzen
- [ ] src/Makefile.am - Alle BTX-Dateien hinzufügen (70+ Referenzen)
- [ ] src/clientversion.h - BUILD=11, User-Agent aktualisieren

### MITTEL - ERWEITERN/ANPASSEN

**Block Structure:**
- [ ] src/primitives/block.h - txoutMasternode Feld hinzufügen
- [ ] src/primitives/block.h - voutSuperblock Vektor hinzufügen
- [ ] src/primitives/block.h - GetAlgoEfficiency() deklarieren

**Mining:**
- [ ] src/miner.cpp - Masternode-Payment zur Coinbase hinzufügen
- [ ] src/miner.cpp - Superblock-Payments integrieren

### NIEDRIG - OPTIONAL/NACH PRIORITÄT

**UI/Qt:**
- [ ] src/qt/masternodelist.* - Masternode UI (falls Qt aktiviert)

**Dokumentation:**
- [ ] doc/ - BTX-spezifische Dokumentation
- [ ] README.md - BTX Branding

---

## 7. STATISTIKEN

**Gesamt BTX-spezifische Dateien:**
- Crypto: 35 Dateien
- Masternode/Governance: 30 Dateien
- Consensus-kritisch: 5 Hauptdateien
- Build-System: 3 Dateien
- **TOTAL: ~73 BTX-spezifische Dateien**

**Codezeilen (geschätzt):**
- Crypto: ~10,000 Zeilen
- Masternode/Governance: ~15,000 Zeilen
- Consensus-Anpassungen: ~2,000 Zeilen
- **TOTAL: ~27,000 Zeilen BTX-spezifischer Code**

---

## 8. KRITISCHE MERGE-KONFLIKTE (Vorbereitet)

### Sehr Hoch
1. src/pow.cpp - Komplett verschieden von Bitcoin
2. src/consensus/params.h - 40+ zusätzliche Felder
3. src/consensus/consensus.h - Block-Size 20MB vs 4MB
4. src/primitives/block.h - Masternode-Felder
5. src/validation.cpp - Masternode-Payment-Validierung

### Hoch
6. src/chainparams.cpp - Genesis, Magic, Ports
7. src/Makefile.am - 70+ zusätzliche Datei-Referenzen
8. configure.ac - Version, Binaries
9. src/miner.cpp - Block-Creation-Logik

### Mittel
10. src/init.cpp - Initialization-Routinen

---

## 9. NÄCHSTE SCHRITTE

**Phase 0 abgeschlossen:**
- [x] Repository Setup
- [x] Component Inventory erstellt

**Phase 0 ausstehend:**
- [ ] Testing Infrastructure aufbauen
- [ ] Build System Analysis durchführen

**Phase 1 vorbereitet:**
- Alle kritischen Dateien identifiziert
- Merge-Konflikte dokumentiert
- Preservation Checklist erstellt

---

**ENDE DES INVENTARS**
