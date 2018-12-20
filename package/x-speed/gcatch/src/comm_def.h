#ifndef _COMM_DEF_H_
#define _COMM_DEF_H_

typedef unsigned char byte;	/* Unsigned 8  bit value type. */
typedef unsigned short word;	/* Unsinged 16 bit value type. */
typedef unsigned long dword;	/* Unsigned 32 bit value type. */

#define DIAG_MAX_RX_PKT_SIZ (8192*2)
#define DIAG_MAX_TX_PKT_SIZ 2048

#define  ASYNC_HDLC_FLAG	0x7E	/* Opening/closing flag           */
#define  ASYNC_HDLC_ESC		0x7D	/* Data Escape                    */
#define  ASYNC_HDLC_ESC_MASK	0x20	/* XOR mask used with Data Escape */
#define  ASYNC_HDLC_CRC         2	/* CRC bytes = 2 */
#define  CRC_SEED		0xFFFF
#define  CRC_END		0xF0B8

#define BAUDRATE           	B115200
#define BLOCK_SIZE        	0

#define CFLAGS_TO_SET 	        (CREAD | HUPCL)
#define CFLAGS_TO_CLEAR 	(CSTOPB | PARENB | CLOCAL)
#define CFLAGS_HARDFLOW 	(CRTSCTS)

#define RAW_DATA_BUF_SIZE (4*1024)
#define RAW_DATA_BUF_COUNT (150)

typedef struct
{
  dword length;
  byte buf[DIAG_MAX_TX_PKT_SIZ];
} tx_pkt_buf_type;

typedef struct
{
  dword length;
  byte buf[DIAG_MAX_RX_PKT_SIZ];
} rx_pkt_buf_type;


typedef struct
{
    dword used;
    byte buf[RAW_DATA_BUF_SIZE];
} raw_data_buf;

#endif
