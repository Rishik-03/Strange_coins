# Bitcoin Transaction Analysis (Legacy vs SegWit)

## Project Overview

This project analyzes Bitcoin transactions created in **Bitcoin Core Regtest mode** and compares:

- **Legacy P2PKH transactions**
- **SegWit P2SH-P2WPKH transactions**

The program parses transaction JSON outputs using **C and the cJSON library** and compares:

- transaction size
- virtual size (vsize)
- weight
- script structure

The goal is to understand **why SegWit transactions are more efficient than Legacy transactions**.

---

# Team Members

Team name - Strange Coins

Charan kumar    – 240001057
Shiv pratap     – 240001069
Rishik          – 240003027
Aravind         –240003082


---

# Repository Structure
team_strange_coins/
│
├── bitcoin_assignment_.c
├── cJSON.c
├── cJSON.h
│
├── legacy_A_B.json
├── legacy_B_C.json
├── segwit_A_B.json
├── segwit_B_C.json
│
└── README.md
│
├──Report.pdf

---

# Requirements

The following tools are required:

- **Bitcoin Core**
- **GCC compiler**
- **cJSON library**

Bitcoin Core is used in **Regtest mode** to generate test transactions.

---

# How the Transactions Were Generated

### Start Bitcoin Regtest
bitcoind -regtest -daemon

---

# Part 1 — Legacy P2PKH Transactions

Create wallet
bitcoin-cli -regtest createwallet LabWallet (-- a typo mistake instaed of legacy we used lab*--)

Generate address
bitcoin-cli -regtest -rpcwallet=LabWallet getnewaddress

Generate blocks
bitcoin-cli -regtest generatetoaddress 101 ADDRESS

Send transaction
bitcoin-cli -regtest -rpcwallet=LabWallet sendtoaddress ADDRESS 10

Mine block to confirm
bitcoin-cli -regtest generatetoaddress 1 ADDRESS

Decode raw transaction
bitcoin-cli -regtest decoderawtransaction HEX

Save output as
legacy_A_B.json
legacy_B_C.json


---

# Part 2 — SegWit P2SH-P2WPKH Transactions

Create SegWit wallet
bitcoin-cli -regtest createwallet SegWitWallet

Generate SegWit address
bitcoin-cli -regtest -rpcwallet=SegWitWallet getnewaddress "" p2sh-segwit

---

# Compile the Program

Navigate to the project folder
cd team_strange_coins

Compile
gcc bitcoin_assignment_.c cJSON.c -o bitcoin_assignment.exe


---

# Run the Program
bitcoin_assignment.exe

---

# Example Output

Generate blocks
bitcoin-cli -regtest generatetoaddress 101 ADDRESS

Send transaction
bitcoin-cli -regtest -rpcwallet=SegWitWallet sendtoaddress ADDRESS 10

Mine block
bitcoin-cli -regtest generatetoaddress 1 ADDRESS

Decode transaction
bitcoin-cli -regtest decoderawtransaction HEX

Save output as
segwit_A_B.json
segwit_B_C.json

==================================================
LEGACY P2PKH TRANSACTION A -> B

size : 191
vsize : 191
weight : 764
scriptPubKey.type : pubkeyhash

==================================================
SEGWIT P2SH-P2WPKH TRANSACTION A' -> B'

size : 247
vsize : 166
weight : 661
scriptPubKey.type : scripthash

==================================================
LEGACY VS SEGWIT COMPARISON
Transaction Size Vsize Weight Type

Legacy A->B   191 191 764 pubkeyhash
Legacy B->C   191 191 764 pubkeyhash
SegWit A'->B' 247 166 661 scripthash
SegWit B'->C' 247 166 661 scripthash


---

# Analysis

### Legacy Transaction

- Signature and public key stored in **scriptSig**
- Larger transaction size
- Higher transaction fee

Script format:
scriptSig → signature + public key


---

### SegWit Transaction

Signature moved to **witness data**
Script structure:

scriptSig → empty
txinwitness → signature + public key


Advantages:
- reduced **vsize**
- lower **transaction fees**
- eliminates **transaction malleability**

---

# Conclusion

SegWit transactions are more efficient because they:
- separate signature data from transaction data
- reduce virtual size
- reduce network bandwidth usage
- reduce transaction fees

