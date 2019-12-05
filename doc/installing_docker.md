# Installing Docker

Building Julius for the Nintendo Switch or PS Vita requires the previous installation of `docker`.

`Docker` is a platform that simulates a different working envirionment than your own. It's useful to get a specific programming environment ready without the hassle of manually installing and configuring everything.

The following steps assume you are using a Debian-based Linux distribution such as Ubuntu.

To install `docker` for Debian-based Linux distributions, do the following:

1. Open a `Terminal` window.

2. Install `docker`using apt:

        $ sudo apt install docker.io

3. Add your user to the `docker` group:

        $ sudo usermod -aG docker $USER
        $ su - $USER

Then proceed to building Julius, either for [Nintendo Switch](building_switch.md#building-julius) or for the [PS Vita](building_vita.md#building-julius).

----------------------------------------------------------------------------

If you have a different Linux distribution or a different operating system, please follow the official instructions:

* [Installing `docker` on Windows](https://docs.docker.com/docker-for-windows/install/);
* [Installing `docker` on Mac OS](https://docs.docker.com/docker-for-mac/install/);
* For another Linux distribution, check the official page on [installing `docker`](https://docs.docker.com/install/).

Then proceed to building Julius, either for [Nintendo Switch](building_switch.md#building-julius) or for the [PS Vita](building_vita.md#building-julius).
