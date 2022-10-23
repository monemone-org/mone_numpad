#! /bin/bash

pushd .
cp -R --preserve=all src/qmk-firmware-keyboard/monenumpad  vial-qmk/keyboards/
cd vial-qmk/
qmk compile -kb monenumpad -km vial 
popd

