
echo "Your chosen folder is $1, is it OK? [YES/NO]"
echo -n ">"
read answer
if [ "$answer" = "YES" ]
then
    rm -rf $1/*
    echo "This is the test sample"> $1/MESSAGE.TXT
    touch $1/WRITE.TXT
    sync
    umount $1
elif [ "$answer" = "NO" ]
then
    echo "Nothing shold be done..."
    exit 0
else
    echo "answer [YES / NO]!!"
fi
