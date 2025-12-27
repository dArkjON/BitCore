# Bitcore 0.18 Test-Plan

## Ziel
Sicherstellen, dass alle Bitcore-spezifischen Features nach dem Bitcoin 0.18 Upgrade funktionieren:
- Multi-Algorithmus Mining (Timetravel10, SHA256, etc.)
- Masternode System
- PrivateSend
- InstantX
- Spork System

---

## Stufe 1: Basis-Funktionalität (SICHER - EMPFOHLEN ZUERST)

### 1.1 Binary-Smoke-Tests ✅ ERLEDIGT
- [x] bitcored --version
- [x] bitcore-cli --version
- [x] bitcore-tx --help

### 1.2 Regtest Modus - Isolierter Test
**Vorteil:** Komplett isoliert, kein Live-Netzwerk, volle Kontrolle

```bash
# Regtest Konfiguration erstellen
mkdir -p /root/.bitcore-test
cat > /root/.bitcore-test/bitcore.conf << 'EOF'
regtest=1
daemon=1
server=1
rpcuser=btxrpcuser
rpcpassword=btxrpcpass123
rpcallowip=127.0.0.1
rpcport=18332

# Bitcore-spezifisch
masternode=0
addnode=
EOF

# Starten
/root/work/Bitcore-0.18/src/bitcored -datadir=/root/.bitcore-test

# Warten auf Start
sleep 5

# Tests
/root/work/Bitcore-0.18/src/bitcore-cli -datadir=/root/.bitcore-test getblockchaininfo
/root/work/Bitcore-0.18/src/bitcore-cli -datadir=/root/.bitcore-test getnetworkinfo
/root/work/Bitcore-0.18/src/bitcore-cli -datadir=/root/.bitcore-test help

# Mining-Algorithmus Test
/root/work/Bitcore-0.18/src/bitcore-cli -datadir=/root/.bitcore-test generate 10

# Cleanup
/root/work/Bitcore-0.18/src/bitcore-cli -datadir=/root/.bitcore-test stop
```

**Zu prüfen:**
- [ ] Daemon startet ohne Fehler
- [ ] RPC Calls funktionieren
- [ ] Block Generation funktioniert (Multi-Algo Check!)
- [ ] Keine Crashes in debug.log

---

## Stufe 2: Funktionale Tests (MITTEL-SICHER)

### 2.1 Bitcoin Core Functional Tests anpassen
**Hinweis:** PSBT Tests müssen übersprungen werden!

```bash
cd /root/work/Bitcore-0.18/test/functional

# Liste verfügbarer Tests
./test_runner.py --help

# Einzelne kritische Tests
./test_runner.py feature_help.py              # Help Commands
./test_runner.py wallet_basic.py              # Wallet Basics
./test_runner.py rpc_blockchain.py            # Blockchain RPC
./test_runner.py p2p_invalid_block.py         # P2P Validierung

# SKIP: rpc_psbt.py (PSBT deaktiviert!)
```

**Zu prüfen:**
- [ ] Wallet-Funktionalität
- [ ] RPC-Schnittstelle
- [ ] P2P-Kommunikation
- [ ] Block-Validierung

---

## Stufe 3: Bitcore-Spezifische Tests (KRITISCH!)

### 3.1 Multi-Algorithmus Mining Test (REGTEST)

```bash
# Regtest mit verschiedenen Algos
cat > /root/test-multi-algo.sh << 'EOF'
#!/bin/bash
CLI="/root/work/Bitcore-0.18/src/bitcore-cli -datadir=/root/.bitcore-test"

echo "=== Multi-Algo Mining Test ==="

# 10 Blocks generieren
for i in {1..10}; do
    echo "Block $i..."
    $CLI generate 1
    BLOCK=$($CLI getbestblockhash)
    HEADER=$($CLI getblockheader $BLOCK)
    echo "$HEADER" | grep -E "version|difficulty"
done

echo "=== Blockchain Info ==="
$CLI getblockchaininfo | grep -E "blocks|difficulty|chain"
EOF

chmod +x /root/test-multi-algo.sh
/root/test-multi-algo.sh
```

**Zu prüfen:**
- [ ] Block Version Bits korrekt (Multi-Algo Encoding!)
- [ ] Verschiedene Schwierigkeiten
- [ ] Keine Consensus-Fehler

### 3.2 Masternode Test (REGTEST - Simulation)

```bash
# Masternode Config Test
cat > /root/.bitcore-test/masternode.conf << 'EOF'
# mn1 <masternode_ip:port> <masternode_privatekey> <txid> <output_index>
# Testdaten für Regtest
EOF

# Masternode RPC Calls testen
$CLI masternode list
$CLI masternode count
$CLI masternode status
```

**Zu prüfen:**
- [ ] Masternode Commands verfügbar
- [ ] Config wird gelesen
- [ ] Keine Syntax-Fehler

### 3.3 PrivateSend Test (Basis)

```bash
# PrivateSend RPC testen
$CLI getpoolinfo
$CLI privatesend stop
```

**Zu prüfen:**
- [ ] PrivateSend RPC verfügbar
- [ ] Keine Crashes

---

## Stufe 4: Testnet Verbindung (SAFER LIVE-TEST)

### 4.1 Testnet Configuration
**Vorteil:** Echtes Netzwerk, aber Test-Coins (wertlos)

```bash
mkdir -p /root/.bitcore-testnet
cat > /root/.bitcore-testnet/bitcore.conf << 'EOF'
testnet=1
daemon=1
server=1
rpcuser=btxrpcuser
rpcpassword=btxrpcpass123
rpcallowip=127.0.0.1
rpcport=18332

# Testnet Ports (Docker blockiert 8555/8556 nicht für Testnet)
port=18555
rpcport=18556

# Testnet Seed Nodes (falls vorhanden)
# addnode=testnet.bitcore.cc

listen=1
maxconnections=20
EOF

# Starten
/root/work/Bitcore-0.18/src/bitcored -datadir=/root/.bitcore-testnet

# Monitoring
tail -f /root/.bitcore-testnet/testnet3/debug.log
```

**Zu prüfen:**
- [ ] Verbindung zu Testnet-Peers
- [ ] Block-Synchronisation
- [ ] Keine Fork-Probleme
- [ ] Multi-Algo Blocks werden akzeptiert

---

## Stufe 5: Mainnet Connection (VORSICHTIG!)

### 5.1 Mainnet Config (Alternative Ports wegen Docker)

```bash
mkdir -p /root/.bitcore-mainnet
cat > /root/.bitcore-mainnet/bitcore.conf << 'EOF'
# Mainnet Mode
daemon=1
server=1
rpcuser=btxrpcuser
rpcpassword=btxrpcpass123
rpcallowip=127.0.0.1

# WICHTIG: Alternative Ports (Docker blockiert 8555/8556)
port=8333
rpcport=8334

# Seed Nodes
addnode=seed1.bitcore.cc
addnode=seed2.bitcore.cc
addnode=seed3.bitcore.cc

listen=1
maxconnections=50

# Logging
debug=net
debug=mempoolrej
EOF

# ERST NUR SYNCHRONISATION - KEIN MINING!
/root/work/Bitcore-0.18/src/bitcored -datadir=/root/.bitcore-mainnet

# Monitoring
tail -f /root/.bitcore-mainnet/debug.log
```

**KRITISCHE CHECKS:**
- [ ] Peer-Verbindungen erfolgreich
- [ ] Block-Download startet
- [ ] Keine Chain-Rejections
- [ ] Block Headers werden akzeptiert
- [ ] Multi-Algo Blocks von Netzwerk akzeptiert

### 5.2 Wallet-Sicherheit

```bash
# WICHTIG: Erst mit leerer Wallet testen!
$CLI getwalletinfo
$CLI getbalance

# Backup vor echtem Einsatz!
$CLI backupwallet /root/backup-wallet-$(date +%Y%m%d).dat
```

---

## Empfohlene Test-Reihenfolge

### Option A: Maximal sicher (EMPFOHLEN)
1. ✅ Regtest (Stufe 1.2) - 30 Minuten
2. ⚠️ Functional Tests (Stufe 2) - 1-2 Stunden
3. ⚠️ Multi-Algo Tests (Stufe 3) - 1 Stunde
4. ⚠️ Testnet Connection (Stufe 4) - 2-4 Stunden (Sync-Zeit)
5. 🔴 Mainnet Observation (Stufe 5) - 24+ Stunden
6. 🔴 Mainnet Production (nur wenn 1-5 erfolgreich)

### Option B: Schneller Live-Test (RISKANTER)
1. ✅ Regtest Basics (Stufe 1.2) - 15 Minuten
2. ⚠️ Testnet Connection (Stufe 4) - 2-4 Stunden
3. 🔴 Mainnet Observation (Stufe 5) - 24+ Stunden

### Option C: Direkt Live (NICHT EMPFOHLEN ohne Tests!)
- ❌ Überspringt wichtige Sicherheits-Checks
- ❌ Risiko von Consensus-Problemen
- ❌ Könnte zu Fork führen

---

## Kritische Bitcore-Features zu testen

### Must-Have Tests:
- [ ] **Multi-Algo Block Akzeptanz** - Verschiedene Algo-Blocks werden verarbeitet
- [ ] **Masternode Payments** - Werden korrekt berechnet
- [ ] **Spork System** - Sporks werden empfangen und verarbeitet
- [ ] **PrivateSend** - Keine Crashes bei Pool-Operationen
- [ ] **InstantX** - Transaktionen werden korrekt gelockt

### Performance Tests:
- [ ] Memory Usage unter Last
- [ ] CPU Usage beim Block-Validation
- [ ] Disk I/O bei Sync
- [ ] Peer Connection Stabilität

---

## Monitoring während Tests

```bash
# Debug Log Monitor
tail -f /root/.bitcore-test/regtest/debug.log | grep -E "ERROR|WARNING|UpdateTip|ProcessNewBlock"

# Resource Monitoring
watch -n 5 'ps aux | grep bitcored; echo "---"; df -h | grep bitcore'

# Network Activity
watch -n 5 '/root/work/Bitcore-0.18/src/bitcore-cli getpeerinfo | grep -E "addr|version|subver"'
```

---

## Rollback-Plan

Falls Probleme auftreten:

```bash
# Stoppe neue Version
pkill bitcored

# Zurück zu alter Version (Docker Container)
docker start bitcore-container

# Backup wiederherstellen
cp /backup/wallet.dat /root/.bitcore/wallet.dat
```

---

## Empfehlung

**Meine Empfehlung: Option A (Maximal sicher)**

1. **JETZT:** Regtest (30 Min) - Schneller Basic-Check
2. **HEUTE:** Functional Tests (2-3 Std) - Tiefere Validierung
3. **MORGEN:** Testnet (24 Std) - Echtes Netzwerk, kein Risiko
4. **ÜBERMORGEN:** Mainnet Observation (48 Std) - Nur beobachten, nicht minen
5. **NACH 3 TAGEN:** Mainnet Production - Wenn alles stabil

**Warum?**
- Minimiert Risiko von Consensus-Bugs
- Multi-Algo ist kritisch - muss gründlich getestet werden
- Testnet zeigt echte P2P-Probleme
- 3 Tage sind besser als ein Fork im Mainnet!

---

Welche Option bevorzugst du?
