#! /bin/bash

pushd .
cp -R --preserve=all ./vial-qmk-keyboards-monenumpad/monenumpad  vial-qmk/keyboards/
cd vial-qmk/
qmk compile -kb monenumpad -km vial 
popd

