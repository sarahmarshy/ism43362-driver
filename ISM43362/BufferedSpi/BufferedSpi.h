
/**
 * @file    BufferedSpi.h
 * @brief   Software Buffer - Extends mbed SPI functionallity
 * @author  Armelle Duboc
 * @version 1.0
 * @see
 *
 * Copyright (c) STMicroelectronics 2017
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BUFFEREDSPI_H
#define BUFFEREDSPI_H
 
#include "mbed.h"
#include "platform/platform.h"
#include "MyBuffer.h"

/** A spi port (SPI) for communication with wifi device
 *
 * Can be used for Full Duplex communication, or Simplex by specifying
 * one pin as NC (Not Connected)
 *
 * Example:
 * @code
 *  #include "mbed.h"
 *  #include "BufferedSerial.h"
 *
 *  BufferedSerial pc(USBTX, USBRX);
 *
 *  int main()
 *  { 
 *      while(1)
 *      {
 *          Timer s;
 *        
 *          s.start();
 *          pc.printf("Hello World - buffered\n");
 *          int buffered_time = s.read_us();
 *          wait(0.1f); // give time for the buffer to empty
 *        
 *          s.reset();
 *          printf("Hello World - blocking\n");
 *          int polled_time = s.read_us();
 *          s.stop();
 *          wait(0.1f); // give time for the buffer to empty
 *        
 *          pc.printf("printf buffered took %d us\n", buffered_time);
 *          pc.printf("printf blocking took %d us\n", polled_time);
 *          wait(0.5f);
 *      }
 *  }
 * @endcode
 */

/**
 *  @class BufferedSpi
 *  @brief Software buffers and interrupt driven tx and rx for Serial
 */  
class BufferedSpi : public SPI, public FileHandle 
{
private:
    MyBuffer <char> _txbuf;
    uint32_t      _buf_size;
    uint32_t      _tx_multiple;
    DigitalOut    nss;
    void rxIrq(void);
    void txIrq(void);
    void prime(void);

    Callback<void()> _cbs[2];
    
public:
    MyBuffer <char> _rxbuf;
    DigitalIn dataready;
    enum IrqType {
        RxIrq = 0,
        TxIrq,

        IrqCnt
    };

    /** Create a BufferedSpi Port, connected to the specified transmit and receive pins
     *  @param SPI mosi pin
     *  @param SPI miso pin
     *  @param SPI sclk pin
     *  @param SPI nss pin
     *  @param Dataready pin
     *  @param buf_size printf() buffer size
     *  @param tx_multiple amount of max printf() present in the internal ring buffer at one time
     *  @param name optional name
    */
    BufferedSpi(PinName mosi, PinName miso, PinName sclk, PinName nss, PinName datareadypin, uint32_t buf_size = 384, uint32_t tx_multiple = 4,const char* name=NULL);
    
    /** Destroy a BufferedSpi Port
     */
    virtual ~BufferedSpi(void);
    
    /** call to SPI frequency Function
     */
    virtual void frequency(int hz);
    
    /** clear the transmit buffer
     */
    virtual void flush_txbuf(void);
    
    /** call to SPI format function 
     */
    virtual void format(int bits, int mode);
    
    virtual void enable_nss(void);
    
    virtual void disable_nss(void);
    
    /** Check on how many bytes are in the rx buffer
     *  @return 1 if something exists, 0 otherwise
     */
    virtual int readable(void);
    
    /** Check to see if the tx buffer has room
     *  @return 1 always has room and can overwrite previous content if too small / slow
     */
    virtual int writeable(void);
    
    /** Get a single byte from the BufferedSpi Port.
     *  Should check readable() before calling this.
     *  @return A byte that came in on the SPI Port
     */
    virtual int getc(void);
    virtual int get16b(void);
    
    /** Write a single byte to the BufferedSpi Port.
     *  @param c The byte to write to the SPI Port
     *  @return The byte that was written to the SPI Port Buffer
     */
    virtual int putc(int c);
    
    /** Write a string to the BufferedSpi Port. Must be NULL terminated
     *  @param s The string to write to the Spi Port
     *  @return The number of bytes written to the Spi Port Buffer
     */
    virtual int puts(const char *s);
    
    /** Write a formatted string to the BufferedSpi Port.
     *  @param format The string + format specifiers to write to the Spi Port
     *  @return The number of bytes written to the Spi Port Buffer
     */
    virtual int printf(const char* format, ...);
    
    /** Write data to the Buffered Spi Port
     *  @param s A pointer to data to send
     *  @param length The amount of data being pointed to
     *  @return The number of bytes written to the Spi Port Buffer
     */
    virtual ssize_t write(const void *s, std::size_t length);
    
    /** Read data from the Spi Port to the _rxbuf
     *  @param max: optional. = max sieze of the input read
     *  @return The number of bytes read from the SPI port and written to the _rxbuf
     */
    virtual ssize_t read();
    virtual ssize_t read(int max);

    /** Read the contents of a file into a buffer
     *
     *  Follows POSIX semantics:
     *
     *  * if no data is available, and non-blocking set return -EAGAIN
     *  * if no data is available, and blocking set, wait until data is available
     *  * If any data is available, call returns immediately
     *
     *  @param buffer   The buffer to read in to
     *  @param length   The number of bytes to read
     *  @return         The number of bytes read, 0 at end of file, negative error on failure
     */
    virtual ssize_t read(void* buffer, size_t length);
    
    /** Close a file
     *
     *  @return         0 on success, negative error code on failure
     */
    virtual int close();

    /** Move the file position to a given offset from from a given location
     *
     * Not valid for a device type FileHandle like UARTSerial.
     * In case of UARTSerial, returns ESPIPE
     *
     *  @param offset   The offset from whence to move to
     *  @param whence   The start of where to seek
     *      SEEK_SET to start from beginning of file,
     *      SEEK_CUR to start from current position in file,
     *      SEEK_END to start from end of file
     *  @return         The new offset of the file, negative error code on failure
     */
    virtual off_t seek(off_t offset, int whence);
    

    /** Attach a function to call whenever a serial interrupt is generated
     *  @param func A pointer to a void function, or 0 to set as none
     *  @param type Which serial interrupt to attach the member function to (Serial::RxIrq for receive, TxIrq for transmit buffer empty)
     */
     virtual void attach(Callback<void()> func, IrqType type=RxIrq);
     
    /** Attach a member function to call whenever a serial interrupt is generated
     *  @param obj pointer to the object to call the member function on
     *  @param method pointer to the member function to call
     *  @param type Which serial interrupt to attach the member function to (Serial::RxIrq for receive, TxIrq for transmit buffer empty)
     */
    template <typename T>
    void attach(T *obj, void (T::*method)(), IrqType type=RxIrq) {
        attach(Callback<void()>(obj, method), type);
    }

    /** Attach a member function to call whenever a serial interrupt is generated
     *  @param obj pointer to the object to call the member function on
     *  @param method pointer to the member function to call
     *  @param type Which serial interrupt to attach the member function to (Serial::RxIrq for receive, TxIrq for transmit buffer empty)
     */
    template <typename T>
    void attach(T *obj, void (*method)(T*), IrqType type=RxIrq) {
        attach(Callback<void()>(obj, method), type);
    }
};
#endif
