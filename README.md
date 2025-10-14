# multi-rfid-inventory

Tracks inventory status across multiple containers using MFRC522 RFID readers and Arduino. 
Uses the [rfid library by miguelbalboa](https://github.com/miguelbalboa/rfid).

## How it Works
When you scan an item on a reader, the system records that it's in that container. Scan it again to mark it as removed. The system provides feedback through LEDs and lets you check inventory status via serial commands.

Multiple RFID readers share SPI pins (CIPOA/MISO, COPIA/MOSI, SCK). Each reader is connected to a unique SS (chip select) pin, which can be any digital pin. 3 unique readers were tested in this example. More readers can be added as long as digital pins are available for chip select on the microcontroller.

## Configuration
### 1. Set up readers
Modify the `NR_OF_READERS` constant to match the number of connected readers. Assign chip select pins for each reader. Assign the RST pin \(shared by all readers\). Update the `ssPins[]` array:

```cpp
#define RST_PIN             9
#define SS_1_PIN            7         
#define SS_2_PIN            8          
#define SS_3_PIN            10

byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN};
```

Upon successful initialization, the serial monitor will print the assigned reader number and the reader's firmware version:

```
Reader 0: Firmware Version: 0x92 = v.2.0
```

If the initialization message does not appear as expected, see `Troubleshooting`.


### 2. Register RFID tags
Uncomment the `dump_byte_array()` function in `loop()`, then scan the tag with any connected MFRC522 reader. Copy the hex values from the seral monitor and add to the `items` array:

```cpp
itemID items[NUM_ITEMS] = 
{
  {{0x5A, 0xF0, 0x93, 0xBB}, 255, "Red Lego"},
  {{0x4A, 0xF0, 0x93, 0xBB}, 255, "Pill Bottle"},
  //etc
};
```

### 3. Set number of tracked items
Update the `NUM_ITEMS` constant to match your inventory size:

```cpp
#define NUM_ITEMS       7   //set number of tracked items
```

## Troubleshooting

### Readers not detected
* Check SPI wiring
* Verify chip select and RST pins match code configuration
* Increase the `delay()` duration in the reader setup loop

### Tags Not Reading
* Reduce distance between tag and reader (optimal: < 3cm)
* Verify antenna gain is set to maximum (done automatically in code)
* Check for interference from metal objects

### Incorrect Tracking
* Send the `r` command to serial monitor to reset inventory
* Verify UID arrays match your physical tags
