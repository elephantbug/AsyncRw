# A sample C++ code demonstrating async read/write.

Building and running the application:

On Ubuntu 14.04. Install QT 5.5 (or probably higher) according to the [instruction on official QT site](http://wiki.qt.io/Install_qt_5_on_ubuntu). Open AsyncRw\AsyncRwDemo.pro in QT Designer and run the app. In the opened console window enter a list of integer numbers separated with whitespace or newlines and type Ctrl+D at the start of a line to signal the end of the input. Alternatively, redirect the input form a file located in “Input” directory with a command like ./AsyncRwDemo < ../AsyncRw/Input/input1K.txt.

On Windows use VS2015 + QtPackage and/or QT Designer. To signal the end of file on Windows type Ctrl+Z.

The sample output is:

AsyncRwDemo.exe < ../Input/input1M.txt

Addition result: 1783293664
Subtraction result: -1783293664
Xor result: 0

With the empty input the application outputs zeros.

How it works:

Provider thread reads the input by chunks and puts the read chunks into the queue. Consumer threads iterate over the read chunks in the queue and mark them as processed. The chunks processed by all the consumer thread are deleted from the queue. The maximum length of the queue and the size of the chunk are defined in C++ as constants, that are not optimized and used only for demonstration purposes.
