/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023 by Malte0621
*/

#ifndef NETLIB_H
#define NETLIB_H

/*
----------------------------------------------------------
CIA2 Serial Port Registers ($DC00-$DC0F)
----------------------------------------------------------
*/
#define CIA2_PORTA_DATA   (*(volatile unsigned char *)0xdc00)
#define CIA2_PORTA_DIR    (*(volatile unsigned char *)0xdc01)
#define CIA2_PORTB_DATA   (*(volatile unsigned char *)0xdc02)
#define CIA2_PORTB_DIR    (*(volatile unsigned char *)0xdc03)
#define CIA2_TIMER_A_LO   (*(volatile unsigned char *)0xdc04)
#define CIA2_TIMER_A_HI   (*(volatile unsigned char *)0xdc05)
#define CIA2_TIMER_B_LO   (*(volatile unsigned char *)0xdc06)
#define CIA2_TIMER_B_HI   (*(volatile unsigned char *)0xdc07)
#define CIA2_PERIPH_CTRL  (*(volatile unsigned char *)0xdc08)
#define CIA2_INTR_PENDING (*(volatile unsigned char *)0xdc09)
#define CIA2_INTR_MASK    (*(volatile unsigned char *)0xdc0a)
#define CIA2_TIMER_SER_CTRL (*(volatile unsigned char *)0xdc0b)
#define CIA2_SPURIOUS_INTR  (*(volatile unsigned char *)0xdc0c)
#define CIA2_SER_INTR_EN  (*(volatile unsigned char *)0xdc0d)
#define CIA2_SER_CTRL     (*(volatile unsigned char *)0xdc0e)
#define CIA2_SER_DATA     (*(volatile unsigned char *)0xdc0f)

/*
----------------------------------------------------------
Serial Control Bits ($DC0E)
----------------------------------------------------------
*/
#define SER_SREN    0x80  /* Serial Receive Enable */
#define SER_STEN    0x40  /* Serial Transmit Enable */
#define SER_SPB     0x01  /* 1 Stop Bit (set) / 2 Stop Bits (clear) */

/*
----------------------------------------------------------
Timer/Serial Control Bits ($DC0B)
----------------------------------------------------------
*/
#define TSC_SPEN    0x80  /* Serial Port Enable */
#define TSC_SPR     0x40  /* Serial Port Receive */
#define TSC_SPT     0x20  /* Serial Port Transmit */
#define TSC_TA_LD   0x10  /* Timer A Load */
#define TSC_TA_PB   0x08  /* Timer A Periodic Burst */
#define TSC_TA_TO   0x04  /* Timer A Time Out */
#define TSC_TA_SP   0x02  /* Timer A Single Period */
#define TSC_TA_FREE 0x01  /* Timer A Free Running */

/*
----------------------------------------------------------
Baud Rates
----------------------------------------------------------
*/
#define NBAUD_50      50
#define NBAUD_75      75
#define NBAUD_110     110
#define NBAUD_150     150
#define NBAUD_300     300
#define NBAUD_600     600
#define NBAUD_1200    1200
#define NBAUD_2400    2400
#define NBAUD_4800    4800
#define NBAUD_9600    9600
#define NBAUD_19200   19200

/*
----------------------------------------------------------
Protocol Constants
----------------------------------------------------------
*/
#define NPROT_MAGIC     0x5A  /* Packet magic byte */
#define NPROT_MAX_DATA  250   /* Max packet payload */
#define NPROT_HANDSHAKE 0x01  /* Handshake packet type */
#define NPROT_DATA      0x02  /* Data packet type */
#define NPROT_INPUT     0x03  /* Input packet type */
#define NPROT_STATE     0x04  /* State sync packet type */
#define NPROT_ACK       0x05  /* Acknowledgment */
#define NPROT_NAK       0x06  /* Negative acknowledgment */
#define NPROT_DISCONNECT 0x07 /* Disconnect notification */

/*
----------------------------------------------------------
Connection State
----------------------------------------------------------
*/
#define NSTATE_DISCONNECTED 0
#define NSTATE_CONNECTING   1
#define NSTATE_CONNECTED    2

/*
----------------------------------------------------------
Network Library API
----------------------------------------------------------
*/

/* Initialize the serial port at the given baud rate.
   Returns 1 on success, 0 on failure. */
unsigned char netlib_init(unsigned int baud);

/* Shut down the serial port and restore CIA2 to defaults. */
void netlib_shutdown(void);

/* Send a single byte over the serial port. */
void netlib_send(unsigned char byte);

/* Receive a single byte (blocking). */
unsigned char netlib_recv(void);

/* Receive a byte with timeout. Returns 0xFF on timeout.
   timeout is in milliseconds (approximate). */
unsigned char netlib_recv_timeout(unsigned int timeout_ms);

/* Check if a byte is available to receive. Returns 1 if data pending. */
unsigned char netlib_available(void);

/* Send a null-terminated string. */
void netlib_send_str(const char *str);

/* Receive a null-terminated string into buf (max max_len bytes including null).
   Returns number of bytes received (excluding null), or 0 on timeout. */
unsigned int netlib_recv_str(char *buf, unsigned int max_len, unsigned int timeout_ms);

/* Send raw data buffer. */
void netlib_send_data(const unsigned char *data, unsigned int len);

/* Receive raw data into buffer. Returns bytes received, or 0 on timeout. */
unsigned int netlib_recv_data(unsigned char *buf, unsigned int max_len, unsigned int timeout_ms);

/*
----------------------------------------------------------
Protocol Layer (reliable packet exchange)
----------------------------------------------------------
*/

/* Attempt a connection handshake with a remote host.
   Sends handshake packets until an ACK is received.
   Returns 1 on success, 0 on failure (after timeout_ms). */
unsigned char netlib_connect(unsigned int timeout_ms);

/* Accept a connection (wait for handshake from remote).
   Returns 1 on success, 0 on timeout. */
unsigned char netlib_accept(unsigned int timeout_ms);

/* Check if we are currently connected. */
unsigned char netlib_connected(void);

/* Get current connection state (NSTATE_*). */
unsigned char netlib_state(void);

/* Send a reliable data packet. Waits for ACK.
   Returns 1 on success, 0 on failure. */
unsigned char netlib_send_packet(unsigned char type, const unsigned char *data, unsigned int len, unsigned int timeout_ms);

/* Receive a reliable data packet. Returns packet type, or 0 on timeout.
   Data is stored in buf (max max_len), actual length in *out_len. */
unsigned char netlib_recv_packet(unsigned char *buf, unsigned int max_len, unsigned int *out_len, unsigned int timeout_ms);

/* Send a game input packet (player controls). */
void netlib_send_input(unsigned char port, unsigned char buttons, unsigned char joy_x, unsigned char joy_y);

/* Receive a game input packet. Returns 1 if input received. */
unsigned char netlib_recv_input(unsigned char *port, unsigned char *buttons, unsigned char *joy_x, unsigned char *joy_y, unsigned int timeout_ms);

/* Send a game state sync packet. */
void netlib_send_state(const unsigned char *state, unsigned int len);

/* Receive a game state sync packet. Returns 1 if state received. */
unsigned char netlib_recv_state(unsigned char *state, unsigned int max_len, unsigned int *out_len, unsigned int timeout_ms);

/* Gracefully disconnect (send disconnect packet). */
void netlib_disconnect(void);

#endif