boot:
	as86 -0 -a -o bootsect.o bootsect.s && ld86 -0 -s -o bootsect bootsect.o && dd bs=1 if=bootsect of=Image skip=32
	cp Image /tmp/; cd ..
	cp /tmp/Image /home/saasbook/oslab1/linux-0.11 ; cd .. ; cd .. ; pwd ; ./run