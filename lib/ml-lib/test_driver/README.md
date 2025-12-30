# ML Library Testing Device Driver (mllibdev)

ML library testing character device driver for the Linux kernel:
- Basic read/write operations
- IOCTL interface for device control
- Sysfs attributes for runtime information
- Procfs entry for debugging

## Features

### Character Device Operations
- **Open/Close**: Device can be opened and closed multiple times
- **Read**: Read data from a kernel buffer
- **Write**: Write data to a kernel buffer (1KB capacity)
- **Seek**: Support for lseek() operations

### IOCTL Commands
- `ML_LIB_TEST_DEV_IOCRESET`: Clear the device buffer
- `ML_LIB_TEST_DEV_IOCGETSIZE`: Get current data size
- `ML_LIB_TEST_DEV_IOCSETSIZE`: Set data size

### Sysfs Attributes
Located at `/sys/class/ml_lib_test/mllibdev`:
- `buffer_size`: Maximum buffer capacity (read-only)
- `data_size`: Current amount of data in buffer (read-only)
- `access_count`: Number of times device has been opened (read-only)
- `stats`: Comprehensive statistics (opens, reads, writes)

### Procfs Entry
Located at `/proc/mllibdev`: Provides formatted driver information

## Building the Driver

### Option 1: Build as a Module

1. Configure the kernel to build mllibdev as a module:
   ```bash
   make menuconfig
   # Navigate to: Library routines -> ML library testing character device driver
   # Select: <M> ML library testing character device driver
   ```

2. Build the module:
   ```bash
   make -j$(nproc) M=lib/ml-lib/test_driver
   ```

### Option 2: Build into Kernel

1. Configure the kernel:
   ```bash
   make menuconfig
   # Navigate to: Library routines -> ML library testing character device driver
   # Select: <*> ML library testing character device driver
   ```

2. Build the kernel:
   ```bash
   make -j$(nproc)
   ```

### Option 3: Quick Module Build (Out-of-Tree)

For quick testing, you can build just the module:

```bash
cd lib/ml-lib/test_driver
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
```

## Loading the Driver

If built as a module:

```bash
# Load the module
sudo insmod /lib/modules/$(uname -r)/build/lib/ml-lib/test_driver/ml_lib_test_dev.ko

# Verify it's loaded
sudo lsmod | grep ml_lib_test_dev

# Check kernel messages
sudo dmesg | tail -20
```

You should see messages like:
```
ml_lib_test_dev: Initializing driver
ml_lib_test_dev: Device number allocated: XXX:0
ml_lib_test_dev: Driver initialized successfully
ml_lib_test_dev: Device created at /dev/mllibdev
ml_lib_test_dev: Proc entry created at /proc/mllibdev
```

## Testing the Driver

### Quick Manual Test

```bash
# Write data to the device
sudo su
echo "Hello, kernel!" > /dev/mllibdev

# Read data back
sudo su
cat /dev/mllibdev

# Check sysfs attributes
cat /sys/class/ml_lib_test/mllibdev/stats

# Check proc entry
cat /proc/mllibdev
```

### Using the Test Program

1. Compile the test program:
   ```bash
   cd lib/ml-lib/test_driver/test_application
   gcc -o ml_lib_test_dev test_ml_lib_char_dev.c
   ```

2. Run the test program:
   ```bash
   sudo ./ml_lib_test_dev
   ```

The test program will:
- Open the device
- Write test data
- Read the data back
- Test all IOCTL commands
- Display sysfs attributes
- Show procfs information

### Example Test Output

```
ML Library Testing Device Driver Test Program
==================================
Device opened successfully: /dev/mllibdev

========== Write Test ==========
Successfully wrote 62 bytes
Data: "Hello from userspace! This is a test of the mllibdev driver."

========== Read Test ==========
Successfully read 62 bytes
Data: "Hello from userspace! This is a test of the mllibdev driver."

========== IOCTL Tests ==========
Current data size: 62 bytes
Set data size to: 50 bytes
Verified new size: 50 bytes
Buffer reset successfully
Size after reset: 0 bytes

========== Sysfs Attributes ==========
buffer_size: 1024
data_size: 0
access_count: 1

stats: Opens: 1
Reads: 1
Writes: 1

========== Procfs Information ==========
ML Library Testing Device Driver Information
=================================
Device name:     mllibdev
Buffer size:     1024 bytes
Data size:       0 bytes
Access count:    1
Read count:      1
Write count:     1

========== Final Test ==========
All tests completed successfully!
```

## Unloading the Driver

```bash
# Remove the module
sudo rmmod ml_lib_test_dev

# Verify it's unloaded
sudo lsmod | grep ml_lib_test_dev

# Check cleanup messages
sudo dmesg | tail -10
```

## Troubleshooting

### Device node doesn't exist
```bash
# Check if udev created the device
ls -l /dev/mllibdev

# Manually create if needed (shouldn't be necessary)
sudo mknod /dev/mllibdev c MAJOR MINOR
```

### Permission denied
```bash
# Run commands with sudo
sudo cat /dev/mllibdev

# Or change permissions
sudo chmod 666 /dev/mllibdev
```

### Module won't load
```bash
# Check kernel messages for errors
dmesg | tail -20

# Verify module dependencies
modinfo lib/ml-lib/test_driver/ml_lib_test_dev.ko
```

## License

This driver is licensed under GPL-2.0.

## Author

Viacheslav Dubeyko <slava@dubeyko.com>

## Version

1.0
