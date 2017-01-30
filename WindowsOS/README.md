WindowsOS folder contains my Programs for Windows Operating System. Some of
these programs work as User Mode Aplication other work in Kernel Mode. 
For Kernel mode aplications your should set up the environment:
* Install virtual machine(VMWare workstation 12 will be cool)
* Install guest OS(Windows7 for example). I advice install 32-bit operating
system for avoid driver signing check. If you have a 64-bit during an every
boot your should press F8, and select Disable Driver Signature Enforcement
* Install WinDDK on the host or a guest. It depands on where your want to 
make build.
* Install WinDbg on the host.
* Install DebugView on the guest.
* Create shared folder for convenient data exchange. For VMWare: go to the
VirtualMachine -> Settings -> Options -> Shared Folders and check Always
enable. After push Add button and create new shared folder. After it boot
VirtualMachine and allow to install VMWare software for this, after reboot
your will have a shared folder.
* Create a COM port for debug. For VMWare:
VirtualMachine -> Settings -> Hardware push Add button, select serial port,
next, select Output a named pipe. Name like: \\.\pipe\com_1. This end is
server, the other end is an aplication. Check Yield CPU on poll.
* In the guest open a Checked Build Environment of WinDDK and:
    bcdedit /set TESTSIGNING OFF
    bcdedit /debug on
    bcdedit /dbgsettings serial debugport:n baudrate:115200
    where n is the number of a COM port on the virtual machine.
    restart
* For make build use: build -Zw
* Install and create via sc for example:
    sc create nameRoutine binpth= path_to_sys_file type= kernel
    sc start nameRoutine
* After work:
    sc stop nameRoutine
    sc delete nameRoutine