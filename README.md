# Repetitive RAM Reviewer

The Repetitive RAM Reviewer cyclical check the free RAM. The free RAM is the gap between heap and stack.
The heap grows from lower to higher addresses and the stack grows from higher to lower addresses.
The stack starts at the end of RAM. The heap starts at the end of you data section. Between both is the free RAM that could be used by stack or heap.

You can find a more specific explanation on the following web page:
<https://www.nongnu.org/avr-libc/user-manual/malloc.html>

The heap is managed in software so it can be checked if there is enough space before allocating heap memory.
The stack pointer (points to the end or top of the stack) is controlled by the hardware and the hardware has no ideas of the heap.
Therefore the stack could grow into the heap and overwrite heap data. If this happens also writing to heap data could overwrite current stack data.
In such a situation the program can behave really strange.

## How it works
At the beginning of your program Repetitive RAM Reviewer writes well known data into the free memory between heap and stack.
If heap or stack grows, this data is overwritten and Repetitive RAM Reviewer can detect this in a loop call. Of cause you could write
the same date like Repetitive RAM Reviewer does so it might not detect all heap or stack usage but I think this is really seldom.
Heap and stack memory can be released but the RAM is still overwritten. So Repetitive RAM Reviewer shows the worst case not the current.
That is why it is OK to check it only from time to time. Please also note that this is also the reason why showing all memory was used
does not automatically mean heap and stack collided. Perhaps heap is growing high but released before stack is growing trough the last heap water mark.
But if Repetitive RAM Reviewer shows some free memory this is typical a good sign that there was no crash.
Please always let some free bytes left. They are needed to handle interrupts.

## Install
Just download the code as zip file. In GitHub click on the `[Code]`-button and select `Download ZIP`.

In Arduino IDE select `Sketch` -> `Include library` -> `Add ZIP Library ...` to add the downloaded ZIP file.

## Usage

    // create an instance
    RepetitiveRamReviewer rrr;

    void setup() {
      // call begin
      rrr.begin();
      ...
    }

    void loop() {
      // Do your stuff here
      ...

      // Measure and dump every 2s
      rrr.loopAndDump(2000);
    }

## Output example

    // 1450 bytes free, that's OK
    |[01de]==( 336)=>|[032e]=>(1450)<=|[08d8]<=(  39)==[08ff]|
    |[01de]==( 700)=>|[049a]=>(1071)<=|[08c9]<=(  54)==[08ff]|
    ...
    |[01de]==( 882)=>|[0550]=>( 805)<=|[0875]<=( 138)==[08ff]|
    ...
    |[01de]==( 882)=>|[0550]=>( 632)<=|[07c8]<=( 311)==[08ff]|
    ...
    |[01de]==( 882)=>|[0550]=>( 532)<=|[0764]<=( 411)==[08ff]|
    ...
    |[01de]==( 882)=>|[0550]=>( 380)<=|[06cc]<=( 563)==[08ff]|
    ...
    // now it becomes critical
    |[01de]==( 882)=>|[0550]=>( 152)<=|[05e8]<=( 791)==[08ff]|
    ...
    |[01de]==( 882)=>|[0550]=>(  20)<=|[0564]<=( 923)==[08ff]|
    ...
    // oh no, that's bad
    |[01de]==( 882)=>|[0550]=>(   0)<=|[0550]<=( 943)==[08ff]|

## License

Repetitive RAM Reviewer is distributed under the [MIT License](LICENSE).
