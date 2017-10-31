# UDP File Sharer C
UDP File Sharer C is a simple file sharing system that can be used in tests or small applications.

## Getting Started
Follow these steps to get UDP File Sharer C running without errors.

### Prerequisites
You need a C compiler installed. To do it, use the following command:
```
sudo apt-get install gcc
```
### Installing
cd into the files directory and compile both. Use -o to choose executable name.
```
gcc sender.c -o sender
gcc receiver.c -o receiver
```
You should see both executable into directory.

## Running
Be certain that you have **superuser permissions**.


For sender choose the file to transfer, ip destiny and port destiny.
Example:
```
./sender example.txt 255.255.255.255 90
```

For receiver choose the port to listen.
```
./receiver 90
```

## Changing parameters

You can (and should!) change *#define# parameters into source-code. Change IP_DESTINY, PACKETSIZE, BUFSIZE and DEFAULT_FILE_NAME to what you need.

## Contact

**Paulo Vítor** - paulo9mv@yahoo.com.br
*Computer Engineering Student at UFMT - Várzea Grande - Brasil*
