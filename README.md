# Confiner

Confiner is an tool allowing you to run containers or sandbox applications.
Confiner relies on Namespaces and Cgroups to perfom isolation from the host, and use capabilities and seccomp filters to enhance security.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

In order to use capabilities and seccomp filter, you need to install the libcap-dev and the libseccomp-dev package.

```
sudo apt install libcap-dev libseccomp-dev
```

### Installing

There is no makefile at the moment.
Just cd into the src directory and compile using :

```
gcc -Wall -Werror -lcap -lseccomp confiner.c include/* core/* -o confiner
```

## Usage

```
sudo ./confiner [run MOUNTPATH | sandbox] [-mnihup] [-c COMMAND]
```
Use run mode to launch a container, use sandbox mode to isolate an application (not supported yet).

mnihup flags correspond respectively to the mount, network, ipc, uts, user and pid namespaces. They are all used by default if any of these flags specified. If one of this flag is specified, confiner will configure only the requested namespaces.

The command flag specifies what command should be run in a container, or what command shoud be sandboxed. If no command is specified, /bin/sh will be launched.


Exemples:
```
## launch a container
sudo ./confiner run <path-to-container>

## launch an http server in a different network namespace
sudo ./confiner sandbox -n -c httpd
```
