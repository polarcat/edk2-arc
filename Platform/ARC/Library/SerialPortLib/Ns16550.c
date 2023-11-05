/** @file
  Minimalist 16550 UART Serial Port driver.

  This implementation is just a stripped down version of
  MdeModulePkg/Library/BaseSerialPortLib16550/BaseSerialPortLib16550.c

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include <Base.h>
#include <Library/SerialPortLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/BaseLib.h>

//
// 16550 UART register offsets and bitfields
//
#define R_UART_RXBUF        0    // LCR_DLAB = 0
#define R_UART_TXBUF        0    // LCR_DLAB = 0

#define R_UART_DLL          0
#define R_UART_DLM          1
#define R_UART_IER          1

#define R_UART_FCR          2
#define B_UART_FCR_FIFOE    BIT0
#define B_UART_FCR_RXSR     BIT1 // Receiver soft reset
#define B_UART_FCR_TXSR     BIT3 // Transmitter soft reset
#define B_UART_FCR_FIFO64   BIT5
#define B_UART_FCRVAL       (B_UART_FCR_FIFOE | B_UART_FCR_RXSR | B_UART_FCR_TXSR)

#define R_UART_LCR          3
#define B_UART_LCR_8N1      0x03
#define B_UART_LCR_DLAB     BIT7

#define R_UART_MCR          4
#define B_UART_MCR_DTR      BIT0
#define B_UART_MCR_RTS      BIT1
#define B_UART_MCRVAL       (B_UART_MCR_DTR | B_UART_MCR_RTS)

#define R_UART_LSR          5
#define B_UART_LSR_RXRDY    BIT0
#define B_UART_LSR_TXRDY    BIT5
#define B_UART_LSR_TEMT     BIT6

#define R_UART_MSR          6
#define B_UART_MSR_CTS      BIT4
#define B_UART_MSR_DSR      BIT5

#undef PcdGetBool
#define PcdGetBool(TokenName) _PCD_VALUE_##TokenName

#undef PcdGet8
#define PcdGet8(TokenName) _PCD_VALUE_##TokenName

#undef PcdGet32
#define PcdGet32(TokenName) _PCD_VALUE_##TokenName

/**
  Read 8-bit 16550 register.

  @param  Base    The base address register of UART device.
  @param  Offset  The offset of the 16550 register to read.

  @return The value read from the 16550 register.
**/
UINT8
SerialIn(
  UINTN  Base,
  UINTN  Offset
  )
{
  UINTN Addr = Base + Offset * PcdGet32(PcdSerialRegisterStride);

  if (!PcdGetBool(PcdSerialUseMmio)) {
    return IoRead8(Addr);
  } else {
    if (PcdGet8(PcdSerialRegisterAccessWidth) == 32) {
      return MmioRead32(Addr);
    }
    return MmioRead8(Addr);
  }
}

/**
  Write 8-bit 16550 register.

  @param  Base    The base address register of UART device.
  @param  Offset  The offset of the 16550 register to write.
  @param  Value   The value to write to the 16550 register specified by Offset.
**/
VOID
SerialOut(
  UINTN  Base,
  UINTN  Offset,
  UINT8  Value
  )
{
  UINTN Addr = Base + Offset * PcdGet32(PcdSerialRegisterStride);

  if (!PcdGetBool(PcdSerialUseMmio)) {
    IoWrite8(Addr, Value);
  } else {
    if (PcdGet8(PcdSerialRegisterAccessWidth) == 32) {
      MmioWrite32(Addr, Value);
    } else {
      MmioWrite8(Addr, Value);
    }
  }
}

/**
  Retrieve the I/O or MMIO base address register for the UART device.

  @return  The base address register of the UART device.
**/
UINTN
GetSerialRegisterBase(VOID)
{
  return PcdGet32(PcdSerialRegisterBase);
}

/**
  Wait for the hardware flow control signal.

  @param  SerialBase The base address register of UART device.

  Logic:

  Case 1. Hardware flow control and cable detection are configured:

  Wait for both DSR and CTS to be set DSR is set if a cable is connected. CTS is
  set if it is ok to transmit data.

  DSR  CTS  Description                               Action
  ===  ===  ========================================  ========
  0    0    No cable connected.                       Wait
  0    1    No cable connected.                       Wait
  1    0    Cable connected, but not clear to send.   Wait
  1    1    Cable connected, and clear to send.       Transmit

  Case 2. Otherwise:

  Wait for both DSR and CTS to be set OR for DSR to be clear. DSR is set if a
  cable is connected. CTS is set if it is ok to transmit data.

  DSR  CTS  Description                               Action
  ===  ===  ========================================  ========
  0    0    No cable connected.                       Transmit
  0    1    No cable connected.                       Transmit
  1    0    Cable connected, but not clear to send.   Wait
  1    1    Cable connected, and clar to send.        Transmit

**/

#define B_UART_IS_WRITABLE (B_UART_MSR_DSR | B_UART_MSR_CTS)

VOID
SerialPortWaitWritable(
  UINTN  SerialBase
  )
{
  while (1) {
    if (!PcdGetBool(PcdSerialUseHardwareFlowControl)) {
      return;
    } else {
      //
      // Wait for the hardware flow control signal
      //
      if (PcdGetBool(PcdSerialDetectCable)) {
        UINT8 Bits = SerialIn(SerialBase, R_UART_MSR) & B_UART_IS_WRITABLE;
        if (Bits == B_UART_IS_WRITABLE) {
          return;
        }
      } else {
        UINT8 Bits = SerialIn(SerialBase, R_UART_MSR) & B_UART_IS_WRITABLE;
        if (Bits != B_UART_MSR_DSR) {
          return;
        }
      }
    }
  }
}

/**
  Wait for both the transmit FIFO and shift register empty.

  @param  SerialBase The base address register of UART device.
**/
VOID
SerialPortFlush(
  UINTN  Base
  )
{
  UINT8 Mask = B_UART_LSR_TEMT | B_UART_LSR_TXRDY;

  while ((SerialIn(Base, R_UART_LSR) & Mask) != Mask) {
  }
}

/**
  Initialize the serial device hardware.

  @retval RETURN_SUCCESS        The serial device was initialized.
  @retval RETURN_SUCCESS        No initialization required.
  @retval RETURN_DEVICE_ERROR   The serial device could not be initialized.
**/
RETURN_STATUS
EFIAPI
SerialPortInitialize(VOID)
{
  UINTN   Base;
  UINT32  Div;
  UINT32  DivRem;
  UINT8   LineCtl;

  // Get the base address of the serial port in either I/O or MMIO space
  Base = GetSerialRegisterBase();
  if (Base == 0) {
    return RETURN_DEVICE_ERROR;
  }

  // Wait until tx buffer is empty
  while ((SerialIn(Base, R_UART_LSR) & B_UART_LSR_TEMT) == 0) {
  }

  // Calculate divisor for baud generator: Ref_Clk_Rate / Baud_Rate / 16
  Div = PcdGet32(PcdSerialClockRate) / (PcdGet32(PcdSerialBaudRate) * 16);
  DivRem = PcdGet32(PcdSerialClockRate) % (PcdGet32(PcdSerialBaudRate) * 16);

  if (DivRem >= PcdGet32(PcdSerialBaudRate) * 8) {
    Div++;
  }

  LineCtl = PcdGet32(PcdSerialLineControl);
  if (LineCtl == 0) {
    LineCtl = B_UART_LCR_8N1; // Make it default
  }

  SerialOut(Base, R_UART_IER, 0);
  SerialOut(Base, R_UART_MCR, B_UART_MCRVAL);
  SerialOut(Base, R_UART_FCR, B_UART_FCRVAL);
  //
  // Set line control and enable access to DLL/DLM registers via DLAB bit.
  //
  SerialOut(Base, R_UART_LCR, LineCtl | B_UART_LCR_DLAB);
  SerialOut(Base, R_UART_DLL, Div & 0xff);
  SerialOut(Base, R_UART_DLM, (Div >> 8) & 0xff);
  SerialOut(Base, R_UART_LCR, LineCtl);

  return Base;
}

/**
  Write data from buffer to serial device.

  @param  Buffer  Pointer to source data buffer.
  @param  Bytes   Number of bytes to write.

  @retval =0      No data has been written or operation has failed.
  @retval >0      The number of written bytes.
**/
UINTN
EFIAPI
SerialPortWrite(
  IN UINT8  *Buffer,
  IN UINTN  Bytes
  )
{
  UINTN Base;
  UINTN Result;
  UINTN Index;
  UINTN FifoSize;

  if (Buffer == NULL) {
    return 0;
  }

  Base = GetSerialRegisterBase();
  if (Base == 0) {
    return 0;
  }

  if (Bytes == 0) {
    SerialPortFlush(Base);
    return 0;
  }

  //
  // Compute the maximum size of the Tx FIFO
  //
  FifoSize = 1;
  if ((PcdGet8(PcdSerialFifoControl) & B_UART_FCR_FIFOE) != 0) {
    if ((PcdGet8(PcdSerialFifoControl) & B_UART_FCR_FIFO64) == 0) {
      FifoSize = 16;
    } else {
      FifoSize = PcdGet32(PcdSerialExtendedTxFifoSize);
    }
  }

  Result = Bytes;
  while (Bytes != 0) {
    SerialPortFlush(Base);

    //
    // Fill entire Tx FIFO
    //
    for (Index = 0; Index < FifoSize && Bytes != 0; Index++, Bytes--) {
      SerialPortWaitWritable(Base);
      SerialOut(Base, R_UART_TXBUF, *Buffer++);
    }
  }

  return Result;
}

/**
  Reads data from serial device into buffer.

  @param  Buffer   Pointer to destination data buffer.
  @param  Bytes    Number of bytes to read from the serial device.

  @retval =0       No data has been read or operation has failed.
  @retval >0       The number of bytes read from the serial device.
**/
UINTN
EFIAPI
SerialPortRead (
  OUT UINT8  *Buffer,
  IN  UINTN  Bytes
  )
{
  UINTN  Base;
  UINTN  Result;
  UINT8  Mcr;

  if (NULL == Buffer) {
    return 0;
  }

  Base = GetSerialRegisterBase();
  if (Base == 0) {
    return 0;
  }

  Mcr = SerialIn(Base, R_UART_MCR) & ~B_UART_MCR_RTS;

  for (Result = 0; Bytes-- != 0; Result++) {
    //
    // Wait for the serial port to have some data.
    //
    while ((SerialIn(Base, R_UART_LSR) & B_UART_LSR_RXRDY) == 0) {
      if (PcdGetBool (PcdSerialUseHardwareFlowControl)) {
        //
        // Set RTS to let the peer send some data
        //
        SerialOut(Base, R_UART_MCR, (UINT8)(Mcr | B_UART_MCR_RTS));
      }
    }

    if (PcdGetBool(PcdSerialUseHardwareFlowControl)) {
      //
      // Clear RTS to prevent peer from sending data
      //
      SerialOut(Base, R_UART_MCR, Mcr);
    }

    //
    // Read byte from the receive buffer.
    //
    *Buffer++ = SerialIn(Base, R_UART_RXBUF);
  }

  return Result;
}
