package main

import (
	"machine"
	"time"
	"unsafe"
)

func main() {
	main2()
}

const ledPin = 25
const ledMask = 1 << ledPin
const addrSIO = 0xd000_0000
const addrGPIO_OUT = addrSIO + 0x010
const addrGPIO_OE = addrSIO + 0x020
const addrIOBank = 0x4001_4000
const sizeIOPinBlk = 4 * 2
const addrGPIO25CTRL = addrIOBank + ledPin*sizeIOPinBlk + 4
const fnSIOGPIO = 5

func main1() {
	// Attach the SIO to the GPIO.
	writeRegister(addrGPIO25CTRL, fnSIOGPIO)
	// Enable Digitial output on that pin in the SIO.
	writeRegister(addrGPIO_OE, ledMask)
	// Turn LED on!
	writeRegister(addrGPIO_OUT, ledMask)
}

func main2() {
	var p Pin = 25
	p.ConfigOutput()
	p.High()
	time.Sleep(time.Second)
	p.Low()
}

func main3() {
	led := machine.LED
	led.Configure(machine.PinConfig{Mode: machine.PinOutput})
	led.High()
}

// This function moves the 3D printing platform by one step.
func stepMotor(direction bool) {
	const pinStep1, pinStep2, pinDir1, pinDir2 = 1, 2, 3, 4
	const addrGPIO_SET = addrSIO + 0x014
	const addrGPIO_CLR = addrSIO + 0x018
	if direction {
		writeRegister(addrGPIO_SET, (1<<pinDir1)|(1<<pinDir2))
	} else {
		writeRegister(addrGPIO_CLR, (1<<pinDir1)|(1<<pinDir2))
	}
	writeRegister(addrGPIO_SET, (1<<pinStep1)|(1<<pinStep2))
	writeRegister(addrGPIO_CLR, (1<<pinStep1)|(1<<pinStep2))
}

func writeRegister(addr uintptr, val uint32) {
	reg := (*uint32)(unsafe.Pointer(addr))
	*reg = val
}
func readRegister(addr uintptr) uint32 {
	reg := (*uint32)(unsafe.Pointer(addr))
	return *reg
}

type Pin uint8

func (p Pin) ConfigOutput() {
	writeRegister(addrIOBank+uintptr(p)*sizeIOPinBlk+4, fnSIOGPIO)
	v := readRegister(addrGPIO_OE)
	writeRegister(addrGPIO_OE, v|(1<<p))
}

func (p Pin) High() {
	v := readRegister(addrGPIO_OUT)
	writeRegister(addrGPIO_OUT, v|(1<<p))
}
func (p Pin) Low() {
	v := readRegister(addrGPIO_OUT)
	writeRegister(addrGPIO_OUT, v&^(1<<p))
}
