# Market Data Feed Handler Sumulation (C++)

This project simulates a low-latency market data feed handler similar to what is used in high-frequency trading systems like those at IMC Trading.

## Features
- Parses binary-encoded UDP messages
- Real-time in-memory order book updates
- Microsecond-precision timestamps
- Multithreaded simulation of sender/receiver
- Extensible for real market data (ITCH, FIX)

## Build

```bash
make
