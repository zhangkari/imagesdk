#
# If you build apk by gradle failed,
# but It's OK build by ant, Please
# check whether libimgsdk.so, vert.shdr & frag.shdr
# in right position
#

rm libs -rvf
rm app/libs -rvf
ndk-build
ln -s libs app/
