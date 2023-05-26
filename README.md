# remote-wrap

Proof-of-concept utility to execute encrypted remote files without touching the filesystem.

Encryption is done using `openssl`'s `EVP` API with 256-bit AES.
Files are downloaded from a remote server using `libCURL` from an specified URL.
`memfd_create()` is used to create in-memory temporary files to store downloaded executables, leaving no filesystem trace.
Then `exec()` is called on the file descriptor path to run them.

An encryption program has been included to create encrypted executables.

To try it out first make sure that both `openssl` and `libCURL` are available.
Use `scripts/test_sample.sh` to easily spin up a python http server and execute a provided sample program.
`scripts/gen_sample.sh` may be used to recompile and reencrypt the sample program.

To use the program directly, use

```bash
make RUNFLAGS="IN OUT" MAIN_USED=encrypt run
```

to encrypt a program, and:

```bash
make RUNFLAGS="URL" run
```

to run it, assuming that the file can be access using `URL`.

## License 

_Note: this is a proof-of-concept program.
Under no circumstance shall the author be held liable for any missuse of it.
It is therefore provided under the following license:_

```
remote-wrap, a proof-of-concept utility to execute encrypted remote files without touching the filesystem.
Copyright © 2023 Antonio de Haro

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```
