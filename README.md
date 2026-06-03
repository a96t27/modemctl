# modemctl

OpenWrt feed providing the `modemctl` utility.

`modemctl` is a command-line tool for querying modems. It can retrieve modem information, network status, SMS messages, and other modem-related data.

## Package

## Dependencies

### modemctl

- libcjson

## Installation

Add feed:

```text
src-git modemctl https://github.com/a96t27/modemctl.git
```

Update and install:

```bash
./scripts/feeds update modemctl
./scripts/feeds install -a -p modemctl
```

## Configuration (menuconfig)

Run configuration menu:

```bash
make menuconfig
```

Locate package:

```text
Network  --->
    <*> modemctl
```

After selecting the package:

- Save configuration
- Exit menuconfig

## Build

Build package:

```bash
make package/modemctl/compile V=s
```

Or build full firmware:

```bash
make -j$(nproc)
```

## Installing produced .ipk package

After building, the generated `.ipk` package will be located in:

```text
bin/packages/<arch>/modemctl/
```

Example:

```text
bin/packages/aarch64_cortex-a53/modemctl/
```

### Find generated package

```bash
find bin/packages -name "modemctl*.ipk"
```

## Installing on a running OpenWrt device

Copy package to the router:

```bash
scp bin/packages/*/modemctl/modemctl*.ipk root@<DEVICE_IP>:/tmp/
```

Install with opkg:

```bash
opkg install /tmp/modemctl*.ipk
```

## Usage

Show available options:

```bash
modemctl --help
```

Examples:

```bash
modemctl --imei
modemctl --all
modemctl --all --json
modemctl --at "AT+GSN"
```

## Verify installation

```bash
opkg list-installed | grep modemctl
```
