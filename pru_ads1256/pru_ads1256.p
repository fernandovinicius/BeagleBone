; PRU program to communicate with SPI ADC ADS1256. 
; Implement SPI signals via bit-banging. 
;
; Use the following pins configuration:
;   CS     :   P9_29    pr1_pru0_pru_r30_1  r30.t1
;   CLK    :   P9_30    pr1_pru0_pru_r30_2  r30.t2
;   MOSI   :   P9_27    pr1_pru0_pru_r30_5  r30.t5
;   MISO   :   P9_28    pr1_pru0_pru_r31_3  r31.t3
;   DRDY   :   P9_25    pr1_pru0_pru_r31_7  r31.t7

; Registers:
;   Delay/Counters Registers: r1, r2, r3, r4, r5
;   SPI Tx Buf: r27
;   SPI Rx Buf: r28

// --------------------------------------------------------------------
// Defines
// --------------------------------------------------------------------
; ADS1256 Pins
#define ADS1256_DRDY    r31.t7
#define ADS1256_MISO    r31.t3
#define ADS1256_MOSI    r30.t5
#define ADS1256_CLK     r30.t2
#define ADS1256_CS      r30.t1

; Registers
#define SPI_TX_REG      r27
#define SPI_RX_REG      r28

; Delays
#define DELAY_1_US      100
#define DELAY_250_NS    25

#define PRU0_R31_VEC_VALID  32  ; allows notification of programs end
#define PRU_EVTOUT_0        3   ; the event number that is sent back

// --------------------------------------------------------------------
// Macros
// --------------------------------------------------------------------
.setcallreg     r29.w2
.origin         0         ; start of program in PRU memory
.entrypoint     START     ; program entry point

// --------------------------------------------------------------------
// Routines
// --------------------------------------------------------------------
START:
  ; Enable the OCP master port -- allows transfer of data to Linux userspace
	LBCO  r0, C4, 4, 4    ; load SYSCFG reg into r0 (use c4 const addr)
	CLR   r0, r0, 4       ; clear bit 4 (STANDBY_INIT)
	SBCO  r0, C4, 4, 4    ; store the modified r0 back at the load addr

  MOV   r1, 0x00000000  ; RAM Data Address
  LBBO  r9, r1, 0, 4    ; r9 is samples number
  LBBO  r8, r1, 4, 4    ; r8 points to data ram addr

; ---------------------------------------------------------------------
; ADS1256 Initial Configuration
; ---------------------------------------------------------------------
  ; Wait DRDY goes Low
  WBC   ADS1256_DRDY

  ; Send setting data
  MOV SPI_TX_REG, 0x50030001  ; 0x50 -> Write Reg, address 0x00 
                              ; 0x03 -> 4 registers to write (N-1)
                              ; 0x00 -> Status: MSB | ACAL_DIS | BUF_DIS
                              ; 0x08 -> Mux: Pos: AIN0, Neg: AINCOM
                              ; 0x00 -> ADCon: CLKOUT_OFF | PGA_GAIN_1
                              ; 0xF0 -> Rate: 30k SMPS

  ; Starts SPI transfer
  CLR   ADS1256_CS  ; Set CS line LOW 

  ; Transfer the first 4 bytes 0x50030001
  CALL  SPI_TRANSFER_BYTE
  CALL  SPI_TRANSFER_BYTE
  CALL  SPI_TRANSFER_BYTE
  CALL  SPI_TRANSFER_BYTE

  ; Transfer the last 2 bytes 0x00F0
  MOV SPI_TX_REG, 0x00F00000
  CALL  SPI_TRANSFER_BYTE
  CALL  SPI_TRANSFER_BYTE

  ; Wait 1us before finish spi transfer -- Datasheet: t10 (min 8 tclk = 1us)
  MOV   r1, DELAY_1_US
  CALL  DELAY_REG1

  ; Finish SPI transfer
  SET ADS1256_CS    ; Set CS line HIGH

  ; Send Stop Read Data Continous command
SDATAC:
  MOV   SPI_TX_REG, 0x0F000000
  CLR   ADS1256_CS          ; Set CS line LOW 
  CALL  SPI_TRANSFER_BYTE   ; Send byte

  ; Wait 1us before finish spi transfer -- Datasheet: t10 (min 8 tclk = 1us)
  MOV   r1, DELAY_1_US
  CALL  DELAY_REG1

  ; Finish SPI transfer
  SET ADS1256_CS    ; Set CS line HIGH

; ---------------------------------------------------------------------
; ADS1256 Read Channel 0 -- 'r9' samples
; ---------------------------------------------------------------------
  MOV   r3, r9  ; number of samples
LOOP_SAMPLE:
  ; Wait DRDY goes Low
  WBC   ADS1256_DRDY

  ; Set channel
SET_CHANNEL:
  MOV   SPI_TX_REG, 0x51000800
  CLR   ADS1256_CS          ; Set CS line LOW 
  CALL  SPI_TRANSFER_BYTE   ; Send 1st byte
  CALL  SPI_TRANSFER_BYTE   ; Send 2nd byte
  CALL  SPI_TRANSFER_BYTE   ; Send 3rd byte
  SET   ADS1256_CS          ; Set CS line LOW, finish SPI transfer

  ; Send Sync Command
SYNC_CMD:
  MOV   SPI_TX_REG, 0xFC000000
  CLR   ADS1256_CS          ; Set CS line LOW 
  CALL  SPI_TRANSFER_BYTE   ; Send byte
  SET   ADS1256_CS          ; Set CS line LOW, finish SPI transfer

  ; Send Wake Up Command
WAKE_CMD:
  MOV   SPI_TX_REG, 0x00000000
  CLR   ADS1256_CS          ; Set CS line LOW 
  CALL  SPI_TRANSFER_BYTE   ; Send byte
  SET   ADS1256_CS          ; Set CS line HIGH, finish SPI transfer

  ; Wait 25 ns before next cmd (dummy instructions) - 
  ADD r1, r1, 1
  ADD r1, r1, 1
  ADD r1, r1, 1
  ADD r1, r1, 1
  ADD r1, r1, 1

  ; Read data command
READ_CMD:
  MOV   SPI_TX_REG, 0x01000000
  CLR   ADS1256_CS          ; Set CS line LOW 
  CALL  SPI_TRANSFER_BYTE   ; Send byte

  ; Wait 30us before receive data
  MOV   r1, 30*DELAY_1_US
DATA_WAIT:
  SUB   r1,        r1, 1
  QBNE  DATA_WAIT, r1, 0

  ; Data stored in SPI_RX_REG
  MOV   SPI_RX_REG, 0x00000000

  MOV   r2, 3
GET_SAMPLE:
  CALL  SPI_TRANSFER_BYTE   ; Receive
  SUB   r2,         r2, 1
  QBNE  GET_SAMPLE, r2, 0

  SET   ADS1256_CS          ; Set CS line HIGH, finish SPI transfer

  ; Store data in RAM
STORE_DATA:
  LSR	  SPI_RX_REG, SPI_RX_REG, 1
  MOV   r1, 0x00FFFFFF
  AND	  SPI_RX_REG, SPI_RX_REG, r1
	SBBO	SPI_RX_REG, r8, 0, 4
	ADD	  r8,         r8, 4

  WBS   ADS1256_DRDY
  
  SUB   r3,          r3, 1
  QBNE  LOOP_SAMPLE, r3, 0

END:
	MOV	r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
	HALT

// --------------------------------------------------------------------
// Procedures
// -------------------------------------------------------------------- 
; ---------------------------------------------------------------------
; SPI_TRANSFER_BYTE
; ---------------------------------------------------------------------
SPI_TRANSFER_BYTE:

  ; Ensure CLK begins LOW
  CLR ADS1256_CLK

  ; Wait 1us before shift bits
  MOV   r5, DELAY_1_US
HOLD_CS:
  SUB   r5,      r5,  1
  QBNE  HOLD_CS, r5,  0

  ; Execute 8 times (8 bits)
  MOV   r5, 8
TX_BYTE:
  SET     ADS1256_CLK               ; Starts with CLK High
  QBBC    MOSI_LOW, SPI_TX_REG.t31  ; Branch to MOSI_LOW if MSB bit is Clear
MOSI_HIGH:
  SET     ADS1256_MOSI      ; Set MOSI line High
  QBA     CLK_HOLD          ; Branch to CLK_HOLD
MOSI_LOW:
  CLR     ADS1256_MOSI      ; Set MOSI line Low
; Wait 250 ns with CLK High
CLK_HOLD:
  MOV     r4, DELAY_250_NS
CLK_HIGH:
  SUB     r4,       r4, 1
  QBNE    CLK_HIGH, r4, 0
  CLR     ADS1256_CLK

  ; Read bit MISO
  QBBC    MISO_LOW,   ADS1256_MISO            ; Branch if MISO bit is 0
  OR      SPI_RX_REG, SPI_RX_REG, 0x00000001  ; If bit = 1, put in reg LSB 
MISO_LOW:
  LSL     SPI_RX_REG, SPI_RX_REG, 1 ; Left shift received bit

; Wait 250 ns with CLK Low
  MOV     r4, DELAY_250_NS
CLK_LOW:
  SUB     r4,       r4, 1
  QBNE    CLK_LOW,  r4, 0

  ; Left shift in SPI_TX_REG
  LSL	    SPI_TX_REG, SPI_TX_REG, 1
  SUB     r5,       r5, 1   ; Decrement Bit counter
  QBNE    TX_BYTE,  r5, 0   ; Repeat until the 8 bits transf finish
  RET

; ---------------------------------------------------------------------
; DELAY_REG1
; ---------------------------------------------------------------------
DELAY_REG1:
D_LABEL:
  SUB     r1,      r1, 1
  QBNE    D_LABEL, r1, 0
  RET

