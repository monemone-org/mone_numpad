# monenumpad

![monenumpad](imgur.com image replace me!)

*A short description of the keyboard/project*

* Keyboard Maintainer: [monehsieh](https://github.com/monehsieh)
* Hardware Supported: *The PCBs, controllers supported*
* Hardware Availability: *Links to where you can find this hardware*

Install qmk

    git clone https://github.com/vial-kb/vial-qmk.git

Make example for this keyboard (after setting up your build environment):

    make monenumpad:vial

Flashing example for this keyboard:

    make monenumpad:vial:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).


See [VIAL Porting Build](https://get.vial.today/docs/)


## Bootloader

Enter the bootloader in one of the following ways:

* **Physical reset button**: Briefly press the button on the back of the PCB - some may have pads you must short instead
* **Ground reset pin**: Short GND and RESET pin on the micro controller



