# Building Julius for Linux

## Non-Ubuntu users

**Please note:** these instructions were created assuming you are using Ubuntu (or any other Debian-based Linux distro). If you're building Julius on a distro that does not use `apt`, you must use whatever app manager your distro uses in order to obtain the necessary dependencies.

In order to follow these instructions, you'll need to install the following packages first:

* `git`
* `SDL2`
* `SDL2_mixer`
* `cmake`

Then skip steps 1. and 2. of the Ubuntu instructions.

## Building for Ubuntu

In general, for Ubuntu, you only need to use a terminal window to install and run Julius.
Also note that you need superuser (`root`) permissions to install the dependencies, by making use of `sudo`.

1. Open a terminal window and type the following command to install all the needed dependencies:

        $ sudo apt install git libsdl2-dev libsdl2-mixer-dev cmake

2. Depending on what you already have installed, `apt` may ask you to install many packages. Confirm the installation.

3. Navigate to the directory where you want the repository folder to be installed. As an example, we're using the `home` folder:

        $ cd ~

4. Clone the Julius github repository to your computer:

        $ git clone https://github.com/bvschaik/julius.git

5. Move to the new `julius` directory:

        $ cd julius

6. Create a `build` directory and move to it:

        $ mkdir build && cd build

7. Run `cmake`:

        $ cmake ..

8. Build Julius:

        $ make

**Success!** Julius should have been built without any errors. Type `./julius` to run it. Optionally you can also type `make test` to confirm that Julius is working properly.
