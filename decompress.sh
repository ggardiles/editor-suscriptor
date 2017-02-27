#!/bin/sh
#!/bin/sh
echo -n "Do you wish to remove files (y/n)? "
read answer
if echo "$answer" | grep -iq "^y" ;then
    echo "##################"
    echo "REMOVING ;)"
    echo "##################"
    rm -rf autores editor/ intermediario/ subscriptor/ memoria.txt
else
    echo "##################"
    echo "OKAY... ¬¬ ..."
    echo "##################"
fi
tar -zxvf EDSU.2016.tar.gz
echo "##################"
echo "files decompressed"
echo "##################"


