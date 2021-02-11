Dockerfile to compile `lib-ledger-core` for Linux and encapsulate it in a jar file.

# Build the `libcore` image

## Option 1: Compile (by default)

To build the `libcore` Docker image using default parameters (compile, build type: `Debug`), run the following command *from the root directory*.

```sh
$ cd lib-ledger-core
$ docker build -t libcore -f tools/builder/Dockerfile .
```

The build type (by default: `Debug`) can be configured. To change the build type, specify it using `--build-arg BUILD_TYPE=(Debug|Release)` (note that the build type is case sensitive).

For instance, to set the built type to `Release`:

```sh
$ docker build -t libcore --build-arg BUILD_TYPE=Release -f tools/builder/Dockerfile .
```

## Option 2: Download

In order to download the library file instead of compiling it, set `DL_LIBCORE` arg to `ON` and specify the version of `LIBCORE_VERSION`:

```sh
$ docker build -t libcore --build-arg DL_LIBCORE=ON --build-arg LIBCORE_VERSION=3.3.0-rc-fb0953 -f tools/builder/Dockerfile .
```

# Retrieve the compiled and jar files

The Linux compiled library file as well as the jar file can be copied from the `libcore` container directly to the Docker host, or indirectly, from the `libcore` image to another Dockerfile.

## Option 1: Copy the files to the host

At runtime, the `.so` and `jar` files are available in container's `/build` directory.

Once the Docker image has been built, run the following command *from a directory where the `.so` and `.jar` files have to be retrieved*.

```sh
$ cd ledger-wallet-daemon/lib
$ docker run -v $(pwd):/build libcore:latest
```

![Screenshot: files copied to macOS host](copy_to_host.png "Copy to host")

## Option 2: Retrieve the files from another Docker image

In the `libcore` image, the `.so` and `jar` files are available in the image's `/libcore/build` directory.

As an example, let's create a `Dockerfile` based on `libcore`:

```yml
FROM libcore:latest

RUN stat /libcore/build/libledger-core.so
RUN stat /libcore/build/ledger-lib-core.jar
```

When this Docker image is being built (`docker build -t test .`), it appears that the `.so` and `.jar` are indeed copied from the base `libcore` image:

```sh
$ docker build -t test .
Sending build context to Docker daemon  416.7MB
Step 1/3 : FROM libcore:latestwd):/build libc
 ---> 0ab4619e4c36
Step 2/3 : RUN stat /libcore/build/libledger-core.so
 ---> Running in a3c11b2fa592
  File: /libcore/build/libledger-core.so
  Size: 322873848   Blocks: 630616     IO Block: 4096   regular file
Device: 71h/113d  Inode: 2228308     Links: 1
Access: (0755/-rwxr-xr-x)  Uid: (    0/    root)   Gid: (    0/    root)
Access: 2020-07-10 14:25:45.000000000 +0000
Modify: 2020-07-10 14:25:45.000000000 +0000
Change: 2020-07-10 14:31:28.685639932 +0000
 Birth: -
Removing intermediate container a3c11b2fa592
 ---> 88aa849141d2
Step 3/3 : RUN stat /libcore/build/ledger-lib-core.jar
 ---> Running in 206b985543e3
  File: /libcore/build/ledger-lib-core.jar
  Size: 93865761    Blocks: 183336     IO Block: 4096   regular file
Device: 71h/113d  Inode: 2228394     Links: 1
Access: (0644/-rw-r--r--)  Uid: (    0/    root)   Gid: (    0/    root)
Access: 2020-07-10 14:31:19.000000000 +0000
Modify: 2020-07-10 14:31:19.000000000 +0000
Change: 2020-07-10 14:31:30.399639926 +0000
 Birth: -
Removing intermediate container 206b985543e3
 ---> 9a2d8aafaa11
```
