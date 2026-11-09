# If `fseek` Doesn’t Leave a Mark, How Does C Remember Where to Read Next?

If `fseek` doesn’t use a pointer to point to a word, how does it know what we have read so far?

Does it change the file’s contents at the binary level?

Those are the questions I wanted to answer by reading the C library documentation and running an experiment.

I called `fgetc` once, but `strace` showed Linux reading seven bytes. I wanted to understand why. Here are the relevant lines from my trace:

```text
read(3, "abcdefg", 4096) = 7
lseek(3, 0, SEEK_CUR) = 7
Position after reading it: 1
lseek(3, 0, SEEK_SET) = 0
read(3, "abcdefg", 4096) = 7
```

The file, `sample.txt`, contained seven bytes: `abcdefg`. The `read` call asked for up to 4096 bytes, and Linux returned the seven bytes in the file.

The C library kept those bytes in a buffer. My program had received only the first byte, `a`, so `ftell` reported the stream position as `1`. Linux’s file offset was `7`, because it had already read all seven bytes into the library’s buffer.

So the two positions describe different things:

- **Linux’s file offset, 7:** the position for the next low-level read from the file.
- **The C stream position, 1:** the position of the next byte my program would get from the stream.

But that still doesn’t answer my original question: how does `fseek` know where the next read should happen?

## The `FILE` stream and the kernel

When I open a file with `fopen`, the C library gives my program a stream, represented by a `FILE` object. The library manages this object and uses it to keep track of the stream’s state, including buffering and position.

Linux also keeps a file offset for the opened file. When the C library needs to reposition the stream, it can ask Linux to change that offset. The trace shows that, in my run, seeking caused Linux to move to offset `0` and read the seven bytes again. After `fseek(file, 3, SEEK_SET)`, my program read `d`, the byte at position `3`.

That is what happened in my experiment; the exact system calls can vary between C library implementations.

## Does `fseek` change the file?

No. `fseek` changes the stream’s position. By itself, it does not change the file’s contents or write a marker into it. A later write can change the contents, but seeking alone just changes where the stream will read or write next.

## Conclusion

There isn’t a pointer or marker inside the file that remembers where I am. The C library manages the stream and its buffer, while Linux keeps a file offset. Because the library can read a chunk into its buffer before giving all of it to my program, the stream position and Linux’s file offset can be different.

In my experiment, Linux’s offset was `7`, while the stream position was `1`. `fseek` changed where the stream would read next without changing the file itself.






## Code and test file

The C program and `sample.txt` used in this experiment are available in my [Fseek GitHub repository](https://github.com/Code-and-star/Fseek).

## References

- [GNU C Library: Stream Buffering](https://sourceware.org/glibc/manual/latest/html_node/Buffering-Concepts.html) — explains that buffered streams can read data in blocks.
- [GNU C Library: File Positioning](https://sourceware.org/glibc/manual/latest/html_node/File-Positioning.html) — explains the stream position and what `fseek` and `ftell` do.
- [Linux `read(2)` manual](https://man7.org/linux/man-pages/man2/read.2.html) — explains that `read` requests up to a number of bytes and reports how many it returned.
- [Linux `open(2)` manual](https://man7.org/linux/man-pages/man2/open.2.html) — explains that Linux keeps the file offset in an open file description.
- [Linux `lseek(2)` manual](https://man7.org/linux/man-pages/man2/lseek.2.html) — explains how Linux changes that file offset.
- [`strace(1)` manual](https://man7.org/linux/man-pages/man1/strace.1.html) — explains how `strace` records system calls.

The system-call behavior shown above is from my experiment on Linux. Other C library implementations may behave differently.

