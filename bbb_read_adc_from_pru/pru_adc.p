; Assembly code to read BBB ADC from PRU.
;
; The number of samples and the sampling rate are controlled with the
; parameters received from host (linux) through RAM memory.
;
; Pin P9_29 (DEBUG_PIN) is used to check the sampling period as debug.
; Each period of signal DEBUG_PIN indicates the sampling of 2*FIFO0_LEN samples.
;

// --------------------------------------------------------------------
// Defines
// --------------------------------------------------------------------
#define PRU0_R31_VEC_VALID  32  ; allows notification of programs end
#define PRU_EVTOUT_0        3   ; the event number that is sent back

; ADC Registers Address/Offset -- AM335x TRM, Chapter: 'Touchscreen Controller'
#define ADC_BASE_ADDR     0x44E0D000
#define ADC_FIFO0_ADDR    0x44E0D100
#define ADC_FIFO1_ADDR    0x44E0D200
#define IRQSTAT           0x28
#define IRQSET            0x2C
#define IRQCLR            0x30
#define CTRL              0x40
#define CLKDIV            0x4C
#define STEPENABLE        0x54
#define STEPCFG1          0x64
#define STEPDELAY1        0x68
#define FIFO0_CNT         0xE4
#define FIFO0_THLD        0xE8

; Registers used in code
#define AUX_REG1        r1      ; Temp1
#define AUX_REG2        r2      ; Temp2
#define AUX_REG3        r3      ; Temp3
#define POOLRAM_PTR     r4      ; Pool RAM Address pointer
#define LOOP_NUM        r5      ; Number of loops to perform
#define DIV_CLK         r6      ; Value that divides the ADC clock
#define ADC_BASE        r7      ; ADC base address
#define CH_CFG          r8      ; Channel register config
#define FIFO0_LEN       r9      ; FIFO0 buffer length

; Debug
#define DEBUG_CLK       r30.t1
#define DBG_PIN_STATE   r20

// --------------------------------------------------------------------
// Macros
// --------------------------------------------------------------------
.setcallreg     r29.w2
.origin         0         ; start of program in PRU memory
.entrypoint     START     ; program entry point

; Debug -- Toggle Debug Pin each FIFO0_LEN samples
.macro DEBUG_ON
  QBBS  DBG_LOW, DBG_PIN_STATE.t0
DBG_HIGH:                   ;
  SET   DEBUG_CLK           ;
  SET   DBG_PIN_STATE.t0    ;
  QBA   FINISH              ;
DBG_LOW:                    ;
  CLR   DEBUG_CLK           ;
  CLR   DBG_PIN_STATE.t0    ;
FINISH:                     ;
.endm

// --------------------------------------------------------------------
// MAIN
// --------------------------------------------------------------------
START:
  ; Enable the OCP master port -- allows transfer of data to Linux userspace
	LBCO  r0, C4, 4, 4    ; load SYSCFG reg into r0 (use c4 const addr)
	CLR   r0, r0, 4       ; clear bit 4 (STANDBY_INIT)
	SBCO  r0, C4, 4, 4    ; store the modified r0 back at the load addr

  ; Load input parameters
  MOV   AUX_REG1,    0x00000000       ; AUX_REG1 points to RAM Data Address
  LBBO  POOLRAM_PTR, AUX_REG1,  0, 4  ; Load Pool RAM Memory Address
  LBBO  DIV_CLK,     AUX_REG1,  4, 4  ; Load Clk div number
  LBBO  LOOP_NUM,    AUX_REG1,  8, 4  ; Load Number of Samples
  LBBO  CH_CFG,      AUX_REG1, 12, 4  ; Load Ch Cfg code
  LBBO  FIFO0_LEN,   AUX_REG1, 16, 4  ; Load fifo0 buffer len

; ---------------------------------------------------------------------
; Clear Shared Mem
; ---------------------------------------------------------------------
  MOV   AUX_REG1,    0x0000           ; Aux1 holds Null Value
  MOV   AUX_REG2,    LOOP_NUM         ; Aux2 holds sample number  (temporarily)
  MOV   AUX_REG3,    POOLRAM_PTR      ; Aux3 points to shared mem (temporarily)

CLR_SHR_MEM:
  SBBO  AUX_REG1,    AUX_REG3, 0, 2   ; Store null data into Shared RAM
  ADD   AUX_REG3,    AUX_REG3, 2      ; Increase pointer by SAMPLE_SIZE
  SUB   AUX_REG2,    AUX_REG2, 1      ; Decrement loop counter
  QBNE  CLR_SHR_MEM, AUX_REG2, 0      ;

; ---------------------------------------------------------------------
; ADC Config
; ---------------------------------------------------------------------
ADC_CONFIG:
  MOV   ADC_BASE, ADC_BASE_ADDR             ; ADC_BASE points to ADC Registers base address

  ; Turn off ADC Module
  LBBO  AUX_REG1, ADC_BASE, CTRL, 4         ; Load Ctrl Register value into Aux1
  CLR   AUX_REG1.t0                         ; Clear BIT0 of Ctrl Register (Turn off ADC)
	SBBO  AUX_REG1, ADC_BASE, CTRL, 4         ;

  ; Clear Interrupt flags
  MOV   AUX_REG1, 0x000000FF                ; Mask interrputs flags
  SBBO  AUX_REG1, ADC_BASE, IRQSTAT, 4      ;

  ; Enable IRQ Interrupts
  MOV   AUX_REG1, 0x00000006                ; End_of_Sequence | FIFO0_Threshold
  SBBO  AUX_REG1, ADC_BASE, IRQSET, 4       ;

  ; Set FIFO0 length
  MOV   AUX_REG1, FIFO0_LEN                 ; Buffer len (N-1)
  SBBO  AUX_REG1, ADC_BASE, FIFO0_THLD, 4   ;

  ; Set ADC Clock Div
  MOV   AUX_REG1, DIV_CLK                   ; Clock will be divided by this
  SBBO  AUX_REG1, ADC_BASE, CLKDIV, 4       ;

  ; Set Step1 delay
  MOV   AUX_REG1, 0                         ; Sample Delay: 0 | Open Delay: 0
  SBBO  AUX_REG1, ADC_BASE, STEPDELAY1, 4   ;

  ; ADC Step1 Config Register
  MOV   AUX_REG1, CH_CFG                    ; Mode SW enabled: Continous, no avarage
  SBBO  AUX_REG1, ADC_BASE, STEPCFG1, 4     ;

  ; Set Channel
  MOV   AUX_REG1, 0x00000002                ; Enable step 1
  SBBO  AUX_REG1, ADC_BASE, STEPENABLE, 4   ;

  ; Turn on ADC Module
  MOV   AUX_REG1, 0x00000005                ; TSC_ADC Enable | STEPCFG reg writable
  SBBO  AUX_REG1, ADC_BASE, CTRL, 4         ;

  ; Initialize debug
  CLR   DEBUG_CLK         ; Starts with debug pin LOW
  MOV   DBG_PIN_STATE, 0  ; Dbg pin state: LOW

; ---------------------------------------------------------------------
; Loop Sampling
; ---------------------------------------------------------------------
SAMPLING:
  DEBUG_ON ; Dbg pin is toogle here

  ; Clear Interrupt flags
  MOV   AUX_REG1,  0xFF
  SBBO  AUX_REG1,  ADC_BASE, IRQSTAT, 4

  ; Check FIFO0 counter before copy data
ADC_COUNT:
  LBBO  AUX_REG1,  ADC_BASE, FIFO0_CNT, 4
  QBNE  ADC_COUNT, AUX_REG1, FIFO0_LEN

  ; Copy FIFO0 data to Shared Memory Space
  MOV   AUX_REG3,    FIFO0_LEN
COPY_DATA:
  MOV   AUX_REG1,    ADC_FIFO0_ADDR
  LBBO  AUX_REG2,    AUX_REG1,      0, 2  ; Load FIFO0 data into Aux3
  SBBO  AUX_REG2,    POOLRAM_PTR,   0, 2  ; Store data into Shared RAM
  ADD   POOLRAM_PTR, POOLRAM_PTR,   2     ; Increase pointer by SAMPLE_SIZE

  SUB   AUX_REG3,    AUX_REG3, 1  ; Decrement FIFO0 counter samples
  QBNE  COPY_DATA,   AUX_REG3, 0  ; Stop if FIFO0 counter samples == 0

  SUB   LOOP_NUM,  LOOP_NUM, 1    ; Decrement loops counter
  QBNE  SAMPLING,  LOOP_NUM, 0    ; Finish if loops counter == 0

END:
	MOV	r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
	HALT
