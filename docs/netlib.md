# Networking - netlib

`netlib` turns the C64's CIA2 serial port into a byte pipe and layers a small reliable packet protocol on top - enough for two-player links between two C64s (or a C64 and any serial peer).

```c
#include "multidore64/netlib.h"
```

!!! note
    Add `src/multidore64/netlib.c` to your build line (see [Building](build.html)).

## Layer 1 - raw serial

```c
unsigned char netlib_init(unsigned int baud);      // 1 = ok, 0 = failed
void netlib_shutdown(void);
void netlib_send(unsigned char byte);
unsigned char netlib_recv(void);                   // blocking
unsigned char netlib_recv_timeout(unsigned int timeout_ms);  // 0xFF = timeout
unsigned char netlib_available(void);              // 1 = byte pending
void netlib_send_str(const char *str);
unsigned int netlib_recv_str(char *buf, unsigned int max_len, unsigned int timeout_ms);
void netlib_send_data(const unsigned char *data, unsigned int len);
unsigned int netlib_recv_data(unsigned char *buf, unsigned int max_len, unsigned int timeout_ms);
```

Baud rates come as constants: `NBAUD_300`, `NBAUD_1200`, `NBAUD_2400`, `NBAUD_9600`, ... up to `NBAUD_19200`.

```c
if (!netlib_init(NBAUD_2400))
    fail();

netlib_send_str("HELLO\n");

char buf[32];
if (netlib_recv_str(buf, sizeof(buf), 2000))    // 2 second timeout
    renderlib_drawstring(0, 2, color_white, buf);
```

## Layer 2 - reliable packets

Every packet carries a magic byte (`NPROT_MAGIC`), a type and a payload of up to `NPROT_MAX_DATA` (250) bytes. Senders wait for `NPROT_ACK`; missing ACKs surface as a 0 return.

| Type | Constant | Meaning |
|---|---|---|
| Handshake | `NPROT_HANDSHAKE` | connection setup |
| Data | `NPROT_DATA` | generic payload |
| Input | `NPROT_INPUT` | player controls |
| State | `NPROT_STATE` | state sync |
| Ack / Nak | `NPROT_ACK` / `NPROT_NAK` | delivery control |
| Disconnect | `NPROT_DISCONNECT` | graceful close |

```c
unsigned char netlib_connect(unsigned int timeout_ms);   // initiate
unsigned char netlib_accept(unsigned int timeout_ms);    // wait for peer
unsigned char netlib_connected(void);
unsigned char netlib_state(void);                        // NSTATE_*

unsigned char netlib_send_packet(unsigned char type, const unsigned char *data,
                                 unsigned int len, unsigned int timeout_ms);
unsigned char netlib_recv_packet(unsigned char *buf, unsigned int max_len,
                                 unsigned int *out_len, unsigned int timeout_ms);
void netlib_disconnect(void);
```

## Two-player handshake

```c
// machine A
if (netlib_init(NBAUD_2400) && netlib_connect(5000))
    renderlib_drawstring(0, 0, color_green, "CONNECTED");

// machine B
if (netlib_init(NBAUD_2400) && netlib_accept(5000))
    renderlib_drawstring(0, 0, color_green, "PEER ARRIVED");
```

## Game input and state sync

Purpose-built wrappers pack and unpack the fields for you:

```c
void netlib_send_input(unsigned char port, unsigned char buttons,
                       unsigned char joy_x, unsigned char joy_y);
unsigned char netlib_recv_input(unsigned char *port, unsigned char *buttons,
                                unsigned char *joy_x, unsigned char *joy_y,
                                unsigned int timeout_ms);

void netlib_send_state(const unsigned char *state, unsigned int len);
unsigned char netlib_recv_state(unsigned char *state, unsigned int max_len,
                                unsigned int *out_len, unsigned int timeout_ms);
```

A minimal lockstep loop:

```c
// each frame: send my input, then wait for the peer's input
netlib_send_input(0, fire, dx, dy);

unsigned char port, buttons, jx, jy;
if (netlib_recv_input(&port, &buttons, &jx, &jy, 100))
    apply_remote_input(buttons, jx, jy);
else
    hold_frame();                  // peer late - hold position
```

## Low-level registers

The CIA2 serial register macros (`CIA2_SER_DATA`, `CIA2_SER_CTRL`, `SER_SREN`, `TSC_SPEN`, ...) are exported for custom transports - see `netlib.h` for the full list with bit meanings.
