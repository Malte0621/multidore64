/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

#include "netlib.h"

/*
----------------------------------------------------------
Internal State
----------------------------------------------------------
*/
static unsigned char net_initialized = 0;
static unsigned char net_state = NSTATE_DISCONNECTED;
static unsigned int net_seq = 0;  /* Packet sequence counter */

/*
----------------------------------------------------------
Baud Rate Setup
----------------------------------------------------------
*/

/* Set the CIA2 Timer A to the value for the given baud rate.
   Formula: TimerValue = 985200 / (32 * baudRate)
   The CIA2 uses a 1MHz clock (actually 985200 Hz on PAL C64). */
static void set_baud(unsigned int baud)
{
    unsigned int timer_val;
    
    if (baud == 0) baud = 9600;
    
    timer_val = 985200 / (32 * baud);
    if (timer_val == 0) timer_val = 1;
    if (timer_val > 255) timer_val = 255;
    
    /* Load Timer A with the baud rate value */
    CIA2_TIMER_A_LO = (unsigned char)(timer_val & 0xFF);
    CIA2_TIMER_A_HI = (unsigned char)((timer_val >> 8) & 0xFF);
}

/*
----------------------------------------------------------
Serial Port Initialization
----------------------------------------------------------
*/

unsigned char netlib_init(unsigned int baud)
{
    if (net_initialized) return 1;
    
    /* Set baud rate */
    set_baud(baud);
    
    /* Enable serial port: 8N1 (8 data bits, no parity, 1 stop bit)
       SER_CTRL: SREN | STEN | SPB
       TSC: SPEN | TA_FREE (Timer A free-running for baud) */
    CIA2_SER_CTRL = SER_SREN | SER_STEN | SER_SPB;
    CIA2_TIMER_SER_CTRL = TSC_SPEN | TSC_TA_FREE;
    
    /* Clear any pending interrupts */
    CIA2_SER_INTR_EN = 0x00;
    CIA2_INTR_PENDING = 0xFF;  /* Read to clear */
    
    net_initialized = 1;
    net_state = NSTATE_DISCONNECTED;
    net_seq = 0;
    
    return 1;
}

void netlib_shutdown(void)
{
    if (!net_initialized) return;
    
    /* Disable serial port */
    CIA2_SER_CTRL = 0x00;
    CIA2_TIMER_SER_CTRL = 0x00;
    CIA2_SER_INTR_EN = 0x00;
    
    net_initialized = 0;
    net_state = NSTATE_DISCONNECTED;
}

/*
----------------------------------------------------------
Basic Byte I/O
----------------------------------------------------------
*/

void netlib_send(unsigned char byte)
{
    if (!net_initialized) return;
    
    /* Wait for transmit buffer to be empty (SPT flag clear) */
    while (CIA2_TIMER_SER_CTRL & TSC_SPT)
        ;
    
    /* Write byte to serial data register */
    CIA2_SER_DATA = byte;
}

unsigned char netlib_recv(void)
{
    if (!net_initialized) return 0xFF;
    
    /* Wait for receive data (SPR flag set) */
    while (!(CIA2_TIMER_SER_CTRL & TSC_SPR))
        ;
    
    /* Read byte from serial data register */
    return CIA2_SER_DATA;
}

unsigned char netlib_recv_timeout(unsigned int timeout_ms)
{
    if (!net_initialized) return 0xFF;
    
    unsigned int count = 0;
    /* Approximate: at 9600 baud, 1 byte takes ~1ms.
       Use a simple loop counter. 1ms ≈ 320 cycles at 1MHz.
       For timeout_ms, we loop timeout_ms * 320 times.
       This is approximate and depends on compiler optimization. */
    while (!(CIA2_TIMER_SER_CTRL & TSC_SPR))
    {
        count++;
        if (count > timeout_ms * 320)
            return 0xFF;
    }
    
    return CIA2_SER_DATA;
}

unsigned char netlib_available(void)
{
    if (!net_initialized) return 0;
    return (CIA2_TIMER_SER_CTRL & TSC_SPR) ? 1 : 0;
}

/*
----------------------------------------------------------
String / Data I/O
----------------------------------------------------------
*/

void netlib_send_str(const char *str)
{
    if (!net_initialized) return;
    while (*str)
    {
        netlib_send((unsigned char)*str);
        str++;
    }
    netlib_send(0x00);  /* Null terminator */
}

unsigned int netlib_recv_str(char *buf, unsigned int max_len, unsigned int timeout_ms)
{
    if (!net_initialized) return 0;
    
    unsigned int len = 0;
    while (len < max_len - 1)
    {
        unsigned char c = netlib_recv_timeout(timeout_ms);
        if (c == 0xFF) break;  /* Timeout */
        if (c == 0x00) break;  /* Null terminator */
        buf[len++] = (char)c;
    }
    buf[len] = 0x00;
    return len;
}

void netlib_send_data(const unsigned char *data, unsigned int len)
{
    if (!net_initialized) return;
    for (unsigned int i = 0; i < len; i++)
        netlib_send(data[i]);
}

unsigned int netlib_recv_data(unsigned char *buf, unsigned int max_len, unsigned int timeout_ms)
{
    if (!net_initialized) return 0;
    
    unsigned int len = 0;
    while (len < max_len)
    {
        unsigned char c = netlib_recv_timeout(timeout_ms);
        if (c == 0xFF) break;  /* Timeout */
        buf[len++] = c;
    }
    return len;
}

/*
----------------------------------------------------------
Protocol Layer
----------------------------------------------------------
*/

/* Send a raw protocol packet:
   [MAGIC][TYPE][SEQ_LO][SEQ_HI][LEN][DATA...] */
static void send_raw_packet(unsigned char type, const unsigned char *data, unsigned int len)
{
    if (!net_initialized) return;
    
    netlib_send(NPROT_MAGIC);
    netlib_send(type);
    netlib_send((unsigned char)(net_seq & 0xFF));
    netlib_send((unsigned char)((net_seq >> 8) & 0xFF));
    netlib_send((unsigned char)len);
    
    if (data && len > 0)
        netlib_send_data(data, len);
    
    net_seq++;
}

/* Try to receive a raw protocol packet.
   Returns 1 if a valid packet was received, 0 on timeout.
   *out_type = packet type, *out_len = data length. */
static unsigned char recv_raw_packet(unsigned char *out_type, unsigned char **out_data, unsigned int *out_len, unsigned int timeout_ms)
{
    if (!net_initialized) return 0;
    
    /* Wait for magic byte */
    unsigned char magic = netlib_recv_timeout(timeout_ms);
    if (magic == 0xFF) return 0;
    if (magic != NPROT_MAGIC) return 0;
    
    /* Read type */
    unsigned char type = netlib_recv_timeout(timeout_ms);
    if (type == 0xFF) return 0;
    
    /* Read sequence */
    unsigned char seq_lo = netlib_recv_timeout(timeout_ms);
    unsigned char seq_hi = netlib_recv_timeout(timeout_ms);
    if (seq_lo == 0xFF || seq_hi == 0xFF) return 0;
    
    /* Read length */
    unsigned char len = netlib_recv_timeout(timeout_ms);
    if (len == 0xFF) return 0;
    
    /* Read data */
    static unsigned char packet_data[NPROT_MAX_DATA];
    for (unsigned int i = 0; i < len; i++)
    {
        unsigned char c = netlib_recv_timeout(timeout_ms);
        if (c == 0xFF) return 0;
        packet_data[i] = c;
    }
    
    *out_type = type;
    *out_data = packet_data;
    *out_len = len;
    return 1;
}

/*
----------------------------------------------------------
Connection Management
----------------------------------------------------------
*/

unsigned char netlib_connect(unsigned int timeout_ms)
{
    if (!net_initialized) return 0;
    
    net_state = NSTATE_CONNECTING;
    
    /* Send handshake packets until we get an ACK */
    unsigned int start = 0;
    for (unsigned int attempt = 0; attempt < 10; attempt++)
    {
        /* Send handshake: [MAGIC][HANDSHAKE][SEQ_LO][SEQ_HI][0] */
        send_raw_packet(NPROT_HANDSHAKE, 0, 0);
        
        /* Wait for response */
        unsigned char type;
        unsigned char *data;
        unsigned int len;
        if (recv_raw_packet(&type, &data, &len, timeout_ms))
        {
            if (type == NPROT_ACK)
            {
                net_state = NSTATE_CONNECTED;
                return 1;
            }
            if (type == NPROT_HANDSHAKE)
            {
                /* Remote is also trying to connect - we win, send ACK */
                send_raw_packet(NPROT_ACK, 0, 0);
                net_state = NSTATE_CONNECTED;
                return 1;
            }
        }
    }
    
    net_state = NSTATE_DISCONNECTED;
    return 0;
}

unsigned char netlib_accept(unsigned int timeout_ms)
{
    if (!net_initialized) return 0;
    
    net_state = NSTATE_CONNECTING;
    
    /* Wait for handshake packet */
    unsigned char type;
    unsigned char *data;
    unsigned int len;
    if (recv_raw_packet(&type, &data, &len, timeout_ms))
    {
        if (type == NPROT_HANDSHAKE)
        {
            /* Reply with ACK */
            send_raw_packet(NPROT_ACK, 0, 0);
            net_state = NSTATE_CONNECTED;
            return 1;
        }
    }
    
    net_state = NSTATE_DISCONNECTED;
    return 0;
}

unsigned char netlib_connected(void)
{
    return (net_state == NSTATE_CONNECTED) ? 1 : 0;
}

unsigned char netlib_state(void)
{
    return net_state;
}

/*
----------------------------------------------------------
Reliable Packet Exchange
----------------------------------------------------------
*/

unsigned char netlib_send_packet(unsigned char type, const unsigned char *data, unsigned int len, unsigned int timeout_ms)
{
    if (!net_initialized || net_state != NSTATE_CONNECTED) return 0;
    if (len > NPROT_MAX_DATA) return 0;
    
    /* Send packet */
    send_raw_packet(type, data, len);
    
    /* Wait for ACK */
    unsigned char rtype;
    unsigned char *rdata;
    unsigned int rlen;
    if (recv_raw_packet(&rtype, &rdata, &rlen, timeout_ms))
    {
        if (rtype == NPROT_ACK)
            return 1;
        if (rtype == NPROT_NAK)
            return 0;
    }
    
    return 0;
}

unsigned char netlib_recv_packet(unsigned char *buf, unsigned int max_len, unsigned int *out_len, unsigned int timeout_ms)
{
    if (!net_initialized || net_state != NSTATE_CONNECTED) return 0;
    
    unsigned char type;
    unsigned char *data;
    unsigned int len;
    
    if (!recv_raw_packet(&type, &data, &len, timeout_ms))
        return 0;
    
    /* Send ACK */
    send_raw_packet(NPROT_ACK, 0, 0);
    
    /* Copy data to buffer */
    if (buf && len <= max_len)
    {
        for (unsigned int i = 0; i < len; i++)
            buf[i] = data[i];
        *out_len = len;
    }
    else
    {
        *out_len = 0;
    }
    
    return type;
}

/*
----------------------------------------------------------
Game-Specific Helpers
----------------------------------------------------------
*/

void netlib_send_input(unsigned char port, unsigned char buttons, unsigned char joy_x, unsigned char joy_y)
{
    if (!net_initialized || net_state != NSTATE_CONNECTED) return;
    
    unsigned char data[4];
    data[0] = port;
    data[1] = buttons;
    data[2] = joy_x;
    data[3] = joy_y;
    
    netlib_send_packet(NPROT_INPUT, data, 4, 100);
}

unsigned char netlib_recv_input(unsigned char *port, unsigned char *buttons, unsigned char *joy_x, unsigned char *joy_y, unsigned int timeout_ms)
{
    if (!net_initialized || net_state != NSTATE_CONNECTED) return 0;
    
    unsigned char buf[4];
    unsigned int len;
    unsigned char type = netlib_recv_packet(buf, 4, &len, timeout_ms);
    
    if (type == NPROT_INPUT && len >= 4)
    {
        *port = buf[0];
        *buttons = buf[1];
        *joy_x = buf[2];
        *joy_y = buf[3];
        return 1;
    }
    
    return 0;
}

void netlib_send_state(const unsigned char *state, unsigned int len)
{
    if (!net_initialized || net_state != NSTATE_CONNECTED) return;
    if (len > NPROT_MAX_DATA) return;
    
    netlib_send_packet(NPROT_STATE, state, len, 100);
}

unsigned char netlib_recv_state(unsigned char *state, unsigned int max_len, unsigned int *out_len, unsigned int timeout_ms)
{
    if (!net_initialized || net_state != NSTATE_CONNECTED) return 0;
    
    unsigned char type = netlib_recv_packet(state, max_len, out_len, timeout_ms);
    return (type == NPROT_STATE) ? 1 : 0;
}

void netlib_disconnect(void)
{
    if (!net_initialized) return;
    
    if (net_state == NSTATE_CONNECTED)
    {
        send_raw_packet(NPROT_DISCONNECT, 0, 0);
    }
    
    net_state = NSTATE_DISCONNECTED;
}
